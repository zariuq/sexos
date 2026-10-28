#!/usr/bin/env python3
"""Incremental HTTP framing, independent expected bodies, and compiler controls."""
import argparse
import hashlib
import io
import http.client
from pathlib import Path
import random
import subprocess
import tempfile

HERE = Path(__file__).resolve().parent.parent
COMPLETE, FAILED = 9, 10

def run(args, data=None):
    result = subprocess.run(list(map(str, args)), cwd=HERE, input=data, capture_output=True)
    if result.returncode:
        raise RuntimeError((args, result.returncode, result.stderr.decode(errors='replace')[-4000:]))
    return result.stdout

class Socket:
    def __init__(self, data): self.data = data
    def makefile(self, *args): return io.BytesIO(self.data)

def cases():
    rng = random.Random(0x483131)
    for status in (200, 201, 404, 503):
        for length in (0, 1, 7, 8, 17, 64, 127, 256):
            body = bytes(rng.randrange(256) for _ in range(length))
            wire = f'HTTP/1.1 {status} Result\r\nContent-Length: {length}\r\nX-Test: ordinary\r\n\r\n'.encode()+body
            ref = http.client.HTTPResponse(Socket(wire)); ref.begin()
            assert ref.status == status and ref.read() == body
            for split in range(len(wire)+1):
                yield wire, split, len(wire), True, COMPLETE, 0, status, body
            yield wire, 0, 1, True, COMPLETE, 0, status, body
    for body in (b'', b'a', b'hello\x00\r\nworld', bytes(range(256))):
        chunks = [body[i:i+7] for i in range(0,len(body),7)]
        wire = b'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n'
        wire += b''.join(f'{len(part):x};extension=yes\r\n'.encode()+part+b'\r\n' for part in chunks)
        wire += b'0\r\nX-Trailer: yes\r\n\r\n'
        ref = http.client.HTTPResponse(Socket(wire)); ref.begin(); assert ref.read() == body
        for split in range(len(wire)+1):
            yield wire, split, 3, True, COMPLETE, 0, 200, body
    for wire, body, status in [
        (b'HTTP/1.0 200 OK\r\n\r\nclose-delimited', b'close-delimited', 200),
        (b'HTTP/1.1 204 No Content\r\n\r\n', b'', 204),
        (b'HTTP/1.1 304 Not Modified\r\nContent-Length: 20\r\n\r\n', b'', 304),
        (b'HTTP/1.1 100 Continue\r\n\r\nHTTP/1.1 200 OK\r\nContent-Length: 1\r\n\r\nx', b'x', 200),
        (b'HTTP/1.1 200 OK\r\ncontent-length:\t1\t\r\nContent-Length: 1\r\n\r\nx', b'x', 200)]:
        for stride in (1,2,3,7,64):
            yield wire, 0, stride, True, COMPLETE, 0, status, body
    negatives = [
        (b'HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\nx', 3, 200, b'x'),
        (b'HTTP/1.1 200 OK\r\nContent-Length: 0\r\nContent-Length: 1\r\n\r\n', 1, 200, b''),
        (b'HTTP/1.1 200 OK\r\nContent-Length: 257\r\n\r\n', 2, 200, b''),
        (b'HTTP/1.1 200 OK\r\nTransfer-Encoding: gzip\r\n\r\n', 4, 200, b''),
        (b'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\nContent-Length: 0\r\n\r\n', 1, 200, b''),
        (b'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n3\r\nx', 3, 200, b'x'),
        (b'HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n101\r\n', 2, 200, b''),
        (b'HTTP/1.1 200 OK\r\nContent-Encoding: gzip\r\n\r\n', 4, 200, b''),
        (b'HTTP/1.1 200 OK\n\n', 1, 0, b''),
        (b'HTTP/1.1 20x OK\r\n\r\n', 1, 0, b''),
        (b'HTTP/1.1 200 OK\r\nX: '+b'a'*1024, 2, 200, b''),
    ]
    for wire, error, status, body in negatives:
        for stride in (1, 7, len(wire)):
            yield wire, 0, stride, True, FAILED, error, status, body
    # Ending a feed is not ending the message or connection.
    yield b'HTTP/1.1 200 OK\r\nContent-Length: 2\r\n\r\nx', 0, 1, False, 2, 0, 200, b'x'

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--cesta', type=Path, required=True)
    args = parser.parse_args()
    work = Path(tempfile.mkdtemp(prefix='network-http-', dir=HERE/'build'))
    source = run([args.cesta, 'export-c', 'src/network_http.cesta', '--'])
    (work/'http.c').write_bytes(source)
    samples = list(cases())
    data = ''.join(f'{int(end)} {split} {stride} {wire.hex()}\n' for wire, split, stride, end, *_ in samples).encode()
    expected = [f'{state} {error} {status} {len(body)} {body.hex()}'.encode() for _,_,_,_,state,error,status,body in samples]
    for compiler in ('gcc', 'clang'):
        for mode in ('O0','O2','sanitize'):
            flags = ['-O1','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if mode == 'sanitize' else ['-'+mode]
            binary = work/(compiler+'-'+mode)
            run([compiler,'-std=c11','-Wall','-Wextra','-Wpedantic','-Werror',*flags,
                 '-Igenerated/include',work/'http.c','tests/network_http/host.c','-o',binary])
            actual = run([binary],data=data).splitlines()
            assert len(actual) == len(expected)
            for index, (a,e) in enumerate(zip(actual,expected)):
                assert a == e, (compiler,mode,index,a,e,samples[index][:4])
            print(compiler, mode, len(samples), 'incremental framing/body cases passed')
    mutants=[
        (b'r->status == 204 || r->status == 304',b'r->status == 200 || r->status == 304'),
        (b'} else r->state = HTTP_UNTIL_CLOSE;',b'} else r->state = HTTP_COMPLETE;'),
        (b'r->body[r->length ++] = byte;',b'r->body[r->length ++] = byte ^ 1;'),
        (b'complete_line(r, r->line_length - 1);',b'complete_line(r, r->line_length);'),
        (b'if (value > NET_HTTP_BODY_CAPACITY - r->length)',b'if (value >= NET_HTTP_BODY_CAPACITY - r->length)'),
        (b'else if (r->state != HTTP_COMPLETE && r->state != HTTP_FAILED) http_fail(r, HTTP_ERROR_INCOMPLETE);',
         b'else if (r->state != HTTP_COMPLETE && r->state != HTTP_FAILED) r->state = HTTP_COMPLETE;'),
    ]
    for index,(needle,replacement) in enumerate(mutants):
        assert source.count(needle)==1,(index,needle,source.count(needle))
        changed=work/('mutant-'+str(index)+'.c');changed.write_bytes(source.replace(needle,replacement))
        binary=work/('mutant-'+str(index))
        run(['gcc','-std=c11','-O2','-Igenerated/include',changed,'tests/network_http/host.c','-o',binary])
        assert run([binary],data=data).splitlines()!=expected,('survived',index)
    print(len(mutants),'HTTP framing and content mutations refused')
    print('HTTP source sha256',hashlib.sha256(source).hexdigest())

if __name__ == '__main__':
    main()
