#include "silk_shell_surfaces.h"
const CommandDef COMMAND_LIST[11]={
{0,"Open Spindle"},
{1,"Open Quil"},
{2,"Open Linen"},
{3,"Open Atlas"},
{4,"Open Bell"},
{5,"Open Collar"},
{6,"Open Mesh"},
{7,"Restore Minimized"},
{8,"Zoom Toggle"},
{9,"Minimize Focused"},
{10,"Open Browser"},
};
uint8_t shell_surfaces_command_code(size_t index){return index<11?COMMAND_LIST[index].command:255;}
ShellPaletteString shell_surfaces_command_label(size_t index){ShellPaletteString result={0};if(index>=11)return result;const char *name=COMMAND_LIST[index].name;while(result.len<127 && name[result.len]){result.bytes[result.len]=name[result.len];++result.len;}return result;}


