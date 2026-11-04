#include <stdio.h>
#include <stdlib.h>
#include "kaleidoscope_asset_view.h"
#include "cases.h"
int main(int argc,char **argv)
{
 if(argc!=2){return 2;}size_t n=(size_t)strtoull(argv[1],NULL,10);if(n>=sizeof(cases)/sizeof(cases[0]))return 3;Case c=cases[n];KaleidoscopeBitmapGeometry geometry={0};bool present=kaleidoscope_asset_geometry(c.w,c.h,c.iw,c.ih,c.mode,&geometry);KaleidoscopeAssetFrame frame;const uint8_t title[]="PNG";kaleidoscope_asset_render(c.w,c.h,c.status,c.mode,c.iw,c.ih,c.custom?title:NULL,c.custom?3:0,&frame);printf("G %u %u,%u,%u,%u,%u C %zu F %zu,%zu,%zu ",(unsigned)present,geometry.x,geometry.y,geometry.width,geometry.height,geometry.scale,kaleidoscope_asset_chrome_rows(c.w),frame.len,frame.rows,frame.cols);for(size_t i=0;i<frame.len;i++)printf("%02x",frame.bytes[i]);putchar('\n');return 0;
}
