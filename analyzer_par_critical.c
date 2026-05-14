#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "hash_table.h"

#define MAX_LINE 1024
#define HASH_SIZE 131071 

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;

    HashTable *ht = ht_create(HASH_SIZE);

    // part 1: construçao sequencial do MAnifest
    FILE *mf = fopen("manifest.txt", "r");
    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, mf)) {
        line[strcspn(line, "\n")] = 0;
        ht_insert(ht, line);
    }
    fclose(mf);

    // carregamento do Log para ,emoria 
    FILE *lf = fopen(argv[1], "r");
    char **logs = malloc(10000000 * sizeof(char*)); // ajjustar conforme necessário
    int count = 0;
    while (fgets(line, MAX_LINE, lf)) {
        logs[count++] = strdup(line);
    }
    fclose(lf);

    // part 2: processamento paralelo com critical
    #pragma omp parallel for
    for (int i = 0; i < count; i++) {
        char url[MAX_LINE];
        // extraçao simplificada da url 
        if (sscanf(logs[i], "%*s - [%*[^]]] %*d \"GET %s", url) == 1) {
            CacheNode *node = ht_get(ht, url);
            if (node) {
                #pragma omp critical 
                {
                    node->hit_count++;
                }
            }
        }
    }

    // Salva resultados para validar
    ht_save_results(ht, "results.csv");

    return 0;
}
