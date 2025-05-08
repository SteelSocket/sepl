#ifndef SEPL_ERRORS
#define SEPL_ERRORS

#include "def.h"
#include "lex.h"

typedef enum {
    SEPL_ERR_OK,

    /* Value buffer errors */
    SEPL_ERR_VOVERFLOW,
    SEPL_ERR_VUNDERFLOW,

    /* Bytecode buffer errors */
    SEPL_ERR_BOVERFLOW,

    /* Compile errors */
    SEPL_ERR_UPTOK, /* unexpected token */
    SEPL_ERR_SYN,   /* generic syntax error */
    SEPL_ERR_EEXPR, /* expected expression */

    SEPL_ERR_IDEN_NDEF, /* identifier not defined */
    SEPL_ERR_IDEN_RDEF, /* identifier redefined */
    SEPL_ERR_PREDEF,    /* set predefined variable */

    SEPL_ERR_CLOSURE,  /* closure not supported */
    SEPL_ERR_FUNC_UPV, /* set function to upvalue */

    /* Runtime Errors */
    SEPL_ERR_BC,        /* undefined bytecode */
    SEPL_ERR_FUNC_RET,  /* returning a sepl function */
    SEPL_ERR_FUNC_CALL, /* calling a non function */
    SEPL_ERR_REFUPV,    /* set reference to upvalue */
    SEPL_ERR_OPER       /* invalid operations on value */
} SeplErrorCode;

typedef struct {
#ifndef NDEBUG
    sepl_size src_line;
#endif
    sepl_size line;
    SeplErrorCode code;
    union {
        /* SEPL_ERR_UPTOK, SEPL_ERR_SYN */
        struct {
            SeplToken up;
            SeplTokenT exp;
        } tokd;

        /* SEPL_ERR_IDEN_NDEF, SEPL_ERR_IDEN_RDEF */
        SeplToken iden;

        /* SEPL_ERR_BC */
        char bc;
    } info;

} SeplError;

#ifndef NDEBUG
#define sepl_err_new(e, ecode) ((e)->code = ecode, (e)->src_line = __LINE__)
#else
#define sepl_err_new(e, ecode) ((e)->code = ecode)
#endif

#define sepl_err_uptok(e, utok, etok)                           \
    (sepl_err_new(e, SEPL_ERR_UPTOK), (e)->info.tokd.up = utok, \
     (e)->info.tokd.exp = etok)
#define sepl_err_iden(e, code, i) (sepl_err_new(e, code), (e)->info.iden = i)

#endif
