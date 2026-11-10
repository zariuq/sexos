use std::io::{self, Read, Write};

#[allow(dead_code)]
#[path = "program_image.rs"]
mod image;

fn main() {
    let stdin = io::stdin();
    let mut input = stdin.lock();
    let stdout = io::stdout();
    let mut output = io::BufWriter::new(stdout.lock());
    loop {
        let mut header = [0u8; 12];
        match input.read_exact(&mut header) {
            Ok(()) => (),
            Err(error) if error.kind() == io::ErrorKind::UnexpectedEof => break,
            Err(error) => panic!("{error}"),
        }
        let window = u64::from_le_bytes(header[..8].try_into().unwrap());
        let length = u32::from_le_bytes(header[8..].try_into().unwrap()) as usize;
        assert!(length <= 64 * 1024 * 1024);
        let mut bytes = vec![0; length];
        input.read_exact(&mut bytes).unwrap();
        match image::parse(&bytes, window) {
            Err(reason) => {
                let code = match reason {
                    image::Reject::Header => 1, image::Reject::Layout => 2,
                    image::Reject::Permissions => 3, image::Reject::Span => 4,
                    image::Reject::Entry => 5, image::Reject::Relocation => 6,
                };
                writeln!(output, "{code}").unwrap();
            }
            Ok(parsed) => {
                write!(output, "0 {} {} {} {} {} {}", parsed.count, parsed.entry,
                       parsed.bias, parsed.rela, parsed.rela_count, parsed.stack).unwrap();
                for s in &parsed.segments[..parsed.count] {
                    write!(output, " {} {} {} {} {} {}", s.offset, s.file, s.window,
                           s.memory, s.writable as u8, s.executable as u8).unwrap();
                }
                for i in 0..parsed.rela_count {
                    let (target, value) = image::relocation(&bytes, &parsed, i, 0).unwrap();
                    write!(output, " {target} {value}").unwrap();
                }
                writeln!(output).unwrap();
            }
        }
    }
}
