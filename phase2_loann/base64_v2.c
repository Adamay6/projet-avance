#include "base64_v2.h"
#include "../phase1_loann/base64.h"
#include "../phase2_adam/chiffrement_blocs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

static int type_valide(const char *type) {
    return type && (strcmp(type, "crypt") == 0 || strcmp(type, "sign") == 0);
}

int export_pubkey_b64(const char *filename, const rsaKey_t *key, const char *type) {
    if (!filename || !key || !type_valide(type)) return -1;

    char buffer[128];
    int len = snprintf(buffer, sizeof(buffer), "DIYPG_PUBKEY_V3 %s %" PRIu64 " %" PRIu64, type, key->E, key->N);
    if (len < 0 || len >= (int)sizeof(buffer)) return -1;

    size_t b64_len = 0;
    char *b64_str = base64_encode((const unsigned char *)buffer, (size_t)len, &b64_len);
    if (!b64_str) return -1;

    FILE *f = fopen(filename, "w");
    if (!f) {
        free(b64_str);
        return -1;
    }

    int ok = (fputs(b64_str, f) != EOF);
    if (fclose(f) != 0) ok = 0;
    free(b64_str);
    return ok ? 0 : -1;
}

int import_pubkey_b64(const char *filename, rsaKey_t *key, char type[PUBKEY_TYPE_TAILLE]) {
    if (!filename || !key || !type) return -1;

    FILE *f = fopen(filename, "r");
    if (!f) return -1;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    long fsize = ftell(f);
    if (fsize <= 0 || fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return -1;
    }

    char *b64_buffer = malloc((size_t)fsize + 1);
    if (!b64_buffer) {
        fclose(f);
        return -1;
    }

    size_t read_bytes = fread(b64_buffer, 1, (size_t)fsize, f);
    int read_err = ferror(f) || (read_bytes != (size_t)fsize);
    if (fclose(f) != 0) read_err = 1;
    if (read_err) {
        free(b64_buffer);
        return -1;
    }
    b64_buffer[read_bytes] = '\0';

    size_t compact_len = 0;
    for (size_t i = 0; i < read_bytes; i++) {
        if (!isspace((unsigned char)b64_buffer[i])) {
            b64_buffer[compact_len++] = b64_buffer[i];
        }
    }
    b64_buffer[compact_len] = '\0';

    size_t dec_len = 0;
    unsigned char *decoded_str = base64_decode(b64_buffer, compact_len, &dec_len);
    free(b64_buffer);
    if (!decoded_str) return -1;

    char *plain_str = malloc(dec_len + 1);
    if (!plain_str) {
        free(decoded_str);
        return -1;
    }
    memcpy(plain_str, decoded_str, dec_len);
    plain_str[dec_len] = '\0';
    free(decoded_str);

    char header[32], type_lu[PUBKEY_TYPE_TAILLE];
    uint64_t e = 0, n = 0;
    if (sscanf(plain_str, "%31s %5s %" SCNu64 " %" SCNu64, header, type_lu, &e, &n) != 4 ||
        strcmp(header, "DIYPG_PUBKEY_V3") != 0 || !type_valide(type_lu)) {
        free(plain_str);
        return -1;
    }

    key->E = e;
    key->N = n;
    strcpy(type, type_lu);
    free(plain_str);
    return 0;
}

int file_to_base64_v2(const char *in_filename, const char *out_filename) {
    return file_to_base64(in_filename, out_filename);
}

int base64_to_file_v2(const char *in_filename, const char *out_filename) {
    return base64_to_file(in_filename, out_filename);
}

int chiffrer_fichier_blocs_b64(const char *fichier_in, const char *fichier_out, const rsaKey_t *pubKey) {
    const char *temp_bin = "temp_encrypted.bin";

    if (chiffrer_fichier_blocs_gmp(fichier_in, temp_bin, pubKey) != 0) {
        return -1;
    }

    if (file_to_base64_v2(temp_bin, fichier_out) != 0) {
        remove(temp_bin);
        return -1;
    }

    remove(temp_bin);
    return 0;
}

int dechiffrer_fichier_blocs_b64(const char *fichier_in, const char *fichier_out, const rsaKey_t *privKey) {
    const char *temp_bin = "temp_decrypted.bin";

    if (base64_to_file_v2(fichier_in, temp_bin) != 0) {
        return -1;
    }

    if (dechiffrer_fichier_blocs_gmp(temp_bin, fichier_out, privKey) != 0) {
        remove(temp_bin);
        return -1;
    }

    remove(temp_bin);
    return 0;
}
