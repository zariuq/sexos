#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "silk_shell_diagnostics_a.h"
static SilkShellProofsAState state;
static SilkShellChromeState chrome;
static ShellTextSinkContext ctx={&state,&chrome};
static uint64_t status, fail_at, call_count;
static const ShellDiagnosticLabel labels[]={{{"begin"},5},{{"move"},4},{{"release"},7},{{"shell"},5},{{""},0}};
void sexos_fail(void){exit(88);}
void sexos_line(const char *format,...){va_list ap;va_start(ap,format);vprintf(format,ap);va_end(ap);putchar('\n');}
SexosReply sexos_pdx_call(uint64_t slot,uint64_t op,uint64_t a,uint64_t b,uint64_t c)
{
    printf("call %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 " %" PRIu64 "\n",slot,op,a,b,c);
    uint64_t result=(fail_at==UINT64_MAX || call_count==fail_at)?status:0;
    ++call_count;return (SexosReply){result,73};
}
static void snapshot(void)
{
    uint64_t flags=/* OBSERVE_FIELDS */;
    printf("S %zu %u %" PRIu64 " %u ",state.SILK_TEXT_LEN,(unsigned)state.SILK_TEXT_PROOF_DONE_EMITTED,flags,state.SHELL_INTERACTION_CONTRACT_STAGE_MASK);
    for(size_t i=0;i<SILK_TEXT_SINK_CAP;++i)printf("%02x",state.SILK_TEXT_BUF[i]);
    puts("");
}
int main(void)
{
    state.silk_text_sink_key__SILK_KEY_ROUTE_BUDGET=32;
    char line[512];
    while(fgets(line,sizeof(line),stdin)){
        uint64_t n[8]={0};char *at=line,*end;
        for(size_t i=0;i<8;++i){n[i]=strtoull(at,&end,10);if(at==end)break;at=end;}
        switch(n[0]){
        case 0:{uint32_t budget=state.silk_text_sink_key__SILK_KEY_ROUTE_BUDGET;memset(&state,0,sizeof(state));state.silk_text_sink_key__SILK_KEY_ROUTE_BUDGET=budget;memset(&chrome,0,sizeof(chrome));status=0;fail_at=UINT64_MAX;call_count=0;break;}
        case 1:printf("R %u\n",(unsigned)shell_diagnostics_silk_text_sink_key(&ctx,n[1],n[2]));break;
        case 2:chrome.FOCUSED_SURFACE_ID=n[1];chrome.SURFACE_100_ALIVE=n[2]!=0;break;
        case 3:state.SILK_TEXT_LEN=n[1];for(size_t i=0;i<SILK_TEXT_SINK_CAP;++i)state.SILK_TEXT_BUF[i]=(uint8_t)(i*17+31);break;
        case 4:printf("R %u\n",(unsigned)shell_diagnostics_silk_text_sink_redraw(&ctx));break;
        case 5:{ShellTextBytes text={.len=n[1]};for(size_t i=0;i<SHELL_TEXT_CAPACITY;++i)text.bytes[i]=(uint8_t)(i*37+n[4]);ShellDiagnosticDraw r=shell_diagnostics_shell_draw_text(n[3],&text,n[2]);printf("R %zu %u\n",r.attempted,(unsigned)r.admitted);break;}
        case 6:printf("R %u\n",(unsigned)shell_diagnostics_send_silkbar_phase2_update(n[1],n[2],n[3]));break;
        case 7:if(n[1]>=sizeof(labels)/sizeof(labels[0]))return 3;shell_diagnostics_bell_send_app_event(&labels[n[1]],n[2]);break;
        case 8:status=n[1];fail_at=n[2];call_count=0;break;
        case 9:switch(n[1]){
            case 0:shell_diagnostics_input_negative_begin_once(&state);break;
            case 1:shell_diagnostics_input_negative_mark_done(&state);break;
            case 2:shell_diagnostics_surface_input_lifetime_begin_once(&state);break;
            case 3:shell_diagnostics_surface_input_lifetime_mark_focus_live(&state,n[2],n[3],n[5]);break;
            case 4:shell_diagnostics_surface_input_lifetime_mark_key_route(&state,n[2],n[3],n[4],n[5]);break;
            case 5:shell_diagnostics_surface_input_lifetime_mark_click_target(&state,n[2],n[3],n[4],n[5]);break;
            case 6:if(n[6]>=sizeof(labels)/sizeof(labels[0]))return 3;shell_diagnostics_surface_input_lifetime_mark_drag_target(&state,n[2],n[3],&labels[n[6]],n[5]);break;
            case 7:shell_diagnostics_surface_input_lifetime_mark_dead_clear(&state,n[2]);break;
            case 8:shell_diagnostics_surface_input_lifetime_try_done(&state);break;
            case 9:shell_diagnostics_shell_interaction_contract_begin_once(&state);break;
            case 10:shell_diagnostics_click_drag_proof_begin_click_once(&state);shell_diagnostics_click_drag_proof_begin_drag_once(&state);break;
            case 11:shell_diagnostics_shell_interaction_contract_mark_stage(&state,n[2]);break;
            case 12:shell_diagnostics_shell_interaction_contract_try_done(&state);break;
            default:return 3;}break;
        case 10:/* SET_FIELDS */break;
        case 11:{uint8_t ch=0;bool found=shell_diagnostics_sink_scancode_to_char(n[1],&ch);printf("R %u %u %u %u\n",(unsigned)shell_diagnostics_is_spindle_text_key(n[1]),(unsigned)shell_diagnostics_is_quil_text_key(n[1]),(unsigned)found,(unsigned)ch);break;}
        case 12:{SilkShellProofsAState other={0};SilkShellChromeState other_chrome={.FOCUSED_SURFACE_ID=100,.SURFACE_100_ALIVE=true};
            ShellTextSinkContext separate={&other,&other_chrome};
            bool accepted=shell_diagnostics_silk_text_sink_key(&separate,16,1);
            printf("R %u %zu %zu %u %u\n",(unsigned)accepted,state.SILK_TEXT_LEN,other.SILK_TEXT_LEN,(unsigned)state.SILK_TEXT_PROOF_DONE_EMITTED,(unsigned)other.SILK_TEXT_PROOF_DONE_EMITTED);break;}
        default:return 3;
        }
        snapshot();
    }
    return 0;
}
