#ifndef BASE64_V2_H
#define BASE64_V2_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "../phase1_nada/rsa_common_header.h"

/* Clef publique : "DIYPG_PUBKEY_V3 <type> <E> <N>" encode en base64, type = "crypt" ou "sign". */
#define PUBKEY_TYPE_TAILLE 6   /* "crypt" + '\0' */

/* Renvoie 0, ou -1 si un parametre est NULL, si le type est inconnu ou si l'ecriture echoue. */
int export_pubkey_b64(const char *filename, const rsaKey_t *key, const char *type);
/* Renvoie 0 et remplit key et type, ou -1 (fichier illisible, format ou type invalide) sans rien modifier. */
int import_pubkey_b64(const char *filename, rsaKey_t *key, char type[PUBKEY_TYPE_TAILLE]);

int file_to_base64_v2(const char *in_filename, const char *out_filename);
int base64_to_file_v2(const char *in_filename, const char *out_filename);

int chiffrer_fichier_blocs_b64(const char *fichier_in, const char *fichier_out, const rsaKey_t *pubKey);
int dechiffrer_fichier_blocs_b64(const char *fichier_in, const char *fichier_out, const rsaKey_t *privKey);

#endif
