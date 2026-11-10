/// \file tests_commandes_nada.c
/// \author Nada ARABA
/// \brief Tests de la phase 3 (partie Nada) : table_chercher provisoire et commande show.
#include <stdio.h>
#include <string.h>
#include "commandes_nada.h"
#include "../phase1_nada/phase1/phase1.h"
#include "../phase1_adam/chiffrement.h"

static int echecs = 0;
#define VERIF(cond) do { if (!(cond)) { fprintf(stderr, "ECHEC ligne %d : %s\n", __LINE__, #cond); echecs++; } } while (0)

/* Table d'essai : une clef crypt, une clef sign, une clef publique importee. */
static entree_clef_t entrees[] = {
    { "alice", CLEF_CRYPT, { 65537, 4295229443ULL }, { 1234567, 4295229443ULL }, true },
    { "alice_sign", CLEF_SIGN, { 3, 0x1a2b3c4dULL }, { 7, 0x1a2b3c4dULL }, true },
    { "bob", CLEF_CRYPT, { 65537, 0xdeadbeefULL }, { 0, 0 }, false }
};
static table_clefs_t table = { entrees, sizeof entrees / sizeof entrees[0], sizeof entrees / sizeof entrees[0] };

static void test_table_chercher(void) {
    table_clefs_t vide = { NULL, 0, 0 }, incoherente = { NULL, 2, 2 };
    VERIF(table_chercher(NULL, "alice") == NULL);
    VERIF(table_chercher(&table, NULL) == NULL);
    VERIF(table_chercher(&vide, "alice") == NULL);
    VERIF(table_chercher(&incoherente, "alice") == NULL);
    VERIF(table_chercher(&table, "alice") == &entrees[0]);
    VERIF(table_chercher(&table, "bob") == &entrees[2]);
    VERIF(table_chercher(&table, "ali") == NULL);          /* pas de recherche par prefixe */
    VERIF(table_chercher(&table, "Alice") == NULL);        /* sensible a la casse */
    VERIF(table_chercher(&table, "") == NULL);
}

static void test_show_erreurs(void) {
    char *sans_id[] = { "show" }, *trop[] = { "show", "alice", "bob" }, *inconnu[] = { "show", "carole" };
    char *id_null[] = { "show", NULL };
    VERIF(cmd_show(NULL, 2, inconnu) == RSA_ERREUR_PARAMETRE);
    VERIF(cmd_show(&table, 2, NULL) == RSA_ERREUR_PARAMETRE);
    VERIF(cmd_show(&table, 1, sans_id) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_show(&table, 3, trop) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_show(&table, 2, id_null) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_show(&table, 2, inconnu) == CMD_ERREUR_ID_INCONNU);
}

/* Lit tout le fichier dans buf (termine par '\0'). */
static void lire_fichier(const char *chemin, char *buf, size_t taille) {
    size_t n = 0;
    FILE *f = fopen(chemin, "r");
    if (f != NULL) {
        n = fread(buf, 1, taille - 1, f);
        fclose(f);
    }
    buf[n] = '\0';
}

/* En dernier : la sortie standard est redirigee vers un fichier pour verifier l'affichage. */
static void test_show_affichage(void) {
    char *a[] = { "show", "alice" }, *s[] = { "show", "alice_sign" }, *b[] = { "show", "bob" };
    char texte[1024];
    fflush(stdout);
    if (freopen("show_sortie.txt", "w", stdout) == NULL) {
        fprintf(stderr, "ECHEC : redirection de la sortie impossible\n");
        echecs++;
        return;
    }
    VERIF(cmd_show(&table, 2, a) == CMD_SUCCES);
    VERIF(cmd_show(&table, 2, s) == CMD_SUCCES);
    VERIF(cmd_show(&table, 2, b) == CMD_SUCCES);
    fflush(stdout);
    lire_fichier("show_sortie.txt", texte, sizeof texte);
    VERIF(strstr(texte, "alice (crypt)\nclef publique : E = 0x10001  N = 0x100040003\n"
                        "clef privee   : E = 0x12d687  N = 0x100040003\n") != NULL);
    VERIF(strstr(texte, "alice_sign (sign)\nclef publique : E = 0x3  N = 0x1a2b3c4d\n") != NULL);
    VERIF(strstr(texte, "bob (crypt)\nclef publique : E = 0x10001  N = 0xdeadbeef\n"
                        "clef privee   : absente\n") != NULL);
}

int main(void) {
    init_log();
    test_table_chercher();
    test_show_erreurs();
    test_show_affichage();
    close_log();
    /* stdout est redirige : le bilan va sur la sortie d'erreur. */
    if (echecs == 0) fprintf(stderr, "Phase 3 (Nada) : tous les tests sont passes.\n");
    else fprintf(stderr, "Phase 3 (Nada) : %d echec(s).\n", echecs);
    return echecs == 0 ? 0 : 1;
}
