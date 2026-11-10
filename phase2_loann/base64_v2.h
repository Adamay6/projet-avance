#ifndef BASE64_V2_H
#define BASE64_V2_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../nada_phase1/rsa_common_header.h"

int export_pubkey_b64(const char *filename, const rsaKey_t *key);
int import_pubkey_b64(const char *filename, rsaKey_t *key);

int file_to_base64_v2(const char *in_filename, const char *out_filename);
int base64_to_file_v2(const char *in_filename, const char *out_filename);

int chiffrer_fichier_blocs_b64(const char *fichier_in, const char *fichier_out, const rsaKey_t *pubKey);
int dechiffrer_fichier_blocs_b64(const char *fichier_in, const char *fichier_out, const rsaKey_t *privKey);

#endif