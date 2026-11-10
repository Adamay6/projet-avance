#include "../contacts_interne.h"
#include "../../console_utf8.h"

static int autorisations = -1;
static size_t allocations_actives = 0;
static void *allouer_test(size_t nombre, size_t taille)
{
    void *p;
    if (autorisations == 0) return NULL;
    if (autorisations > 0) --autorisations;
    p = calloc(nombre, taille);
    if (p != NULL) ++allocations_actives;
    return p;
}
static void liberer_test(void *p)
{
    if (p != NULL) --allocations_actives;
    free(p);
}
#undef calloc
#undef free
#define calloc allouer_test
#define free liberer_test
#include "../contacts.c"
#include "../contacts_io.c"
#undef calloc
#undef free

static unsigned int controles = 0, echecs = 0;
#define VERIFIER(c) do { ++controles; if (!(c)) { \
    fprintf(stderr, "Echec memoire ligne %d : %s\n", __LINE__, #c); ++echecs; \
} } while (0)

int main(void)
{
    annuaire_contacts_t source = {NULL}, garde = {NULL};
    rsaKey_t cle = {65537, UINT64_C(4295229443)};   /* clef crypt : N > 2^32 - 1 */
    FILE *f;
    size_t initiales;
    initialiser_console_utf8();
    VERIFIER(contacts_ajouter(&source, "a", "", "", "") == 0);
    VERIFIER(contacts_ajouter_cle(&source, "a", "k", CONTACT_CRYPT, &cle) == 0);
    VERIFIER(contacts_ajouter(&source, "b", "", "", "") == 0);
    VERIFIER(contacts_ajouter_cle(&source, "b", "k", CONTACT_SIGN, &cle) == 0);
    VERIFIER(contacts_ajouter(&garde, "garder", "", "", "") == 0);
    initiales = allocations_actives;
    autorisations = 0;
    VERIFIER(contacts_ajouter(&garde, "nouveau", "", "", "") == CONTACTS_ERREUR_MEMOIRE);
    VERIFIER(contacts_ajouter_cle(&garde, "garder", "k", CONTACT_CRYPT, &cle) == CONTACTS_ERREUR_MEMOIRE);
    autorisations = -1;
    VERIFIER(allocations_actives == initiales && contacts_nombre(&garde) == 1);
    f = fopen("memoire.bin", "wb");
    if (f == NULL) return EXIT_FAILURE;
    VERIFIER(contacts_sauver(f, &source) == 0);
    VERIFIER(fclose(f) == 0);
    for (int budget = 0; budget < 4; ++budget) {
        f = fopen("memoire.bin", "rb");
        if (f == NULL) return EXIT_FAILURE;
        autorisations = budget;
        VERIFIER(contacts_charger(f, &garde) == CONTACTS_ERREUR_MEMOIRE);
        autorisations = -1;
        VERIFIER(fclose(f) == 0);
        VERIFIER(contacts_nombre(&garde) == 1 && contacts_trouver(&garde, "garder") != NULL);
        VERIFIER(allocations_actives == initiales);
    }
    f = fopen("memoire.bin", "rb");
    if (f == NULL) return EXIT_FAILURE;
    VERIFIER(contacts_charger(f, &garde) == 0);
    VERIFIER(fclose(f) == 0);
    VERIFIER(contacts_nombre(&garde) == 2);
    contacts_liberer(&garde);
    contacts_liberer(&source);
    VERIFIER(allocations_actives == 0);
    printf("Memoire contacts : %u verifications, %u echecs.\n", controles, echecs);
    return echecs ? EXIT_FAILURE : EXIT_SUCCESS;
}
