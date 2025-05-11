#include "mod.h"
#include "env.h"
#include "err.h"
#include "val.h"

SEPL_LIB SeplModule sepl_mod_new(unsigned char bytes[], sepl_size bsize,
                                 SeplValue values[], sepl_size vsize) {
    SeplModule mod = {0};
    mod.bytes = bytes;
    mod.bsize = bsize;
    mod.values = values;
    mod.vsize = vsize;
    return mod;
}

SEPL_LIB sepl_size sepl_mod_bc(SeplModule *mod, SeplByteCode bc, SeplError *e) {
    if (mod->bpos + 1 > mod->bsize) {
        sepl_err_new(e, SEPL_ERR_BOVERFLOW);
        return 0;
    }
    mod->bytes[mod->bpos] = bc;
    return mod->bpos++;
}

SEPL_LIB sepl_size sepl_mod_bcnum(SeplModule *mod, double n, SeplError *e) {
    if (mod->bpos + sizeof(n) > mod->bsize) {
        sepl_err_new(e, SEPL_ERR_BOVERFLOW);
        return 0;
    }
    *(double *)(mod->bytes + mod->bpos) = n;
    mod->bpos += sizeof(n);
    return mod->bpos - sizeof(n);
}

SEPL_LIB sepl_size sepl_mod_bcsize(SeplModule *mod, sepl_size s, SeplError *e) {
    if (mod->bpos + sizeof(s) > mod->bsize) {
        sepl_err_new(e, SEPL_ERR_BOVERFLOW);
        return 0;
    }
    *(sepl_size *)(mod->bytes + mod->bpos) = s;
    mod->bpos += sizeof(s);
    return mod->bpos - sizeof(s);
}

SEPL_LIB sepl_size sepl_mod_val(SeplModule *mod, SeplValue v, SeplError *e) {
    if (mod->vpos >= mod->vsize) {
        sepl_err_new(e, SEPL_ERR_VOVERFLOW);
        return 0;
    }

    mod->values[mod->vpos] = v;
    return mod->vpos++;
}

SEPL_API void sepl__free(SeplValue _) { (void)_; }

SEPL_LIB void sepl_mod_init(SeplModule *mod, SeplError *e, SeplEnv env) {
    sepl_size i;
    mod->vpos = 0;
    e->code = SEPL_ERR_OK;

    if (env.free == SEPL_NULL) {
        env.free = sepl__free;
    }

    for (i = 0; i < env.predef_len; i++) {
        sepl_mod_val(mod, env.predef[i].value, e);
    }
    for (i = 0; i < mod->esize; i++) {
        sepl_mod_val(mod, SEPL_NONE, e);
    }
}

SEPL_LIB void sepl_mod_cleanup(SeplModule *mod, SeplEnv env) {
    sepl_size i;
    for (i = mod->vpos; i-- > env.predef_len;) {
        SeplValue v = mod->values[i];
        if (sepl_val_isobj(v))
            env.free(v);
    }
}

SEPL_API double sepl__todbl(SeplError *err, SeplValue v) {
    if (sepl_val_isnum(v)) {
        return v.as.num;
    } else if (sepl_val_isobj(v)) {
        return v.as.num;
    } else if (sepl_val_isref(v)) {
        return sepl__todbl(err, *(SeplValue *)v.as.obj);
    }
    sepl_err_new(err, SEPL_ERR_OPER);
    return 0.0;
}

SEPL_LIB SeplValue sepl_mod_step(SeplModule *mod, SeplError *e, SeplEnv env) {
#define sepl__rddbl()           \
    (mod->pc += sizeof(double), \
     *(double *)(mod->bytes + mod->pc - sizeof(double)))
#define sepl__rdsz()               \
    (mod->pc += sizeof(sepl_size), \
     *(sepl_size *)(mod->bytes + mod->pc - sizeof(sepl_size)))

#define sepl__pushv(val) (sepl_mod_val(mod, val, e))
#define sepl__popv()                \
    (mod->vpos > env.predef_len     \
         ? mod->values[--mod->vpos] \
         : (sepl_err_new(e, SEPL_ERR_VUNDERFLOW), SEPL_NONE))
#define sepl__peekv(offset) (mod->values[mod->vpos - offset - 1])
#define sepl__popd()                                 \
    (sepl_val_isobj(sepl__peekv(0))                  \
         ? (env.free(sepl__popv()), sepl__peekv(-1)) \
         : sepl__popv())

#define sepl__unaryop(op)                   \
    do {                                    \
        SeplValue v = sepl__popv();         \
        double d = sepl__todbl(e, v);       \
        if (e->code)                        \
            break;                          \
        sepl__pushv(sepl_val_number(op d)); \
    } while (0)
#define sepl__binaryop(op)                                       \
    do {                                                         \
        SeplValue v2 = sepl__popv(), v1 = sepl__popv();          \
        double d1 = sepl__todbl(e, v1), d2 = sepl__todbl(e, v2); \
        if (e->code)                                             \
            break;                                               \
        sepl__pushv(sepl_val_number(d1 op d2));                  \
    } while (0)

    SeplByteCode bc = mod->bytes[mod->pc++];

    switch (bc) {
        case SEPL_BC_RETURN: {
            SeplValue retv, v;
            e->code = SEPL_ERR_OK;
            if (mod->vpos <= env.predef_len)
                return SEPL_NONE;

            retv = sepl__popv();

            if (sepl_val_isfun(retv)) {
                sepl_err_new(e, SEPL_ERR_FUNC_RET);
                return SEPL_NONE;
            }

            /* Pop all values until scope block end */
            while (1) {
                v = sepl__popv();

                /* Prevent the return ref object from being freed */
                if (sepl_val_isref(retv) &&
                    retv.as.obj == (mod->values + mod->vpos)) {
                    retv = v; /* Deference */
                    continue;
                } else if (sepl_val_isobj(v)) {
                    env.free(v);
                } else if (sepl_val_isscp(v)) {
                    break;
                }
            }

            if (v.as.pos >= mod->bpos) {
                mod->pc = mod->bpos;
                return retv;
            }

            sepl__pushv(retv);
            mod->pc = v.as.pos;
            break;
        }
        case SEPL_BC_JUMP: {
            sepl_size jump = sepl__rdsz();
            mod->pc = jump;
            break;
        }
        case SEPL_BC_JUMPIF: {
            sepl_size jump = sepl__rdsz();
            SeplValue cond = sepl__peekv(0);
            if (!cond.as.num) {
                mod->pc = jump;
            }
            sepl__popd();
            break;
        }

        case SEPL_BC_CALL: {
            sepl_size offset = sepl__rdsz();
            SeplValue v = mod->values[mod->vpos - offset - 1];

            if (sepl_val_iscfun(v)) {
                SeplArgs args;
                SeplValue result;
                args.values = mod->values + mod->vpos - offset;
                args.size = offset;
                result = v.as.cfunc(args, e);

                /* Pop arguments */
                while (offset--) sepl__popd();
                mod->vpos--; /* Pop function variable */
                sepl__pushv(result);
            } else if (sepl_val_isfun(v)) {
                sepl_size param_c;
                mod->values[mod->vpos - offset - 1] = sepl_val_scope(mod->pc);
                mod->pc = v.as.pos;
                param_c = sepl__rdsz();

                if (param_c < offset) {
                    while (param_c++ != offset) sepl__popd();
                } else if (param_c > offset) {
                    while (param_c-- != offset) sepl__pushv(SEPL_NONE);
                }

            } else {
                sepl_err_new(e, SEPL_ERR_FUNC_CALL);
                return SEPL_NONE;
            }
            break;
        }
        case SEPL_BC_POP: {
            sepl__popd();
            break;
        }

        case SEPL_BC_SET: {
            sepl_size i = sepl__rdsz();
            SeplValue peek = sepl__peekv(0);
            sepl_size offset = mod->vpos - i;

            /* Do not set the obj to its own reference */
            if (sepl_val_isref(peek) &&
                (SeplValue *)peek.as.obj == mod->values + offset) {
                sepl__popv();
                break;
            }
            if (sepl_val_isobj(mod->values[offset]))
                env.free(mod->values[offset]);
            mod->values[offset] = sepl__popv();
            break;
        }
        case SEPL_BC_GET: {
            sepl_size i = sepl__rdsz();
            sepl_size offset = mod->vpos - i;
            SeplValue v = mod->values[offset];

            if (sepl_val_isobj(v)) {
                sepl__pushv(sepl_val_asref(&mod->values[offset]));
                break;
            }
            sepl__pushv(v);
            break;
        }

        case SEPL_BC_SET_UP: {
            sepl_size offset = sepl__rdsz();
            SeplValue peek = sepl__peekv(0);

            /* Do not set the obj to its own reference */
            if (sepl_val_isref(peek)) {
                sepl_err_new(e, SEPL_ERR_REFUPV);
                break;
            }
            if (sepl_val_isobj(mod->values[offset]))
                env.free(mod->values[offset]);
            mod->values[offset] = sepl__popv();
            break;
        }
        case SEPL_BC_GET_UP: {
            sepl_size offset = sepl__rdsz();
            SeplValue v = mod->values[offset];

            if (sepl_val_isobj(v)) {
                sepl__pushv(sepl_val_asref(&mod->values[offset]));
                break;
            }
            sepl__pushv(v);
            break;
        }

        case SEPL_BC_NONE: {
            sepl__pushv(SEPL_NONE);
            break;
        }
        case SEPL_BC_CONST: {
            double v = sepl__rddbl();
            sepl__pushv(sepl_val_number(v));
            break;
        }
        case SEPL_BC_STR: {
            sepl_size len = sepl__rdsz();
            sepl__pushv(sepl_val_str((char*)(mod->bytes + mod->pc)));
            mod->pc += len + 1;
            break;
        }
        case SEPL_BC_SCOPE: {
            sepl_size end_pos = sepl__rdsz();
            sepl__pushv(sepl_val_scope(end_pos));
            break;
        }
        case SEPL_BC_FUNC: {
            sepl_size skip_pos = sepl__rdsz();
            sepl__pushv(sepl_val_func(mod->pc));
            mod->pc = skip_pos;
            break;
        }

        case SEPL_BC_NEG: {
            sepl__unaryop(-);
            break;
        }

        case SEPL_BC_ADD: {
            sepl__binaryop(+);
            break;
        }
        case SEPL_BC_SUB: {
            sepl__binaryop(-);
            break;
        }
        case SEPL_BC_MUL: {
            sepl__binaryop(*);
            break;
        }
        case SEPL_BC_DIV: {
            sepl__binaryop(/);
            break;
        }
        case SEPL_BC_NOT: {
            sepl__unaryop(!);
            break;
        }

        case SEPL_BC_LT: {
            sepl__binaryop(<);
            break;
        }
        case SEPL_BC_LTE: {
            sepl__binaryop(<=);
            break;
        }
        case SEPL_BC_GT: {
            sepl__binaryop(>);
            break;
        }
        case SEPL_BC_GTE: {
            sepl__binaryop(>=);
            break;
        }
        case SEPL_BC_EQ: {
            sepl__binaryop(==);
            break;
        }
        case SEPL_BC_NEQ: {
            sepl__binaryop(!=);
            break;
        }

        default: {
            sepl_err_new(e, SEPL_ERR_BC);
            e->info.bc = bc;
            return SEPL_NONE;
        }
    }
    return SEPL_NONE;
}

SEPL_LIB SeplValue sepl_mod_exec(SeplModule *mod, SeplError *e, SeplEnv env) {
    SeplValue retv = SEPL_NONE;

    while (mod->pc < mod->bpos) {
        retv = sepl_mod_step(mod, e, env);
        if (e->code != SEPL_ERR_OK)
            return SEPL_NONE;
    }
    return retv;
}

SEPL_LIB void sepl_mod_initfunc(SeplModule *mod, SeplError *e, SeplValue func,
                                SeplArgs args) {
    sepl_size args_count, i;
    if (!sepl_val_isfun(func)) {
        sepl_err_new(e, SEPL_ERR_FUNC_CALL);
        return;
    }

    mod->pc = func.as.pos;
    args_count = sepl__rdsz();

    sepl_mod_val(mod, sepl_val_scope(mod->bpos), e);
    for (i = 0; i < args.size; i++) {
        sepl_mod_val(mod, args.values[i], e);
    }
    if (e->code != SEPL_ERR_OK)
        return;

    if (args_count > args.size) {
        for (i = 0; i < args_count - args.size; i++) {
            sepl_mod_val(mod, SEPL_NONE, e);
        }
    } else if (args_count < args.size) {
        mod->vpos -= args.size - args_count;
    }
    if (e->code != SEPL_ERR_OK)
        return;
}

SEPL_LIB SeplValue sepl_mod_getexport(SeplModule *mod, SeplEnv env,
                                      const char *key) {
    sepl_size i;
    for (i = 0; i < mod->esize; i++) {
        const char *e_key = mod->exports[i];
        const char *k_key = key;
        char match = 1;

        while (*e_key && *k_key) {
            match = *e_key++ == *k_key++;
            if (!match)
                break;
        }
        if (match && *e_key == *k_key) {
            return mod->values[env.predef_len + i];
        }
    }

    return SEPL_NONE;
}
