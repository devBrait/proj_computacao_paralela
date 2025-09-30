# Guilherme Teodoro de Oliveira RA: 10425362
# Luís Henrique Ribeiro Fernandes RA: 10420079
# Vinícius Brait Lorimier - 10420046

# Define o compilador
CC = gcc

# Flags de compilação
# -Wall para mostrar todos os warnings
# -pthread é essencial para linkar a biblioteca Pthreads
CFLAGS = -Wall
LDFLAGS = -pthread

# Nomes dos executáveis
SEQ_TARGET = log_analyzer_seq
PAR_TARGET = log_analyzer_par

# Nomes dos arquivos
SEQ_SOURCE = log_analyzer_seq.c
PAR_SOURCE = log_analyzer_par.c

# Regra para compilar os dois arquivos
all: $(SEQ_TARGET) $(PAR_TARGET)

# Regra para compilar a versão sequencial
$(SEQ_TARGET): $(SEQ_SOURCE)
	$(CC) $(CFLAGS) -o $(SEQ_TARGET) $(SEQ_SOURCE)

# Regra para compilar a versão paralela
$(PAR_TARGET): $(PAR_SOURCE)
	$(CC) $(CFLAGS) -o $(PAR_TARGET) $(PAR_SOURCE) $(LDFLAGS)

# Regra para limpar os arquivos gerados
clean:
	rm -f $(SEQ_TARGET) $(PAR_TARGET) *.o