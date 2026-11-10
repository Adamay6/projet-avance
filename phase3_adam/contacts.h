#ifndef CONTACTS_ADAM_H
#define CONTACTS_ADAM_H

#include "../phase1_nada/rsa_common_header.h"

#define CONTACT_TEXTE_TAILLE 128u
#define CONTACT_COMMENTAIRE_TAILLE 512u

typedef enum { CONTACT_CRYPT = 1, CONTACT_SIGN = 2 } contact_type_cle_t;
typedef enum {
    CONTACTS_SUCCES = 0,
    CONTACTS_COMMANDE_INCONNUE = 1,
    CONTACTS_ERREUR_PARAMETRE = -1,
    CONTACTS_ERREUR_IDENTIFIANT = -2,
    CONTACTS_ERREUR_INTROUVABLE = -3,
    CONTACTS_ERREUR_DOUBLON = -4,
    CONTACTS_ERREUR_MEMOIRE = -5,
    CONTACTS_ERREUR_FICHIER = -6,
    CONTACTS_ERREUR_FORMAT = -7,
    CONTACTS_ERREUR_TYPE = -8,
    CONTACTS_ERREUR_AMBIGU = -9,
    CONTACTS_ERREUR_SAISIE = -10,
    CONTACTS_ERREUR_PRIVEE = -11
} contacts_resultat_t;

typedef struct {
    char identifiant[NAME_MAX_SIZE + 1];
    char nom[CONTACT_TEXTE_TAILLE];
    char prenom[CONTACT_TEXTE_TAILLE];
    char commentaire[CONTACT_COMMENTAIRE_TAILLE];
} contact_infos_t;

typedef struct {
    char identifiant[NAME_MAX_SIZE + 1];
    contact_type_cle_t type;
    rsaKey_t publique;
} contact_cle_t;

typedef struct contact_s contact_t;
typedef struct { contact_t *premier; } annuaire_contacts_t;

/* L'annuaire doit etre initialise avant usage et libere a la fin. */
void contacts_initialiser(annuaire_contacts_t *annuaire);
void contacts_liberer(annuaire_contacts_t *annuaire);
size_t contacts_nombre(const annuaire_contacts_t *annuaire);

/* Identifiants : 1 a NAME_MAX_SIZE octets, sans espace, controle ou '/'. */
int contacts_ajouter(annuaire_contacts_t *annuaire, const char *id,
                    const char *nom, const char *prenom, const char *commentaire);
int contacts_modifier(annuaire_contacts_t *annuaire, const char *id,
                     const char *nom, const char *prenom, const char *commentaire);
int contacts_supprimer(annuaire_contacts_t *annuaire, const char *id);
/* Le pointeur renvoye appartient a l'annuaire. */
/* Une modification ou une suppression demande une nouvelle recherche. */
const contact_infos_t *contacts_trouver(const annuaire_contacts_t *annuaire,
                                       const char *id);

/* Un nom de famille doit identifier un seul contact ; l'identifiant est prioritaire. */
int contacts_ajouter_cle(annuaire_contacts_t *annuaire, const char *contact_ou_nom,
                        const char *id_cle, contact_type_cle_t type,
                        const rsaKey_t *publique);
int contacts_supprimer_cle(annuaire_contacts_t *annuaire, const char *reference);
/* Les sorties sont laissees intactes en cas d'erreur. */
int contacts_lire_cle(const annuaire_contacts_t *annuaire, const char *reference,
                     contact_cle_t *cle);
int contacts_resoudre(const annuaire_contacts_t *annuaire, const char *reference,
                     contact_type_cle_t type_attendu, rsaKey_t *publique);
int contacts_afficher(const annuaire_contacts_t *annuaire, const char *filtre,
                      const char *nom, FILE *sortie);

/*
 * Section binaire a integrer au save/load de Loann. Les flux restent ouverts.
 * Charger remplace l'annuaire seulement si toute la section est valide.
 * La protection du fichier complet appartient au save/load de l'interpreteur.
 */
int contacts_sauver(FILE *sortie, const annuaire_contacts_t *annuaire);
int contacts_charger(FILE *entree, annuaire_contacts_t *annuaire);

/*
 * Raccordement a l'interpreteur, apres son decoupage de la ligne :
 * commande est le nom ; argc/argv contiennent seulement ses arguments.
 * Gere les commandes V2 et rmkeys/show pour une reference contact/clef.
 * Les autres commandes renvoient CONTACTS_COMMANDE_INCONNUE (1).
 * Les menus utilisent les flux fournis, ce qui permet de rejouer un script.
 */
int contacts_executer(annuaire_contacts_t *annuaire, const char *commande,
                     int argc, const char *const argv[], FILE *entree, FILE *sortie);
const char *contacts_message_erreur(int resultat);

#endif
