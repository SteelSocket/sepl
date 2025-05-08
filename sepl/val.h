#ifndef SEPL_VALUE
#define SEPL_VALUE

#include "def.h"
#include "err.h"

typedef struct SeplValue SeplValue;

typedef enum {
    SEPL_VAL_NONE,
    SEPL_VAL_SCOPE,
    SEPL_VAL_NUM,
    SEPL_VAL_FUNC,
    SEPL_VAL_CFUNC,
    SEPL_VAL_REF,
    SEPL_VAL_OBJ
} SeplValueType;

typedef struct {
    SeplValue *values;
    sepl_size size;
} SeplArgs;

typedef SeplValue (*sepl_c_func)(SeplArgs, SeplError *);
typedef void (*sepl_free_func)(SeplValue);

struct SeplValue {
    sepl_size type;
    union {
        /* SEPL_VAL_SCOPE, SEPL_VAL_FUNC */
        sepl_size pos;
        /* SEPL_VAL_NUM */
        double num;
        /* SEPL_VAL_CFUNC */
        sepl_c_func cfunc;
        /* SEPL_VAL_REF, SEPL_VAL_OBJ */
        void *obj;
    } as;
};

extern const SeplValue SEPL_NONE;

#define sepl_val_isnone(val) (val.type == SEPL_VAL_NONE)
#define sepl_val_isscp(val) (val.type == SEPL_VAL_SCOPE)
#define sepl_val_isnum(val) (val.type == SEPL_VAL_NUM)
#define sepl_val_isfun(val) (val.type == SEPL_VAL_FUNC)
#define sepl_val_iscfun(val) (val.type == SEPL_VAL_CFUNC)
#define sepl_val_isref(val) (val.type == SEPL_VAL_REF)
#define sepl_val_isobj(val) (val.type >= SEPL_VAL_OBJ)

SEPL_LIB SeplValue sepl_val_asref(void *v);
SEPL_LIB SeplValue sepl_val_scope(sepl_size pos);
SEPL_LIB SeplValue sepl_val_number(double vnum);
SEPL_LIB SeplValue sepl_val_func(sepl_size pos);
SEPL_LIB SeplValue sepl_val_cfunc(sepl_c_func cfunc);
SEPL_LIB SeplValue sepl_val_object(void *vobj);
SEPL_LIB SeplValue sepl_val_type(void *vobj, sepl_size custom_id);

#endif
