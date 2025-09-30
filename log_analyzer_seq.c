/*
    Guilherme Teodoro de Oliveira RA: 10425362
    Luís Henrique Ribeiro Fernandes RA: 10420079
    Vinícius Brait Lorimier - 10420046
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_FILE "access_log_large.txt"

// Extração dos status code e os bytes de uma linha
void parse_line(const char *line, int *status_code, long long *bytes_sent){
    const char *quote_ptr = strstr(line, "\" ");

    if(quote_ptr){
        // sscanf para extrair os dois números após aspas e o espaço
        if (sscanf(quote_ptr + 2, "%d %lld", status_code, bytes_sent) != 2) {
            *status_code = 0;
            *bytes_sent = 0;
        }
    }else{
        *status_code = 0;
        *bytes_sent = 0;
    }
}

int main(){
    long long total_bytes = 0;
    long long errors_404 = 0;

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Abre o arquivo de log
    FILE *file = fopen(LOG_FILE, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo de log!\n");
        return 1;
    }

    // Medição de tempo de execução
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Leitura do arquivo de log
    while((read = getline(&line, &len, file)) != -1){
        int status_code;
        long long bytes_sent;
        
        parse_line(line, &status_code, &bytes_sent); 

        // Contagem dos status code
        if (status_code == 404) {
            errors_404++;
        } else if (status_code == 200) {
            total_bytes += bytes_sent;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calcula o tempo de execução em segundos
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Exibe todos os dados finais coletados
    printf("========================================================================\n");
    printf("--- Versão Sequencial ---\n");
    printf("Resultados Finais: \n");
    printf("- Tempo de execução: %.4f segundos\n", time_spent);
    printf("- Total de erros (códigos 404): %lld\n", errors_404);
    printf("- Total de bytes transferidos (códigos 200): %lld\n", total_bytes);
    printf("========================================================================\n");

    // Fecha o arquivo de log
    fclose(file);
    if (line)
        free(line);

    return 0;
}