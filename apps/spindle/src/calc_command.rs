// SPDX-License-Identifier: GPL-3.0-only
//! `calc <expression>`: exact decimal arithmetic on what the user typed.
//!
//! `ans` is the previous successful result in this Spindle session.
//! `**` raises to a whole power up to 64; `sqrt(x)` is the square root;
//! `0x..`/`0b..` whole numbers (and `<expr> in hex|bin|oct` to show a whole result in that base); `abs min max round floor ceil` work on the exact values; `pi` and `e`
//! are 9-place constants (results using them are marked `~`).
//! Numbers are fixed point with 9 decimal places in an i128, so `0.1+0.2`
//! is exactly `0.3`. Operators: `+ - * / %`, unary minus and parentheses.
//! Parsing is recursive descent with a depth bound; every operation is
//! checked, and overflow, division by zero or malformed input is an error,
//! never a wrapped or rounded-away answer (division rounds toward zero at
//! the 9th decimal and says so with `~`).
//!
//! A number with an `i` suffix is imaginary (`4i`, `0.5i`, `1i`), so values
//! are complex, with both parts in the same fixed point: `1i*1i` is exactly
//! `-1` and `(3+4i)/5` is exactly `0.6+0.8i`. Two real operands keep the
//! real arithmetic above unchanged. `re im conj` take a value apart,
//! `norm(z)` is |z|^2, `abs(z)` is |z|, and `interfere(a, b)` is
//! |a+b|^2 - |a|^2 - |b|^2: positive where two amplitudes reinforce,
//! negative where they cancel, `interfere(1, -1)` = -2 and `norm(1 + -1)` = 0.
//! Whole complex values show in hex, bin and oct too: `= 0xff+0x10i`.

const SCALE: i128 = 1_000_000_000;
const MAX_DEPTH: u8 = 16;
pub const LINE_MAX: usize = 64;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Error { Syntax, DivideByZero, Overflow, TooDeep, NoAnswer, Exponent, Domain, Unknown, NotWhole, NotReal }

/// The real part is `scaled`; `imag` is the imaginary part, zero for a real value.
#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub struct Value { pub scaled: i128, pub imag: i128, pub inexact: bool }

/// A complex value: (real part, imaginary part), each scaled.
type C = (i128, i128);

struct Parser<'a> { s: &'a [u8], at: usize, depth: u8, inexact: bool, ans: Option<C>, vars: &'a [([u8; 8], u8, C)] }

/// The real part of a value that must be real.
fn real(v: C) -> Result<i128, Error> { if v.1 == 0 { Ok(v.0) } else { Err(Error::NotReal) } }

/// A checked product of two raw parts.
fn times(x: i128, y: i128) -> Result<i128, Error> { x.checked_mul(y).ok_or(Error::Overflow) }

/// The integer square root, rounded down (Newton from above; no floats in Spindle).
fn isqrt(n: u128) -> u128 {
    let mut r: u128 = if n == 0 { 0 } else { 1u128 << ((128 - n.leading_zeros() + 1) / 2) };
    while r > 0 { let next = (r + n / r) / 2; if next >= r { break; } r = next; }
    r
}

impl Parser<'_> {
    fn skip(&mut self) { while self.at < self.s.len() && self.s[self.at] == b' ' { self.at += 1; } }
    fn peek(&mut self) -> Option<u8> { self.skip(); self.s.get(self.at).copied() }
    fn expr(&mut self) -> Result<C, Error> {
        let mut v = self.term()?;
        while let Some(op @ (b'+' | b'-')) = self.peek() {
            self.at += 1;
            let r = self.term()?;
            let (re, im) = if op == b'+' { (v.0.checked_add(r.0), v.1.checked_add(r.1)) } else { (v.0.checked_sub(r.0), v.1.checked_sub(r.1)) };
            v = re.zip(im).ok_or(Error::Overflow)?;
        }
        Ok(v)
    }
    fn term(&mut self) -> Result<C, Error> {
        let mut v = self.unary()?;
        while let Some(op @ (b'*' | b'/' | b'%')) = self.peek() {
            if self.s[self.at..].starts_with(b"**") { break; }
            self.at += 1;
            let r = self.unary()?;
            v = match op {
                b'*' => self.mul(v, r)?,
                b'/' => self.div(v, r)?,
                _ => { let (v, r) = (real(v)?, real(r)?); if r == 0 { return Err(Error::DivideByZero); } (v % r, 0) }
            };
        }
        Ok(v)
    }
    fn unary(&mut self) -> Result<C, Error> {
        if self.peek() == Some(b'-') {
            self.at += 1;
            let v = self.guard(|p| p.unary())?;
            return v.0.checked_neg().zip(v.1.checked_neg()).ok_or(Error::Overflow);
        }
        if self.peek() == Some(b'+') { self.at += 1; return self.guard(|p| p.unary()); }
        self.power()
    }
    /// `a ** n` for a whole exponent |n| <= 64 (right-associative, binds
    /// tighter than unary minus: `-2**2` is -4).
    fn power(&mut self) -> Result<C, Error> {
        let base = self.atom()?;
        if !(self.peek() == Some(b'*') && self.s[self.at..].starts_with(b"**")) { return Ok(base); }
        self.at += 2;
        let e = self.guard(|p| p.unary())?;
        if e.1 != 0 || e.0 % SCALE != 0 || (e.0 / SCALE).abs() > 64 { return Err(Error::Exponent); }
        let n = e.0 / SCALE;
        let mut v = (SCALE, 0);
        for _ in 0..n.abs() { v = self.mul(v, base)?; }
        if n < 0 { v = self.div((SCALE, 0), v)?; }
        Ok(v)
    }
    /// A product of two scaled values, rescaled; a dropped digit makes it inexact.
    fn rescale(&mut self, p: i128) -> i128 { if p % SCALE != 0 { self.inexact = true; } p / SCALE }
    fn mul(&mut self, v: C, r: C) -> Result<C, Error> {
        if v.1 == 0 && r.1 == 0 {
            let p = v.0.checked_mul(r.0).ok_or(Error::Overflow)?;
            return Ok((self.rescale(p), 0));
        }
        // (a+bi)(c+di) = (ac-bd) + (ad+bc)i
        let re = times(v.0, r.0)?.checked_sub(times(v.1, r.1)?).ok_or(Error::Overflow)?;
        let im = times(v.0, r.1)?.checked_add(times(v.1, r.0)?).ok_or(Error::Overflow)?;
        Ok((self.rescale(re), self.rescale(im)))
    }
    /// `n / d` of a numerator already scaled up by SCALE; a remainder makes it inexact.
    fn quotient(&mut self, n: i128, d: i128) -> i128 { if n % d != 0 { self.inexact = true; } n / d }
    fn div(&mut self, v: C, r: C) -> Result<C, Error> {
        if r == (0, 0) { return Err(Error::DivideByZero); }
        let up = |x: i128| x.checked_mul(SCALE).ok_or(Error::Overflow);
        if r.1 == 0 {
            let re = self.quotient(up(v.0)?, r.0);
            return Ok((re, if v.1 == 0 { 0 } else { self.quotient(up(v.1)?, r.0) }));
        }
        // (a+bi)/(c+di) = ((ac+bd) + (bc-ad)i) / (c^2+d^2)
        let den = times(r.0, r.0)?.checked_add(times(r.1, r.1)?).ok_or(Error::Overflow)?;
        let re = times(v.0, r.0)?.checked_add(times(v.1, r.1)?).ok_or(Error::Overflow)?;
        let im = times(v.1, r.0)?.checked_sub(times(v.0, r.1)?).ok_or(Error::Overflow)?;
        Ok((self.quotient(up(re)?, den), self.quotient(up(im)?, den)))
    }
    fn guard(&mut self, f: impl FnOnce(&mut Self) -> Result<C, Error>) -> Result<C, Error> {
        if self.depth >= MAX_DEPTH { return Err(Error::TooDeep); }
        self.depth += 1; let r = f(self); self.depth -= 1; r
    }
    fn atom(&mut self) -> Result<C, Error> {
        match self.peek() {
            Some(b'(') => {
                self.at += 1;
                let v = self.guard(|p| p.expr())?;
                if self.peek() != Some(b')') { return Err(Error::Syntax); }
                self.at += 1; Ok(v)
            }
            Some(b'0'..=b'9' | b'.') => {
                let v = self.number()?;
                // `4i`: an imaginary number.
                if self.s.get(self.at) == Some(&b'i') && !self.s.get(self.at + 1).is_some_and(u8::is_ascii_alphanumeric) {
                    self.at += 1;
                    return Ok((0, v));
                }
                Ok((v, 0))
            }
            Some(b'a' | b'm' | b'r' | b'f' | b'c' | b'i' | b'n') if [&b"abs("[..], b"min(", b"max(", b"round(", b"floor(", b"ceil(", b"re(", b"im(", b"conj(", b"norm(", b"interfere("].iter().any(|f| self.s[self.at..].starts_with(f)) => {
                let name_len = self.s[self.at..].iter().position(|&c| c == b'(').unwrap_or(0);
                let name = &self.s[self.at..self.at + name_len];
                self.at += name_len + 1;
                let mut vals = [(0i128, 0i128); 8];
                let mut n = 0;
                loop {
                    if n == vals.len() { return Err(Error::Syntax); }
                    vals[n] = self.guard(|p| p.expr())?;
                    n += 1;
                    match self.peek() { Some(b',') => self.at += 1, Some(b')') => { self.at += 1; break } _ => return Err(Error::Syntax) }
                }
                let one = || if n == 1 { Ok(vals[0]) } else { Err(Error::Syntax) };
                let reals = || -> Result<[i128; 8], Error> { let mut r = [0i128; 8]; for k in 0..n { r[k] = real(vals[k])?; } Ok(r) };
                Ok(match name {
                    b"abs" => {
                        let z = one()?;
                        if z.1 == 0 { (z.0.checked_abs().ok_or(Error::Overflow)?, 0) } else {
                            // |z| = sqrt(re^2 + im^2): both parts carry SCALE, so the root does too.
                            let sq = |x: i128| (x.unsigned_abs()).checked_mul(x.unsigned_abs()).ok_or(Error::Overflow);
                            let n = sq(z.0)?.checked_add(sq(z.1)?).ok_or(Error::Overflow)?;
                            let r = isqrt(n);
                            if r * r != n { self.inexact = true; }
                            (i128::try_from(r).map_err(|_| Error::Overflow)?, 0)
                        }
                    }
                    b"min" => (*reals()?[..n].iter().min().unwrap(), 0),
                    b"max" => (*reals()?[..n].iter().max().unwrap(), 0),
                    // Whole numbers: round half away from zero, floor, ceil.
                    b"round" => { let v = real(one()?)?; let r = (v.abs() + SCALE / 2) / SCALE * SCALE; (if v < 0 { -r } else { r }, 0) }
                    b"floor" => { let v = real(one()?)?; (v.div_euclid(SCALE) * SCALE, 0) }
                    b"ceil" => { let v = real(one()?)?; (-((-v).div_euclid(SCALE) * SCALE), 0) }
                    b"re" => (one()?.0, 0),
                    b"im" => (one()?.1, 0),
                    b"conj" => { let z = one()?; (z.0, z.1.checked_neg().ok_or(Error::Overflow)?) }
                    b"norm" => { let z = one()?; (self.rescale(times(z.0, z.0)?.checked_add(times(z.1, z.1)?).ok_or(Error::Overflow)?), 0) }
                    // 2 Re(a conj(b)) = 2(re a re b + im a im b): the cross term of |a+b|^2.
                    _ => {
                        if n != 2 { return Err(Error::Syntax); }
                        let (a, b) = (vals[0], vals[1]);
                        let cross = times(a.0, b.0)?.checked_add(times(a.1, b.1)?).and_then(|c| c.checked_mul(2)).ok_or(Error::Overflow)?;
                        (self.rescale(cross), 0)
                    }
                })
            }
            Some(b's') if self.s[self.at..].starts_with(b"sqrt(") => {
                self.at += 4;
                let v = real(self.atom()?)?;
                if v < 0 { return Err(Error::Domain); }
                // sqrt(v / S) * S == sqrt(v * S), rounded down at the 9th place.
                let n = (v as u128).checked_mul(SCALE as u128).ok_or(Error::Overflow)?;
                let r = isqrt(n);
                if r * r != n { self.inexact = true; }
                Ok((r as i128, 0))
            }
            Some(b'p' | b'e') if [&b"pi"[..], b"e"].iter().any(|k| self.s[self.at..].starts_with(k) && !self.s.get(self.at + k.len()).is_some_and(u8::is_ascii_alphanumeric)) => {
                // Constants to 9 places; any use makes the answer approximate.
                let pi = self.s[self.at..].starts_with(b"pi");
                self.at += if pi { 2 } else { 1 };
                self.inexact = true;
                Ok((if pi { 3_141_592_654 } else { 2_718_281_828 }, 0))
            }
            Some(b'a') if self.s[self.at..].starts_with(b"ans") && !self.s.get(self.at + 3).is_some_and(u8::is_ascii_alphanumeric) => {
                self.at += 3;
                self.ans.ok_or(Error::NoAnswer)
            }
            Some(c) if c.is_ascii_alphabetic() => {
                // A variable set earlier with `name = expr`.
                let start = self.at;
                while self.s.get(self.at).is_some_and(|c| c.is_ascii_alphanumeric()) { self.at += 1; }
                let name = &self.s[start..self.at];
                self.vars.iter().find(|(n, l, _)| &n[..*l as usize] == name).map(|v| v.2).ok_or(Error::Unknown)
            }
            _ => Err(Error::Syntax),
        }
    }
    fn number(&mut self) -> Result<i128, Error> {
        // 0x... hexadecimal and 0b... binary whole numbers.
        if let Some(radix) = match self.s.get(self.at..self.at + 2) { Some(b"0x") | Some(b"0X") => Some(16), Some(b"0b") | Some(b"0B") => Some(2), _ => None } {
            self.at += 2;
            let (mut v, mut n) = (0i128, 0);
            while let Some(d) = self.s.get(self.at).and_then(|&c| (c as char).to_digit(radix)) {
                v = v.checked_mul(radix as i128).and_then(|v| v.checked_add(d as i128)).ok_or(Error::Overflow)?;
                self.at += 1; n += 1;
            }
            if n == 0 { return Err(Error::Syntax); }
            return v.checked_mul(SCALE).ok_or(Error::Overflow);
        }
        let (mut whole, mut frac, mut places, mut digits, mut dot) = (0i128, 0i128, 0u32, 0, false);
        while let Some(&c) = self.s.get(self.at) {
            match c {
                b'0'..=b'9' if dot => { if places < 9 { frac = frac * 10 + (c - b'0') as i128; places += 1; } else { self.inexact |= c != b'0'; } }
                b'0'..=b'9' => whole = whole.checked_mul(10).and_then(|w| w.checked_add((c - b'0') as i128)).ok_or(Error::Overflow)?,
                b'.' if !dot => dot = true,
                _ => break,
            }
            if c != b'.' { digits += 1; }
            self.at += 1;
        }
        if digits == 0 { return Err(Error::Syntax); }
        whole.checked_mul(SCALE).and_then(|w| w.checked_add(frac * 10i128.pow(9 - places))).ok_or(Error::Overflow)
    }
}

pub fn eval(input: &[u8]) -> Result<Value, Error> { eval_with(input, None) }
/// Evaluate with `ans` bound to a previous real result (scaled), if any.
pub fn eval_with(input: &[u8], ans: Option<i128>) -> Result<Value, Error> {
    let mut env = Env { ans: ans.map(|a| (a, 0)), vars: [([0; 8], 0, (0, 0)); VARS], n: 0 };
    eval_env(input, &mut env).map(|(v, _)| v)
}

pub const VARS: usize = 8;
/// A Spindle session's calculator state: `ans` and up to 8 named values,
/// each as (real, imaginary) parts.
pub struct Env { pub ans: Option<(i128, i128)>, vars: [([u8; 8], u8, (i128, i128)); VARS], n: usize }
impl Env { pub const fn new() -> Self { Self { ans: None, vars: [([0; 8], 0, (0, 0)); VARS], n: 0 } } }

/// Evaluate `expr` or `name = expr` (a 1-8 letter name, not a built-in).
/// Returns the value and the name assigned, if any; `ans` is updated.
pub fn eval_env<'a>(input: &'a [u8], env: &mut Env) -> Result<(Value, Option<&'a [u8]>), Error> {
    if input.len() > 200 { return Err(Error::Syntax); }
    let trimmed = { let a = input.iter().position(|c| *c != b' ').unwrap_or(input.len()); &input[a..] };
    let name_len = trimmed.iter().position(|c| !c.is_ascii_alphanumeric()).unwrap_or(trimmed.len());
    let after = &trimmed[name_len..];
    let after_eq = after.iter().position(|c| *c != b' ').map(|k| &after[k..]);
    let (name, expr) = match after_eq {
        Some(rest) if name_len > 0 && trimmed[0].is_ascii_alphabetic() && rest.first() == Some(&b'=') => (Some(&trimmed[..name_len]), &rest[1..]),
        _ => (None, input),
    };
    if let Some(n) = name {
        if n.len() > 8 || matches!(n, b"ans" | b"pi" | b"e" | b"sqrt" | b"abs" | b"min" | b"max" | b"round" | b"floor" | b"ceil") { return Err(Error::Syntax); }
    }
    let v = eval_raw(expr, env.ans, &env.vars[..env.n])?;
    if let Some(n) = name {
        let slot = env.vars[..env.n].iter().position(|(m, l, _)| &m[..*l as usize] == n);
        let k = match slot { Some(k) => k, None => { if env.n == VARS { return Err(Error::Overflow); } env.n += 1; env.n - 1 } };
        let mut m = [0u8; 8];
        m[..n.len()].copy_from_slice(n);
        env.vars[k] = (m, n.len() as u8, (v.scaled, v.imag));
    }
    env.ans = Some((v.scaled, v.imag));
    Ok((v, name))
}

fn eval_raw(input: &[u8], ans: Option<C>, vars: &[([u8; 8], u8, C)]) -> Result<Value, Error> {
    let mut p = Parser { s: input, at: 0, depth: 0, inexact: false, ans, vars };
    let v = p.expr()?;
    if p.peek().is_some() { return Err(Error::Syntax); }
    Ok(Value { scaled: v.0, imag: v.1, inexact: p.inexact })
}

/// `<expr> in hex|bin|oct`: the expression and the base to show it in.
pub fn split_radix(input: &[u8]) -> (&[u8], u32) {
    let end = input.iter().rposition(|c| *c != b' ').map_or(0, |k| k + 1);
    let t = &input[..end];
    for (suffix, radix) in [(&b" in hex"[..], 16), (b" in bin", 2), (b" in oct", 8)] {
        if t.ends_with(suffix) { return (&t[..t.len() - suffix.len()], radix); }
    }
    (input, 10)
}

/// Like `render`, in base 16, 2 or 8 (`= 0xff`, `= 0xff+0x10i`); only exact
/// whole parts. A value longer than the line is refused.
pub fn render_radix(v: Value, radix: u32, out: &mut [u8; LINE_MAX]) -> Result<usize, Error> {
    if radix == 10 {
        if v.imag == 0 { return Ok(render(v, out)); }
        let mut wide = [0u8; 2 * LINE_MAX];
        let n = render_parts(v, &mut wide);
        if n > LINE_MAX { return Err(Error::Overflow); }
        out[..n].copy_from_slice(&wide[..n]);
        return Ok(n);
    }
    if v.inexact || v.scaled % SCALE != 0 || v.imag % SCALE != 0 { return Err(Error::NotWhole); }
    let mut line = [0u8; 256];
    line[..2].copy_from_slice(b"= ");
    let mut n = 2;
    if v.imag == 0 || v.scaled != 0 { put_whole(v.scaled, radix, false, &mut line, &mut n); }
    if v.imag != 0 { put_whole(v.imag, radix, v.scaled != 0, &mut line, &mut n); line[n] = b'i'; n += 1; }
    if n > LINE_MAX { return Err(Error::Overflow); }
    out[..n].copy_from_slice(&line[..n]);
    Ok(n)
}

/// Appends a whole part as `-0x1f`; `plus` writes `+` before a non-negative part.
fn put_whole(part: i128, radix: u32, plus: bool, line: &mut [u8; 256], n: &mut usize) {
    let mut digits = [0u8; 128];
    let (mut k, mut w) = (0, (part / SCALE).unsigned_abs());
    loop { digits[k] = b"0123456789abcdef"[(w % radix as u128) as usize]; k += 1; w /= radix as u128; if w == 0 { break; } }
    if part < 0 { line[*n] = b'-'; *n += 1; } else if plus { line[*n] = b'+'; *n += 1; }
    let prefix: &[u8] = match radix { 16 => b"0x", 2 => b"0b", _ => b"0o" };
    line[*n..*n + 2].copy_from_slice(prefix); *n += 2;
    while k > 0 { k -= 1; line[*n] = digits[k]; *n += 1; }
}

/// Writes one scaled part's digits; trailing zeros trimmed.
fn put_fixed(a: u128, put: &mut impl FnMut(u8)) {
    let (whole, mut frac) = (a / SCALE as u128, a % SCALE as u128);
    let mut digits = [0u8; 40]; let mut k = 0; let mut w = whole;
    loop { digits[k] = b'0' + (w % 10) as u8; k += 1; w /= 10; if w == 0 { break; } }
    while k > 0 { k -= 1; put(digits[k]); }
    if frac != 0 {
        put(b'.');
        let mut places = 9; while frac % 10 == 0 { frac /= 10; places -= 1; }
        let mut f = [0u8; 9]; for i in (0..places).rev() { f[i] = b'0' + (frac % 10) as u8; frac /= 10; }
        for &d in &f[..places] { put(d); }
    }
}

/// `= 3+4i`, `~ 0.333333333i`: the real part when nonzero, then the imaginary part.
fn render_parts(v: Value, out: &mut [u8]) -> usize {
    let mut n = 0;
    let cap = out.len();
    let mut put = |b: u8| { if n < cap { out[n] = b; n += 1; } };
    put(if v.inexact { b'~' } else { b'=' }); put(b' ');
    if v.scaled != 0 {
        if v.scaled < 0 { put(b'-'); }
        put_fixed(v.scaled.unsigned_abs(), &mut put);
        put(if v.imag < 0 { b'-' } else { b'+' });
    } else if v.imag < 0 { put(b'-'); }
    put_fixed(v.imag.unsigned_abs(), &mut put);
    put(b'i');
    n
}

/// `= 1.5` or `~ 0.333333333`; trailing zeros trimmed. A complex value
/// longer than the line is cut here; `render_radix` refuses it instead.
pub fn render(v: Value, out: &mut [u8; LINE_MAX]) -> usize {
    if v.imag != 0 { return render_parts(v, out); }
    let mut n = 0;
    let mut put = |b: u8| { if n < LINE_MAX { out[n] = b; n += 1; } };
    put(if v.inexact { b'~' } else { b'=' }); put(b' ');
    if v.scaled < 0 { put(b'-'); }
    put_fixed(v.scaled.unsigned_abs(), &mut put);
    n
}

#[cfg(test)]
mod tests {
    use super::*;
    fn show(s: &str) -> String { match eval(s.as_bytes()) { Ok(v) => { let mut o = [0; LINE_MAX]; let n = render(v, &mut o); String::from_utf8(o[..n].to_vec()).unwrap() } Err(e) => format!("{e:?}") } }
    #[test] fn arithmetic_is_exact_decimal() {
        assert_eq!(show("2*(3+4)/5"), "= 2.8");
        assert_eq!(show("0.1+0.2"), "= 0.3");
        assert_eq!(show(" -3 - -4 "), "= 1");
        assert_eq!(show("10 % 4"), "= 2");
        assert_eq!(show("7.5%2"), "= 1.5");
        assert_eq!(show("1/3"), "~ 0.333333333");
        assert_eq!(show("1/3*3"), "~ 0.999999999");
        assert_eq!(show("12345678901234567890"), "= 12345678901234567890");
        assert_eq!(show(".5*4"), "= 2");
        assert_eq!(show("2**10"), "= 1024");
        assert_eq!(show("-2**2"), "= -4");
        assert_eq!(show("2**3**2"), "= 512");
        assert_eq!(show("1.5**2"), "= 2.25");
        assert_eq!(show("2**-2"), "= 0.25");
        assert_eq!(show("3**-1"), "~ 0.333333333");
        assert_eq!(show("2*3**2"), "= 18");
        assert_eq!(show("sqrt(16)"), "= 4");
        assert_eq!(show("sqrt(2)"), "~ 1.414213562");
        assert_eq!(show("sqrt(2.25)+1"), "= 2.5");
        assert_eq!(show("sqrt(-1)"), "Domain");
        assert_eq!(show("sqrt 4"), "Unknown");
        assert_eq!(show("abs(-2.5)+max(1, 7, 3)-min(4, -1)"), "= 10.5");
        assert_eq!(show("round(2.5) + round(-2.5) + floor(-1.2) + ceil(1.2)"), "= 0");
        assert_eq!(show("round(1, 2)"), "Syntax");
        assert_eq!(show("maxx(1)"), "Unknown");
        assert_eq!(show("2*pi"), "~ 6.283185308");
        assert_eq!(show("e"), "~ 2.718281828");
        assert_eq!(show("pie"), "Unknown");
        assert_eq!(show("0xff + 0b101"), "= 260");
        assert_eq!(show("0x"), "Syntax");
        assert_eq!(show("x + 1"), "Unknown");
    }
    #[test] fn errors_are_named_not_wrapped() {
        assert_eq!(show("1/0"), "DivideByZero");
        assert_eq!(show("5%0"), "DivideByZero");
        assert_eq!(show("99999999999999999999999999999*99999999999"), "Overflow");
        assert_eq!(show(""), "Syntax");
        assert_eq!(show("2+"), "Syntax");
        assert_eq!(show("(1"), "Syntax");
        assert_eq!(show("1 2"), "Syntax");
        assert_eq!(show("abc"), "Unknown");
        assert_eq!(show("1..2"), "Syntax");
        assert_eq!(show(&("(".repeat(40) + "1" + &")".repeat(40))), "TooDeep");
        assert_eq!(show(&"-".repeat(40)), "TooDeep");
        assert_eq!(show("ans+1"), "NoAnswer");
        assert_eq!(show("2**0.5"), "Exponent");
        assert_eq!(show("2**65"), "Exponent");
        assert_eq!(show("10**40"), "Overflow");
        assert_eq!(show("0**-1"), "DivideByZero");
        assert_eq!(show("answer"), "Unknown");
    }
    #[test] fn complex_values_are_exact() {
        assert_eq!(show("1i*1i"), "= -1");
        assert_eq!(show("(3+4i)*(3-4i)"), "= 25");
        assert_eq!(show("(3+4i)/5"), "= 0.6+0.8i");
        assert_eq!(show("norm((3+4i)/5)"), "= 1");
        assert_eq!(show("abs(3+4i)"), "= 5");
        assert_eq!(show("abs(1+1i)"), "~ 1.414213562");
        assert_eq!(show("conj(2-3i)"), "= 2+3i");
        assert_eq!(show("re(2-3i) + im(2-3i)"), "= -1");
        assert_eq!(show("1/1i"), "= -1i");
        assert_eq!(show("(1+1i)**2"), "= 2i");
        assert_eq!(show("(1+1i)**-2"), "= -0.5i");
        assert_eq!(show("1i/3"), "~ 0.333333333i");
        assert_eq!(show("(1+2i)/(3-4i)"), "= -0.2+0.4i");
        assert_eq!(show("-(0.5i)"), "= -0.5i");
        assert_eq!(show("0xai"), "= 10i");
    }
    #[test] fn amplitudes_interfere() {
        // Two equal paths add up to more than their separate intensities...
        assert_eq!(show("norm(1 + 1)"), "= 4");
        assert_eq!(show("norm(1) + norm(1)"), "= 2");
        assert_eq!(show("interfere(1, 1)"), "= 2");
        // ...opposite phases cancel although each path is there...
        assert_eq!(show("norm(1 + -1)"), "= 0");
        assert_eq!(show("interfere(1, -1)"), "= -2");
        assert_eq!(show("interfere((3+4i)/5, (-3-4i)/5)"), "= -2");
        // ...and a quarter turn apart they do not interfere at all.
        assert_eq!(show("interfere(1i, 1)"), "= 0");
        assert_eq!(show("norm(1i + 1) - norm(1i) - norm(1)"), "= 0");
        assert_eq!(show("interfere(0.000000001, 0.000000001i)"), "= 0", "only the real cross term rounds");
        assert_eq!(show("interfere(0.5, 0.5)"), "= 0.5");
        assert_eq!(show("interfere(1)"), "Syntax");
    }
    #[test] fn real_only_operations_say_so() {
        assert_eq!(show("min(1i, 2)"), "NotReal");
        assert_eq!(show("round(0.5i)"), "NotReal");
        assert_eq!(show("sqrt(1i)"), "NotReal");
        assert_eq!(show("5%1i"), "NotReal");
        assert_eq!(show("2**1i"), "Exponent");
        assert_eq!(show("i"), "Unknown");
        assert_eq!(show("2in"), "Syntax");
        assert_eq!(show("4ii"), "Syntax");
        assert_eq!(show("99999999999999999999i*99999999999999999999i"), "Overflow");
        let wide = eval(b"123456789012345678901.123456789+123456789012345678901.123456789i").unwrap();
        assert_eq!(render_radix(wide, 10, &mut [0; LINE_MAX]), Err(Error::Overflow), "longer than a line");
    }
    #[test] fn named_values_persist_in_the_environment() {
        let mut env = Env::new();
        let r = |env: &mut Env, s: &str| { let mut o = [0; LINE_MAX]; match eval_env(s.as_bytes(), env) { Ok((v, _)) => { let n = render(v, &mut o); String::from_utf8(o[..n].to_vec()).unwrap() } Err(e) => format!("{e:?}") } };
        assert_eq!(r(&mut env, "r = 2"), "= 2");
        assert_eq!(r(&mut env, "area = pi * r**2"), "~ 12.566370616");
        assert_eq!(r(&mut env, "r = r + 1"), "= 3");
        assert_eq!(r(&mut env, "r * 2 + ans"), "= 9");
        assert_eq!(r(&mut env, "pi = 3"), "Syntax");
        assert_eq!(r(&mut env, "x == 1"), "Syntax");
        assert_eq!(r(&mut env, "missing"), "Unknown");
        assert_eq!(r(&mut env, "i = 3"), "= 3", "i stays an ordinary name");
        assert_eq!(r(&mut env, "z = 3+4i"), "= 3+4i");
        assert_eq!(r(&mut env, "z*conj(z) + i"), "= 28");
        assert_eq!(r(&mut env, "ans*1i"), "= 28i");
    }
    #[test] fn results_show_in_other_bases() {
        let show = |s: &str| { let (e, r) = split_radix(s.as_bytes()); let mut o = [0; LINE_MAX]; render_radix(eval(e).unwrap(), r, &mut o).map(|n| String::from_utf8(o[..n].to_vec()).unwrap()) };
        assert_eq!(show("255 in hex").unwrap(), "= 0xff");
        assert_eq!(show("0xff+1 in bin ").unwrap(), "= 0b100000000");
        assert_eq!(show("-8 in oct").unwrap(), "= -0o10");
        assert_eq!(show("0 in hex").unwrap(), "= 0x0");
        assert_eq!(show("6/4").unwrap(), "= 1.5");
        assert_eq!(show("6/4 in hex"), Err(Error::NotWhole));
        assert_eq!(show("1/3*3 in hex"), Err(Error::NotWhole), "inexact");
        assert_eq!(show("2**64 in bin"), Err(Error::Overflow), "longer than a line");
        assert_eq!(split_radix(b"x in hexagon"), (&b"x in hexagon"[..], 10));
        assert_eq!(show("(255+16i) in hex").unwrap(), "= 0xff+0x10i");
        assert_eq!(show("1i in hex").unwrap(), "= 0x1i");
        assert_eq!(show("-1i in bin").unwrap(), "= -0b1i");
        assert_eq!(show("(8-8i) in oct").unwrap(), "= 0o10-0o10i");
        assert_eq!(show("(3+4i)*(3-4i) in bin").unwrap(), "= 0b11001");
        assert_eq!(show("(0.5+1i) in hex"), Err(Error::NotWhole));
        assert_eq!(show("1i/3*3 in hex"), Err(Error::NotWhole), "inexact");
        assert_eq!(show("(2**40)*(1+1i) in bin"), Err(Error::Overflow), "longer than a line");
    }
    #[test] fn ans_is_the_previous_result() {
        let first = eval(b"2*(3+4)/5").unwrap();
        let next = eval_with(b"ans*2 - ans", Some(first.scaled)).unwrap();
        let mut o = [0; LINE_MAX]; let n = render(next, &mut o);
        assert_eq!(&o[..n], b"= 2.8");
        assert_eq!(eval_with(b"(ans)/3", Some(SCALE)).map(|v| v.inexact), Ok(true));
    }
}
