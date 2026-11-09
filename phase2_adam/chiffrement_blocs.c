#include "chiffrement_blocs.h"

#ifdef _WIN32
#include <windows.h>
#define TAILLE_NOM_TEMPORAIRE MAX_PATH
#else
#define TAILLE_NOM_TEMPORAIRE 1
#endif

static const uint8_t signature[8] = {'D', 'I', 'Y', 'P', 'G', '2', '\r', '\n'};

static int verifier_cle_blocs(const rsaKey_t *cle)
{
    if (cle == NULL) {
        return RSA_ERREUR_PARAMETRE;
    }
    if (cle->N <= UINT32_MAX || cle->E == 0 || cle->E >= cle->N) {
        return RSA_ERREUR_CLE;
    }
    return RSA_SUCCES;
}

static void encoder_uint64(uint64_t valeur, uint8_t octets[8])
{
    for (size_t i = 0; i < 8; ++i) {
        octets[i] = (uint8_t)(valeur >> (8u * i));
    }
}

static uint64_t decoder_uint64(const uint8_t octets[8])
{
    uint64_t valeur = 0;
    for (size_t i = 0; i < 8; ++i) {
        valeur |= (uint64_t)octets[i] << (8u * i);
    }
    return valeur;
}

static int lire_exactement(FILE *entree, uint8_t *tampon, size_t taille)
{
    if (fread(tampon, 1, taille, entree) != taille) {
        return ferror(entree) ? RSA_ERREUR_FICHIER : RSA_ERREUR_CHIFFRE;
    }
    return RSA_SUCCES;
}

static int appliquer_bloc(rsa_operation_bloc_t operation, uint64_t entree,
                          const rsaKey_t *cle, uint64_t *sortie)
{
    int resultat = operation(entree, cle, sortie);
    if (resultat != RSA_SUCCES) {
        return resultat < 0 ? resultat : RSA_ERREUR_CHIFFRE;
    }
    return *sortie < cle->N ? RSA_SUCCES : RSA_ERREUR_CHIFFRE;
}

static int chiffrer_flux_blocs(FILE *entree, FILE *sortie, const rsaKey_t *cle,
                              rsa_operation_bloc_t operation)
{
    uint8_t entete[RSA_FICHIER_ENTETE_TAILLE] = {0};
    uint64_t longueur = 0;
    size_t taille;

    memcpy(entete, signature, sizeof signature);
    if (fwrite(entete, 1, sizeof entete, sortie) != sizeof entete) {
        return RSA_ERREUR_FICHIER;
    }
    for (;;) {
        uint8_t clair[RSA_BLOC_CLAIR_TAILLE] = {0};
        uint8_t octets_chiffres[RSA_BLOC_CHIFFRE_TAILLE];
        uint64_t valeur = 0, chiffre = 0;
        int resultat;

        taille = fread(clair, 1, sizeof clair, entree);
        if (ferror(entree)) {
            return RSA_ERREUR_FICHIER;
        }
        if (taille == 0) {
            break;
        }
        if (longueur > UINT64_MAX - taille) {
            return RSA_ERREUR_TAILLE;
        }
        longueur += taille;
        for (size_t i = 0; i < sizeof clair; ++i) {
            valeur |= (uint64_t)clair[i] << (8u * i);
        }
        resultat = appliquer_bloc(operation, valeur, cle, &chiffre);
        if (resultat != RSA_SUCCES) {
            return resultat;
        }
        encoder_uint64(chiffre, octets_chiffres);
        if (fwrite(octets_chiffres, 1, sizeof octets_chiffres, sortie) !=
            sizeof octets_chiffres) {
            return RSA_ERREUR_FICHIER;
        }
    }
    encoder_uint64(longueur, entete + sizeof signature);
    /* Seul le temporaire est repositionne, jamais le fichier d'entree. */
    if (fseek(sortie, 0, SEEK_SET) != 0 ||
        fwrite(entete, 1, sizeof entete, sortie) != sizeof entete) {
        return RSA_ERREUR_FICHIER;
    }
    return RSA_SUCCES;
}

static int dechiffrer_flux_blocs(FILE *entree, FILE *sortie, const rsaKey_t *cle,
                                rsa_operation_bloc_t operation)
{
    uint8_t entete[RSA_FICHIER_ENTETE_TAILLE];
    uint64_t longueur, nombre_blocs;
    int resultat = lire_exactement(entree, entete, sizeof entete);

    if (resultat != RSA_SUCCES) {
        return resultat;
    }
    if (memcmp(entete, signature, sizeof signature) != 0) {
        return RSA_ERREUR_CHIFFRE;
    }
    longueur = decoder_uint64(entete + sizeof signature);
    nombre_blocs = longueur / RSA_BLOC_CLAIR_TAILLE +
                   (longueur % RSA_BLOC_CLAIR_TAILLE != 0);
    for (uint64_t i = 0; i < nombre_blocs; ++i) {
        uint8_t octets_chiffres[RSA_BLOC_CHIFFRE_TAILLE];
        uint8_t clair[RSA_BLOC_CLAIR_TAILLE];
        uint64_t chiffre, valeur = 0;
        size_t utiles = RSA_BLOC_CLAIR_TAILLE;

        resultat = lire_exactement(entree, octets_chiffres, sizeof octets_chiffres);
        if (resultat != RSA_SUCCES) {
            return resultat;
        }
        chiffre = decoder_uint64(octets_chiffres);
        if (chiffre >= cle->N) {
            return RSA_ERREUR_CHIFFRE;
        }
        resultat = appliquer_bloc(operation, chiffre, cle, &valeur);
        if (resultat != RSA_SUCCES) {
            return resultat;
        }
        if (valeur > UINT32_MAX) {
            return RSA_ERREUR_CHIFFRE;
        }
        if (i == nombre_blocs - 1 && longueur % RSA_BLOC_CLAIR_TAILLE != 0) {
            utiles = (size_t)(longueur % RSA_BLOC_CLAIR_TAILLE);
            if ((valeur >> (8u * utiles)) != 0) {
                return RSA_ERREUR_CHIFFRE;
            }
        }
        for (size_t j = 0; j < sizeof clair; ++j) {
            clair[j] = (uint8_t)(valeur >> (8u * j));
        }
        if (fwrite(clair, 1, utiles, sortie) != utiles) {
            return RSA_ERREUR_FICHIER;
        }
    }
    /* Un fichier tronque ou suivi de donnees en trop est refuse. */
    if (fgetc(entree) != EOF) {
        return RSA_ERREUR_CHIFFRE;
    }
    return ferror(entree) ? RSA_ERREUR_FICHIER : RSA_SUCCES;
}

static FILE *creer_temporaire_blocs(char *nom)
{
#ifdef _WIN32
    char dossier[MAX_PATH];
    DWORD taille = GetTempPathA(MAX_PATH, dossier);
    FILE *fichier;

    if (taille == 0 || taille >= MAX_PATH ||
        GetTempFileNameA(dossier, "blc", 0, nom) == 0) {
        return NULL;
    }
    fichier = fopen(nom, "wb+");
    if (fichier == NULL) {
        remove(nom);
        nom[0] = '\0';
    }
    return fichier;
#else
    nom[0] = '\0';
    return tmpfile();
#endif
}

static int copier_resultat(FILE *temporaire, const char *chemin)
{
    FILE *sortie = fopen(chemin, "wb");
    uint8_t tampon[4096];
    size_t taille;
    int resultat = RSA_SUCCES;

    if (sortie == NULL) {
        return RSA_ERREUR_FICHIER;
    }
    while ((taille = fread(tampon, 1, sizeof tampon, temporaire)) != 0) {
        if (fwrite(tampon, 1, taille, sortie) != taille) {
            resultat = RSA_ERREUR_FICHIER;
            break;
        }
    }
    if (ferror(temporaire) || ferror(sortie)) {
        resultat = RSA_ERREUR_FICHIER;
    }
    if (fclose(sortie) != 0) {
        resultat = RSA_ERREUR_FICHIER;
    }
    return resultat;
}

typedef int (*transformation_blocs_t)(FILE *, FILE *, const rsaKey_t *,
                                     rsa_operation_bloc_t);

static int transformer_fichier_blocs(const char *chemin_entree,
                                     const char *chemin_sortie,
                                     const rsaKey_t *cle,
                                     rsa_operation_bloc_t operation,
                                     transformation_blocs_t transformer)
{
    FILE *entree, *temporaire;
    char nom[TAILLE_NOM_TEMPORAIRE] = {0};
    int resultat;

    if (chemin_entree == NULL || chemin_sortie == NULL || operation == NULL ||
        chemin_entree[0] == '\0' || chemin_sortie[0] == '\0') {
        return RSA_ERREUR_PARAMETRE;
    }
    resultat = verifier_cle_blocs(cle);
    if (resultat != RSA_SUCCES) {
        return resultat;
    }
    entree = fopen(chemin_entree, "rb");
    if (entree == NULL) {
        return RSA_ERREUR_FICHIER;
    }
    temporaire = creer_temporaire_blocs(nom);
    if (temporaire == NULL) {
        fclose(entree);
        return RSA_ERREUR_FICHIER;
    }
    resultat = transformer(entree, temporaire, cle, operation);
    if (fclose(entree) != 0 && resultat == RSA_SUCCES) {
        resultat = RSA_ERREUR_FICHIER;
    }
    if (resultat == RSA_SUCCES &&
        (fflush(temporaire) != 0 || fseek(temporaire, 0, SEEK_SET) != 0)) {
        resultat = RSA_ERREUR_FICHIER;
    }
    if (resultat == RSA_SUCCES) {
        resultat = copier_resultat(temporaire, chemin_sortie);
    }
    if (fclose(temporaire) != 0 && resultat == RSA_SUCCES) {
        resultat = RSA_ERREUR_FICHIER;
    }
    if (nom[0] != '\0' && remove(nom) != 0 && resultat == RSA_SUCCES) {
        resultat = RSA_ERREUR_FICHIER;
    }
    return resultat;
}

int chiffrer_fichier_blocs(const char *entree, const char *sortie,
                          const rsaKey_t *cle, rsa_operation_bloc_t chiffrer)
{
    return transformer_fichier_blocs(entree, sortie, cle, chiffrer,
                                     chiffrer_flux_blocs);
}

int dechiffrer_fichier_blocs(const char *entree, const char *sortie,
                            const rsaKey_t *cle, rsa_operation_bloc_t dechiffrer)
{
    return transformer_fichier_blocs(entree, sortie, cle, dechiffrer,
                                     dechiffrer_flux_blocs);
}
