#include "val.h"

const SeplValue SEPL_NONE = {0};

SEPL_LIB SeplValue sepl_val_asref(void *v) {
    SeplValue r;
    r.type = SEPL_VAL_REF;
    r.as.obj = v;
    return r;
}

SEPL_LIB SeplValue sepl_val_scope(sepl_size pos) {
    SeplValue r;
    r.type = SEPL_VAL_SCOPE;
    r.as.pos = pos;
    return r;
}

SEPL_LIB SeplValue sepl_val_number(double vnum) {
    SeplValue r;
    r.type = SEPL_VAL_NUM;
    r.as.num = vnum;
    return r;
}

SEPL_LIB SeplValue sepl_val_str(char *str) {
    SeplValue r;
    r.type = SEPL_VAL_STR;
    r.as.obj = str;
    return r;
}

SEPL_LIB SeplValue sepl_val_func(sepl_size pos) {
    SeplValue r;
    r.type = SEPL_VAL_FUNC;
    r.as.pos = pos;
    return r;
}

SEPL_LIB SeplValue sepl_val_cfunc(sepl_c_func cfunc) {
    SeplValue r;
    r.type = SEPL_VAL_CFUNC;
    r.as.cfunc = cfunc;
    return r;
}

SEPL_LIB SeplValue sepl_val_object(void *vobj) {
    return sepl_val_type(vobj, 0);
}

SEPL_LIB SeplValue sepl_val_type(void *vobj, sepl_size custom_id) {
    SeplValue r;
    r.type = SEPL_VAL_OBJ + custom_id;
    r.as.obj = vobj;
    return r;
}
