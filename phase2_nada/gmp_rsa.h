/// \file gmp_rsa.h
/// \author Nada ARABA
/// \brief Phase 2 : RSA avec GMP -- generation de clefs 64 bits et chiffrement d'un bloc de 4 octets.
///
/// Pourquoi GMP ? Un bloc de 4 octets vaut jusqu'a 2^32 - 1, donc il faut N > 2^32.
/// Dans cette zone, a*a deborde en uint64 (puissance_mod_n de la phase 1 devient fausse).
/// Les clefs restent stockees dans rsaKey_t (E et N sur 64 bits) ; GMP ne sert qu'aux calculs.
#ifndef GMP_RSA_H
#define GMP_RSA_H

#include <stdint.h>
#include <gmp.h>
#include "convert_bytes.h"
#include "../phase1_adam/chiffrement.h"   /* rsaKey_t, BLOCK_SIZE, rsa_resultat_t */

/* Un bloc clair de 4 octets (lu en little endian, comme le demande le sujet) devient un entier < N, stocke sur 8 octets. */
#define RSA_BLOC_CHIFFRE_TAILLE 8u
#define RSA_BITS_PREMIER_MIN 17u   /* p, q > 2^16  =>  N > 2^32 */
#define RSA_BITS_PREMIER_MAX 31u   /* p, q < 2^32  =>  N < 2^64 */

/* Signature imposee par le cahier des charges : res = a^e mod n (mpz_powm). Si n == 0, res = 0. */
void puissance_mod_n_gmp(mpz_t res, uint64_t a, uint64_t e, uint64_t n);

/* Meme calcul, resultat rendu en uint64_t (utile car n < 2^64 donc le resultat tient sur 64 bits). */
uint64_t puissance_mod_n_gmp_u64(uint64_t a, uint64_t e, uint64_t n);

/*
 * Genere une paire de clefs RSA dont N > 2^32 (donc adaptee aux blocs de 4 octets).
 * bits_premier : taille de p et q en bits, entre 17 et 31 ; E = 65537 (ou le suivant premier avec phi).
 * pub = (E, N), priv = (D, N).
 * Retourne RSA_SUCCES, RSA_ERREUR_PARAMETRE (pointeurs NULL / bits hors bornes).
 * Le generateur aleatoire n'est PAS cryptographique (projet pedagogique).
 */
int gen_keys_gmp(rsaKey_t *pub, rsaKey_t *priv, unsigned bits_premier);

/* Valide une clef pour les blocs de 4 octets : 0 < E < N et N > 2^32 - 1. */
int cle_bloc_valide(const rsaKey_t *cle);

/* Chiffre un bloc de 4 octets. Retourne RSA_SUCCES, RSA_ERREUR_PARAMETRE ou RSA_ERREUR_CLE. */
int chiffrer_bloc_gmp(const uint8_t bloc[BLOCK_SIZE], const rsaKey_t *cle, uint64_t *chiffre);

/*
 * Dechiffre un bloc. RSA_ERREUR_CHIFFRE si chiffre >= N ou si le resultat depasse 4 octets
 * (mauvaise clef ou chiffre altere). Le bloc de sortie n'est modifie qu'en cas de succes.
 */
int dechiffrer_bloc_gmp(uint64_t chiffre, const rsaKey_t *cle, uint8_t bloc[BLOCK_SIZE]);

#endif
