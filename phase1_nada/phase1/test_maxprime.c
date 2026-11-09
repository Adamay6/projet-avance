/// \file test_maxprime.c
/// \author Nada ARABA
/// \brief Phase 1.0, question 6 : à partir de quelle valeur de MAX_PRIME les uint64 débordent-ils ?
///
/// Utilisation :   ./test_maxprime <max_prime> <nombre_de_paires>
/// Exemple     :   ./test_maxprime 70000 2000
///
/// Pour chaque paire de clefs, on chiffre puis déchiffre les 256 valeurs d'un octet.
/// Avec des clefs valides, un aller-retour faux met en evidence un probleme de calcul.
/// Dans puissance_mod_n, on calcule a*a avant le "% n" : ça tient dans un uint64 tant que
/// a < 2^32, donc tant que N = p*q reste inférieur à 2^32 (= 4 294 967 296).
#include "../rsa_common_header.h"
#include "phase1.h"
#include <errno.h>
#include <limits.h>
#include "../../console_utf8.h"

static int argument(const char *texte, uint64_t min, uint64_t max, uint64_t *n){
  char *fin;
  if (texte[0]<'0' || texte[0]>'9') return 0;
  errno=0;
  unsigned long long valeur=strtoull(texte,&fin,10);
  if (errno==ERANGE || *fin!='\0' || valeur<min || valeur>max) return 0;
  *n=(uint64_t)valeur;
  return 1;
}

int main(int argc, char *argv[]){
  initialiser_console_utf8();
  uint64_t max_prime, nombre, graine=(unsigned)time(NULL);
  if ((argc!=3 && argc!=4) || !argument(argv[1],20,MAX_PRIME_ETUDE,&max_prime) ||
      !argument(argv[2],1,1000000,&nombre) ||
      (argc==4 && !argument(argv[3],0,UINT_MAX,&graine))){
    fprintf(stderr, "usage : %s <max_prime entre 20 et 5000000> <nombre_de_paires> [graine]\n", argv[0]);
    return 2;
  }
  int nb_paires = (int)nombre;

  srand((unsigned) graine);
  init_log();

  uint64_t n_max = 0;
  int paires_n_trop_grand = 0;   // paires où N > 2^32 (zone à risque)
  int paires_en_echec = 0;       // paires où au moins un octet ne se déchiffre pas bien

  for (int i = 0; i < nb_paires; i++){
    keyPair_t k;
    // L'etude depasse volontairement la borne du chiffrement courant.
    genKeysRabin(&k.pubKey, &k.privKey, max_prime);
    if (k.pubKey.N > n_max) n_max = k.pubKey.N;
    if (k.pubKey.N > ((uint64_t)1 << 32)) paires_n_trop_grand++;

    for (int octet = 0; octet < 256; octet++){
      uint64_t c = puissance_mod_n(octet, k.pubKey.E,  k.pubKey.N);
      uint64_t m = puissance_mod_n(c,     k.privKey.E, k.privKey.N);
      if (m != (uint64_t) octet){
        paires_en_echec++;
        break;
      }
    }
  }

  printf("MAX_PRIME=%-10" PRIu64 " paires=%-5d N_max=%-20" PRIu64 " paires_avec_N>2^32=%-5d paires_en_echec=%d\n",
         max_prime, nb_paires, n_max, paires_n_trop_grand, paires_en_echec);
  close_log();
  return 0;
}
