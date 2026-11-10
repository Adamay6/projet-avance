/// \file tests_commandes_nada.c
/// \author Nada ARABA
/// \brief Tests de la phase 3 (partie Nada) : table_chercher provisoire, show, crypt, uncrypt et savepub.
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "commandes_nada.h"
#include "../phase1_nada/phase1/phase1.h"
#include "../phase1_adam/chiffrement.h"
#include "../phase2_nada/gmp_rsa.h"   /* gen_keys_gmp : clefs avec N > 2^32 */
#include "../phase1_loann/base64.h"
#include "../phase2_loann/base64_v2.h"

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

static int ecrire_fichier(const char *chemin, const uint8_t *donnees, size_t taille) {
    FILE *f = fopen(chemin, "wb");
    if (f == NULL) return 0;
    int ok = fwrite(donnees, 1, taille, f) == taille;
    return fclose(f) == 0 && ok;
}

/* 1 si les deux fichiers ont exactement le meme contenu. */
static int fichiers_egaux(const char *a, const char *b) {
    FILE *fa = fopen(a, "rb"), *fb = fopen(b, "rb");
    int egaux = fa != NULL && fb != NULL, ca, cb;
    while (egaux) {
        ca = fgetc(fa);
        cb = fgetc(fb);
        if (ca != cb) egaux = 0;
        if (ca == EOF || cb == EOF) break;
    }
    if (fa != NULL) fclose(fa);
    if (fb != NULL) fclose(fb);
    return egaux;
}

static void test_crypt_uncrypt(void) {
    rsaKey_t pub, priv, pub2, priv2;
    VERIF(gen_keys_gmp(&pub, &priv, 25) == RSA_SUCCES);
    VERIF(gen_keys_gmp(&pub2, &priv2, 25) == RSA_SUCCES);
    entree_clef_t e[5];
    e[0] = (entree_clef_t){ "alice", CLEF_CRYPT, pub, priv, true };
    e[1] = (entree_clef_t){ "alice_sign", CLEF_SIGN, pub, priv, true };
    e[2] = (entree_clef_t){ "bob_pub", CLEF_CRYPT, pub, { 0, 0 }, false };
    e[3] = (entree_clef_t){ "phase1", CLEF_CRYPT, { 3, 1000003ULL }, { 666011, 1000003ULL }, true };  /* N <= 2^32 */
    e[4] = (entree_clef_t){ "autre", CLEF_CRYPT, pub2, priv2, true };
    table_clefs_t t = { e, 5, 5 };

    /* aller-retour : tailles du cahier des charges, puis un fichier binaire avec les 256 octets */
    static const size_t tailles[] = { 0, 1, 3, 4, 5, 1000, 256 };
    uint8_t donnees[1000];
    for (size_t i = 0; i < sizeof tailles / sizeof tailles[0]; i++) {
        for (size_t j = 0; j < tailles[i]; j++)
            donnees[j] = tailles[i] == 256 ? (uint8_t)j : (uint8_t)("Message UTF-8 : é è à !"[j % 26]);
        char *c[] = { "crypt", "alice", "clair.bin", "chiffre.bin" };
        char *u[] = { "uncrypt", "alice", "chiffre.bin", "restitue.bin" };
        VERIF(ecrire_fichier("clair.bin", donnees, tailles[i]));
        VERIF(cmd_crypt(&t, 4, c) == RSA_SUCCES);
        VERIF(cmd_uncrypt(&t, 4, u) == RSA_SUCCES);
        VERIF(fichiers_egaux("clair.bin", "restitue.bin"));
        VERIF(!fichiers_egaux("clair.bin", "chiffre.bin"));
    }

    char *c_pub[] = { "crypt", "bob_pub", "clair.bin", "chiffre_bob.bin" };
    char *u_pub[] = { "uncrypt", "bob_pub", "chiffre_bob.bin", "restitue.bin" };
    VERIF(cmd_crypt(&t, 4, c_pub) == RSA_SUCCES);                    /* la clef publique suffit pour chiffrer */
    VERIF(cmd_uncrypt(&t, 4, u_pub) == CMD_ERREUR_PAS_DE_PRIVEE);

    char *c_court[] = { "crypt", "alice", "clair.bin" };
    char *c_long[] = { "crypt", "alice", "a", "b", "c" };
    char *c_null[] = { "crypt", "alice", NULL, "b" };
    char *c_inconnu[] = { "crypt", "carole", "clair.bin", "x.bin" };
    char *c_sign[] = { "crypt", "alice_sign", "clair.bin", "x.bin" };
    char *u_sign[] = { "uncrypt", "alice_sign", "chiffre.bin", "x.bin" };
    char *c_phase1[] = { "crypt", "phase1", "clair.bin", "x.bin" };
    char *c_absent[] = { "crypt", "alice", "fichier_absent.bin", "x.bin" };
    char *u_clair[] = { "uncrypt", "alice", "clair.bin", "x.bin" };
    VERIF(cmd_crypt(NULL, 4, c_inconnu) == RSA_ERREUR_PARAMETRE);
    VERIF(cmd_uncrypt(&t, 4, NULL) == RSA_ERREUR_PARAMETRE);
    VERIF(cmd_crypt(&t, 3, c_court) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_uncrypt(&t, 5, c_long) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_crypt(&t, 4, c_null) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_crypt(&t, 4, c_inconnu) == CMD_ERREUR_ID_INCONNU);
    VERIF(cmd_crypt(&t, 4, c_sign) == CMD_ERREUR_TYPE);
    VERIF(cmd_uncrypt(&t, 4, u_sign) == CMD_ERREUR_TYPE);
    VERIF(cmd_crypt(&t, 4, c_phase1) == RSA_ERREUR_CLE);             /* clef de la phase 1 refusee */
    VERIF(cmd_crypt(&t, 4, c_absent) == RSA_ERREUR_FICHIER);
    VERIF(cmd_uncrypt(&t, 4, u_clair) == RSA_ERREUR_CHIFFRE);        /* pas un fichier DIYPG2 */

    /* mauvaise clef : on ne doit jamais retrouver le clair */
    uint8_t texte[] = "Rendez-vous a 14h";
    char *c_ok[] = { "crypt", "alice", "clair.bin", "chiffre.bin" };
    char *u_autre[] = { "uncrypt", "autre", "chiffre.bin", "restitue_autre.bin" };
    VERIF(ecrire_fichier("clair.bin", texte, sizeof texte - 1));
    VERIF(cmd_crypt(&t, 4, c_ok) == RSA_SUCCES);
    int r = cmd_uncrypt(&t, 4, u_autre);
    VERIF(r != RSA_SUCCES || !fichiers_egaux("clair.bin", "restitue_autre.bin"));
}

static void lire_fichier(const char *chemin, char *buf, size_t taille);

static void test_savepub(void) {
    rsaKey_t pub, priv, pub_s, priv_s, importee = { 0, 0 };
    char type[PUBKEY_TYPE_TAILLE] = "";
    VERIF(gen_keys_gmp(&pub, &priv, 27) == RSA_SUCCES);
    VERIF(gen_keys_gmp(&pub_s, &priv_s, 27) == RSA_SUCCES);
    entree_clef_t e[4];
    e[0] = (entree_clef_t){ "alice", CLEF_CRYPT, pub, priv, true };
    e[1] = (entree_clef_t){ "alice_sign", CLEF_SIGN, pub_s, priv_s, true };
    e[2] = (entree_clef_t){ "bob_pub", CLEF_CRYPT, { 65537, 0xdeadbeefULL }, { 0, 0 }, false };
    e[3] = (entree_clef_t){ "", CLEF_CRYPT, { 0, 0 }, { 0, 0 }, false };   /* rempli apres import */
    table_clefs_t t = { e, 3, 4 };

    /* Le fichier contient exactement "DIYPG_PUBKEY_V3 <type> <E> <N>" en base64 : jamais la clef privee. */
    char *s[] = { "savepub", "alice", "alice_pub.b64" };
    char b64[256], attendu[128];
    VERIF(cmd_savepub(&t, 3, s) == CMD_SUCCES);
    lire_fichier("alice_pub.b64", b64, sizeof b64);
    size_t n = 0;
    unsigned char *clair = base64_decode(b64, strlen(b64), &n);
    snprintf(attendu, sizeof attendu, "DIYPG_PUBKEY_V3 crypt %" PRIu64 " %" PRIu64, pub.E, pub.N);
    VERIF(clair != NULL && n == strlen(attendu) && memcmp(clair, attendu, n) == 0);
    free(clair);
    VERIF(import_pubkey_b64("alice_pub.b64", &importee, type) == 0);
    VERIF(importee.E == pub.E && importee.N == pub.N && strcmp(type, "crypt") == 0);

    /* echange : un contact importe la clef, chiffre pour alice, alice dechiffre */
    e[3] = (entree_clef_t){ "alice_importee", CLEF_CRYPT, importee, { 0, 0 }, false };
    t.nb = 4;
    uint8_t msg[] = "Message pour Alice";
    char *c[] = { "crypt", "alice_importee", "pour_alice.txt", "pour_alice.bin" };
    char *u[] = { "uncrypt", "alice", "pour_alice.bin", "pour_alice_lu.txt" };
    VERIF(ecrire_fichier("pour_alice.txt", msg, sizeof msg - 1));
    VERIF(cmd_crypt(&t, 4, c) == RSA_SUCCES);
    VERIF(cmd_uncrypt(&t, 4, u) == RSA_SUCCES);
    VERIF(fichiers_egaux("pour_alice.txt", "pour_alice_lu.txt"));

    /* clef sign et clef publique seule : exportables aussi */
    char *s_sign[] = { "savepub", "alice_sign", "sign_pub.b64" };
    char *s_pub[] = { "savepub", "bob_pub", "bob_pub.b64" };
    VERIF(cmd_savepub(&t, 3, s_sign) == CMD_SUCCES);
    VERIF(import_pubkey_b64("sign_pub.b64", &importee, type) == 0 && importee.E == pub_s.E && importee.N == pub_s.N);
    VERIF(strcmp(type, "sign") == 0);   /* le type sign est conserve */
    VERIF(cmd_savepub(&t, 3, s_pub) == CMD_SUCCES);
    VERIF(import_pubkey_b64("bob_pub.b64", &importee, type) == 0 && importee.E == 65537 && importee.N == 0xdeadbeefULL);
    VERIF(strcmp(type, "crypt") == 0);

    char *court[] = { "savepub", "alice" }, *trop[] = { "savepub", "alice", "a.b64", "b.b64" };
    char *f_null[] = { "savepub", "alice", NULL }, *inconnu[] = { "savepub", "carole", "c.b64" };
    char *ecriture[] = { "savepub", "alice", "dossier_absent/alice.b64" };
    VERIF(cmd_savepub(NULL, 3, s) == RSA_ERREUR_PARAMETRE);
    VERIF(cmd_savepub(&t, 3, NULL) == RSA_ERREUR_PARAMETRE);
    VERIF(cmd_savepub(&t, 2, court) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_savepub(&t, 4, trop) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_savepub(&t, 3, f_null) == CMD_ERREUR_SYNTAXE);
    VERIF(cmd_savepub(&t, 3, inconnu) == CMD_ERREUR_ID_INCONNU);
    VERIF(cmd_savepub(&t, 3, ecriture) == RSA_ERREUR_FICHIER);
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
    test_crypt_uncrypt();
    test_savepub();
    test_show_affichage();
    base64_cleanup();
    close_log();
    /* stdout est redirige : le bilan va sur la sortie d'erreur. */
    if (echecs == 0) fprintf(stderr, "Phase 3 (Nada) : tous les tests sont passes.\n");
    else fprintf(stderr, "Phase 3 (Nada) : %d echec(s).\n", echecs);
    return echecs == 0 ? 0 : 1;
}
