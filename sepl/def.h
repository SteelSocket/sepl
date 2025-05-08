#ifndef SEPL_DEFINITIONS
#define SEPL_DEFINITIONS

#define sepl__static_assert(cond, name) \
    typedef char sepl__assert_##name[(cond) ? 1 : -1]
#define sepl__is_unsigned(type) ((type)(-1) > (type)(0))

#ifndef SEPL_DEF_SIZE
typedef unsigned long sepl_size;
#else
typedef SEPL_DEF_SIZE sepl_size;
sepl__static_assert(sepl__is_unsigned(sepl_size), is_unsigned);
#endif

#ifndef SEPL_NULL
#define SEPL_NULL ((void *)0)
#endif

#ifndef SEPL_LIB
#define SEPL_LIB extern
#endif

#ifndef SEPL_API
#define SEPL_API static
#endif

#endif
