#include <stdio.h>
#include <stdlib.h>
#include "base64.h"
#include "../console_utf8.h"

int main(void) {
    initialiser_console_utf8();
    printf("\nTEST BASE64 (PHASE 1.3)\n\n");

    int res = test_base64();

    if (res == 1) {
        printf("\n[RÉSULTAT] Tout est conforme pour la Phase 1.3 !\n\n");
        return EXIT_SUCCESS;
    } else {
        printf("\n[RÉSULTAT] Échec d'un ou plusieurs tests.\n\n");
        return EXIT_FAILURE;
    }
}
