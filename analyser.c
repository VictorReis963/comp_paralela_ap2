#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "hash_table.h"

#define MAX_URL_LEN 1024
#define MAX_LOG_LINES 1000000 // Ajuste conforme o tamanho do log

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_de_log>\n", argv[0]);
        return 1;
    }

    // --- FASE 1: CONSTRUÇAO DA TABELA (SEQUENCIAL) ---
    HashTable* ht = ht_create(131071);
    FILE* f_manifest = fopen("manifest.txt", "r");
    if (!f_manifest) { perror("Erro manifest.txt"); return 1; }

    char buffer[MAX_URL_LEN];
    while (fgets(buffer, sizeof(buffer), f_manifest)) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        ht_insert(ht, buffer); // Insere URL com hit_count inicial 0
    }
    fclose(f_manifest);

    // --- CARREGAMENTO DO LOG PARA A MEMoRIA ---
    // Precisa disso para que o OpenMP possa distribuir o indice i do loop
    char **log_data = malloc(MAX_LOG_LINES * sizeof(char*));
    int total_lines = 0;

    FILE* f_log = fopen(argv[1], "r");
    if (!f_log) { perror("Erro log"); return 1; }

    while (fgets(buffer, sizeof(buffer), f_log) && total_lines < MAX_LOG_LINES) {
        buffer[strcspn(buffer, "\r\n")] = 0;
        log_data[total_lines] = strdup(buffer);
        total_lines++;
    }
    fclose(f_log);

    // --- FASE 2: PROCESSAMENTO PARALELO (CRITICAL) ---
    // Iniciam a medição de tempo aqui para o relatorio
    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < total_lines; i++) {
        // Busca o no na tabela hash (operação de leitura, pode ser paralela)
        CacheNode* node = ht_get(ht, log_data[i]);

        if (node) {
            // Sincronizaçao: Apenas uma thread por vez entra aqui
            #pragma omp critical
            {
                node->hit_count++;
            }
        }
    }

    double end_time = omp_get_wtime();
    printf("Tempo de processamento: %f segundos\n", end_time - start_time);

    // --- FASE 3: RESULTADOS E LIMPEZA ---
    ht_save_results(ht, "results.csv");

    // Liberar memoria do log e da tabela
    for(int i=0; i<total_lines; i++) free(log_data[i]);
    free(log_data);
    ht_destroy(ht);

    return 0;
}
