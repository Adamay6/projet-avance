/// \file keys_io.c
/// \author Nada ARABA
/// \brief Phase 1.0 : génération, affichage hexadécimal, sauvegarde et chargement des clefs RSA
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>
#include "../rsa_common_header.h"
#include "phase1.h"

/// Format du fichier de clefs (texte, lisible à l'oeil, valeurs en hexadécimal) :
///     DIYPG-KEYPAIR 1
///     PUB  <E> <N>
///     PRIV <E> <N>
/// Le "1" est un numéro de version : si on change le format plus tard (phase 3 : sauvegarde
/// de tout l'annuaire), on pourra encore lire les anciens fichiers.

void print_key_hex(const char *label, const rsaKey_t *key){
  /// \brief affiche une clef en hexadécimal, ex :  clef publique : E = 0x3  N = 0x1a2b3c
  printf("%s : E = 0x%" PRIx64 "  N = 0x%" PRIx64 "\n", label, key->E, key->N);
}

void print_keypair_hex(const keyPair_t *kp){
  /// \brief affiche la paire de clefs mais affiche aussi la clef privée
  /// \brief à ne servir que pour les tests / la commande "show" de l'interpréteur.
  print_key_hex("clef publique", &kp->pubKey);
  print_key_hex("clef privee  ", &kp->privKey);
}

int keypair_valide(const keyPair_t *kp){
  if (kp==NULL || kp->pubKey.N<=255 || kp->pubKey.N>UINT64_C(4294967296) ||
      kp->pubKey.N!=kp->privKey.N || kp->pubKey.E==0 || kp->privKey.E==0 ||
      kp->pubKey.E>=kp->pubKey.N || kp->privKey.E>=kp->pubKey.N)
    return 0;
  for (unsigned int octet=0; octet<256; octet++){
    uint64_t c = puissance_mod_n(octet,kp->pubKey.E,kp->pubKey.N);
    if (puissance_mod_n(c,kp->privKey.E,kp->privKey.N)!=octet)
      return 0;
  }
  return 1;
}

int gen_keypair(keyPair_t *kp, uint64_t max_prime){
  /// \brief génère une paire de clefs (petite enveloppe autour de genKeysRabin du code fourni)
  if (kp==NULL || max_prime<20 || max_prime>MAX_PRIME_PHASE1){
    errno=EINVAL;
    return -1;
  }
  genKeysRabin(&kp->pubKey, &kp->privKey, max_prime);
  return keypair_valide(kp) ? 0 : -1;
}

int save_keypair(const char *path, const keyPair_t *kp){
  /// \brief écrit la paire de clefs dans un fichier texte
  /// \returns 0 si tout s'est bien passé, -1 sinon
  if (path==NULL || path[0]=='\0' || !keypair_valide(kp))
    return -1;
  FILE *f = fopen(path, "w");
  if (f == NULL)
    return -1;
  int ok = fprintf(f, "DIYPG-KEYPAIR 1\n") >= 0 &&
    fprintf(f, "PUB %" PRIx64 " %" PRIx64 "\n",  kp->pubKey.E,  kp->pubKey.N) >= 0 &&
    fprintf(f, "PRIV %" PRIx64 " %" PRIx64 "\n", kp->privKey.E, kp->privKey.N) >= 0;
  if (ferror(f)) ok=0;
  if (fclose(f) != 0)     // fclose peut échouer (disque plein...) : on le vérifie
    return -1;
  if (!ok) return -1;
  // Le fichier contient la clef privée : lecture / écriture réservées à l'utilisateur (rw-------)
  if (chmod(path, S_IRUSR | S_IWUSR) != 0) return -1;
  return 0;
}

static int lire_hex(const char *texte, uint64_t *valeur){
  char *fin;
  if (texte[0]=='\0' || texte[0]=='-' || texte[0]=='+') return 0;
  errno=0;
  unsigned long long n = strtoull(texte,&fin,16);
  if (errno==ERANGE || fin==texte || *fin!='\0') return 0;
  *valeur=(uint64_t)n;
  return 1;
}

int load_keypair(const char *path, keyPair_t *kp){
  /// \brief relit une paire de clefs sauvegardée par save_keypair
  /// \returns 0 si ok, -1 si fichier absent ou mal formé. kp n'est modifié QUE en cas de succès.
  if (path==NULL || path[0]=='\0' || kp==NULL) return -1;
  FILE *f = fopen(path, "r");
  if (f == NULL)
    return -1;

  char tag[32], nom1[8], nom2[8];
  char version[8], e_pub[32], n_pub[32], e_priv[32], n_priv[32];
  keyPair_t tmp;
  int ok =
    fscanf(f, "%31s %7s", tag, version) == 2 &&
    strcmp(tag, "DIYPG-KEYPAIR") == 0 && strcmp(version,"1") == 0 &&
    fscanf(f, "%7s %31s %31s", nom1, e_pub, n_pub) == 3 &&
    strcmp(nom1, "PUB") == 0 &&
    fscanf(f, "%7s %31s %31s", nom2, e_priv, n_priv) == 3 &&
    strcmp(nom2, "PRIV") == 0 &&
    lire_hex(e_pub,&tmp.pubKey.E) && lire_hex(n_pub,&tmp.pubKey.N) &&
    lire_hex(e_priv,&tmp.privKey.E) && lire_hex(n_priv,&tmp.privKey.N);
  if (ok){
    int c;
    do { c=fgetc(f); } while (c!=EOF && isspace((unsigned char)c));
    if (c!=EOF) ok=0;
  }
  if (ferror(f)) ok=0;
  if (fclose(f)!=0) ok=0;

  // Un fichier mal formé ou rempli de zéros ne doit pas donner de "clefs" silencieusement
  if (!ok || !keypair_valide(&tmp))
    return -1;

  *kp = tmp;
  return 0;
}
