# analisador de logs - openmp critical
este projeto faz a contagem de acessos de urls em arquivos de log gigantes usando computacao paralela com openmp

1. gerar os dados de teste
primeiro voce precisa criar os arquivos de log e o gabarito. o script vai gerar os textos e compactar em um arquivo zip automaticamente.

```Bash
python3 generate_cdn_data.py
```

2. extrair os arquivos
extraia os logs que estao dentro do pacote zip para a pasta atual.

```Bash
unzip cdn_data_logs.zip
```
3. compilar o codigo
use o gcc com a flag de openmp e otimizacao de desempenho.

```Bash
gcc -O2 -fopenmp analyzer_par_critical.c hash_table.c -o analyzer_par_critical
```

4. rodar o programa
defina a quantidade de nucleos (threads) que quer usar e rode o programa passando o arquivo de log.

```Bash
export OMP_NUM_THREADS=4 && time ./analyzer_par_critical log_distribuido.txt
```

5. validar os resultados
para saber se o codigo contou tudo certo, ordene os arquivos e use o diff para comparar com o gabarito oficial.

```Bash
sort results.csv > sorted_res.csv
sort gabarito_distribuido.csv > sorted_gab.csv
diff -s sorted_res.csv sorted_gab.csv
```
se aparecer a mensagem "files are identical", O codigo esta correto.

6. limpar arquivos temporarios
limpe os executaveis e arquivos gerados.

```Bash
rm analyzer_par_critical results.csv sorted_res.csv sorted_gab.csv
``` 
notas de desempenho:

log_distribuido.txt: baixa disputa entre as threads.

log_concorrente.txt: alta disputa (hotspots), a versao critical deve ficar mais lenta aqu
