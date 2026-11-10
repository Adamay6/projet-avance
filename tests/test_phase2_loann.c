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
    char type[PUBKEY_TYPE_TAILLE] = "";

    FILE *fichier = NULL;

    initialiser_console_utf8();
    VERIFIER(gen_keys_gmp(&publique, &privee, 31) == RSA_SUCCES);
    VERIFIER(export_pubkey_b64("publique.b64", &publique, "crypt") == 0);
    VERIFIER(import_pubkey_b64("publique.b64", &importee, type) == 0);
    VERIFIER(importee.E == publique.E && importee.N == publique.N);
    VERIFIER(strcmp(type, "crypt") == 0);

    /* Le type est conserve, et un type inconnu est refuse a l'export comme a l'import. */
    rsaKey_t signature = {0};
    VERIFIER(export_pubkey_b64("signature.b64", &publique, "sign") == 0);
    VERIFIER(import_pubkey_b64("signature.b64", &signature, type) == 0);
    VERIFIER(strcmp(type, "sign") == 0 && signature.N == publique.N);
    VERIFIER(export_pubkey_b64("inconnu.b64", &publique, "autre") == -1);
    VERIFIER(export_pubkey_b64("inconnu.b64", &publique, NULL) == -1);
    fichier = fopen("ancien.b64", "w");   /* "DIYPG_PUBKEY_V2 3 33" : ancien format, sans type */
    VERIFIER(fichier != NULL);
    VERIFIER(fputs("RElZUEdfUFVCS0VZX1YyIDMgMzM=", fichier) != EOF);
    VERIFIER(fclose(fichier) == 0);
    VERIFIER(import_pubkey_b64("ancien.b64", &importee, type) == -1);
    VERIFIER(importee.N == publique.N && strcmp(type, "sign") == 0);   /* rien n'est modifie */

    fichier = fopen("source.bin", "wb");
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
