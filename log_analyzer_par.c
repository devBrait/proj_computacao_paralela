/*
    Guilherme Teodoro de Oliveira RA: 10425362
    Luís Henrique Ribeiro Fernandes RA: 10420079
    Vinícius Brait Lorimier - 10420046
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define LOG_FILE "access_log_large.txt"
#define MAX_LINES 1000000
#define MAX_LEN 256

// Struct para armazenar as estatísticas 
typedef struct {
    long long errors404;
    long long total_bytes;
} Stats;

// Struct para argumentos das threads
typedef struct {
    int thread_id;
    int num_threads;
} ThreadArgs;

char *lines[MAX_LINES];
int line_count = 0;

Stats global_stats = {0, 0};
pthread_mutex_t stats_mutex; // // Mutex global

// Extração dos status code e os bytes de uma linha
void parse_line(const char *line, int *status_code, long long *bytes_sent){
    const char *quote_ptr = strstr(line, "\" ");

    if (quote_ptr){
        // sscanf para extrair os dois números após aspas e o espaço
        if (sscanf(quote_ptr + 2, "%d %lld", status_code, bytes_sent) != 2){
            *status_code = 0;
            *bytes_sent = 0;
        }
    }else{
        *status_code = 0;
        *bytes_sent = 0;
    }
}

// Função executada por cada thread
void* process_log_chunk(void* arg){
    ThreadArgs* args = (ThreadArgs*)arg;
    int thread_id = args->thread_id;
    int num_threads = args->num_threads;

    // Realiza a divisão para cada thread
    int chunk_size = line_count / num_threads;
    int start_index = thread_id * chunk_size;
    int end_index = (thread_id == num_threads - 1) ? line_count : start_index + chunk_size;

    long long local_errors_404 = 0;
    long long local_total_bytes = 0;

    for (int i = start_index; i < end_index; i++){
        int status_code;
        long long bytes_sent;
        
        parse_line(lines[i], &status_code, &bytes_sent);
        // Contagem dos status code
        if(status_code == 404){
            local_errors_404++;
        }else if(status_code == 200){
            local_total_bytes += bytes_sent;
        }
    }

    // Trava o mutex, atualiza os contadores globais e destrava 
    pthread_mutex_lock(&stats_mutex);
    global_stats.errors404 += local_errors_404;

    global_stats.total_bytes += local_total_bytes;
    pthread_mutex_unlock(&stats_mutex);
    return NULL;
}

int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(stderr, "Formato incorreto, deveria ser: %s <numero_de_threads>\n", argv[0]);
        return 1;
    }

    // Identifica o número de threads para execução
    int num_threads = atoi(argv[1]);
    if(num_threads <= 0){
        fprintf(stderr, "O número de threads deve ser maior que 0.\n");
        return 1;
    }

     // Abre o arquivo de log
    FILE *file = fopen(LOG_FILE, "r");
    if(file == NULL){
        perror("Erro ao abrir o arquivo de log!\n");
        return 1;
    }

    while(line_count < MAX_LINES && (lines[line_count] = malloc(MAX_LEN)) != NULL){
        if (fgets(lines[line_count], MAX_LEN,file) == NULL) {
            free(lines[line_count]); // Libera a memória se fgets falhar no final
            break;
        }
        line_count++;
    }
    fclose(file);

    // Inicializa o processamento
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t threads[num_threads];
    ThreadArgs thread_args[num_threads];
    pthread_mutex_init(&stats_mutex, NULL); // Inicializa o mutex

    // Dispara as threads 
    for(int i = 0; i < num_threads; i++){
        thread_args[i].thread_id = i;
        thread_args[i].num_threads = num_threads;

        if(pthread_create(&threads[i], NULL, process_log_chunk, &thread_args[i]) != 0){
            perror("Erro ao criar a thread!");
            return 1;
        }
    }

    // Aguarda o fim de todas as threads
    for(int i = 0; i < num_threads; i++){
        if(pthread_join(threads[i], NULL) != 0){
            perror("Erro ao esperar pela thread!");
            return 1;
        }
    }

    // Destroi o mutex
    pthread_mutex_destroy(&stats_mutex); 
    clock_gettime(CLOCK_MONOTONIC, &end);
    double time_spent = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Exibe todos os dados finais coletados
    printf("========================================================================\n");
    printf("--- Versão Paralela (%d threads) ---\n", num_threads);
    printf("Resultados Finais: \n");
    printf("- Tempo de execução: %.4f segundos\n", time_spent);
    printf("- Total de erros (códigos 404): %lld\n", global_stats.errors404);
    printf("- Total de bytes transferidos (códigos 200): %lld\n", global_stats.total_bytes);
    printf("========================================================================\n");

    // Libera a memória alocada para as linhas
    for(int i = 0; i < line_count; i++)
        free(lines[i]);
    return 0;
}