#ifndef KALEIDOSCOPE_SCRIPT_CORE_H
#define KALEIDOSCOPE_SCRIPT_CORE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kaleidoscope_script_lexer.h"
#include "kaleidoscope_regex.h"
#define KALEIDOSCOPE_SCRIPT_MAX_OUTPUT ((size_t)1024)
#define KALEIDOSCOPE_SCRIPT_ARENA_STORAGE ((size_t)8192)
#ifndef KALEIDOSCOPE_SCRIPT_ARENA_LIMIT
#define KALEIDOSCOPE_SCRIPT_ARENA_LIMIT ((size_t)4096)
#endif
#define KALEIDOSCOPE_SCRIPT_VARS_STORAGE ((size_t)32)
#ifndef KALEIDOSCOPE_SCRIPT_VARS_LIMIT
#define KALEIDOSCOPE_SCRIPT_VARS_LIMIT KALEIDOSCOPE_SCRIPT_VARS_STORAGE
#endif
#define KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE ((size_t)16)
#ifndef KALEIDOSCOPE_SCRIPT_FUNCS_LIMIT
#define KALEIDOSCOPE_SCRIPT_FUNCS_LIMIT KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE
#endif
#define KALEIDOSCOPE_SCRIPT_ELEMS_STORAGE ((size_t)512)
#ifndef KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT
#define KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT ((size_t)256)
#endif
#define KALEIDOSCOPE_SCRIPT_ARRAYS ((size_t)32)
#define KALEIDOSCOPE_SCRIPT_CLOSURES ((size_t)32)
#define KALEIDOSCOPE_SCRIPT_ENVS ((size_t)8)
#define KALEIDOSCOPE_SCRIPT_OBJECTS ((size_t)16)
#define KALEIDOSCOPE_SCRIPT_PROPS_STORAGE ((size_t)256)
#ifndef KALEIDOSCOPE_SCRIPT_PROPS_LIMIT
#define KALEIDOSCOPE_SCRIPT_PROPS_LIMIT ((size_t)128)
#endif
#define KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE ((size_t)6)
#ifndef KALEIDOSCOPE_SCRIPT_PARAMS_LIMIT
#define KALEIDOSCOPE_SCRIPT_PARAMS_LIMIT KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE
#endif
#define KALEIDOSCOPE_SCRIPT_NAME ((size_t)16)
#define KALEIDOSCOPE_SCRIPT_MAX_DEPTH ((uint8_t)32)
#ifndef KALEIDOSCOPE_SCRIPT_DEPTH_LIMIT
#define KALEIDOSCOPE_SCRIPT_DEPTH_LIMIT KALEIDOSCOPE_SCRIPT_MAX_DEPTH
#endif
enum {KALEIDOSCOPE_SCRIPT_VALUE_NUMBER=1,KALEIDOSCOPE_SCRIPT_VALUE_STRING=2,KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN=3,KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED=4,KALEIDOSCOPE_SCRIPT_VALUE_ARRAY=5,KALEIDOSCOPE_SCRIPT_VALUE_OBJECT=6,KALEIDOSCOPE_SCRIPT_VALUE_NULL=7,KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION=8,KALEIDOSCOPE_SCRIPT_VALUE_REGEX=9};
typedef struct KaleidoscopeScriptValue {uint8_t kind,index;bool boolean;uint16_t start,len;double number;} KaleidoscopeScriptValue;
typedef struct KaleidoscopeScriptVariable {uint8_t name[KALEIDOSCOPE_SCRIPT_NAME];uint8_t len,kind;KaleidoscopeScriptValue value;} KaleidoscopeScriptVariable;
typedef struct KaleidoscopeScriptPosition {KaleidoscopeScriptLexer lexer;KaleidoscopeScriptToken token;} KaleidoscopeScriptPosition;
typedef struct KaleidoscopeScriptFunction {uint8_t name[KALEIDOSCOPE_SCRIPT_NAME];uint8_t len;KaleidoscopeScriptPosition position;} KaleidoscopeScriptFunction;
typedef struct KaleidoscopeScriptClosure {uint8_t function,environment;} KaleidoscopeScriptClosure;
typedef struct KaleidoscopeScriptArray {uint16_t start,len,capacity;} KaleidoscopeScriptArray;
typedef struct KaleidoscopeScriptProperty {uint16_t start,len;KaleidoscopeScriptValue value;} KaleidoscopeScriptProperty;
typedef struct KaleidoscopeScriptRegex {uint16_t pattern_start,pattern_len,flags_start,flags_len;} KaleidoscopeScriptRegex;
typedef struct KaleidoscopeScriptClass {uint16_t name_start,name_len,method_start,method_count,static_start,static_count,accessor_start,accessor_count;bool constructor_present,parent_present;uint8_t constructor,parent;} KaleidoscopeScriptClass;
typedef struct KaleidoscopeScriptClassMethod {uint16_t name_start,name_len;uint8_t function;} KaleidoscopeScriptClassMethod;
typedef struct KaleidoscopeScriptClassAccessor {uint16_t name_start,name_len;uint8_t function;bool setter;} KaleidoscopeScriptClassAccessor;
typedef struct KaleidoscopeScriptAccessor {uint16_t name_start,name_len;uint8_t object,getter,setter;bool getter_present,setter_present;} KaleidoscopeScriptAccessor;
typedef struct KaleidoscopeScriptWorkspace {
 uint8_t arena[KALEIDOSCOPE_SCRIPT_ARENA_STORAGE];size_t arena_len;
 KaleidoscopeScriptVariable vars[KALEIDOSCOPE_SCRIPT_VARS_STORAGE];size_t block_base,nvars;
 uint8_t output[KALEIDOSCOPE_SCRIPT_MAX_OUTPUT];size_t output_len;uint8_t depth;
 KaleidoscopeScriptLexer lexer;
 uint32_t steps;KaleidoscopeScriptFunction funcs[KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE];size_t nfuncs,frame_base;
 KaleidoscopeScriptClosure closures[KALEIDOSCOPE_SCRIPT_CLOSURES];size_t closure_count;
 KaleidoscopeScriptVariable env_vars[KALEIDOSCOPE_SCRIPT_ENVS][KALEIDOSCOPE_SCRIPT_VARS_STORAGE];size_t env_counts[KALEIDOSCOPE_SCRIPT_ENVS],env_count;
 bool skim,live,ret_present,jump_present,jump_break,jump_label_present,label_next_present,this_next_present,thrown_present,func_now_present;uint8_t loops,labels,func_now;uint16_t jump_label_start,jump_label_len,label_next_start,label_next_len;KaleidoscopeScriptValue ret,this_value,this_next,thrown;
 KaleidoscopeScriptValue elems[KALEIDOSCOPE_SCRIPT_ELEMS_STORAGE];size_t elem_count;
 KaleidoscopeScriptArray arrays[KALEIDOSCOPE_SCRIPT_ARRAYS];size_t narrays;
 KaleidoscopeScriptProperty props[KALEIDOSCOPE_SCRIPT_PROPS_STORAGE];size_t prop_count;
 KaleidoscopeScriptArray objects[KALEIDOSCOPE_SCRIPT_OBJECTS];uint8_t made_by[KALEIDOSCOPE_SCRIPT_OBJECTS];size_t nobjects;
 KaleidoscopeScriptRegex regexes[8];size_t regex_count;KaleidoscopeRegex regex_work;KaleidoscopeRegexText regex_text;
 KaleidoscopeScriptClass classes[8];size_t class_count;KaleidoscopeScriptClassMethod class_methods[16],class_static_methods[16];KaleidoscopeScriptClassAccessor class_accessors[16];size_t class_method_count,class_static_method_count,class_accessor_count;
 KaleidoscopeScriptAccessor accessors[32];size_t accessor_count;
} KaleidoscopeScriptWorkspace;
typedef struct KaleidoscopeScriptResult {uint8_t error;size_t len;uint8_t bytes[KALEIDOSCOPE_SCRIPT_MAX_OUTPUT];} KaleidoscopeScriptResult;
uint8_t kaleidoscope_script_run(const KaleidoscopeScriptSource *source,KaleidoscopeScriptWorkspace *workspace,KaleidoscopeScriptResult *result);
#endif


