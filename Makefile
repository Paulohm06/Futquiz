# Makefile do Futquiz
#   make        -> compila
#   make run    -> compila e executa
#   make clean  -> apaga objetos e executável
#   make reset  -> apaga o binário de dados (próxima execução relê o CSV)
#   make docs   -> gera a documentação com Doxygen

CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11 -g -MMD -MP
SRC_DIR = src
OBJ_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

ifeq ($(OS),Windows_NT)
    EXEC      = futquiz.exe
    RUN_CMD   = $(EXEC)
    MKDIR_OBJ = if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)
    CLEAN_CMD = if exist $(OBJ_DIR) rmdir /S /Q $(OBJ_DIR) & if exist $(EXEC) del /Q $(EXEC)
    RESET_CMD = if exist dados\perguntas.bin del /Q dados\perguntas.bin
else
    EXEC      = futquiz
    RUN_CMD   = ./$(EXEC)
    MKDIR_OBJ = mkdir -p $(OBJ_DIR)
    CLEAN_CMD = rm -rf $(OBJ_DIR) $(EXEC)
    RESET_CMD = rm -f dados/perguntas.bin
endif

.PHONY: all run clean reset docs

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	$(MKDIR_OBJ)

run: $(EXEC)
	$(RUN_CMD)

clean:
	$(CLEAN_CMD)

reset:
	$(RESET_CMD)

docs:
	doxygen Doxyfile

-include $(DEPS)
