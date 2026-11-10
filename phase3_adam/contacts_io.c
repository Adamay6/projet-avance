#include "contacts_interne.h"

static const uint8_t signature_contacts[8] = {'D','Y','C','O','N','T','0','1'};

static int ecrire_nombre(FILE *f, uint64_t valeur, size_t taille)
{
    uint8_t octets[8];
    for (size_t i = 0; i < taille; ++i) octets[i] = (uint8_t)(valeur >> (8u * i));
    return fwrite(octets, 1, taille, f) == taille ? 0 : CONTACTS_ERREUR_FICHIER;
}

static int lire_nombre(FILE *f, uint64_t *valeur, size_t taille)
{
    uint8_t octets[8];
    if (fread(octets, 1, taille, f) != taille)
        return ferror(f) ? CONTACTS_ERREUR_FICHIER : CONTACTS_ERREUR_FORMAT;
    *valeur = 0;
    for (size_t i = 0; i < taille; ++i) *valeur |= (uint64_t)octets[i] << (8u * i);
    return 0;
}

static int ecrire_texte(FILE *f, const char *texte)
{
    size_t taille = strlen(texte);
    if (ecrire_nombre(f, taille, 2) != 0 ||
        fwrite(texte, 1, taille, f) != taille) return CONTACTS_ERREUR_FICHIER;
    return 0;
}

static int lire_texte(FILE *f, char *texte, size_t capacite)
{
    uint64_t taille;
    int resultat = lire_nombre(f, &taille, 2);
    if (resultat != 0) return resultat;
    if (taille >= capacite) return CONTACTS_ERREUR_FORMAT;
    if (fread(texte, 1, (size_t)taille, f) != taille)
        return ferror(f) ? CONTACTS_ERREUR_FICHIER : CONTACTS_ERREUR_FORMAT;
    if (memchr(texte, '\0', (size_t)taille) != NULL) return CONTACTS_ERREUR_FORMAT;
    texte[taille] = '\0';
    return 0;
}

int contacts_sauver(FILE *sortie, const annuaire_contacts_t *annuaire)
{
    size_t nombre;
    if (sortie == NULL || annuaire == NULL) return CONTACTS_ERREUR_PARAMETRE;
    nombre = contacts_nombre(annuaire);
    if (nombre > UINT32_MAX) return CONTACTS_ERREUR_FORMAT;
    if (fwrite(signature_contacts, 1, sizeof signature_contacts, sortie) !=
        sizeof signature_contacts || ecrire_nombre(sortie, nombre, 4) != 0)
        return CONTACTS_ERREUR_FICHIER;
    for (contact_t *p = annuaire->premier; p != NULL; p = p->suivant) {
        size_t nombre_cles = 0;
        for (contact_cle_noeud_t *cle = p->cles; cle != NULL; cle = cle->suivant)
            ++nombre_cles;
        if (nombre_cles > UINT32_MAX) return CONTACTS_ERREUR_FORMAT;
        if (ecrire_texte(sortie, p->infos.identifiant) != 0 ||
            ecrire_texte(sortie, p->infos.nom) != 0 ||
            ecrire_texte(sortie, p->infos.prenom) != 0 ||
            ecrire_texte(sortie, p->infos.commentaire) != 0 ||
            ecrire_nombre(sortie, nombre_cles, 4) != 0) return CONTACTS_ERREUR_FICHIER;
        for (contact_cle_noeud_t *cle = p->cles; cle != NULL; cle = cle->suivant) {
            if (ecrire_texte(sortie, cle->infos.identifiant) != 0 ||
                ecrire_nombre(sortie, cle->infos.type, 1) != 0 ||
                ecrire_nombre(sortie, cle->infos.publique.E, 8) != 0 ||
                ecrire_nombre(sortie, cle->infos.publique.N, 8) != 0)
                return CONTACTS_ERREUR_FICHIER;
        }
    }
    return ferror(sortie) ? CONTACTS_ERREUR_FICHIER : 0;
}

static int lire_contact(FILE *entree, annuaire_contacts_t *temporaire)
{
    contact_infos_t infos;
    uint64_t nombre_cles;
    int resultat;
    if ((resultat = lire_texte(entree, infos.identifiant, sizeof infos.identifiant)) != 0 ||
        (resultat = lire_texte(entree, infos.nom, sizeof infos.nom)) != 0 ||
        (resultat = lire_texte(entree, infos.prenom, sizeof infos.prenom)) != 0 ||
        (resultat = lire_texte(entree, infos.commentaire, sizeof infos.commentaire)) != 0 ||
        (resultat = lire_nombre(entree, &nombre_cles, 4)) != 0) return resultat;
    resultat = contacts_ajouter(temporaire, infos.identifiant, infos.nom,
                               infos.prenom, infos.commentaire);
    if (resultat != 0)
        return resultat == CONTACTS_ERREUR_MEMOIRE ? resultat : CONTACTS_ERREUR_FORMAT;
    for (uint64_t i = 0; i < nombre_cles; ++i) {
        contact_cle_t cle;
        uint64_t type;
        if ((resultat = lire_texte(entree, cle.identifiant, sizeof cle.identifiant)) != 0 ||
            (resultat = lire_nombre(entree, &type, 1)) != 0 ||
            (resultat = lire_nombre(entree, &cle.publique.E, 8)) != 0 ||
            (resultat = lire_nombre(entree, &cle.publique.N, 8)) != 0) return resultat;
        if (type != CONTACT_CRYPT && type != CONTACT_SIGN) return CONTACTS_ERREUR_FORMAT;
        resultat = contacts_ajouter_cle(temporaire, infos.identifiant, cle.identifiant,
                                       (contact_type_cle_t)type, &cle.publique);
        if (resultat != 0)
            return resultat == CONTACTS_ERREUR_MEMOIRE ? resultat : CONTACTS_ERREUR_FORMAT;
    }
    return 0;
}

int contacts_charger(FILE *entree, annuaire_contacts_t *annuaire)
{
    uint8_t signature[8];
    uint64_t nombre;
    annuaire_contacts_t temporaire = {NULL};
    int resultat;
    if (entree == NULL || annuaire == NULL) return CONTACTS_ERREUR_PARAMETRE;
    if (fread(signature, 1, sizeof signature, entree) != sizeof signature)
        return ferror(entree) ? CONTACTS_ERREUR_FICHIER : CONTACTS_ERREUR_FORMAT;
    if (memcmp(signature, signature_contacts, sizeof signature) != 0)
        return CONTACTS_ERREUR_FORMAT;
    resultat = lire_nombre(entree, &nombre, 4);
    if (resultat != 0) return resultat;
    for (uint64_t i = 0; i < nombre; ++i) {
        resultat = lire_contact(entree, &temporaire);
        if (resultat != 0) break;
    }
    if (ferror(entree)) resultat = CONTACTS_ERREUR_FICHIER;
    if (resultat == 0) {
        contacts_liberer(annuaire);
        *annuaire = temporaire;
    } else {
        contacts_liberer(&temporaire);
    }
    return resultat;
}
