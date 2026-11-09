#include <inttypes.h>
#include "../chiffrement_blocs.h"
#include "../../console_utf8.h"
#include "../../phase2_nada/gmp_rsa.h"

static unsigned int controles = 0, echecs = 0;
#define VERIFIER(condition) do { \
    ++controles; \
    if (!(condition)) { \
        fprintf(stderr, "Echec ligne %d : %s\n", __LINE__, #condition); \
        ++echecs; \
    } \
} while (0)

static const keyPair_t paires[] = {
    {{UINT64_C(65537), UINT64_C(4295229443)},
     {UINT64_C(65537), UINT64_C(4295229443)}},
    {{UINT64_C(65537), UINT64_C(18446743979220271189)},
     {UINT64_C(9331878932546167513), UINT64_C(18446743979220271189)}}
};

static int ecrire(const char *nom, const uint8_t *octets, size_t taille)
{
    FILE *f = fopen(nom, "wb");
    int ok;
    if (f == NULL) return 0;
    ok = fwrite(octets, 1, taille, f) == taille;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

static int lire(const char *nom, uint8_t *octets, size_t capacite, size_t *taille)
{
    FILE *f = fopen(nom, "rb");
    int ok;
    if (f == NULL) return 0;
    *taille = fread(octets, 1, capacite, f);
    ok = !ferror(f) && fgetc(f) == EOF;
    if (fclose(f) != 0) ok = 0;
    return ok;
}

static uint64_t lire64(const uint8_t *octets)
{
    uint64_t valeur = 0;
    for (size_t i = 0; i < 8; ++i) valeur |= (uint64_t)octets[i] << (8u * i);
    return valeur;
}

static void ecrire64(uint8_t *octets, uint64_t valeur)
{
    for (size_t i = 0; i < 8; ++i) octets[i] = (uint8_t)(valeur >> (8u * i));
}

static int operation_en_erreur(uint64_t entree, const rsaKey_t *cle, uint64_t *sortie)
{
    (void)entree; (void)cle; (void)sortie;
    return RSA_ERREUR_CHIFFRE;
}

static int operation_positive(uint64_t entree, const rsaKey_t *cle, uint64_t *sortie)
{
    (void)entree; (void)cle; (void)sortie;
    return 1;
}

static int operation_hors_module(uint64_t entree, const rsaKey_t *cle, uint64_t *sortie)
{
    (void)entree;
    *sortie = cle->N;
    return RSA_SUCCES;
}

static int operation_hors_uint32(uint64_t entree, const rsaKey_t *cle, uint64_t *sortie)
{
    (void)entree; (void)cle;
    *sortie = UINT64_C(4294967296);
    return RSA_SUCCES;
}

static int destination_preservee(void)
{
    uint8_t lu[16];
    size_t taille = 0;
    return lire("destination.bin", lu, sizeof lu, &taille) &&
           taille == 6 && memcmp(lu, "garder", 6) == 0;
}

static void invalide_preserve(const uint8_t *octets, size_t taille)
{
    VERIFIER(ecrire("invalide.bin", octets, taille));
    VERIFIER(ecrire("destination.bin", (const uint8_t *)"garder", 6));
    VERIFIER(dechiffrer_fichier_blocs_gmp("invalide.bin", "destination.bin",
                                        &paires[1].privKey) == RSA_ERREUR_CHIFFRE);
    VERIFIER(destination_preservee());
}

int main(void)
{
    static const size_t tailles[] = {
        0, 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 255, 256, 257,
        999, 1000, 4095, 4096, 4097, 100003
    };
    keyPair_t generee;
    const keyPair_t *paires_test[] = {&generee, &paires[0], &paires[1]};
    uint8_t *source = malloc(100004);
    uint8_t *retour = malloc(100004);
    uint8_t *chiffre = malloc(200040);
    size_t taille_chiffre = 0, taille_retour = 0;
    rsaKey_t cle_invalide;

    initialiser_console_utf8();
    VERIFIER(gen_keys_gmp(&generee.pubKey, &generee.privKey, 31) == RSA_SUCCES);
    if (echecs || source == NULL || retour == NULL || chiffre == NULL) {
        free(source); free(retour); free(chiffre);
        return EXIT_FAILURE;
    }
    for (size_t k = 0; k < sizeof paires_test / sizeof paires_test[0]; ++k) {
        const keyPair_t *paire = paires_test[k];
        for (size_t t = 0; t < sizeof tailles / sizeof tailles[0]; ++t) {
            size_t taille = tailles[t];
            for (size_t i = 0; i < taille; ++i)
                source[i] = (uint8_t)((i * 73u + i / 11u) & 255u);
            /* Les zeros finaux doivent rester, meme apres un bloc incomplet. */
            for (size_t i = 0; i < taille && i < 3; ++i) source[taille - 1 - i] = 0;
            VERIFIER(ecrire("source.bin", source, taille));
            VERIFIER(chiffrer_fichier_blocs_gmp("source.bin", "chiffre.bin",
                                               &paire->pubKey) == RSA_SUCCES);
            VERIFIER(lire("chiffre.bin", chiffre, 200040, &taille_chiffre));
            VERIFIER(taille_chiffre == 16 + ((taille / 4) + (taille % 4 != 0)) * 8);
            VERIFIER(memcmp(chiffre, "DIYPG2\r\n", 8) == 0);
            VERIFIER(lire64(chiffre + 8) == taille);
            VERIFIER(dechiffrer_fichier_blocs_gmp("chiffre.bin", "retour.bin",
                                                 &paire->privKey) == RSA_SUCCES);
            VERIFIER(lire("retour.bin", retour, 100004, &taille_retour));
            VERIFIER(taille_retour == taille);
            VERIFIER(memcmp(source, retour, taille) == 0);
        }
    }
    /* Les trois fichiers precedents sont aussi compares a Python par le lanceur. */
    VERIFIER(ecrire("meme.bin", source, 100003));
    VERIFIER(chiffrer_fichier_blocs_gmp("meme.bin", "meme.bin",
                                       &paires[1].pubKey) == RSA_SUCCES);
    VERIFIER(dechiffrer_fichier_blocs_gmp("meme.bin", "meme.bin",
                                         &paires[1].privKey) == RSA_SUCCES);
    VERIFIER(lire("meme.bin", retour, 100004, &taille_retour));
    VERIFIER(taille_retour == 100003 && memcmp(source, retour, 100003) == 0);

    VERIFIER(ecrire("destination.bin", (const uint8_t *)"garder", 6));
    VERIFIER(chiffrer_fichier_blocs_gmp(NULL, "destination.bin",
                                       &paires[1].pubKey) == RSA_ERREUR_PARAMETRE);
    VERIFIER(chiffrer_fichier_blocs_gmp("", "destination.bin",
                                       &paires[1].pubKey) == RSA_ERREUR_PARAMETRE);
    VERIFIER(chiffrer_fichier_blocs_gmp("source.bin", NULL,
                                       &paires[1].pubKey) == RSA_ERREUR_PARAMETRE);
    VERIFIER(chiffrer_fichier_blocs_gmp("source.bin", "",
                                       &paires[1].pubKey) == RSA_ERREUR_PARAMETRE);
    VERIFIER(chiffrer_fichier_blocs_gmp("source.bin", "destination.bin",
                                       NULL) == RSA_ERREUR_PARAMETRE);
    VERIFIER(chiffrer_fichier_blocs("source.bin", "destination.bin",
                                   &paires[1].pubKey, NULL) == RSA_ERREUR_PARAMETRE);
    cle_invalide = paires[1].pubKey; cle_invalide.N = UINT32_MAX;
    VERIFIER(chiffrer_fichier_blocs_gmp("source.bin", "destination.bin",
                                       &cle_invalide) == RSA_ERREUR_CLE);
    cle_invalide = paires[1].pubKey; cle_invalide.E = 0;
    VERIFIER(chiffrer_fichier_blocs_gmp("source.bin", "destination.bin",
                                       &cle_invalide) == RSA_ERREUR_CLE);
    cle_invalide.E = cle_invalide.N;
    VERIFIER(chiffrer_fichier_blocs_gmp("source.bin", "destination.bin",
                                       &cle_invalide) == RSA_ERREUR_CLE);
    VERIFIER(chiffrer_fichier_blocs_gmp("absent/source.bin", "destination.bin",
                                       &paires[1].pubKey) == RSA_ERREUR_FICHIER);
    VERIFIER(chiffrer_fichier_blocs("source.bin", "destination.bin",
                                   &paires[1].pubKey, operation_en_erreur) == RSA_ERREUR_CHIFFRE);
    VERIFIER(chiffrer_fichier_blocs("source.bin", "destination.bin",
                                   &paires[1].pubKey, operation_positive) == RSA_ERREUR_CHIFFRE);
    VERIFIER(chiffrer_fichier_blocs("source.bin", "destination.bin",
                                   &paires[1].pubKey, operation_hors_module) == RSA_ERREUR_CHIFFRE);
    VERIFIER(destination_preservee());
    VERIFIER(dechiffrer_fichier_blocs("chiffre.bin", "destination.bin",
                                     &paires[1].privKey, operation_en_erreur) == RSA_ERREUR_CHIFFRE);
    VERIFIER(dechiffrer_fichier_blocs("chiffre.bin", "destination.bin",
                                     &paires[1].privKey, operation_hors_uint32) == RSA_ERREUR_CHIFFRE);
    VERIFIER(destination_preservee());
    VERIFIER(chiffrer_fichier_blocs_gmp("source.bin", "absent/resultat.bin",
                                       &paires[1].pubKey) == RSA_ERREUR_FICHIER);

    /* Toutes les coupures possibles d'un petit fichier chiffre. */
    VERIFIER(ecrire("petit.bin", (const uint8_t *)"abcde", 5));
    VERIFIER(chiffrer_fichier_blocs_gmp("petit.bin", "petit.rsa",
                                       &paires[1].pubKey) == RSA_SUCCES);
    VERIFIER(lire("petit.rsa", retour, 100004, &taille_retour));
    for (size_t i = 0; i < taille_retour; ++i) invalide_preserve(retour, i);
    retour[0] ^= 1; invalide_preserve(retour, taille_retour); retour[0] ^= 1;
    retour[taille_retour] = 0; invalide_preserve(retour, taille_retour + 1);
    /* Longueur plus courte : les octets non nuls ne peuvent pas etre du bourrage. */
    ecrire64(retour + 8, 1); invalide_preserve(retour, 24);
    ecrire64(retour + 8, UINT64_MAX); invalide_preserve(retour, taille_retour);
    ecrire64(retour + 8, 4);
    ecrire64(retour + 16, paires[1].pubKey.N); invalide_preserve(retour, 24);
    ecrire64(retour + 8, 0); invalide_preserve(retour, 24);

    /* Les quatre octets a 0xff atteignent UINT32_MAX, sans decalage signe. */
    memset(retour, 255, 4);
    VERIFIER(ecrire("maximum.bin", retour, 4));
    VERIFIER(chiffrer_fichier_blocs_gmp("maximum.bin", "maximum.rsa",
                                       &paires[1].pubKey) == RSA_SUCCES);
    VERIFIER(dechiffrer_fichier_blocs_gmp("maximum.rsa", "maximum.out",
                                         &paires[1].privKey) == RSA_SUCCES);
    VERIFIER(lire("maximum.out", retour, 100004, &taille_retour));
    VERIFIER(taille_retour == 4 && lire64((const uint8_t[8]){retour[0],retour[1],retour[2],retour[3],0,0,0,0}) == UINT32_MAX);

    free(source); free(retour); free(chiffre);
    printf("Fichiers par blocs : %u verifications, %u echecs.\n", controles, echecs);
    return echecs ? EXIT_FAILURE : EXIT_SUCCESS;
}
