#include "../chiffrement_blocs.h"
#include "../../console_utf8.h"
#include <errno.h>

static int panne = 0, fermeture = 0, copie_finale = 0;
static size_t lire_test(void *p, size_t s, size_t n, FILE *f)
{
    if (panne == 1) { errno = EIO; return 0; }
    return fread(p, s, n, f);
}
static size_t ecrire_test(const void *p, size_t s, size_t n, FILE *f)
{
    if (panne == 2 || (panne == 7 && copie_finale)) {
        errno = ENOSPC; return 0;
    }
    return fwrite(p, s, n, f);
}
static int fermer_test(FILE *f)
{
    int resultat = fclose(f);
    ++fermeture;
    if ((panne == 3 && fermeture == 1) ||
        (panne == 8 && fermeture == 2) ||
        (panne == 9 && fermeture == 3)) { errno = EIO; return EOF; }
    return resultat;
}
static int erreur_test(FILE *f)
{
    return panne == 1 ? 1 : ferror(f);
}
static int vider_test(FILE *f)
{
    if (panne == 4) { errno = ENOSPC; return EOF; }
    return fflush(f);
}
static int positionner_test(FILE *f, long p, int origine)
{
    if (panne == 5) { errno = EIO; return -1; }
    return fseek(f, p, origine);
}
static FILE *ouvrir_test(const char *p, const char *mode)
{
    if (panne == 6 && strcmp(mode, "wb+") == 0) { errno = ENOSPC; return NULL; }
    if (strcmp(mode, "wb") == 0) copie_finale = 1;
    return fopen(p, mode);
}
#ifndef _WIN32
static FILE *temporaire_test(void)
{
    if (panne == 6) { errno = ENOSPC; return NULL; }
    return tmpfile();
}
#endif

#undef fread
#undef fwrite
#undef fclose
#undef ferror
#undef fflush
#undef fseek
#undef fopen
#define fread lire_test
#define fwrite ecrire_test
#define fclose fermer_test
#define ferror erreur_test
#define fflush vider_test
#define fseek positionner_test
#define fopen ouvrir_test
#ifndef _WIN32
#define tmpfile temporaire_test
#endif
#include "../chiffrement_blocs.c"
#undef fread
#undef fwrite
#undef fclose
#undef ferror
#undef fflush
#undef fseek
#undef fopen
#ifndef _WIN32
#undef tmpfile
#endif

static int identite(uint64_t entree, const rsaKey_t *cle, uint64_t *sortie)
{
    (void)cle; *sortie = entree; return RSA_SUCCES;
}
static int preparer(const char *nom, const char *texte)
{
    FILE *f = fopen(nom, "wb");
    int ok;
    if (f == NULL) return 0;
    ok = fwrite(texte, 1, strlen(texte), f) == strlen(texte);
    if (fclose(f) != 0) ok = 0;
    return ok;
}
static int intacte(void)
{
    char tampon[8] = {0};
    FILE *f = fopen("destination_io.bin", "rb");
    size_t taille;
    if (f == NULL) return 0;
    taille = fread(tampon, 1, sizeof tampon, f);
    return fclose(f) == 0 && taille == 6 && memcmp(tampon, "garder", 6) == 0;
}
int main(void)
{
    rsaKey_t cle = {1, UINT64_C(4295229443)};
    unsigned int controles = 0, echecs = 0;
    initialiser_console_utf8();
    if (!preparer("source_io.bin", "abcde")) return EXIT_FAILURE;
    for (int mode = 1; mode <= 9; ++mode) {
        int resultat;
        if (!preparer("destination_io.bin", "garder")) return EXIT_FAILURE;
        panne = mode; fermeture = 0; copie_finale = 0;
        resultat = chiffrer_fichier_blocs("source_io.bin", "destination_io.bin",
                                         &cle, identite);
        panne = 0;
        ++controles;
        if (resultat != RSA_ERREUR_FICHIER) {
            fprintf(stderr, "Panne %d : erreur non remontee (%d).\n", mode, resultat);
            ++echecs;
        }
        if (mode <= 6) {
            ++controles;
            if (!intacte()) {
                fprintf(stderr, "Panne %d : destination modifiee.\n", mode);
                ++echecs;
            }
        }
    }
    printf("E/S blocs : %u verifications, %u echecs.\n", controles, echecs);
    return echecs ? EXIT_FAILURE : EXIT_SUCCESS;
}
