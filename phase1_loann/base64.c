#include "../phase1_nada/rsa_common_header.h"
#include "../phase1_nada/phase1/phase1.h"
#include "base64.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

int file_to_base64(const char *in, const char *out) {
    if (in == NULL || out == NULL || in[0] == '\0' || out[0] == '\0') return -1;
    FILE *f = fopen(in, "rb");
    if (f == NULL) {
        perror("Erreur : le fichier entrant n'existe pas");
        return -1;
    }

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long len = ftell(f);
    if (len < 0 || (long)(size_t)len != len || (size_t)len == SIZE_MAX ||
        fseek(f, 0, SEEK_SET) != 0) { fclose(f); return -1; }

    unsigned char *read_data = malloc(len ? (size_t)len : 1);
    if (read_data == NULL) {
        perror("Erreur d'allocation mémoire");
        fclose(f);
        return -1;
    }

    size_t bytes_read = fread(read_data, 1, (size_t)len, f);
    int read_error = ferror(f);
    if (fclose(f) != 0) read_error = 1;
    if (read_error || bytes_read != (size_t)len) { free(read_data); return -1; }

    size_t b64_len = 0;
    char *b64_data = base64_encode(read_data, len, &b64_len);
    free(read_data);

    if (b64_data == NULL) {
        return -1;
    }

    FILE *fout = fopen(out, "wb");
    if (fout == NULL) {
        perror("Erreur lors de la création du fichier sortant");
        free(b64_data);
        return -1;
    }

    int write_error = fwrite(b64_data, 1, b64_len, fout) != b64_len;
    if (fclose(fout) != 0) write_error = 1;
    free(b64_data);

    return write_error ? -1 : 0;
}

int base64_to_file(const char *in, const char *out) {
    if (in == NULL || out == NULL || in[0] == '\0' || out[0] == '\0') return -1;
    FILE *f = fopen(in, "rb");
    if (f == NULL) {
        perror("Erreur : le fichier Base64 entrant n'existe pas");
        return -1;
    }

    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return -1; }
    long len = ftell(f);
    if (len < 0 || (long)(size_t)len != len || (size_t)len == SIZE_MAX ||
        fseek(f, 0, SEEK_SET) != 0) { fclose(f); return -1; }

    char *read_data = malloc((size_t)len + 1);
    if (read_data == NULL) {
        perror("Erreur d'allocation mémoire");
        fclose(f);
        return -1;
    }

    size_t bytes_read = fread(read_data, 1, (size_t)len, f);
    read_data[bytes_read] = '\0';
    int read_error = ferror(f);
    if (fclose(f) != 0) read_error = 1;
    if (read_error || bytes_read != (size_t)len) { free(read_data); return -1; }

    // Les fichiers produits par la commande base64 peuvent contenir des retours ligne.
    size_t compact_length = 0;
    for (size_t i = 0; i < bytes_read; i++) {
        if (!isspace((unsigned char)read_data[i]))
            read_data[compact_length++] = read_data[i];
    }
    read_data[compact_length] = '\0';
    bytes_read = compact_length;

    size_t bin_len = 0;
    
    unsigned char *bin_data = base64_decode(read_data, bytes_read, &bin_len);
    free(read_data);

    if (bin_data == NULL) {
        return -1;
    }

    FILE *fout = fopen(out, "wb");
    if (fout == NULL) {
        perror("Erreur lors de la création du fichier binaire sortant");
        free(bin_data);
        return -1;
    }

    int write_error = fwrite(bin_data, 1, bin_len, fout) != bin_len;
    if (fclose(fout) != 0) write_error = 1;
    free(bin_data);

    return write_error ? -1 : 0;
}
