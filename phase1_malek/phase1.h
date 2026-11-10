#ifndef PHASE1_MALEK_H
#define PHASE1_MALEK_H

#include "../phase1_nada/phase1/phase1.h"
#include "../phase1_adam/chiffrement.h"
#include "../phase1_loann/base64.h"

// Affichage et gestion des clés
// Les fonctions de ce fichier gardent 1 pour un succes, 0 pour un echec.
void printKeyHex(rsaKey_t *key, const char *name);
int saveKeyPair(const char *filename, keyPair_t *kp);
int loadKeyPair(const char *filename, keyPair_t *kp);

// Chiffrement / Déchiffrement de tableaux d'octets
uint8_t* encryptByteArray(const uint8_t *input, size_t length, rsaKey_t *key, size_t *out_len);
uint8_t* decryptByteArray(const uint8_t *input, size_t length, rsaKey_t *key, size_t *out_len);

// Chiffrement / Déchiffrement de fichiers
int encryptFile(const char *fileIn, const char *fileOut, rsaKey_t *key);
int decryptFile(const char *fileIn, const char *fileOut, rsaKey_t *key);

// Conversions Base64 pour les fichiers
// Ces deux fonctions sont celles de Loann : 0 pour un succes, -1 pour un echec.
int file_to_base64(const char *fileIn, const char *fileOut);
int base64_to_file(const char *fileIn, const char *fileOut);

#endif
