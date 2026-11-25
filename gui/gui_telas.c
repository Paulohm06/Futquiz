/**
 * @file gui_telas.c
 * @brief Telas gráficas fora da partida: menu, nome do jogador, resultado,
 *        ranking e navegação das perguntas por nível.
 * @author Equipe Futquiz
 */

#include "gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/**
 * @brief Desenha um título grande dourado com sombra, centralizado.
 * @param gui Estado da interface.
 * @param texto Título.
 * @param y Posição vertical.
 * @param tamanho Tamanho da fonte.
 */
static void tituloTela(const Gui *gui, const char *texto, float y, float tamanho)
{
    gui_textoCentralizado(gui->fontes.titulo, texto, LARGURA_TELA / 2.0f + 3, y + 4, tamanho, (Color){ 0, 0, 0, 120 });
    gui_textoCentralizado(gui->fontes.titulo, texto, LARGURA_TELA / 2.0f, y, tamanho, COR_OURO);
}

/**
 * @brief Desenha um troféu dourado.
 * @param centro Centro do troféu.
 * @param escala Tamanho relativo (1 = 160 px de altura).
 */
static void desenharTrofeu(Vector2 centro, float escala)
{
    Color ouro = COR_OURO;
    Color sombra = COR_OURO_ESCURO;

    DrawRing((Vector2){ centro.x - 52 * escala, centro.y - 40 * escala }, 18 * escala, 26 * escala, 90, 270, 24, sombra);
    DrawRing((Vector2){ centro.x + 52 * escala, centro.y - 40 * escala }, 18 * escala, 26 * escala, -90, 90, 24, sombra);
    DrawCircleSector((Vector2){ centro.x, centro.y - 70 * escala }, 60 * escala, 0, 180, 32, ouro);
    DrawRectangleRec((Rectangle){ centro.x - 60 * escala, centro.y - 80 * escala, 120 * escala, 12 * escala }, ouro);
    DrawRectangleRec((Rectangle){ centro.x - 10 * escala, centro.y - 12 * escala, 20 * escala, 40 * escala }, sombra);
    DrawRectangleRounded((Rectangle){ centro.x - 46 * escala, centro.y + 26 * escala, 92 * escala, 26 * escala },
                         0.3f, 8, ouro);
    DrawRectangleRec((Rectangle){ centro.x - 56 * escala, centro.y + 50 * escala, 112 * escala, 14 * escala }, sombra);
    DrawCircleGradient((int)(centro.x - 22 * escala), (int)(centro.y - 50 * escala), 22 * escala,
                       (Color){ 255, 255, 255, 120 }, (Color){ 255, 255, 255, 0 });
}

/* ------------------------------------------------------------------ */
/* Menu inicial                                                        */
/* ------------------------------------------------------------------ */

void gui_telaMenu(Gui *gui)
{
    float centroX = LARGURA_TELA / 2.0f;
    float quique = fabsf(sinf(gui->tempo * 2.6f)) * 26.0f;
    char info[64];

    gui_desenharBola((Vector2){ centroX, 96 - quique }, 40, gui->tempo * 90.0f);
    DrawEllipse((int)centroX, 146, (int)(34 - quique * 0.4f), 7, (Color){ 0, 0, 0, 80 });
    tituloTela(gui, "FUTQUIZ", 152, 150);
    gui_textoCentralizado(gui->fontes.texto, "O Jogo do Milhão da Bola", centroX, 318, 30, COR_TEXTO);

    if (gui_botao(gui, (Rectangle){ centroX - 190, 384, 380, 64 }, "JOGAR", BOTAO_DESTAQUE, 1) ||
        IsKeyPressed(KEY_ENTER)) {
        gui->tela = TELA_NOME;
    }
    if (gui_botao(gui, (Rectangle){ centroX - 190, 462, 380, 56 }, "RANKING", BOTAO_PADRAO, 1)) {
        gui->tela = TELA_RANKING;
    }
    if (gui_botao(gui, (Rectangle){ centroX - 190, 530, 380, 56 }, "VER PERGUNTAS POR NÍVEL", BOTAO_PADRAO, 1)) {
        gui_carregarNivelNavegacao(gui, gui->nivelNavegacao > 0 ? gui->nivelNavegacao : 1);
        gui->tela = TELA_NAVEGAR;
    }
    if (gui_botao(gui, (Rectangle){ centroX - 190, 598, 380, 56 }, "SAIR", BOTAO_PADRAO, 1)) {
        gui->tela = TELA_SAIR;
    }

    snprintf(info, sizeof(info), "%d perguntas no banco", gui->banco->quantidade);
    gui_texto(gui->fontes.pequeno, info, 44, ALTURA_TELA - 44, 18, COR_TEXTO_SUAVE);
    gui_texto(gui->fontes.pequeno, "Equipe Futquiz · FP2 · UTFPR Apucarana",
              LARGURA_TELA - 44 - gui_larguraTexto(gui->fontes.pequeno, "Equipe Futquiz · FP2 · UTFPR Apucarana", 18),
              ALTURA_TELA - 44, 18, COR_TEXTO_SUAVE);
}

/* ------------------------------------------------------------------ */
/* Nome do jogador e categoria                                         */
/* ------------------------------------------------------------------ */

/**
 * @brief Lê as teclas digitadas e atualiza o nome (aceita acentos).
 * @param gui Estado da interface.
 */
static void lerNomeDigitado(Gui *gui)
{
    int codigo = GetCharPressed();
    int tamanhoUtf8;
    size_t tamanhoAtual;
    const char *utf8;

    while (codigo > 0) {
        utf8 = CodepointToUTF8(codigo, &tamanhoUtf8);
        tamanhoAtual = strlen(gui->nome);
        if (codigo >= 32 && tamanhoAtual + (size_t)tamanhoUtf8 < TAM_NOME_JOGADOR - 1) {
            memcpy(gui->nome + tamanhoAtual, utf8, (size_t)tamanhoUtf8);
            gui->nome[tamanhoAtual + (size_t)tamanhoUtf8] = '\0';
        }
        codigo = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
        tamanhoAtual = strlen(gui->nome);
        /* Volta um caractere UTF-8 inteiro (pula os bytes 10xxxxxx). */
        while (tamanhoAtual > 0) {
            tamanhoAtual--;
            if (((unsigned char)gui->nome[tamanhoAtual] & 0xC0) != 0x80) {
                break;
            }
        }
        gui->nome[tamanhoAtual] = '\0';
    }
}

/**
 * @brief Remove espaços do começo e do fim do nome.
 * @param nome Nome a ajustar.
 */
static void apararNome(char *nome)
{
    size_t inicio = 0;
    size_t fim = strlen(nome);

    while (nome[inicio] == ' ') {
        inicio++;
    }
    while (fim > inicio && nome[fim - 1] == ' ') {
        fim--;
    }
    memmove(nome, nome + inicio, fim - inicio);
    nome[fim - inicio] = '\0';
}

/**
 * @brief Desenha um "chip" de categoria selecionável.
 * @param gui Estado da interface.
 * @param x Posição x.
 * @param y Posição y.
 * @param texto Rótulo.
 * @param selecionado 1 se é a categoria escolhida.
 * @return Largura ocupada, ou o negativo dela se foi clicado.
 */
static float chipCategoria(Gui *gui, float x, float y, const char *texto, int selecionado)
{
    float largura = gui_larguraTexto(gui->fontes.pequeno, texto, 22) + 40;
    Rectangle area = { x, y, largura, 46 };
    int sobre = gui_mouseSobre(gui, area);

    DrawRectangleRounded(area, 1.0f, 16, selecionado ? COR_OURO : (sobre ? (Color){ 34, 70, 50, 245 } : COR_PAINEL));
    DrawRectangleRoundedLinesEx(area, 1.0f, 16, 1.5f, selecionado ? COR_OURO : COR_BORDA);
    gui_textoCentralizado(gui->fontes.pequeno, texto, x + largura / 2, y + 11, 22,
                          selecionado ? (Color){ 30, 22, 6, 255 } : COR_TEXTO);
    if (sobre && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        gui_tocar(gui, gui->sons.clique);
        return -largura;
    }
    return largura;
}

void gui_telaNome(Gui *gui)
{
    Rectangle painel = { (LARGURA_TELA - 780) / 2.0f, 120, 780, 520 };
    Rectangle caixa = { painel.x + 60, painel.y + 120, painel.width - 120, 70 };
    float x = painel.x + 60;
    float y = painel.y + 270;
    float largura;
    int categoria;
    const char *nomeCategoria;
    int comecar;

    lerNomeDigitado(gui);
    tituloTela(gui, "QUEM VAI ENTRAR EM CAMPO?", 34, 64);
    gui_desenharPainel(painel, COR_PAINEL);

    gui_texto(gui->fontes.pequeno, "NOME DO JOGADOR", caixa.x, caixa.y - 36, 22, COR_TEXTO_SUAVE);
    DrawRectangleRounded(caixa, 0.3f, 10, (Color){ 0, 0, 0, 120 });
    DrawRectangleRoundedLinesEx(caixa, 0.3f, 10, 2, COR_OURO);
    gui_texto(gui->fontes.texto, gui->nome, caixa.x + 22, caixa.y + 15, 38, COR_TEXTO);
    if (fmodf(gui->tempo, 1.0f) < 0.55f) {
        float xCursor = caixa.x + 24 + gui_larguraTexto(gui->fontes.texto, gui->nome, 38);
        DrawRectangle((int)xCursor, (int)caixa.y + 16, 3, 38, COR_OURO);
    }

    gui_texto(gui->fontes.pequeno, "CATEGORIA PREFERIDA", x, y - 40, 22, COR_TEXTO_SUAVE);
    for (categoria = CATEGORIA_TODAS; categoria <= NUM_CATEGORIAS; categoria++) {
        nomeCategoria = (categoria == CATEGORIA_TODAS) ? "Todas" : pergunta_nomeCategoria((Categoria)categoria);
        largura = gui_larguraTexto(gui->fontes.pequeno, nomeCategoria, 22) + 40;
        if (x + largura > painel.x + painel.width - 60) {
            x = painel.x + 60;
            y += 58;
        }
        largura = chipCategoria(gui, x, y, nomeCategoria, gui->categoria == categoria);
        if (largura < 0) {
            gui->categoria = categoria;
            largura = -largura;
        }
        x += largura + 12;
    }
    gui_texto(gui->fontes.pequeno, "Se faltar pergunta da categoria em algum nível, o sorteio completa com outras.",
              painel.x + 60, y + 66, 18, COR_TEXTO_SUAVE);

    comecar = gui_botao(gui, (Rectangle){ painel.x + painel.width - 360, painel.y + painel.height - 90, 300, 62 },
                        "ENTRAR EM CAMPO", BOTAO_DESTAQUE, 1) || IsKeyPressed(KEY_ENTER);
    if (gui_botao(gui, (Rectangle){ painel.x + 60, painel.y + painel.height - 90, 160, 62 }, "VOLTAR",
                  BOTAO_PADRAO, 1) || IsKeyPressed(KEY_ESCAPE)) {
        gui->tela = TELA_MENU;
        return;
    }
    if (comecar) {
        apararNome(gui->nome);
        if (gui->nome[0] == '\0') {
            gui_mostrarAviso(gui, "Digite o nome do jogador.");
        } else if (!gui_iniciarPartida(gui)) {
            gui_mostrarAviso(gui, "Não há perguntas suficientes em todos os níveis.");
        }
    }
}

/* ------------------------------------------------------------------ */
/* Resultado                                                           */
/* ------------------------------------------------------------------ */

void gui_telaResultado(Gui *gui)
{
    const Partida *partida = &gui->partida;
    float centroX = LARGURA_TELA / 2.0f;
    char premio[32];
    char texto[96];
    const char *titulo;

    if (partida->estado == PARTIDA_CAMPEAO) {
        titulo = "CAMPEÃO!";
        desenharTrofeu((Vector2){ centroX, 150 }, 0.9f + 0.03f * sinf(gui->tempo * 3.0f));
    } else {
        titulo = (partida->estado == PARTIDA_PAROU) ? "VOCÊ PAROU" : "FIM DE JOGO";
        gui_desenharBola((Vector2){ centroX, 126 }, 50, gui->tempo * 30.0f);
    }
    tituloTela(gui, titulo, 222, 96);

    partida_formatarPremio(partida->premioFinal, premio, sizeof(premio));
    gui_textoCentralizado(gui->fontes.pequeno, "PRÊMIO", centroX, 358, 22, COR_TEXTO_SUAVE);
    gui_textoCentralizado(gui->fontes.titulo, premio, centroX, 384, 84, COR_TEXTO);

    snprintf(texto, sizeof(texto), "%s acertou %d de %d perguntas", gui->nome, partida_acertos(partida), NUM_RODADAS);
    gui_textoCentralizado(gui->fontes.texto, texto, centroX, 490, 28, COR_TEXTO);
    if (gui->posicaoRanking > 0) {
        snprintf(texto, sizeof(texto), "Entrou no ranking em %dº lugar!", gui->posicaoRanking);
        gui_textoCentralizado(gui->fontes.texto, texto, centroX, 530, 26, COR_ACERTO);
    } else {
        gui_textoCentralizado(gui->fontes.texto, "Não entrou no ranking desta vez.", centroX, 530, 26,
                              COR_TEXTO_SUAVE);
    }

    if (gui_botao(gui, (Rectangle){ centroX - 400, 600, 250, 62 }, "JOGAR DE NOVO", BOTAO_DESTAQUE, 1)) {
        gui_iniciarPartida(gui);
    }
    if (gui_botao(gui, (Rectangle){ centroX - 125, 600, 250, 62 }, "RANKING", BOTAO_PADRAO, 1)) {
        gui->tela = TELA_RANKING;
    }
    if (gui_botao(gui, (Rectangle){ centroX + 150, 600, 250, 62 }, "MENU", BOTAO_PADRAO, 1) ||
        IsKeyPressed(KEY_ESCAPE)) {
        gui->tela = TELA_MENU;
    }
}

/* ------------------------------------------------------------------ */
/* Ranking                                                             */
/* ------------------------------------------------------------------ */

void gui_telaRanking(Gui *gui)
{
    static const Color medalhas[3] = { { 242, 193, 78, 255 }, { 200, 206, 212, 255 }, { 205, 127, 50, 255 } };
    Rectangle painel = { (LARGURA_TELA - 820) / 2.0f, 130, 820, 480 };
    Rectangle linha;
    const RegistroRanking *registro;
    char texto[32];
    int i;

    tituloTela(gui, "RANKING DOS CRAQUES", 34, 72);
    gui_desenharPainel(painel, COR_PAINEL);

    gui_texto(gui->fontes.titulo, "POS", painel.x + 30, painel.y + 16, 24, COR_OURO);
    gui_texto(gui->fontes.titulo, "JOGADOR", painel.x + 120, painel.y + 16, 24, COR_OURO);
    gui_texto(gui->fontes.titulo, "ACERTOS", painel.x + 470, painel.y + 16, 24, COR_OURO);
    gui_texto(gui->fontes.titulo, "PRÊMIO", painel.x + painel.width - 30 - gui_larguraTexto(gui->fontes.titulo,
              "PRÊMIO", 24), painel.y + 16, 24, COR_OURO);

    if (gui->ranking->quantidade == 0) {
        gui_textoCentralizado(gui->fontes.texto, "Ninguém jogou ainda. Seja o primeiro!", LARGURA_TELA / 2.0f,
                              painel.y + 220, 30, COR_TEXTO_SUAVE);
    }
    for (i = 0; i < gui->ranking->quantidade; i++) {
        registro = &gui->ranking->registros[i];
        linha = (Rectangle){ painel.x + 12, painel.y + 56 + i * 41.0f, painel.width - 24, 38 };
        if (i % 2 == 0) {
            DrawRectangleRounded(linha, 0.4f, 8, (Color){ 255, 255, 255, 10 });
        }
        snprintf(texto, sizeof(texto), "%d", i + 1);
        if (i < 3) {
            DrawCircle((int)linha.x + 36, (int)(linha.y + 19), 15, medalhas[i]);
            gui_textoCentralizado(gui->fontes.titulo, texto, linha.x + 36, linha.y + 6, 22, (Color){ 30, 22, 6, 255 });
        } else {
            gui_textoCentralizado(gui->fontes.titulo, texto, linha.x + 36, linha.y + 6, 22, COR_TEXTO_SUAVE);
        }
        gui_texto(gui->fontes.texto, registro->nome, painel.x + 120, linha.y + 6, 24, COR_TEXTO);
        snprintf(texto, sizeof(texto), "%d/%d", registro->acertos, NUM_RODADAS);
        gui_texto(gui->fontes.titulo, texto, painel.x + 480, linha.y + 6, 24, COR_TEXTO);
        partida_formatarPremio(registro->premio, texto, sizeof(texto));
        gui_texto(gui->fontes.titulo, texto, painel.x + painel.width - 30 - gui_larguraTexto(gui->fontes.titulo,
                  texto, 24), linha.y + 6, 24, i == 0 ? COR_OURO : COR_TEXTO);
    }

    if (gui_botao(gui, (Rectangle){ (LARGURA_TELA - 240) / 2.0f, 636, 240, 58 }, "VOLTAR", BOTAO_PADRAO, 1) ||
        IsKeyPressed(KEY_ESCAPE)) {
        gui->tela = TELA_MENU;
    }
}

/* ------------------------------------------------------------------ */
/* Navegação por nível                                                 */
/* ------------------------------------------------------------------ */

void gui_carregarNivelNavegacao(Gui *gui, int nivel)
{
    free(gui->indicesNavegacao);
    gui->nivelNavegacao = nivel;
    gui->indicesNavegacao = banco_indicesPorNivel(gui->banco, (NivelDificuldade)nivel, &gui->quantidadeNavegacao);
    gui->atualNavegacao = 0;
    gui->mostrarResposta = 0;
}

/**
 * @brief Avança ou volta na navegação, com aviso nos limites.
 * @param gui Estado da interface.
 * @param passo +1 para a próxima, -1 para a anterior.
 */
static void moverNavegacao(Gui *gui, int passo)
{
    int nova = gui->atualNavegacao + passo;

    if (nova < 0 || nova >= gui->quantidadeNavegacao) {
        gui_tocar(gui, gui->sons.erro);
        gui_mostrarAviso(gui, passo > 0 ? "Esta já é a última pergunta do nível."
                                        : "Esta já é a primeira pergunta do nível.");
        return;
    }
    gui_tocar(gui, gui->sons.clique);
    gui->atualNavegacao = nova;
    gui->mostrarResposta = 0;
}

/**
 * @brief Desenha as abas de nível e troca de nível ao clicar.
 * @param gui Estado da interface.
 */
static void abasNivel(Gui *gui)
{
    int nivel;
    float x = (LARGURA_TELA - (5 * 200.0f + 4 * 12.0f)) / 2.0f;
    Rectangle aba;
    Color cor;
    int ativo;

    for (nivel = 1; nivel <= NUM_NIVEIS; nivel++) {
        aba = (Rectangle){ x, 120, 200, 50 };
        cor = gui_corNivel((NivelDificuldade)nivel);
        ativo = gui->nivelNavegacao == nivel;
        DrawRectangleRounded(aba, 0.5f, 10, ativo ? cor : (gui_mouseSobre(gui, aba) ? Fade(cor, 0.35f) : COR_PAINEL));
        DrawRectangleRoundedLinesEx(aba, 0.5f, 10, 2, cor);
        gui_textoCentralizado(gui->fontes.titulo, pergunta_nomeNivel((NivelDificuldade)nivel), aba.x + 100, aba.y + 10,
                              26, ativo ? (Color){ 20, 20, 20, 255 } : COR_TEXTO);
        if ((CheckCollisionPointRec(GetMousePosition(), aba) && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) ||
            IsKeyPressed(KEY_ONE + nivel - 1)) {
            gui_tocar(gui, gui->sons.clique);
            gui_carregarNivelNavegacao(gui, nivel);
        }
        x += 212;
    }
}

void gui_telaNavegar(Gui *gui)
{
    Rectangle cartao = { 140, 190, LARGURA_TELA - 280, 160 };
    Rectangle alternativa;
    const Pergunta *pergunta;
    char texto[96];
    char letra[2] = { 0, 0 };
    int i;
    int correta;

    tituloTela(gui, "PERGUNTAS POR NÍVEL", 30, 64);
    abasNivel(gui);

    if (gui->quantidadeNavegacao == 0) {
        gui_desenharPainel(cartao, COR_PAINEL);
        gui_textoCentralizado(gui->fontes.texto, "Não há perguntas neste nível.", LARGURA_TELA / 2.0f,
                              cartao.y + 62, 30, COR_TEXTO_SUAVE);
    } else {
        pergunta = &gui->banco->itens[gui->indicesNavegacao[gui->atualNavegacao]];
        gui_desenharPainel(cartao, COR_PAINEL);
        DrawRectangle((int)cartao.x, (int)cartao.y + 20, 5, (int)cartao.height - 40, gui_corNivel(pergunta->nivel));
        snprintf(texto, sizeof(texto), "Pergunta %d de %d  ·  %s  ·  ID %d", gui->atualNavegacao + 1,
                 gui->quantidadeNavegacao, pergunta_nomeCategoria(pergunta->categoria), pergunta->id);
        gui_texto(gui->fontes.pequeno, texto, cartao.x + 30, cartao.y + 14, 20, COR_TEXTO_SUAVE);
        gui_textoQuebrado(gui->fontes.texto, pergunta->enunciado,
                          (Rectangle){ cartao.x + 30, cartao.y + 44, cartao.width - 60, cartao.height - 56 },
                          32, 20, 1, COR_TEXTO);

        for (i = 0; i < NUM_ALTERNATIVAS; i++) {
            alternativa = (Rectangle){ 140 + (i % 2) * ((LARGURA_TELA - 280) / 2.0f + 6), 366 + (i / 2) * 74.0f,
                                       (LARGURA_TELA - 280) / 2.0f - 6, 62 };
            correta = gui->mostrarResposta && (pergunta->correta - 'A') == i;
            DrawRectangleRounded(alternativa, 0.5f, 12, correta ? (Color){ 30, 150, 80, 245 } : COR_PAINEL);
            DrawRectangleRoundedLinesEx(alternativa, 0.5f, 12, 2, correta ? COR_ACERTO : COR_BORDA);
            letra[0] = (char)('A' + i);
            DrawCircle((int)alternativa.x + 36, (int)alternativa.y + 31, 20, COR_OURO);
            gui_textoCentralizado(gui->fontes.titulo, letra, alternativa.x + 36, alternativa.y + 15, 28,
                                  (Color){ 30, 22, 6, 255 });
            gui_textoQuebrado(gui->fontes.texto, pergunta->alternativas[i],
                              (Rectangle){ alternativa.x + 72, alternativa.y + 6, alternativa.width - 90, 50 },
                              24, 16, 0, COR_TEXTO);
        }
    }

    if (gui_botao(gui, (Rectangle){ 140, 540, 220, 58 }, "< ANTERIOR", BOTAO_PADRAO, 1) || IsKeyPressed(KEY_LEFT)) {
        moverNavegacao(gui, -1);
    }
    if (gui_botao(gui, (Rectangle){ (LARGURA_TELA - 300) / 2.0f, 540, 300, 58 },
                  gui->mostrarResposta ? "ESCONDER RESPOSTA" : "MOSTRAR RESPOSTA", BOTAO_DESTAQUE,
                  gui->quantidadeNavegacao > 0) || (IsKeyPressed(KEY_R) && gui->quantidadeNavegacao > 0)) {
        gui->mostrarResposta = !gui->mostrarResposta;
    }
    if (gui_botao(gui, (Rectangle){ LARGURA_TELA - 360, 540, 220, 58 }, "PRÓXIMA >", BOTAO_PADRAO, 1) ||
        IsKeyPressed(KEY_RIGHT)) {
        moverNavegacao(gui, +1);
    }
    if (gui_botao(gui, (Rectangle){ (LARGURA_TELA - 200) / 2.0f, 626, 200, 52 }, "VOLTAR", BOTAO_PADRAO, 1) ||
        IsKeyPressed(KEY_ESCAPE)) {
        gui->tela = TELA_MENU;
    }
    gui_texto(gui->fontes.pequeno, "Teclas: 1-5 nível · setas navegam · R resposta", 140, 640, 18, COR_TEXTO_SUAVE);
}
