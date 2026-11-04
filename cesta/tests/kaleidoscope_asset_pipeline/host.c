#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "kaleidoscope_asset_pipeline.h"
#include "kaleidoscope_asset_view.h"
#include "kaleidoscope_bitmap.h"
#include "kaleidoscope_presentation.h"
#include "fixtures.h"

static void load(KaleidoscopeQoiSource *source,const uint8_t *bytes,size_t length)
{memset(source,0,sizeof(*source));memcpy(source->bytes,bytes,length);source->len=length;}
int main(void)
{
 KaleidoscopeAssetImage image;KaleidoscopeQoiSource source;kaleidoscope_asset_image_init(&image);load(&source,qoi_fixture,sizeof qoi_fixture);assert(kaleidoscope_asset_image_begin(&image,&source,7));bool ready=false;while(kaleidoscope_asset_image_pending(&image)){assert(kaleidoscope_asset_image_advance(&image,&ready));}assert(ready&&image.ready&&image.width==2&&image.height==1&&kaleidoscope_asset_image_pixel_count(&image)==2);assert(image.pixels.values[0]==UINT32_C(0xff010203)&&image.pixels.values[1]==UINT32_C(0x80040506)&&image.source.len==0);
 KaleidoscopeBitmapGeometry geometry={0};assert(kaleidoscope_asset_geometry(512,219,2,1,KALEIDOSCOPE_ASSET_FIT,&geometry));KaleidoscopePresentation presentation;kaleidoscope_presentation_init(&presentation);assert(kaleidoscope_presentation_text_applied(&presentation,1,1,1));KaleidoscopeBitmapOutbox outbox;kaleidoscope_bitmap_init(&outbox,(KaleidoscopeBitmapIdentity){15,77,300,0},0);KaleidoscopeBitmapQueueResult queued=kaleidoscope_bitmap_queue(&outbox,&image.pixels,2,geometry,(KaleidoscopeBitmapMetadata){1,7});assert(!queued.error&&kaleidoscope_presentation_expect(&presentation,geometry,1,7,queued.sequence,kaleidoscope_bitmap_hash(&image.pixels,2)));
 size_t operations=0;for(;;){KaleidoscopeBitmapPollResult poll=kaleidoscope_bitmap_prepare(&outbox,&image.pixels,2);if(!poll.operation_present){assert(poll.state==KALEIDOSCOPE_BITMAP_POLL_PENDING);break;}assert(poll.operation.surface==300);operations++;poll=kaleidoscope_bitmap_resolve(&outbox,(KaleidoscopeBitmapAdmission){KALEIDOSCOPE_BITMAP_ADMIT_ACCEPTED,0});assert(poll.state==KALEIDOSCOPE_BITMAP_POLL_PENDING);}assert(operations==3);KaleidoscopeBitmapReceipt receipt={0};uint64_t word=UINT64_C(0x57)|(UINT64_C(300)<<8)|((uint64_t)queued.sequence<<24);assert(kaleidoscope_bitmap_receive(&outbox,1,word,&receipt));assert(kaleidoscope_presentation_applied(&presentation,&receipt,1,7,image.wire_hash?receipt.hash:0)&&kaleidoscope_presentation_can_queue(&presentation,1)==false);
 kaleidoscope_asset_image_clear(&image);load(&source,png_fixture,sizeof png_fixture);assert(kaleidoscope_asset_image_begin(&image,&source,8));assert(kaleidoscope_asset_image_pending(&image)&&!image.ready);ready=false;assert(kaleidoscope_asset_image_advance(&image,&ready));assert(ready&&image.ready&&image.format==KALEIDOSCOPE_ASSET_PNG&&image.pixels.values[0]==UINT32_C(0xff010203)&&image.pixels.values[1]==UINT32_C(0x80040506));kaleidoscope_asset_image_clear(&image);assert(!image.ready&&!image.wire_bytes&&!image.pixels.values[0]);puts("asset pipeline ok: qoi png geometry begin chunk commit receipt clear");return 0;
}
