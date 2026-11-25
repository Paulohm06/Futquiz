# Makefile do Futquiz
#   make          -> compila a versão de terminal
#   make run      -> compila e executa a versão de terminal
#   make gui      -> compila a versão gráfica (precisa da raylib)
#   make run-gui  -> compila e executa a versão gráfica
#   make clean    -> apaga objetos e executáveis
#   make reset    -> apaga os binários de dados (perguntas e ranking)
#   make docs     -> gera a documentação com Doxygen
#
# Se a raylib não estiver instalada no sistema, informe a pasta dela:
#   make gui RAYLIB_PATH=C:/raylib/raylib     (Windows, instalador oficial)
#   make gui RAYLIB_PATH=/caminho/da/raylib   (pasta com include/ e lib/)

CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -std=c11 -g -MMD -MP
SRC_DIR = src
GUI_DIR = gui
OBJ_DIR = build

# ---- Versão de terminal: todos os .c de src/ ----
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

# ---- Versão gráfica: núcleo de src/ (sem as telas de terminal) + gui/ ----
TERMINAL_ONLY = $(SRC_DIR)/main.c $(SRC_DIR)/menu.c $(SRC_DIR)/jogo.c
CORE_SRCS = $(filter-out $(TERMINAL_ONLY),$(SRCS))
GUI_SRCS  = $(wildcard $(GUI_DIR)/*.c)
GUI_OBJS  = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(CORE_SRCS)) \
            $(patsubst $(GUI_DIR)/%.c,$(OBJ_DIR)/gui/%.o,$(GUI_SRCS))

DEPS = $(OBJS:.o=.d) $(GUI_OBJS:.o=.d)

GUI_CFLAGS = -I$(SRC_DIR)
GUI_LDFLAGS =
ifneq ($(RAYLIB_PATH),)
    GUI_CFLAGS  += -I$(RAYLIB_PATH)/include -I$(RAYLIB_PATH)/src
    GUI_LDFLAGS += -L$(RAYLIB_PATH)/lib -L$(RAYLIB_PATH)/src
endif

ifeq ($(OS),Windows_NT)
    EXEC      = futquiz.exe
    GUI_EXEC  = futquiz_gui.exe
    RUN_CMD   = $(EXEC)
    RUN_GUI   = $(GUI_EXEC)
    GUI_LIBS  = -lraylib -lopengl32 -lgdi32 -lwinmm
    MKDIR_OBJ = if not exist $(OBJ_DIR)\gui mkdir $(OBJ_DIR)\gui
    CLEAN_CMD = if exist $(OBJ_DIR) rmdir /S /Q $(OBJ_DIR) & if exist $(EXEC) del /Q $(EXEC) & if exist $(GUI_EXEC) del /Q $(GUI_EXEC)
    RESET_CMD = if exist dados\perguntas.bin del /Q dados\perguntas.bin & if exist dados\ranking.bin del /Q dados\ranking.bin
else
    EXEC      = futquiz
    GUI_EXEC  = futquiz_gui
    RUN_CMD   = ./$(EXEC)
    RUN_GUI   = ./$(GUI_EXEC)
    UNAME_S  := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        GUI_LIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    else
        GUI_LIBS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
    endif
    MKDIR_OBJ = mkdir -p $(OBJ_DIR)/gui
    CLEAN_CMD = rm -rf $(OBJ_DIR) $(EXEC) $(GUI_EXEC)
    RESET_CMD = rm -f dados/perguntas.bin dados/ranking.bin
endif

.PHONY: all run gui run-gui clean reset docs

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

gui: $(GUI_EXEC)

$(GUI_EXEC): $(GUI_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(GUI_LDFLAGS) $(GUI_LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/gui/%.o: $(GUI_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) $(GUI_CFLAGS) -c $< -o $@

$(OBJ_DIR):
	$(MKDIR_OBJ)

run: $(EXEC)
	$(RUN_CMD)

run-gui: $(GUI_EXEC)
	$(RUN_GUI)

clean:
	$(CLEAN_CMD)

reset:
	$(RESET_CMD)

docs:
	doxygen Doxyfile

-include $(DEPS)
