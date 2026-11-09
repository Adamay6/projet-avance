#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include "../phase1_nada/rsa_common_header.h"
#include "../phase1_nada/phase1/phase1.h"
#include "base64.h"

static int test_type(const void *data, size_t size) {
    size_t encoded_len = 0, decoded_len = 0;
    char *encoded = base64_encode(data, size, &encoded_len);
    unsigned char *decoded = encoded ? base64_decode(encoded, encoded_len, &decoded_len) : NULL;
    int identical = decoded && decoded_len == size && memcmp(data, decoded, size) == 0;
    free(encoded);
    free(decoded);
    return identical;
}

int test_base64(void) {
    printf("Debut des Tests de l'en codage en BASE64 :\n\n");

    int global_success = 1;

    // --- TEST 1 : Test unitaire "Hi!" -> "SGkh" ---
    printf("1 Test unitaire ('Hi!')... ");
    const char *test_hi = "Hi!";
    size_t out_len = 0;
    char *encoded_hi = base64_encode((const unsigned char *)test_hi, strlen(test_hi), &out_len);

    if (encoded_hi && strcmp(encoded_hi, "SGkh") == 0) {
        printf("OK (Résultat : %s)\n", encoded_hi);
    } else {
        printf("ÉCHEC (Attendu : SGkh, Obtenu : %s)\n", encoded_hi ? encoded_hi : "NULL");
        global_success = 0;
    }
    free(encoded_hi);

    // --- TEST 2 : Encodage de types variés (uint32, uint64, clés) ---
    printf("2 Test d'encodage de types de données (uint32, uint64, clés)... ");
    
    uint32_t val32 = 0x12345678;
    uint64_t val64 = 0x123456789ABCDEF0ULL;
    rsaKey_t key = {.E = 65537, .N = 8990393};

    // Conversion en tableau d'octets et aller-retour
    size_t dec_len = 0;
    char *enc_key = base64_encode((const unsigned char *)&key, sizeof(rsaKey_t), &out_len);
    rsaKey_t *dec_key = enc_key ? (rsaKey_t *)base64_decode(enc_key, out_len, &dec_len) : NULL;

    if (test_type(&val32, sizeof val32) && test_type(&val64, sizeof val64) &&
        dec_key && dec_len == sizeof key && dec_key->E == key.E && dec_key->N == key.N) {
        printf("OK\n");
    } else {
        printf("ÉCHEC\n");
        global_success = 0;
    }
    free(enc_key);
    free(dec_key);

    // --- TEST 3 : Fichier Binaire -> Base64 -> Fichier Binaire ---
    printf("3 Test aller-retour sur fichier binaire... ");
    const char *f_in = "test_source.bin";
    const char *f_b64 = "test_out.b64";
    const char *f_res = "test_restored.bin";

    // Création d'un fichier binaire avec du contenu arbitraire
    FILE *f = fopen(f_in, "wb");
    if (f == NULL) { perror("Creation du fichier de test"); return 0; }
    uint8_t dummy_data[256];
    for (int i = 0; i < 256; i++) dummy_data[i] = (uint8_t)i;
    int write_error = fwrite(dummy_data, 1, 256, f) != 256;
    if (fclose(f) != 0) write_error = 1;
    if (write_error) return 0;

    // Exécution des fonctions de conversion de fichiers
    if (file_to_base64(f_in, f_b64) == 0 && base64_to_file(f_b64, f_res) == 0) {
        // Vérification de l'intégrité binaire
        FILE *f1 = fopen(f_in, "rb");
        FILE *f2 = fopen(f_res, "rb");
        if (!f1 || !f2) {
            if (f1) fclose(f1);
            if (f2) fclose(f2);
            return 0;
        }
        int identical = 1;

        for (int i = 0; i < 256; i++) {
            if (fgetc(f1) != fgetc(f2)) {
                identical = 0;
                break;
            }
        }
        if (fgetc(f1) != EOF || fgetc(f2) != EOF || ferror(f1) || ferror(f2)) identical = 0;
        if (fclose(f1) != 0) identical = 0;
        if (fclose(f2) != 0) identical = 0;

        if (identical) {
            printf("OK (Intégrité du fichier conservée)\n");
        } else {
            printf("ÉCHEC (Contenu différent après aller-retour)\n");
            global_success = 0;
        }
    } else {
        printf("ÉCHEC (Erreur lors de la manipulation des fichiers)\n");
        global_success = 0;
    }

    // --- TEST 4 : Information pour validation Unix ---
    printf("4 Validation de conformité Unix...\n");
    printf("Pour vérifier la conformité avec l'outil Unix, exécute :\n");
    printf("$ base64 %s | tr -d '\\r\\n' > unix_ref.b64\n", f_in);
    printf("$ diff -s %s unix_ref.b64\n", f_b64);

    printf("\nBilan Phase 1.3 : %s\n\n", global_success ? "REUSSITE" : "ECHEC");
    return global_success;

}
