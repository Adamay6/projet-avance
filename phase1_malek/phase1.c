#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../nada_phase1/rsa_common_header.h"
#include "phase1.h"

// Déclarations externes des fonctions de base64 si besoin (ou inclus via header si dispo)
// Les declarations sont maintenant dans base64.h, inclus par phase1.h.

// 1. Affichage des clés en hexadécimal
void printKeyHex(rsaKey_t *key, const char *name) {
    if (!key || !name) return;
    printf("--- Clé %s ---\n", name);
    printf("Exposant (E) : 0x%llX (%llu)\n", (unsigned long long)key->E, (unsigned long long)key->E);
    printf("Modulo (N)     : 0x%llX (%llu)\n", (unsigned long long)key->N, (unsigned long long)key->N);
}

// 2. Sauvegarder une paire de clés dans un fichier
int saveKeyPair(const char *filename, keyPair_t *kp) {
    return save_keypair(filename, kp) == 0;
}

// 3. Charger une paire de clés depuis un fichier
int loadKeyPair(const char *filename, keyPair_t *kp) {
    return load_keypair(filename, kp) == 0;
}

// 4. Chiffrer un tableau d'octets
uint8_t* encryptByteArray(const uint8_t *input, size_t length, rsaKey_t *key, size_t *out_len) {
    if (!out_len) return NULL;
    *out_len = 0;
    if (length > SIZE_MAX / RSA_OCTET_CHIFFRE_TAILLE) return NULL;
    size_t capacity = length * RSA_OCTET_CHIFFRE_TAILLE;
    uint8_t *output = malloc(capacity ? capacity : 1);
    if (!output) return NULL;

    if (chiffrer_tableau(input, length, key, output, capacity, out_len) != RSA_SUCCES) {
        free(output);
        return NULL;
    }
    return output;
}

// 5. Déchiffrer un tableau d'octets
uint8_t* decryptByteArray(const uint8_t *input, size_t length, rsaKey_t *key, size_t *out_len) {
    if (!out_len) return NULL;
    *out_len = 0;
    size_t capacity = length / RSA_OCTET_CHIFFRE_TAILLE;
    uint8_t *output = malloc(capacity ? capacity : 1);
    if (!output) return NULL;

    if (dechiffrer_tableau(input, length, key, output, capacity, out_len) != RSA_SUCCES) {
        free(output);
        return NULL;
    }
    return output;
}

// 6. Chiffrer un message lu dans un fichier vers un autre fichier
int encryptFile(const char *fileIn, const char *fileOut, rsaKey_t *key) {
    return chiffrer_fichier(fileIn, fileOut, key) == RSA_SUCCES;
}

// 7. Déchiffrer un message lu dans un fichier vers un autre fichier
int decryptFile(const char *fileIn, const char *fileOut, rsaKey_t *key) {
    return dechiffrer_fichier(fileIn, fileOut, key) == RSA_SUCCES;
}

// 8. Convertir un fichier binaire en fichier base64
// Fonction definie dans phase1_loann/base64.c.

// 9. Convertir un fichier base64 en fichier binaire
// Fonction definie dans phase1_loann/base64.c.
