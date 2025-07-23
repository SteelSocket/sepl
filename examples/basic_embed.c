#include <stdio.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"
#include "../sepl_com.h"

int main() {
    /* Define bytecode buffer and value buffer */
    unsigned char bc[1024];
    SeplValue vl[100];

    /* Define empty environment */
    SeplEnv env = {0};
    SeplModule mod = sepl_mod_new(bc, 1024, vl, 100);

    /* Compile the function */
    SeplCompiler com = sepl_com_init("{return 1 + 1;}", &mod, env);
    sepl_com_block(&com);
    SeplError error = sepl_com_finish(&com);
    if (error.code != SEPL_ERR_OK) {
        printf("Failed to compile!\n");
        return 1;
    }

    /* Call the function */
    SeplValue result = sepl_mod_exec(&mod, &error, env);
    if (error.code != SEPL_ERR_OK) {
        printf("Failed to run!\n");
        return 1;
    }

    printf("The number returned: %lf\n", result.as.num);

    return 0;
}
