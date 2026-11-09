#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../console_utf8.h"

static int fault = 0;

static size_t read_test(void *p, size_t size, size_t n, FILE *f) {
    if (fault == 1) { errno = EIO; return 0; }
    return fread(p, size, n, f);
}

static size_t write_test(const void *p, size_t size, size_t n, FILE *f) {
    if (fault == 2) { errno = ENOSPC; return 0; }
    return fwrite(p, size, n, f);
}

static int print_test(FILE *f, const char *format, ...) {
    if (fault == 3) { errno = ENOSPC; return -1; }
    va_list args;
    va_start(args, format);
    int result = vfprintf(f, format, args);
    va_end(args);
    return result;
}

static int close_test(FILE *f) {
    int result = fclose(f);
    if (fault == 4) { errno = EIO; return EOF; }
    return result;
}

// On simule les erreurs dans ces deux sources seulement. Les fichiers du projet restent les memes.
#undef fread
#undef fwrite
#undef fprintf
#undef fclose
#define fread read_test
#define fwrite write_test
#define fprintf print_test
#define fclose close_test
#include "../phase1_loann/base64.c"
#include "../phase1_nada/phase1/keys_io.c"
#undef fread
#undef fwrite
#undef fprintf
#undef fclose

static unsigned int checks = 0, errors = 0;
#define CHECK(condition) do { \
    checks++; \
    if (!(condition)) { fprintf(stderr, "Echec E/S ligne %d : %s\n", __LINE__, #condition); errors++; } \
} while (0)

static int writeFile(const char *name, const char *data) {
    FILE *f = fopen(name, "wb");
    if (!f) return 0;
    int ok = fwrite(data, 1, strlen(data), f) == strlen(data);
    if (fclose(f) != 0) ok = 0;
    return ok;
}

int main(void) {
    initialiser_console_utf8();
    CHECK(writeFile("source.bin", "ABC"));
    CHECK(writeFile("source.b64", "QUJD"));
    keyPair_t keys = {{17, 3233}, {2753, 3233}};
    fault = 1;
    CHECK(file_to_base64("source.bin", "output.b64") == -1);
    CHECK(base64_to_file("source.b64", "output.bin") == -1);
    fault = 2;
    CHECK(file_to_base64("source.bin", "output.b64") == -1);
    CHECK(base64_to_file("source.b64", "output.bin") == -1);
    fault = 3;
    CHECK(save_keypair("output.key", &keys) == -1);
    fault = 4;
    CHECK(file_to_base64("source.bin", "output.b64") == -1);
    CHECK(base64_to_file("source.b64", "output.bin") == -1);
    CHECK(save_keypair("output.key", &keys) == -1);
    fault = 0;
    CHECK(file_to_base64("source.bin", "output.b64") == 0);
    CHECK(base64_to_file("source.b64", "output.bin") == 0);
    CHECK(save_keypair("output.key", &keys) == 0);
    base64_cleanup();
    printf("Erreurs d'E/S : %u verifications, %u echecs.\n", checks, errors);
    return errors ? EXIT_FAILURE : EXIT_SUCCESS;
}
