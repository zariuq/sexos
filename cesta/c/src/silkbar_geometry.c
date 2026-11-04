#if defined(__aarch64__) && defined(__ARM_PCS_AAPCS64)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8 && __SIZEOF_LONG_DOUBLE__ == 16 && __LDBL_MANT_DIG__ == 113 && (char)-1 > 0, "ownership ABI profile requires AArch64 AAPCS64");
#else
#if defined(__SIZEOF_POINTER__) && defined(__SIZEOF_LONG__)
_Static_assert(__SIZEOF_POINTER__ == 8 && __SIZEOF_LONG__ == 8, "ownership ABI profile requires x86_64 SysV");
#endif
_Static_assert((char)-1 < 0, "ownership ABI profile requires signed x86 plain char");
#endif
_Static_assert((unsigned char)256 == 0 && sizeof(void *) == 8 && sizeof(long) == 8 && sizeof(int) == 4 && sizeof(short) == 2 && sizeof(long long) == 8 && sizeof(float) == 4 && sizeof(double) == 8 && sizeof(long double) == 16 && sizeof(enum { __cesta_ownership_abi_enum = 1 }) == 4 && _Alignof(void *) == 8 && _Alignof(long) == 8 && _Alignof(long double) == 16 && sizeof(struct { char a; long b; }) == 16 && sizeof(struct { char a; long double b; }) == 32, "ownership ABI profile requires a supported LP64 target");
#include "silkbar_geometry.h"
#include "silkbar_model.h"
static bool silkbar_geometry_fit(size_t width,SilkBar *out)
{
    SilkBar bar=DEFAULT_SILK_BAR;
    if(CHIP_X3>SIZE_MAX-CLOCK_W)return false;
    size_t edge=CHIP_X3+CLOCK_W;
    if(edge>SIZE_MAX-16)return false;
    edge+=16;
    size_t shift=edge>width?edge-width:0;
    size_t left_end=0;
    for(size_t i=0;i<6;++i){
        size_t end=bar.layout[i].x+bar.layout[i].w;
        if(end>left_end)left_end=end;
    }
    if(CHIP_X0<shift || left_end>SIZE_MAX-16 || CHIP_X0-shift<left_end+16)return false;
    for(size_t i=6;i<LAYOUT_COUNT;++i){
        if(bar.layout[i].x<shift)return false;
        bar.layout[i].x-=shift;
    }
    *out=bar;
    return true;
}
SilkBarDispatch silkbar_geometry_dispatch(size_t width,size_t x,size_t y)
{
    SilkBar bar=DEFAULT_SILK_BAR;
    (void)silkbar_geometry_fit(width,&bar);
    SilkBarAction action=silkbar_hit_test_action(&bar,x,y);
    return (SilkBarDispatch){(uint8_t)action.tag,action.workspace};
}


