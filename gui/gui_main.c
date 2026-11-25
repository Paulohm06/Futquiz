/**
 * @file gui_main.c
 * @brief Ponto de entrada da versão gráfica (raylib) do Futquiz.
 *
 * Usa os mesmos arquivos de dados da versão de terminal: lê o binário
 * (ou o CSV na primeira execução), salva o binário ao sair e grava o
 * ranking ao fim de cada partida.
 *
 * @author Equipe Futquiz
 */

#include "gui.h"
#include "arquivo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Carrega o banco: binário se existir, senão o CSV inicial.
 * @param banco Banco de destino.
 */
static void carregarBanco(BancoPerguntas *banco)
{
    if (arquivo_existe(CAMINHO_BINARIO) && arquivo_carregarBinario(banco, CAMINHO_BINARIO) >= 0) {
        return;
    }
    banco_liberar(banco);
    if (arquivo_carregarCSV(banco, CAMINHO_CSV_INICIAL) < 0) {
        TraceLog(LOG_WARNING, "Não foi possível abrir %s.", CAMINHO_CSV_INICIAL);
    }
}

/**
 * @brief Desenha a tela atual.
 * @param gui Estado da interface.
 */
static void desenharTela(Gui *gui)
{
    switch (gui->tela) {
    case TELA_MENU:      gui_telaMenu(gui);      break;
    case TELA_NOME:      gui_telaNome(gui);      break;
    case TELA_JOGO:      gui_telaJogo(gui);      break;
    case TELA_RESULTADO: gui_telaResultado(gui); break;
    case TELA_RANKING:   gui_telaRanking(gui);   break;
    case TELA_NAVEGAR:   gui_telaNavegar(gui);   break;
    default:                                     break;
    }
}

/**
 * @brief Função principal da versão gráfica.
 * @return EXIT_SUCCESS ao fechar normalmente.
 */
int main(void)
{
    BancoPerguntas banco;
    Ranking ranking;
    Gui gui;
    Tela telaAnterior;

    srand((unsigned int)time(NULL));
    banco_inicializar(&banco);
    carregarBanco(&banco);
    ranking_carregar(&ranking, CAMINHO_RANKING);

    memset(&gui, 0, sizeof(gui));
    gui.banco = &banco;
    gui.ranking = &ranking;
    gui.tela = TELA_MENU;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Futquiz - O Jogo do Milhão da Bola");
    SetExitKey(KEY_NULL); /* ESC é usado pelas telas, não fecha a janela */
    SetTargetFPS(60);
    gui_carregarRecursos(&gui);

    while (!WindowShouldClose() && gui.tela != TELA_SAIR) {
        gui.tempo += GetFrameTime();
        gui.mouseSobreBotao = 0;
        telaAnterior = gui.tela;

        BeginDrawing();
        ClearBackground(BLACK);
        gui_desenharFundo(&gui);
        desenharTela(&gui);
        gui_atualizarConfetes(&gui);
        gui_desenharAviso(&gui);
        EndDrawing();

        SetMouseCursor(gui.mouseSobreBotao ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);
        if (gui.tela != telaAnterior && gui.tela != TELA_RESULTADO) {
            gui.tempoAviso = 0.0f; /* aviso não "vaza" para outra tela */
        }
    }

    gui_liberarRecursos(&gui);
    CloseWindow();

    if (!arquivo_salvarBinario(&banco, CAMINHO_BINARIO)) {
        fprintf(stderr, "Falha ao salvar %s.\n", CAMINHO_BINARIO);
    }
    free(gui.indicesNavegacao);
    banco_liberar(&banco);
    return EXIT_SUCCESS;
}
