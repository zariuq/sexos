
#ifndef KALEIDOSCOPE_SCRIPT_CORE_H
#define KALEIDOSCOPE_SCRIPT_CORE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "kaleidoscope_script_lexer.h"
#include "kaleidoscope_regex.h"
#define KALEIDOSCOPE_SCRIPT_MAX_OUTPUT ((size_t)4096)
#define KALEIDOSCOPE_SCRIPT_ARENA_STORAGE ((size_t)24576)
#ifndef KALEIDOSCOPE_SCRIPT_ARENA_LIMIT
#define KALEIDOSCOPE_SCRIPT_ARENA_LIMIT ((size_t)12288)
#endif
#define KALEIDOSCOPE_SCRIPT_VARS_STORAGE ((size_t)48)
#ifndef KALEIDOSCOPE_SCRIPT_VARS_LIMIT
#define KALEIDOSCOPE_SCRIPT_VARS_LIMIT KALEIDOSCOPE_SCRIPT_VARS_STORAGE
#endif
#define KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE ((size_t)32)
#ifndef KALEIDOSCOPE_SCRIPT_FUNCS_LIMIT
#define KALEIDOSCOPE_SCRIPT_FUNCS_LIMIT KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE
#endif
#define KALEIDOSCOPE_SCRIPT_ELEMS_STORAGE ((size_t)1024)
#ifndef KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT
#define KALEIDOSCOPE_SCRIPT_ELEMS_LIMIT ((size_t)512)
#endif
#define KALEIDOSCOPE_SCRIPT_ARRAYS ((size_t)128)
#define KALEIDOSCOPE_SCRIPT_CLOSURES ((size_t)64)
#define KALEIDOSCOPE_SCRIPT_ENVS ((size_t)16)
#define KALEIDOSCOPE_SCRIPT_OBJECTS ((size_t)64)
#define KALEIDOSCOPE_SCRIPT_PROPS_STORAGE ((size_t)512)
#ifndef KALEIDOSCOPE_SCRIPT_PROPS_LIMIT
#define KALEIDOSCOPE_SCRIPT_PROPS_LIMIT ((size_t)256)
#endif
#define KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE ((size_t)6)
#ifndef KALEIDOSCOPE_SCRIPT_PARAMS_LIMIT
#define KALEIDOSCOPE_SCRIPT_PARAMS_LIMIT KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE
#endif
#define KALEIDOSCOPE_SCRIPT_NAME ((size_t)16)
#define KALEIDOSCOPE_SCRIPT_REPLACEMENTS ((size_t)256)
/* Function values past the declared functions and closures: `bind` results from BOUND,
   Promise resolvers from RESOLVERS, String/Number/Boolean from BUILTIN_FUNCS. */
#define KALEIDOSCOPE_SCRIPT_BOUND ((uint8_t)100)
#define KALEIDOSCOPE_SCRIPT_BOUNDS ((size_t)64)
#define KALEIDOSCOPE_SCRIPT_RESOLVERS ((uint8_t)200)
#define KALEIDOSCOPE_SCRIPT_BUILTIN_FUNCS ((uint8_t)250)
#define KALEIDOSCOPE_SCRIPT_PROTOS ((size_t)32)
#define KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK ((size_t)1024)
#define KALEIDOSCOPE_SCRIPT_MAX_DEPTH ((uint8_t)64)
#ifndef KALEIDOSCOPE_SCRIPT_DEPTH_LIMIT
#define KALEIDOSCOPE_SCRIPT_DEPTH_LIMIT KALEIDOSCOPE_SCRIPT_MAX_DEPTH
#endif
enum {
    KALEIDOSCOPE_SCRIPT_VALUE_NUMBER = 1,
    KALEIDOSCOPE_SCRIPT_VALUE_STRING = 2,
    KALEIDOSCOPE_SCRIPT_VALUE_BOOLEAN = 3,
    KALEIDOSCOPE_SCRIPT_VALUE_UNDEFINED = 4,
    KALEIDOSCOPE_SCRIPT_VALUE_ARRAY = 5,
    KALEIDOSCOPE_SCRIPT_VALUE_OBJECT = 6,
    KALEIDOSCOPE_SCRIPT_VALUE_NULL = 7,
    KALEIDOSCOPE_SCRIPT_VALUE_FUNCTION = 8,
    KALEIDOSCOPE_SCRIPT_VALUE_REGEX = 9
};

typedef struct KaleidoscopeScriptValue {
    uint8_t kind, index;
    bool boolean;
    uint16_t start, len;
    double number;
} KaleidoscopeScriptValue;

typedef struct KaleidoscopeScriptVariable {
    uint8_t name[KALEIDOSCOPE_SCRIPT_NAME];
    uint8_t len, kind;
    KaleidoscopeScriptValue value;
} KaleidoscopeScriptVariable;

typedef struct KaleidoscopeScriptPosition {
    KaleidoscopeScriptLexer lexer;
    KaleidoscopeScriptToken token;
} KaleidoscopeScriptPosition;

typedef struct KaleidoscopeScriptFunction {
    uint8_t name[KALEIDOSCOPE_SCRIPT_NAME];
    uint8_t len;
    KaleidoscopeScriptPosition position;
} KaleidoscopeScriptFunction;

typedef struct KaleidoscopeScriptClosure {
    uint8_t function, environment;
} KaleidoscopeScriptClosure;

typedef struct KaleidoscopeScriptArray {
    uint16_t start, len, capacity;
} KaleidoscopeScriptArray;

typedef struct KaleidoscopeScriptProperty {
    uint16_t start, len;
    KaleidoscopeScriptValue value;
} KaleidoscopeScriptProperty;

/* A regular expression literal and its `lastIndex` (for `exec` with `g`). */
typedef struct KaleidoscopeScriptRegex {
    uint16_t pattern_start, pattern_len, flags_start, flags_len;
    size_t last_index;
} KaleidoscopeScriptRegex;

typedef struct KaleidoscopeScriptClass {
    uint16_t name_start, name_len, method_start, method_count, static_start, static_count,
        accessor_start, accessor_count, field_start, field_count;
    bool constructor_present, parent_present;
    uint8_t constructor, parent, static_object;
} KaleidoscopeScriptClass;

/* A class field (`x = expr` / `x;`, `static x = expr`): its name in the source and where its
   initializer starts. */
typedef struct KaleidoscopeScriptClassField {
    uint16_t name_start, name_len;
    bool init_present, is_static;
    KaleidoscopeScriptPosition init;
} KaleidoscopeScriptClassField;

typedef struct KaleidoscopeScriptClassMethod {
    uint16_t name_start, name_len;
    uint8_t function;
} KaleidoscopeScriptClassMethod;

typedef struct KaleidoscopeScriptClassAccessor {
    uint16_t name_start, name_len;
    uint8_t function;
    bool setter;
} KaleidoscopeScriptClassAccessor;

/* What a settled promise does next: call `fulfilled`/`rejected` (absent: pass the state on) and
   settle `child` with the outcome. `finally` handlers see no value and pass the original outcome
   on; a `slot` feeds a `Promise.all` (gather, index). */
typedef struct KaleidoscopeScriptReaction {
    bool fulfilled_present, rejected_present, child_present, finally, slot_present;
    uint8_t fulfilled, rejected, child, gather;
    uint16_t slot;
} KaleidoscopeScriptReaction;

/* A promise: its object, state (0 pending, 1 fulfilled, 2 rejected) and value. */
typedef struct KaleidoscopeScriptPromise {
    uint8_t object, state;
    KaleidoscopeScriptValue value;
} KaleidoscopeScriptPromise;

/* A settled reaction to run after the script (a microtask). */
typedef struct KaleidoscopeScriptJob {
    KaleidoscopeScriptReaction reaction;
    uint8_t state;
    KaleidoscopeScriptValue value;
} KaleidoscopeScriptJob;

/* `Promise.all`: the result promise, the results array and the items still pending. */
typedef struct KaleidoscopeScriptGather {
    uint8_t promise, results;
    size_t pending;
} KaleidoscopeScriptGather;

/* A Set or Map: its object and the arrays of its keys and values (a Set's values array is its
   keys array). */
typedef struct KaleidoscopeScriptCollection {
    uint8_t object, keys, values;
} KaleidoscopeScriptCollection;

/* `f.bind(this, ...)`: the function, `this` and the leading arguments. */
typedef struct KaleidoscopeScriptBound {
    uint8_t function, count;
    KaleidoscopeScriptValue this_value;
    KaleidoscopeScriptValue args[KALEIDOSCOPE_SCRIPT_PARAMS_STORAGE];
} KaleidoscopeScriptBound;

/* An accessor property: its name is a range of the script source or of the arena. */
typedef struct KaleidoscopeScriptAccessor {
    uint16_t name_start, name_len;
    uint8_t object, getter, setter;
    bool getter_present, setter_present, name_in_source;
} KaleidoscopeScriptAccessor;

typedef struct KaleidoscopeScriptWorkspace {
    uint8_t arena[KALEIDOSCOPE_SCRIPT_ARENA_STORAGE];
    size_t arena_len;
    KaleidoscopeScriptVariable vars[KALEIDOSCOPE_SCRIPT_VARS_STORAGE];
    size_t block_base, nvars;
    uint8_t output[KALEIDOSCOPE_SCRIPT_MAX_OUTPUT];
    size_t output_len;
    uint8_t depth;
    KaleidoscopeScriptLexer lexer;
    uint32_t steps;
    KaleidoscopeScriptFunction funcs[KALEIDOSCOPE_SCRIPT_FUNCS_STORAGE];
    size_t nfuncs, frame_base;
    KaleidoscopeScriptClosure closures[KALEIDOSCOPE_SCRIPT_CLOSURES];
    size_t closure_count;
    KaleidoscopeScriptVariable env_vars[KALEIDOSCOPE_SCRIPT_ENVS][KALEIDOSCOPE_SCRIPT_VARS_STORAGE];
    size_t env_counts[KALEIDOSCOPE_SCRIPT_ENVS], env_count;
    bool skim, live, ret_present, jump_present, jump_break, jump_label_present, label_next_present,
        this_next_present, thrown_present, func_now_present;
    uint8_t loops, labels, func_now;
    uint16_t jump_label_start, jump_label_len, label_next_start, label_next_len;
    KaleidoscopeScriptValue ret, this_value, this_next, thrown;
    KaleidoscopeScriptValue elems[KALEIDOSCOPE_SCRIPT_ELEMS_STORAGE];
    size_t elem_count;
    KaleidoscopeScriptArray arrays[KALEIDOSCOPE_SCRIPT_ARRAYS];
    size_t narrays;
    KaleidoscopeScriptProperty props[KALEIDOSCOPE_SCRIPT_PROPS_STORAGE];
    size_t prop_count;
    KaleidoscopeScriptArray objects[KALEIDOSCOPE_SCRIPT_OBJECTS];
    /* What made each object: 0 a literal, 1+k class k, 256+f function f as a constructor,
       64 an Error, 65 a Set, 66 a Map, 67 a Promise. */
    uint16_t made_by[KALEIDOSCOPE_SCRIPT_OBJECTS];
    size_t nobjects;
    KaleidoscopeScriptRegex regexes[16];
    size_t regex_count;
    KaleidoscopeRegex regex_work;
    KaleidoscopeRegexText regex_text;
    KaleidoscopeScriptClass classes[16];
    size_t class_count;
    KaleidoscopeScriptClassMethod class_methods[64], class_static_methods[64];
    KaleidoscopeScriptClassAccessor class_accessors[64];
    KaleidoscopeScriptClassField class_fields[64];
    size_t class_method_count, class_static_method_count, class_accessor_count, class_field_count;
    KaleidoscopeScriptAccessor accessors[64];
    size_t accessor_count;
    /* Texts returned by `replace` callbacks, kept until the result is assembled (a stack:
       callbacks may replace too). */
    uint16_t replacement_start[KALEIDOSCOPE_SCRIPT_REPLACEMENTS],
        replacement_len[KALEIDOSCOPE_SCRIPT_REPLACEMENTS];
    size_t replacement_count;
    /* `F.prototype` objects, by function. */
    uint8_t proto_function[KALEIDOSCOPE_SCRIPT_PROTOS], proto_object[KALEIDOSCOPE_SCRIPT_PROTOS];
    size_t proto_count;
    KaleidoscopeScriptBound bounds[KALEIDOSCOPE_SCRIPT_BOUNDS];
    size_t bound_count;
    /* The arguments of the calls running now (for `arguments`); the innermost call's are at
       args_now_start. */
    KaleidoscopeScriptValue argument_stack[KALEIDOSCOPE_SCRIPT_ARGUMENT_STACK];
    size_t argument_top, args_now_start, args_now_count;
    /* Objects passed to `Object.freeze` (assignments to them are ignored). */
    uint8_t frozen[KALEIDOSCOPE_SCRIPT_OBJECTS];
    size_t frozen_count;
    KaleidoscopeScriptCollection collections[32];
    size_t collection_count;
    KaleidoscopeScriptPromise promises[64];
    size_t promise_count;
    /* Reactions waiting for a pending promise, in the order they were added. */
    KaleidoscopeScriptReaction waiting[256];
    uint8_t waiting_promise[256];
    size_t waiting_count;
    KaleidoscopeScriptJob jobs[256];
    size_t job_count;
    /* `resolve`/`reject` handed to executors: Func(RESOLVERS + k). */
    uint8_t resolver_promise[50];
    bool resolver_rejects[50];
    size_t resolver_count;
    KaleidoscopeScriptGather gathers[32];
    size_t gather_count;
} KaleidoscopeScriptWorkspace;

typedef struct KaleidoscopeScriptResult {
    uint8_t error;
    size_t len;
    uint8_t bytes[KALEIDOSCOPE_SCRIPT_MAX_OUTPUT];
} KaleidoscopeScriptResult;

uint8_t kaleidoscope_script_run(const KaleidoscopeScriptSource *source,
                                KaleidoscopeScriptWorkspace *workspace,
                                KaleidoscopeScriptResult *result);
/* The lowest stack address a script may reach (0: no floor). The Browser sets it from its
   mapped stack, so deep nesting is refused (Limit) before the stack runs out, whatever the
   frame sizes. */
void kaleidoscope_script_set_stack_floor(uintptr_t address);
/* Scripts refused at the stack floor so far. */
size_t kaleidoscope_script_floor_hits(void);
#endif
