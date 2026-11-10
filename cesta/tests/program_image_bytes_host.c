#define _GNU_SOURCE
#include "program_image_bytes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef uint32_t (*ReadU32)(cesta_read_bytes, size_t);

void program_image_bounds_failure(void) { _Exit(73); }

static int exercise(ReadU32 read_word) {
    uint8_t data[260], before[260];
    uint32_t seed=UINT32_C(0x1234567);
    for(size_t i=0;i<sizeof data;++i) {
        seed=seed*UINT32_C(1664525)+UINT32_C(1013904223);
        data[i]=(uint8_t)(seed>>24);
    }
    memcpy(before,data,sizeof data);
    size_t count=0;
    for(size_t length=4;length<=128;++length) {
        for(size_t offset=0;offset+4<=length;++offset) {
            const uint8_t *p=data+1+offset;
            uint32_t expected=(uint32_t)p[0] | ((uint32_t)p[1]<<8) |
                ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24);
            if(read_word((cesta_read_bytes){data+1,length},offset)!=expected) return 1;
            ++count;
        }
    }
    if(memcmp(before,data,sizeof data)) return 2;
    printf("u32 decoder: %zu offsets/lengths, unaligned input and source preservation passed\n",count);
    return 0;
}

#ifdef IMAGE_BYTES_NATIVE
int main(int argc, char **argv) {
    uint8_t bytes[8]={0};
    if(argc>1) {
        unsigned mode=(unsigned)strtoul(argv[1],NULL,10);
        switch(mode) {
        case 0: return (int)program_image_bytes_u32_at((cesta_read_bytes){NULL,0},0);
        case 1: return (int)program_image_bytes_u32_at((cesta_read_bytes){bytes,3},0);
        case 2: return (int)program_image_bytes_u32_at((cesta_read_bytes){bytes,8},5);
        case 3: return (int)program_image_bytes_u32_at((cesta_read_bytes){bytes,8},SIZE_MAX);
        case 4: return (int)program_image_bytes_u32_at((cesta_read_bytes){bytes,SIZE_MAX},0);
        default: return 99;
        }
    }
    return exercise(program_image_bytes_u32_at);
}
#else
/* Only valid inputs execute extracted machine bytes. Sanitizers instrument
   the separate generated-C route, not this raw instruction mapping. */
int main(int argc, char **argv) {
    if(argc!=2)return 10;
    FILE *input=fopen(argv[1],"rb");if(!input)return 11;
    if(fseek(input,0,SEEK_END))return 12;
    long size=ftell(input);if(size<=0 || fseek(input,0,SEEK_SET))return 13;
    long page=sysconf(_SC_PAGESIZE);if(page<=0)return 14;
    size_t extent=((size_t)size+(size_t)page-1)/(size_t)page*(size_t)page;
    void *code=mmap(NULL,extent,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if(code==MAP_FAILED)return 15;
    if(fread(code,1,(size_t)size,input)!=(size_t)size || fclose(input))return 16;
    if(mprotect(code,extent,PROT_READ|PROT_EXEC))return 17;
    ReadU32 function;_Static_assert(sizeof function==sizeof code,"x86 function pointer ABI");
    memcpy(&function,&code,sizeof function);
    int result=exercise(function);
    if(munmap(code,extent))return 18;
    return result;
}
#endif
