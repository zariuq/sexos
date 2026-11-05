#!/usr/bin/env python3
"""Compare the growing Cesta evaluator with its original pinned semantic core."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from layout_text import LayoutText


HERE = Path(__file__).resolve().parent.parent
REPO = HERE.parent
PIN = "d58fcf4af6b9d37d77a6793bfec6d9fd655a3377"
SOURCE_SHA256 = "61b42d63cb819a938d34f8cdb11ce9682e4def5a45348a190394bfdda38c41a1"


def run(argv, *, env=None):
    result = subprocess.run(list(map(str, argv)), cwd=HERE, capture_output=True, env=env)
    if result.returncode:
        raise RuntimeError((argv, result.returncode, result.stdout[-2000:], result.stderr[-5000:]))
    return result.stdout


def cases():
    return [
        b"", b"; ;\n;", b"document.write('hello')", b"document.writeln(2+2)",
        b"var x=2;let y=3;const z=4;document.write(x+y*z)",
        b"var x='a';x=x+2;document.write(x,'/',3+'b')",
        b"var x=7\nx=x*6\ndocument.write(x)",
        b"/* head */ var x=9;// line\ndocument.write(x%4)",
        rb"document.write('a\nb\tc\r\0d')",
        b"document.write((2+3)*(4-1),-5,+6)",
        b"document.write(1/0,-1/0,0/0,NaN,Infinity)",
        b"document.write(5%2,-5%2,5%-2,-5%-2)",
        b"document.write(' 12.5 '+1,'', ' '+2)",
        b"document.write(0,1,10,1000000,0.000001)",
        b"document.write(1e21,1e-7,1e20)",
        b"var abcdefghijklmnop=1;document.write(abcdefghijklmnop)",
        b"var abcdefghijklmnopq=1", b"x=1", b"var document=1",
        b"document.write()", b"document.writeln()",
        b"document.print(1)", b"document.write(1 2)", b"var x=",
        b"'unterminated", b"/* unterminated", b"@", b"var x=1 var y=2",
        b"document.write((((((((1))))))))",
        b"document.write(((((((((1)))))))))",
        b"document.write(" + b"+-" * 15 + b"+1)",
        b"var a=1;var a=2;document.write(a)",
        b"document.write('x'+'y'+'z')",
        b"document.write(0.1+0.2,1/3,2.5*4)",
        b"document.write(999999999999999,9007199254740991)",
        b"document.write(-0, -0.0)",
        b"document.write('\\\\','\\\"',\"\\'\")",
        b"if(true)document.write('yes');else document.write('no')",
        b"if(false){document.write('bad')}else{document.write('good')}",
        b"var x=2;if(x>1&&x<=2){x=9}document.write(x)",
        b"document.write(true,false,!true,!0,!NaN,!'',!'x')",
        b"document.write(1<2,2>3,2<=2,3>=4,'a'<'b')",
        b"document.write(1===1,1==='1',1=='1',1!==2,1!='1')",
        b"document.write(false||'r',true&&'a',0&&'bad',1||'yes')",
        b"document.write(true?'a':'b',false?1:2)",
        b"document.write('Ab c'.length,'Ab c'.toUpperCase(),'Ab C'.toLowerCase())",
        b"document.write(Math.floor(2.9),Math.ceil(-2.1),Math.round(2.5),Math.abs(-4))",
        b"document.write(Math.min(4,2,8),Math.max(4,2,8),Math.min(),Math.max())",
        b"if(false)unknown=1", b"if(false)document.write(unknown)",
        b"var true=1", b"if(true){var x=1", b"else document.write(1)",
        b"var a=[1,2];a.push(3);document.write(a.length,a[0],a[2],a.join('-'))",
        b"var a=[1,2];a[1]=9;a[2]=4;document.write(a.join(','),a[8])",
        b"document.write('  Ab c  '.trim(),'banana'.indexOf('na'),'abc'.charAt(1))",
        b"document.write('abcdef'.slice(1,4),'abcdef'.slice(-3,-1),'abcdef'.substring(4,1))",
        b"document.write('a,b,c'.split(',').join('-'),'ab'.split('').join('/'))",
        b"document.write(String(12),Number(' 2.5 '),Number(),parseInt(' -42x'))",
        b"document.write(Math.trunc(-2.9),Math.sign(-8),Math.sign(0),Math.sqrt(9),Math.pow(2,10),Math.PI)",
        b"var x=0;while(x<5){x++}document.write(x)",
        b"var s=0;for(var i=0;i<5;i++){s+=i}document.write(s)",
        b"function add(a,b){return a+b}document.write(add(2,3))",
        b"var x=7;function f(x){var y=x*2;return y}document.write(f(3),x)",
        b"function write(x){document.write(x)}write('effect')",
        b"function f(a,b,c,d){return a+b+c+d}document.write(f(1,2,3,4))",
        b"function f(a,b,c,d,e){return 1}",
        b"while(true){}",
        b"var x=3;document.write(`value=${x+2}!`)",
        rb"document.write(`a\nb\t${'C'.toLowerCase()}`)",
        b"document.write(`unterminated ${1`)",
        b"var x=0;while(true){x++;if(x==3)break}document.write(x)",
        b"var s=0;for(var i=0;i<6;i++){if(i==2)continue;if(i==5)break;s+=i}document.write(s)",
        b"break", b"continue",
        b"function f(){break}f()",
        b"var o={a:1,'b':2};document.write(o.a,o['b'],o.missing)",
        b"var o={a:1};o.a+=2;o.a++;o['x']=4;document.write(o.a,o.x)",
        b"var k='name';var o={name:'value'};document.write(o[k])",
        b"var o={a:1,b:2};document.write(Object.keys(o).join('-'))",
        b"var o={a:1,a:3};document.write(o.a,Object.keys(o).length)",
        b"document.write(String({a:1}))",
        b"document.write(JSON.stringify({a:1,b:'x',c:true,d:undefined}))",
        b"document.write(JSON.stringify([1,undefined,NaN,Infinity,'x']))",
        rb'''document.write(JSON.stringify({q:'"\\\n\t',n:{x:2}}))''',
        b"document.write(JSON.stringify(undefined))",
        b"Object.keys(1)", b"var JSON=1", b"var Object=1",
        b"document.write(null,undefined,String(null),String(undefined),Number(null),Number(undefined))",
        b"document.write(null==undefined,null===undefined,null===null,undefined===undefined)",
        b"var a=[];var b=a;var c=[];document.write(a===b,a===c);var o={};var p=o;document.write(o===p,o==={})",
        b"document.write(typeof 1,typeof 'x',typeof true,typeof null,typeof undefined,typeof [],typeof {})",
        b"document.write(typeof missing,typeof NaN,typeof Math)",
        b"document.write((2.5).toFixed(),(2.5).toFixed(2),(-2.5).toFixed(0))",
        b"document.write((1.005).toFixed(2),(9.999).toFixed(2),(0).toFixed(3))",
        b"document.write((1e21).toFixed(2),NaN.toFixed(2),Infinity.toFixed(2))",
        b"document.write((1).toFixed(21))", b"var null=1", b"var undefined=1", b"var typeof=1",
        b"var f=function(x){return x*2};document.write(f(4),typeof f)",
        b"var f=x=>x+1;var g=(a,b)=>a*b;document.write(f(2),g(3,4))",
        b"var f=x=>{return x-1};document.write(f(9))",
        b"function twice(f,x){return f(f(x))}document.write(twice(x=>x+3,1))",
        b"document.write([1,2,3].map(x=>x*2).join(','))",
        b"document.write([1,2,3,4].filter(x=>x%2==0).join(','))",
        b"document.write([1,2,3].find(x=>x>1),[1,2,3].findIndex(x=>x==3))",
        b"document.write([1,2,3].some(x=>x==2),[1,2,3].every(x=>x>0))",
        b"document.write([1,2,3,4].reduce((a,b)=>a+b,0))",
        b"var s=0;[1,2,3].forEach(x=>{s+=x});document.write(s)",
        b"var a=[3,1,2];a.sort((x,y)=>x-y);document.write(a.join(','),a.reverse().join(','))",
        b"var a=['b','a',undefined,'c'];a.sort();document.write(a.join('-'))",
        b"var a=[1,2,3];document.write(a.pop(),a.shift(),a.join(','))",
        b"document.write([1,2,3].indexOf(2),[1,2].includes(3),[1,2,3].slice(1).join(','))",
        b"document.write([1,2].concat([3,4],5).join(','))",
        b"document.write('abcabc'.replace('ab','X'),'abcabc'.replaceAll('ab','X'))",
        b"document.write('abcdef'.startsWith('ab'),'abcdef'.endsWith('ef'),'abcdef'.includes('cd'),'ababa'.lastIndexOf('ba'))",
        b"document.write('ab'.repeat(3),'x'.padStart(3,'0'),'x'.padEnd(3,'0'))",
        b"var x=0;do{x++}while(x<3);document.write(x)",
        b"var x=2;switch(x){case 1:document.write('a');break;case 2:document.write('b');default:document.write('c')} ",
        b"var x=1;switch(x){case 1:document.write('a');case 2:document.write('b');break;default:document.write('c')}",
        b"var x=12;x*=2;x/=3;x%=5;document.write(x)",
        b"var do=1", b"var switch=1", b"var case=1", b"var default=1",
        b"var a=[2,3];document.write([1,...a,4].join(','))",
        b"function f(a,b,c,d){return a+b+c+d}var x=[2,3];document.write(f(1,...x,4))",
        b"document.write(Math.max(...[1,8,3]))",
        b"var [a,b,c]=[1,2];document.write(a,b,c)",
        b"var {a,b}={a:1,b:2};document.write(a,b)",
        b"var o={a:1};document.write('a' in o,'b' in o);var x=[7,8];document.write('0' in x,'2' in x)",
        b"var o={a:1,b:2};delete o.a;document.write(o.a,Object.keys(o).join(','))",
        b"document.write((255).toString(16),(-10).toString(2),(35).toString(36))",
        b"document.write(parseFloat('  -12.5xyz'),parseFloat('1e2!'),parseFloat('x'))",
        b"document.write(isNaN('x'),isNaN('2'),Number.isNaN(NaN),Number.isNaN('x'))",
        b"document.write(Array.isArray([]),Array.isArray({}),Number.isInteger(2),Number.isInteger(2.5),Number.isFinite(Infinity))",
        b"var o={a:1,b:2};document.write(Object.values(o).join(','),JSON.stringify(Object.entries(o)))",
        b"var in=1", b"var delete=1", b"var Array=1",
        b"document.write(0xff,0X10,0xabcdef)",
        b"document.write('ABC'.charCodeAt(1),'ABC'.charCodeAt(9))",
        b"document.write(String.fromCharCode(65,66,67))",
        b"document.write(String.fromCharCode(0x20ac,0x00a3))",
        b"String.fromCharCode(-1)",
        b"var x=1;{let x=2;document.write(x)}document.write(x)",
        b"{var x=3}document.write(x)",
        b"{let x=3}document.write(typeof x)",
        b"const x=1;x=2",
        b"let x=1;{let x=2;{let x=3;document.write(x)}document.write(x)}document.write(x)",
        b"for(let i=0;i<2;i++){}document.write(typeof i)",
        b"var o={x:7,f:function(){return this.x}};document.write(o.f())",
        b"var o={x:3,f:function(a){return this.x+a}};document.write(o.f(4))",
        b"try{throw 'bad'}catch(e){document.write(e)}",
        b"try{document.write('a')}catch(e){document.write('b')}finally{document.write('c')}",
        b"try{throw 2}finally{document.write('f')}",
        b"throw 'uncaught'",
        rb"document.write('\x41\u0042\u{20ac}')",
        b"var a=[1,2,3];a.fill(9,1,3);document.write(a.join(','),a.at(-1))",
        b"document.write([1,[2,3],4].flat().join(','))",
        b"document.write('  x  '.trimStart(),'  x  '.trimEnd(),'abc'.at(-1))",
        b"document.write(Array.from('abc').join('-'))",
        b"document.write(Array.from({length:3}).length)",
        b"var this=1", b"var throw=1", b"var try=1", b"var catch=1", b"var finally=1",
        b"document.write(/a+/.test('caaab'),/z/.test('abc'))",
        b"var r=/a(b+)/gi;document.write(r.source,r.flags,String(r))",
        b"document.write('abcabc'.search(/bc/),'abc'.search(/z/))",
        b"document.write('abbb'.match(/a(b+)/).join('|'))",
        b"document.write('a1a2'.match(/a./g).join(','))",
        b"document.write('a1b2'.split(/([0-9])/).join('|'))",
        b"document.write('abcabc'.replace(/a/,'X'),'abcabc'.replace(/a/g,'X'))",
        b"document.write('ab12'.replace(/([a-z]+)([0-9]+)/,'$2-$1-$&-$$'))",
        b"document.write('a1b2'.replace(/([0-9])/g,x=>'['+x+']'))",
        b"document.write('AaA'.replaceAll(/a/gi,'x'))",
        b"'aaa'.replaceAll(/a/,'x')",
        b"document.write(/^[a-z]+$/i.test('AbC'))",
        b"var o=JSON.parse('{\"a\":1,\"b\":[true,null,\"x\"]}');document.write(o.a,o.b.length,o.b[2])",
        b"document.write(JSON.stringify(JSON.parse('[1,{\"x\":2}]')))",
        b"try{JSON.parse('{bad}')}catch(e){document.write(e)}",
        b"var a=new Array(3);document.write(a.length,a[0]);document.write((new Array(1,2)).join(','))",
        b"var o=new Object();o.x=2;document.write(o.x)",
        b"var e=new TypeError('bad');document.write(e.name,e.message)",
        b"function C(x){this.x=x}var o=new C(7);document.write(o.x)",
        b"class C{constructor(x){this.x=x}get(){return this.x}}var o=new C(9);document.write(o.get())",
        b"class C{twice(x){return x*2}}document.write((new C()).twice(4))",
        b"var d=new Date(0);document.write(d.getTime(),d.getUTCFullYear(),d.getUTCMonth(),d.getUTCDate())",
        b"var d=new Date('1970-01-02T03:04:05.006Z');document.write(d.toISOString(),d.getUTCHours(),d.getUTCMinutes())",
        b"document.write(Date.UTC(1970,0,2,0))",
        b"new Date()", b"Date.now()", b"class C extends D{}",
        b"var new=1", b"var class=1", b"var Date=1",
        b"var x=0;function f(){x++;return true}document.write(true||f(),false&&f(),x)",
        b"var x=0;function f(){x++;return 9}document.write(null??f(),3??f(),x)",
        b"var x=0;function f(){x++;return 1}document.write(true?f():f(),x)",
        b"var o=null;document.write(o?.x,o?.['x'])",
        b"var o={x:{y:3}};document.write(o?.x.y,o.x?.y)",
        b"var a=1,b,c=3;document.write(a,b,c)",
        b"let a,b=2;document.write(a,b)",
        b"const a",
        b"class A{constructor(x){this.x=x}get(){return this.x}}class B extends A{constructor(x){super(x+1)}get(){return super.get()+1}}var b=new B(4);document.write(b.get())",
        b"class A{constructor(x){this.x=x}}class B extends A{}document.write((new B(7)).x)",
        b"class A{m(){return 2}}class B extends A{m(){return super.m()*3}}document.write((new B()).m())",
        b"var o=null;o?.x()",
        b"function f(a=2,b=a+1){return a+b}document.write(f(),f(5))",
        b"function f(a,...rest){return a+rest.join('-')}document.write(f('x',1,2,3))",
        b"var f=(a=3,...r)=>a+r.length;document.write(f(),f(1,2,3))",
        b"class C{m(a=2,...r){return a+r.length}}document.write((new C()).m(undefined,1,2))",
        b"var s='';for(var x of [1,2,3]){s+=x}document.write(s)",
        b"var s='';for(let x of 'abc'){s+=x}document.write(s)",
        b"var s='';for(var k in {a:1,b:2}){s+=k}document.write(s)",
        b"var s='';for(var k in [7,8]){s+=k}document.write(s)",
        b"var s=0;for(var x of [1,2,3,4]){if(x==3)break;s+=x}document.write(s)",
        b"var t={a:1};Object.assign(t,{b:2},{a:3},null,undefined);document.write(t.a,t.b)",
        b"Object.assign({},1)",
        b"document.write([1,2,1].lastIndexOf(1),[1,2,3].toString())",
        b"var a=[1,2,3,4];var r=a.splice(1,2,8,9);document.write(r.join('-'),a.join('-'))",
        b"var a=[1,2,3];document.write(a.splice(1).join('-'),a.join('-'))",
        b"var a=[3,4];document.write(a.unshift(1,2),a.join('-'))",
        b"document.write([1,4,2,5].findLast(x=>x<4),[1,4,2,5].findLastIndex(x=>x<4))",
        b"document.write([1,2].flatMap(x=>[x,x+10]).join('-'))",
        rb"document.write('\u{20ac}'.codePointAt(0),'\u{20ac}'.codePointAt(1),'x'.codePointAt(9))",
        rb"document.write(encodeURIComponent('a b!'),encodeURIComponent('\u{20ac}'))",
        b"document.write(decodeURIComponent('a%20b%21'),decodeURIComponent('%E2%82%AC'))",
        b"decodeURIComponent('%ZZ')",
        b"document.write(Array.of(1,'x',true).join('-'))",
        b"var o=Object.fromEntries([['a',1],[2,'b']]);document.write(o.a,o[2])",
        b"Object.fromEntries([1])",
        b"document.write('a,b,c'.split(',',2).join('-'),'abc'.split('',0).length)",
        b"document.write('a1b2'.split(/([0-9])/,3).join('|'))",
        b"document.write(2**3,2**3**2,(-2)**3)",
        b"var x=2;x**=3;document.write(x)",
        b"document.write(6&3,4|1,7^3,~0,1<<4,-8>>2,-1>>>1)",
        b"var x=3;x|=4;x&=6;x^=1;document.write(x)",
        b"var x=0,y=1,z=null;x||=4;y&&=5;z??=6;document.write(x,y,z)",
        b"var x=7;x||=missing;var y=0;y&&=missing;var z=1;z??=missing;document.write(x,y,z)",
        b"document.write(void 9,void(1+2))",
        b"document.write((function(x){return x+1})(4))",
        b"var fs=[x=>x*3];document.write(fs[0](5))",
        b"document.write(Number('0xff'),Number('0o10'),Number('0b101'),Number('0x'))",
        b"document.write(Number('Infinity'),Number('-Infinity'),Number('inf'))",
        b"function C(x){this.x=x}var o=new C(3);document.write(o instanceof C,o instanceof Object,o instanceof Array)",
        b"class A{}class B extends A{}var b=new B();document.write(b instanceof B,b instanceof A,b instanceof Object)",
        b"document.write([] instanceof Array,[] instanceof Object,(x=>x) instanceof Function)",
        b"var e=new TypeError('x');document.write(e instanceof Error,e instanceof TypeError,e instanceof RangeError)",
        b"class A{static n(){return 3}}class B extends A{static m(){return 4}}document.write(A.n(),B.n(),B.m())",
        b"var static=1",
        b"var x=1;++x;--x;document.write(x)",
        b"(function(){document.write('iife')})();[1,2].map(x=>x+1);",
        b"function six(a,b,c,d,e,f){return a+b+c+d+e+f}document.write(six(1,2,3,4,5,6))",
        b"function seven(a,b,c,d,e,f,g){return 0}",
        b"var x=1;document.write(x++,x,++x,x)",
        b"var x=1;document.write(x--,--x,x)",
        b"var x=1;document.write(x)\n++x;document.write(x)",
        b"var o={a:{b:[{c:2}]}};o.a.b[0].c+=5;document.write(o.a.b[0].c)",
        b"var o={a:{b:1}};o['a']['b']*=3;document.write(o.a.b)",
        b"var a=[1,2];a[0]++;a[1]**=3;document.write(a.join(','))",
        b"var a=[1,2,3];a.length=1;document.write(a.length,a.join(','))",
        b"var a=[1];a.length=3",
        b"var o={a:{b:0}};o.a.b||=7;document.write(o.a.b)",
        b"var fs=[x=>x+2];document.write(fs[0](5))",
        b"var a=2,b=3;var o={a,b};document.write(o.a,o.b)",
        b"var o={x:4,m(a){return this.x+a}};document.write(o.m(3))",
        b"var k='name';var o={[k]:7,1:'one'};document.write(o.name,o[1])",
        b"var a={x:1,y:2};var b={...a,y:3,...null};document.write(b.x,b.y)",
        b"document.write(JSON.stringify([function(){},1]),JSON.stringify({f:function(){},x:2}))",
        b"var [a,,b=9,...r]=[1,2];document.write(a,b,r.length)",
        b"var [a=4,b=5]=[undefined,null];document.write(a,b)",
        b"var n=0;var [a=++n]=[3];document.write(a,n)",
        b"var {a,b:c=7,...r}={a:1,d:4};document.write(a,c,r.d,r.a)",
        b"var {x:y=3}={};document.write(y)",
        b"var s='';for(const [k,v] of [['a',1],['b',2]]){s+=k+v}document.write(s)",
        b"var s=0;for(const {x,y=1} of [{x:2},{x:3,y:4}]){s+=x+y}document.write(s)",
        b"var [...r]=[1,2,3];document.write(r.join('-'))",
        b"var {...r}={a:1,b:2};document.write(Object.keys(r).join('-'))",
        b"function counter(x){return ()=>{x++;return x}}var c=counter(2);document.write(c(),c())",
        b"function pair(x){return {inc:()=>{x++},get:()=>x}}var p=pair(3);p.inc();p.inc();document.write(p.get())",
        b"function pair(x){return [()=>{x++},()=>x]}var p=pair(5);p[0]();document.write(p[1]())",
        b"function counter(x){return ()=>++x}var a=counter(0),b=counter(10);document.write(a(),b(),a(),b())",
        b"var x=1;function f(x){var y=x+1;return y}document.write(f(4),x)",
        b"var x=1;function f(){var x=2;{var y=3}return x+y}document.write(f(),x)",
        b"function f(){var x=2;var x;return x}document.write(f())",
        b"var x=7;function f(x){x++;return x}document.write(f(1),x)",
        b"function maker(x){return {next(){x++;return x}}}var m=maker(8);document.write(m.next(),m.next())",
        b"function maker(x){function C(){this.x=x}return C}var C=maker(9);var o=new C();document.write(o.x,o instanceof C)",
        b"var o={_x:1,get x(){return this._x},set x(v){this._x=v}};document.write(o.x);o.x=7;document.write(o.x,Object.keys(o).join(','))",
        b"var o={get x(){return 2},x:5};document.write(o.x,Object.keys(o).join(','))",
        b"var o={get x(){return 3}};o.x=9;document.write(o.x);var p={set x(v){this.y=v}};p.x=4;document.write(p.x,p.y)",
        b"class C{constructor(){this._x=2}get x(){return this._x}set x(v){this._x=v}}var c=new C();document.write(c.x);c.x=8;document.write(c.x,Object.keys(c).join(','))",
        b"class A{get x(){return 3}}class B extends A{}document.write((new B()).x)",
        b"var o={get x(){return 2},a:1};document.write(JSON.stringify(o))",
        b"var s='';outer:for(var i=0;i<3;i++){for(var j=0;j<3;j++){s+=i+''+j;if(i==1&&j==1)break outer}}document.write(s)",
        b"var s='';outer:for(var i=0;i<3;i++){for(var j=0;j<2;j++){if(j==0)continue outer;s+='x'}s+=i}document.write(s)",
        b"var x=0;done:{x=1;break done;x=2}document.write(x)",
        b"var x=0;outer:do{x++;if(x<3)continue outer}while(x<3);document.write(x)",
        b"break missing",
        b"document.write(Boolean(0),Boolean(1),Boolean(''),Boolean('x'),Boolean(null))",
        b"document.write(['1','2.5','x'].map(Number).join(','))",
        b"document.write([1,true,null].map(String).join(','))",
        b"document.write([0,1,'',null,[]].map(Boolean).join(','))",
        b"document.write(typeof String,typeof Number,typeof Boolean)",
        b"document.write([1].map(String)[0],[].map(Number).length)",
        b"var Number=x=>x+10;document.write([1,2].map(Number).join(','))",
    ]


def rust_slice(data):
    return "&[" + ",".join(map(str, data)) + "]"


def c_row(data):
    body = ",".join(map(str, data)) if data else "0"
    return "{{" + body + "}," + str(len(data)) + "}"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cesta", type=Path, required=True)
    parser.add_argument("--work", type=Path)
    parser.add_argument("--mutations", action="store_true")
    args = parser.parse_args()
    work = (args.work or Path(tempfile.mkdtemp(
        prefix="kaleidoscope-script-core-", dir=HERE / "build"
    ))).resolve()
    work.mkdir(parents=True, exist_ok=True)
    source = subprocess.check_output(["git", "show", PIN + ":apps/kaleidoscope/src/script.rs"], cwd=REPO)
    assert hashlib.sha256(source).hexdigest() == SOURCE_SHA256
    regex_source = subprocess.check_output(["git", "show", PIN + ":apps/kaleidoscope/src/regex.rs"], cwd=REPO)
    text = ("extern crate alloc;\nmod regex {\n" + regex_source.decode().replace("//!", "//") +
            "\n}\n" + source.decode().replace("//!", "//"))
    selected = cases()
    rows = ",\n".join(rust_slice(case) for case in selected)
    text += "\nstatic CASES:&[&[u8]]=&[" + rows + "];\n"
    text += r'''
fn main(){let n:usize=std::env::args().nth(1).unwrap().parse().unwrap();match run(CASES[n]){
 Ok(bytes)=>{print!("O ");for b in bytes{print!("{:02x}",b)}println!()},
 Err(e)=>println!("E {}",match e{Error::Unsupported=>'U',Error::Syntax=>'S',Error::Limit=>'L',Error::Thrown=>'T'})}}
'''
    (work / "reference.rs").write_text(text)
    reference = work / "reference"
    run(["rustc", "--edition=2021", "-C", "opt-level=2", work / "reference.rs", "-o", reference])
    expected = [run([reference, i]) for i in range(len(selected))]
    (work / "cases.h").write_text(
        "static const KaleidoscopeScriptSource cases[]={\n" + ",\n".join(c_row(case) for case in selected) + "\n};\n"
    )
    host = HERE / "tests/kaleidoscope_script_core/host.c"
    builds = []
    exports = {}
    for compiler in ("gcc", "clang"):
        lexer = work / f"lexer-{compiler}.c"
        regex = work / f"regex-{compiler}.c"
        matcher = work / f"regex-match-{compiler}.c"
        core = work / f"core-{compiler}.c"
        run([args.cesta, "export-c", "src/kaleidoscope_script_lexer.cesta", "--cc", compiler, "--std=c11",
             "-o", lexer, "--", "-Igenerated/include"])
        run([args.cesta, "export-c", "src/kaleidoscope_script_core.cesta", "--cc", compiler, "--std=c11",
             "-o", core, "--", "-Igenerated/include"])
        run([args.cesta, "export-c", "src/kaleidoscope_regex.cesta", "--cc", compiler, "--std=c11",
             "-o", regex, "--", "-Igenerated/include"])
        run([args.cesta, "export-c", "src/kaleidoscope_regex_match.cesta", "--cc", compiler, "--std=c11",
             "-o", matcher, "--", "-Igenerated/include"])
        exports[compiler] = core
        common = ["-std=c11", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-DKALEIDOSCOPE_SCRIPT_DEPTH_LIMIT=32", "-DKALEIDOSCOPE_SCRIPT_ARENA_LIMIT=4096",
                  "-DKALEIDOSCOPE_SCRIPT_VARS_LIMIT=32", "-DKALEIDOSCOPE_SCRIPT_FUNCS_LIMIT=16",
                  "-DKALEIDOSCOPE_SCRIPT_ELEMS_LIMIT=256", "-DKALEIDOSCOPE_SCRIPT_PARAMS_LIMIT=6",
                  "-DKALEIDOSCOPE_SCRIPT_PROPS_LIMIT=128",
                  "-Igenerated/include", "-I", work]
        for mode, extra in (
            ("O0", ["-O0"]), ("O2", ["-O2"]),
            ("SAN", ["-O1", "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                     "-fno-pie", "-no-pie"]),
        ):
            binary = work / f"{compiler}-{mode}"
            run([compiler, *common, *extra, host, lexer, regex, matcher, core, "-o", binary])
            for i, wanted in enumerate(expected):
                actual = run([binary, i], env={**os.environ,
                    "ASAN_OPTIONS": "detect_leaks=1:abort_on_error=1"})
                assert actual == wanted, (compiler, mode, i, selected[i], actual, wanted)
            builds.append((compiler, mode))
    mutations = []
    if args.mutations:
        base = LayoutText(exports["gcc"].read_text())
        changes = [
            ("output-bound", "output_len+s.len>KALEIDOSCOPE_SCRIPT_MAX_OUTPUT", "false"),
            ("const-required", "if(v->kind==2&&!declare)return KALEIDOSCOPE_SCRIPT_UNSUPPORTED", "if(false)return KALEIDOSCOPE_SCRIPT_UNSUPPORTED"),
            ("remainder-sign", "a-t*b", "a+t*b"),
            ("concat-order", "script_to_text(m,left,&left)", "script_to_text(m,right,&left)"),
            ("depth-bound", "depth>=KALEIDOSCOPE_SCRIPT_DEPTH_LIMIT", "depth>KALEIDOSCOPE_SCRIPT_DEPTH_LIMIT"),
        ]
        lexer = work / "lexer-gcc.c"
        regex = work / "regex-gcc.c"
        matcher = work / "regex-match-gcc.c"
        common = ["-std=c11", "-O2", "-g0", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
                  "-DKALEIDOSCOPE_SCRIPT_DEPTH_LIMIT=32", "-DKALEIDOSCOPE_SCRIPT_ARENA_LIMIT=4096",
                  "-DKALEIDOSCOPE_SCRIPT_VARS_LIMIT=32", "-DKALEIDOSCOPE_SCRIPT_FUNCS_LIMIT=16",
                  "-DKALEIDOSCOPE_SCRIPT_ELEMS_LIMIT=256", "-DKALEIDOSCOPE_SCRIPT_PARAMS_LIMIT=6",
                  "-DKALEIDOSCOPE_SCRIPT_PROPS_LIMIT=128",
                  "-Igenerated/include", "-I", work]
        for name, old, new in changes:
            assert base.count(old) >= 1, (name, base.count(old))
            mutant = work / f"{name}.c"
            mutant.write_text(base.replace(old, new))
            binary = work / name
            built = subprocess.run(["gcc", *common, host, lexer, regex, matcher, mutant, "-o", binary], cwd=HERE, capture_output=True)
            if built.returncode == 0:
                assert any(subprocess.run([binary, str(i)], cwd=HERE, capture_output=True).stdout != wanted
                           for i, wanted in enumerate(expected)), ("survived", name)
            mutations.append(name)
    summary = {"pin": PIN, "source_sha256": SOURCE_SHA256, "cases": len(selected),
               "builds": builds, "mutations": mutations}
    (work / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print("kaleidoscope script core:", len(selected), "Rust cases,", len(builds),
          "builds,", len(mutations), "mutants")


if __name__ == "__main__":
    main()
