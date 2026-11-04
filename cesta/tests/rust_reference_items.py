"""Reference extraction must preserve item boundaries before source hashing."""
from silk_shell_input_differential import item_end,declarations


def main():
    examples=[
        ('const fn first()->bool{true}\nfn second(){}\nconst N:u8=7;', 'const fn first()->bool{true}'),
        ('pub const unsafe fn first()->bool{true}\nfn second(){}', 'pub const unsafe fn first()->bool{true}'),
        ('const A:[u8;2]={let x=1;[x,2]};\nfn second(){}', 'const A:[u8;2]={let x=1;[x,2]};'),
        ('static mut A:[u8;2]=[1,2];\nfn second(){}','static mut A:[u8;2]=[1,2];'),
        ('fn first(){/* } nested /* { */ ; */println!("};");}\nconst N:u8=7;','fn first(){/* } nested /* { */ ; */println!("};");}'),
    ]
    for source,want in examples:assert source[:item_end(source,0)]==want,(source,want)
    source='#[derive(Clone,Copy)]struct Row {n:u8}\n#[allow(dead_code)] const N:u8=7;'
    items=declarations(source);assert set(items)=={'Row','N'}and items['N'].endswith(';'),items
    assert 'derive(Clone,Copy)'in items['Row']
    assert declarations('const _:()=();\npub const unsafe fn f()->u8{1}\nconst N:u8=2;')=={'N':'const N:u8=2;'}
    for source in ('fn unfinished(){','fn unfinished(){/* never closed'):
        try:item_end(source,0)
        except AssertionError:pass
        else:raise AssertionError(('unterminated reference item accepted',source))
    print('Rust reference extraction:',len(examples)+5,'positive/negative item boundaries')


if __name__=='__main__':main()
