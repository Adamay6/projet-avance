#include <stdio.h>
#include <stdlib.h>
#include "../phase1_nada/rsa_common_header.h"
#include "phase1.h"
#include "../phase1_adam/tests_chiffrement.h"
#include "../console_utf8.h"

static int sameFile(const char *first, const char *second) {
    FILE *f1 = fopen(first, "rb");
    FILE *f2 = fopen(second, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }
    int a, b, identical = 1;
    do {
        a = fgetc(f1);
        b = fgetc(f2);
        if (a != b) identical = 0;
    } while (identical && a != EOF);
    if (ferror(f1) || ferror(f2)) identical = 0;
    if (fclose(f1) != 0) identical = 0;
    if (fclose(f2) != 0) identical = 0;
    return identical;
}

int main(void) {
    initialiser_console_utf8();
    init_log();
    srand((unsigned)time(NULL));
    int errors = 0;
    uint64_t max_prime = MAX_PRIME;
    keyPair_t kp;

    printf("=== LANCEMENT DES TESTS PHASE 1 ===\n\n");

    // 1. Génération des clés avec Rabin
    printf("1. Generation des cles...\n");
    if (gen_keypair(&kp, max_prime) != 0) {
        fprintf(stderr, "ECHEC : generation des cles.\n");
        close_log();
        return EXIT_FAILURE;
    }
    printKeyHex(&kp.pubKey, "Publique");
    printKeyHex(&kp.privKey, "Privee");

    // 2. Test sauvegarde et chargement
    printf("\n2. Test sauvegarde et chargement des cles...\n");
    if (saveKeyPair("keys_test.txt", &kp)) {
        printf("-> Cles sauvegardees avec succes dans keys_test.txt\n");
    } else {
        fprintf(stderr, "ECHEC : sauvegarde des cles.\n");
        errors++;
    }
    keyPair_t kp_loaded;
    if (loadKeyPair("keys_test.txt", &kp_loaded)) {
        printf("-> Cles chargees avec succes !\n");
        printKeyHex(&kp_loaded.pubKey, "Chargee Publique");
        if (kp_loaded.pubKey.E != kp.pubKey.E || kp_loaded.pubKey.N != kp.pubKey.N ||
            kp_loaded.privKey.E != kp.privKey.E || kp_loaded.privKey.N != kp.privKey.N) {
            fprintf(stderr, "ECHEC : les cles rechargees sont differentes.\n");
            errors++;
        }
    } else {
        fprintf(stderr, "ECHEC : chargement des cles.\n");
        errors++;
    }

    // 3. Test de chiffrement / déchiffrement de fichiers
    printf("\n3. Test chiffrement/dechiffrement de fichiers...\n");
    FILE *f = fopen("msg_clair.txt", "w");
    if (!f) {
        fprintf(stderr, "ECHEC : creation du message de test.\n");
        close_log();
        return EXIT_FAILURE;
    }
    int write_error = fprintf(f, "Bonjour, ceci est un test secret pour la phase 1 !") < 0;
    if (fclose(f) != 0) write_error = 1;
    if (write_error) {
        fprintf(stderr, "ECHEC : ecriture du message de test.\n");
        close_log();
        return EXIT_FAILURE;
    }

    if (encryptFile("msg_clair.txt", "msg_chiffre.txt", &kp.pubKey)) {
        printf("-> Fichier chiffre avec succes dans msg_chiffre.txt\n");
    } else {
        fprintf(stderr, "ECHEC : chiffrement du fichier.\n");
        errors++;
    }
    if (decryptFile("msg_chiffre.txt", "msg_dechiffre.txt", &kp.privKey)) {
        printf("-> Fichier dechiffre avec succes dans msg_dechiffre.txt\n");
    } else {
        fprintf(stderr, "ECHEC : dechiffrement du fichier.\n");
        errors++;
    }
    if (!sameFile("msg_clair.txt", "msg_dechiffre.txt")) {
        fprintf(stderr, "ECHEC : le fichier dechiffre differe de l'original.\n");
        errors++;
    }

    // 4. Test Base64
    printf("\n4. Test conversion Base64...\n");
    if (file_to_base64("msg_chiffre.txt", "msg_chiffre.b64") == 0) {
        printf("-> Fichier converti en Base64 (msg_chiffre.b64)\n");
    } else {
        fprintf(stderr, "ECHEC : encodage du fichier en Base64.\n");
        errors++;
    }
    if (base64_to_file("msg_chiffre.b64", "msg_recup.bin") == 0) {
        printf("-> Fichier Base64 reconverti en binaire (msg_recup.bin)\n");
    } else {
        fprintf(stderr, "ECHEC : decodage du fichier Base64.\n");
        errors++;
    }
    if (!sameFile("msg_chiffre.txt", "msg_recup.bin")) {
        fprintf(stderr, "ECHEC : le chiffre a change pendant la conversion Base64.\n");
        errors++;
    }

    printf("\n5. Tests des modules...\n");
    if (tester_chiffrement_adam(".") != 0) errors++;
    if (test_base64() != 1) errors++;

    if (errors == 0)
        printf("\n=== TOUS LES TESTS SONT TERMINES AVEC SUCCES ===\n");
    else
        fprintf(stderr, "\n=== ECHEC : %d controle(s) incorrect(s) ===\n", errors);
    base64_cleanup();
    close_log();
    return errors == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
