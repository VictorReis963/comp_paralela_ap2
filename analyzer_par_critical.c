#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "hash_table.h"

#define MAX_LINE 1024
#define HASH_SIZE 131071 
#define MAX_LOG_LINES 10000000 

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_de_log>\n", argv[0]);
        return 1;
    }

    HashTable *ht = ht_create(HASH_SIZE);

    // Fase 1 - construção sequencial do manifest
    FILE *mf = fopen("manifest.txt", "r");
    if (!mf) {
        perror("Erro ao abrir manifest.txt");
        return 1;
    }

    char line[MAX_LINE];
    while (fgets(line, MAX_LINE, mf)) {
        line[strcspn(line, "\r\n")] = 0; 
        if (strlen(line) > 0) {
            ht_insert(ht, line);
        }
    }
    fclose(mf);

    // carregamento do Log para memoria
    FILE *lf = fopen(argv[1], "r");
    if (!lf) {
        perror("Erro ao abrir arquivo de log");
        return 1;
    }

    char **logs = malloc(MAX_LOG_LINES * sizeof(char*));
    int count = 0;
    while (count < MAX_LOG_LINES && fgets(line, MAX_LINE, lf)) {
        logs[count++] = strdup(line);
    }
    fclose(lf);

    // fase 2 - Processamento paralelo com critical
    #pragma omp parallel for
    for (int i = 0; i < count; i++) {
        char *start = strstr(logs[i], "GET ");
        if (start) {
            start += 4; 
            char *end = strchr(start, ' '); 
            if (end) {
                int url_len = end - start;
                char url_buffer[MAX_LINE];
                strncpy(url_buffer, start, url_len);
                url_buffer[url_len] = '\0';

                CacheNode *node = ht_get(ht, url_buffer);
                if (node) {
                    #pragma omp critical
                    {
                        node->hit_count++;
                    }
                }
            }
        }
    }


    ht_save_results(ht, "results.csv");

    // Limpeza
    for(int i = 0; i < count; i++) free(logs[i]);
    free(logs);

    return 0;
}
