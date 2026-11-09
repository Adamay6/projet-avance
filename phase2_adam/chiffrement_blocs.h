#ifndef CHIFFREMENT_BLOCS_ADAM_H
#define CHIFFREMENT_BLOCS_ADAM_H

#include "../phase1_adam/chiffrement.h"

#define RSA_BLOC_CLAIR_TAILLE 4u
#define RSA_BLOC_CHIFFRE_TAILLE 8u
#define RSA_FICHIER_ENTETE_TAILLE 16u

/*
 * Fonction de calcul d'un bloc.
 * Elle transforme l'entier avec la cle passee en parametre.
 * Elle renvoie RSA_SUCCES et remplit sortie, ou un code d'erreur negatif.
 * Le calcul RSA reste dans le module de blocs.
 */
typedef int (*rsa_operation_bloc_t)(uint64_t entree, const rsaKey_t *cle,
                                   uint64_t *sortie);

/*
 * Format : "DIYPG2\r\n", longueur claire sur 8 octets little endian,
 * puis un uint64 little endian pour chaque bloc clair de 4 octets.
 * Le dernier bloc est complete par des zeros ; la longueur est conservee.
 *
 * La cle doit verifier N > UINT32_MAX et 0 < E < N.
 * Le resultat est prepare dans un temporaire avant d'ouvrir la destination.
 * Une erreur de format ou de calcul conserve donc la destination existante.
 * Une erreur pendant la copie finale peut laisser un fichier partiel.
 * Les chemins d'entree et de sortie peuvent designer le meme fichier.
 */
int chiffrer_fichier_blocs(const char *entree, const char *sortie,
                          const rsaKey_t *cle, rsa_operation_bloc_t chiffrer);
int dechiffrer_fichier_blocs(const char *entree, const char *sortie,
                            const rsaKey_t *cle, rsa_operation_bloc_t dechiffrer);

/* Mêmes fonctions avec le calcul GMP de la phase 2. */
int chiffrer_fichier_blocs_gmp(const char *entree, const char *sortie,
                              const rsaKey_t *cle);
int dechiffrer_fichier_blocs_gmp(const char *entree, const char *sortie,
                                const rsaKey_t *cle);

#endif
