/// \file table_clefs_provisoire.c
/// \author Nada ARABA
/// \brief Version provisoire de table_chercher, pour tester les commandes de Nada
///        en attendant le module de table de Loann. A supprimer quand il sera integre.
#include <string.h>
#include "interpreteur.h"

const entree_clef_t *table_chercher(const table_clefs_t *table, const char *id) {
    if (table == NULL || id == NULL || (table->entrees == NULL && table->nb > 0)) return NULL;
    for (size_t i = 0; i < table->nb; i++) {
        if (strcmp(table->entrees[i].id, id) == 0) return &table->entrees[i];
    }
    return NULL;
}
