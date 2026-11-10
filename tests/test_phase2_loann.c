#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../console_utf8.h"
#include "../phase1_loann/base64.h"
#include "../phase2_loann/base64_v2.h"
#include "../phase2_nada/gmp_rsa.h"

#define VERIFIER(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "Échec ligne %d : %s\n", __LINE__, #condition); \
        return EXIT_FAILURE; \
    } \
} while (0)

int main(void)
{
    const uint8_t clair[] = {0x41, 0x00, 0xff, 0x42, 0x00};
    uint8_t retour[sizeof clair + 1];
    rsaKey_t publique, privee;
    rsaKey_t importee = {0};

    initialiser_console_utf8();
    VERIFIER(gen_keys_gmp(&publique, &privee, 31) == RSA_SUCCES);
    VERIFIER(export_pubkey_b64("publique.b64", &publique) == 0);
    VERIFIER(import_pubkey_b64("publique.b64", &importee) == 0);
    VERIFIER(importee.E == publique.E && importee.N == publique.N);

    FILE *fichier = fopen("source.bin", "wb");
    VERIFIER(fichier != NULL);
    size_t taille = fwrite(clair, 1, sizeof clair, fichier);
    int fermeture = fclose(fichier);
    VERIFIER(taille == sizeof clair && fermeture == 0);

    VERIFIER(chiffrer_fichier_blocs_b64("source.bin", "chiffre.b64", &importee) == 0);
    VERIFIER(dechiffrer_fichier_blocs_b64("chiffre.b64", "retour.bin", &privee) == 0);
    base64_cleanup();

    fichier = fopen("retour.bin", "rb");
    VERIFIER(fichier != NULL);
    taille = fread(retour, 1, sizeof retour, fichier);
    int lecture_ok = !ferror(fichier);
    fermeture = fclose(fichier);
    VERIFIER(lecture_ok && fermeture == 0 && taille == sizeof clair);
    VERIFIER(memcmp(clair, retour, sizeof clair) == 0);

    puts("Phase 2 (Loann) : Base64 et échange de clefs réussis.");
    return EXIT_SUCCESS;
}
