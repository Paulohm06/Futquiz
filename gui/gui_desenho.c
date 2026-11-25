/**
 * @file gui_desenho.c
 * @brief Funções de desenho reaproveitadas pelas telas: fundo de gramado,
 *        painéis, textos com quebra de linha, botões, bola, avisos e confetes.
 * @author Equipe Futquiz
 */

#include "gui.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

/** Espaçamento entre letras usado em todos os textos. */
#define ESPACAMENTO 1.0f
/** Quantidade máxima de linhas na quebra de texto. */
#define MAX_LINHAS 8
/** Tamanho máximo de uma linha na quebra de texto. */
#define TAM_LINHA 320
/** Raio dos cantos dos painéis, em pixels. */
#define RAIO_PAINEL 14.0f

void gui_desenharFundo(const Gui *gui)
{
    int faixa;
    const int larguraFaixa = 80;
    Color linha = { 255, 255, 255, 34 };
    Vector2 centro = { LARGURA_TELA / 2.0f, ALTURA_TELA / 2.0f };

    (void)gui;
    /* Faixas do gramado cortado. */
    for (faixa = 0; faixa * larguraFaixa < LARGURA_TELA; faixa++) {
        DrawRectangle(faixa * larguraFaixa, 0, larguraFaixa, ALTURA_TELA,
                      (faixa % 2 == 0) ? COR_GRAMA_CLARA : COR_GRAMA_ESCURA);
    }
    /* Linhas do campo. */
    DrawRectangleLinesEx((Rectangle){ 28, 28, LARGURA_TELA - 56, ALTURA_TELA - 56 }, 3, linha);
    DrawLineEx((Vector2){ centro.x, 28 }, (Vector2){ centro.x, ALTURA_TELA - 28 }, 3, linha);
    DrawRing(centro, 104, 107, 0, 360, 64, linha);
    DrawCircleV(centro, 6, linha);
    DrawRectangleLinesEx((Rectangle){ 28, centro.y - 170, 190, 340 }, 3, linha);
    DrawRectangleLinesEx((Rectangle){ LARGURA_TELA - 218, centro.y - 170, 190, 340 }, 3, linha);
    DrawRectangleLinesEx((Rectangle){ 28, centro.y - 80, 70, 160 }, 3, linha);
    DrawRectangleLinesEx((Rectangle){ LARGURA_TELA - 98, centro.y - 80, 70, 160 }, 3, linha);

    /* Luz dos refletores e escurecimento das bordas. */
    DrawCircleGradient(LARGURA_TELA / 2, -120, 620, (Color){ 255, 255, 230, 40 }, (Color){ 255, 255, 230, 0 });
    DrawRectangleGradientV(0, 0, LARGURA_TELA, 160, (Color){ 0, 0, 0, 120 }, (Color){ 0, 0, 0, 0 });
    DrawRectangleGradientV(0, ALTURA_TELA - 200, LARGURA_TELA, 200, (Color){ 0, 0, 0, 0 }, (Color){ 0, 0, 0, 150 });
    DrawRectangleGradientH(0, 0, 220, ALTURA_TELA, (Color){ 0, 0, 0, 110 }, (Color){ 0, 0, 0, 0 });
    DrawRectangleGradientH(LARGURA_TELA - 220, 0, 220, ALTURA_TELA, (Color){ 0, 0, 0, 0 }, (Color){ 0, 0, 0, 110 });
}

void gui_desenharPainel(Rectangle area, Color cor)
{
    /* A raylib usa arredondamento relativo; convertemos para um raio fixo. */
    float arredondamento = fminf(1.0f, 2.0f * RAIO_PAINEL / fminf(area.width, area.height));

    DrawRectangleRounded(area, arredondamento, 12, cor);
    DrawRectangleRoundedLinesEx(area, arredondamento, 12, 1.5f, COR_BORDA);
}

void gui_texto(Font fonte, const char *texto, float x, float y, float tamanho, Color cor)
{
    DrawTextEx(fonte, texto, (Vector2){ roundf(x), roundf(y) }, tamanho, ESPACAMENTO, cor);
}

float gui_larguraTexto(Font fonte, const char *texto, float tamanho)
{
    return MeasureTextEx(fonte, texto, tamanho, ESPACAMENTO).x;
}

void gui_textoCentralizado(Font fonte, const char *texto, float centroX, float y, float tamanho, Color cor)
{
    gui_texto(fonte, texto, centroX - gui_larguraTexto(fonte, texto, tamanho) / 2.0f, y, tamanho, cor);
}

/**
 * @brief Divide um texto em linhas que caibam em uma largura.
 * @param fonte Fonte.
 * @param texto Texto UTF-8.
 * @param largura Largura máxima.
 * @param tamanho Tamanho da fonte.
 * @param linhas Matriz que recebe as linhas.
 * @return Quantidade de linhas (MAX_LINHAS + 1 se não couber).
 */
static int quebrarLinhas(Font fonte, const char *texto, float largura, float tamanho,
                         char linhas[MAX_LINHAS][TAM_LINHA])
{
    char palavra[TAM_LINHA];
    char candidata[TAM_LINHA * 2];
    int quantidade = 0;
    size_t tamanhoPalavra;
    const char *c = texto;

    linhas[0][0] = '\0';
    while (*c != '\0') {
        while (*c == ' ') {
            c++;
        }
        tamanhoPalavra = strcspn(c, " ");
        if (tamanhoPalavra == 0) {
            break;
        }
        if (tamanhoPalavra >= TAM_LINHA) {
            tamanhoPalavra = TAM_LINHA - 1;
        }
        memcpy(palavra, c, tamanhoPalavra);
        palavra[tamanhoPalavra] = '\0';
        c += tamanhoPalavra;

        snprintf(candidata, sizeof(candidata), "%s%s%s", linhas[quantidade],
                 linhas[quantidade][0] != '\0' ? " " : "", palavra);
        if (linhas[quantidade][0] != '\0' && gui_larguraTexto(fonte, candidata, tamanho) > largura) {
            quantidade++;
            if (quantidade >= MAX_LINHAS) {
                return MAX_LINHAS + 1;
            }
            snprintf(linhas[quantidade], TAM_LINHA, "%s", palavra);
        } else {
            /* Cópia limitada: a linha nunca passa de TAM_LINHA - 1 bytes. */
            tamanhoPalavra = strlen(candidata);
            if (tamanhoPalavra >= TAM_LINHA) {
                tamanhoPalavra = TAM_LINHA - 1;
            }
            memcpy(linhas[quantidade], candidata, tamanhoPalavra);
            linhas[quantidade][tamanhoPalavra] = '\0';
        }
    }
    return quantidade + 1;
}

void gui_textoQuebrado(Font fonte, const char *texto, Rectangle area, float tamanho, float tamanhoMinimo,
                       int centralizar, Color cor)
{
    char linhas[MAX_LINHAS][TAM_LINHA];
    int quantidade;
    int i;
    float alturaLinha;
    float y;

    for (;;) {
        quantidade = quebrarLinhas(fonte, texto, area.width, tamanho, linhas);
        alturaLinha = tamanho * 1.18f;
        if ((quantidade <= MAX_LINHAS && quantidade * alturaLinha <= area.height) || tamanho <= tamanhoMinimo) {
            break;
        }
        tamanho -= 2.0f;
    }
    if (quantidade > MAX_LINHAS) {
        quantidade = MAX_LINHAS;
    }
    y = area.y + (area.height - quantidade * alturaLinha) / 2.0f;
    for (i = 0; i < quantidade; i++) {
        if (centralizar) {
            gui_textoCentralizado(fonte, linhas[i], area.x + area.width / 2.0f, y, tamanho, cor);
        } else {
            gui_texto(fonte, linhas[i], area.x, y, tamanho, cor);
        }
        y += alturaLinha;
    }
}

int gui_mouseSobre(Gui *gui, Rectangle area)
{
    int sobre = CheckCollisionPointRec(GetMousePosition(), area);

    if (sobre) {
        gui->mouseSobreBotao = 1;
    }
    return sobre;
}

int gui_botao(Gui *gui, Rectangle area, const char *texto, EstiloBotao estilo, int ativo)
{
    int sobre = ativo && gui_mouseSobre(gui, area);
    int pressionado = sobre && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    Color fundo;
    Color corTexto = COR_TEXTO;
    float tamanho = fminf(area.height * 0.42f, 28.0f);
    Font fonte = (tamanho > 24.0f) ? gui->fontes.texto : gui->fontes.pequeno;
    Rectangle desenho = area;

    switch (estilo) {
    case BOTAO_DESTAQUE:
        fundo = sobre ? (Color){ 255, 214, 110, 255 } : COR_OURO;
        corTexto = (Color){ 30, 22, 6, 255 };
        break;
    case BOTAO_PERIGO:
        fundo = sobre ? (Color){ 245, 100, 84, 255 } : (Color){ 192, 57, 43, 255 };
        break;
    default:
        fundo = sobre ? (Color){ 34, 70, 50, 245 } : COR_PAINEL;
        break;
    }
    if (!ativo) {
        fundo = (Color){ 20, 30, 25, 200 };
        corTexto = COR_DESATIVADO;
    }
    if (pressionado) {
        desenho.y += 2.0f;
    }
    if (ativo && estilo != BOTAO_PADRAO) {
        DrawRectangleRounded((Rectangle){ area.x, area.y + 4, area.width, area.height }, 0.35f, 10,
                             (Color){ 0, 0, 0, 90 });
    }
    DrawRectangleRounded(desenho, 0.35f, 10, fundo);
    DrawRectangleRoundedLinesEx(desenho, 0.35f, 10, 1.5f, sobre ? COR_OURO : COR_BORDA);
    gui_textoCentralizado(fonte, texto, desenho.x + desenho.width / 2.0f,
                          desenho.y + (desenho.height - tamanho) / 2.0f, tamanho, corTexto);

    if (sobre && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        gui_tocar(gui, gui->sons.clique);
        return 1;
    }
    return 0;
}

void gui_desenharBola(Vector2 centro, float raio, float rotacao)
{
    int i;
    float angulo;
    Vector2 gomo;
    Color preto = { 24, 24, 28, 255 };

    DrawCircleV((Vector2){ centro.x + raio * 0.08f, centro.y + raio * 0.12f }, raio, (Color){ 0, 0, 0, 70 });
    DrawCircleV(centro, raio, (Color){ 246, 246, 242, 255 });
    DrawPoly(centro, 5, raio * 0.34f, rotacao, preto);
    for (i = 0; i < 5; i++) {
        angulo = (rotacao + 36.0f + 72.0f * (float)i) * DEG2RAD;
        gomo = (Vector2){ centro.x + cosf(angulo) * raio * 0.80f, centro.y + sinf(angulo) * raio * 0.80f };
        DrawPoly(gomo, 5, raio * 0.22f, rotacao + 36.0f, preto);
        /* Costura saindo de cada vértice do gomo central até a borda. */
        angulo = (rotacao + 72.0f * (float)i) * DEG2RAD;
        DrawLineEx((Vector2){ centro.x + cosf(angulo) * raio * 0.34f, centro.y + sinf(angulo) * raio * 0.34f },
                   (Vector2){ centro.x + cosf(angulo) * raio * 0.98f, centro.y + sinf(angulo) * raio * 0.98f },
                   fmaxf(1.0f, raio * 0.035f), (Color){ 60, 60, 66, 200 });
    }
    DrawRing(centro, raio, raio + raio * 0.06f, 0, 360, 48, (Color){ 220, 220, 214, 255 });
    DrawCircleGradient((int)(centro.x - raio * 0.35f), (int)(centro.y - raio * 0.4f), raio * 0.5f,
                       (Color){ 255, 255, 255, 90 }, (Color){ 255, 255, 255, 0 });
}

Color gui_corNivel(NivelDificuldade nivel)
{
    switch (nivel) {
    case NIVEL_MUITO_FACIL:   return (Color){ 46, 204, 113, 255 };
    case NIVEL_FACIL:         return (Color){ 52, 172, 224, 255 };
    case NIVEL_MEDIO:         return (Color){ 241, 196, 15, 255 };
    case NIVEL_DIFICIL:       return (Color){ 230, 126, 34, 255 };
    case NIVEL_MUITO_DIFICIL: return (Color){ 214, 69, 132, 255 };
    }
    return COR_TEXTO;
}

void gui_mostrarAviso(Gui *gui, const char *mensagem)
{
    snprintf(gui->aviso, sizeof(gui->aviso), "%s", mensagem);
    gui->tempoAviso = 2.6f;
}

void gui_desenharAviso(Gui *gui)
{
    float largura;
    float alfa;
    Rectangle caixa;

    if (gui->tempoAviso <= 0.0f) {
        return;
    }
    gui->tempoAviso -= GetFrameTime();
    alfa = fminf(1.0f, gui->tempoAviso / 0.4f);
    largura = gui_larguraTexto(gui->fontes.pequeno, gui->aviso, 24) + 48;
    /* Na partida o aviso fica no topo para não cobrir os valores de Errar/Parar/Acertar. */
    caixa = (Rectangle){ (LARGURA_TELA - largura) / 2.0f, gui->tela == TELA_JOGO ? 18.0f : ALTURA_TELA - 92.0f, largura, 48 };
    DrawRectangleRounded(caixa, 0.5f, 12, Fade((Color){ 10, 14, 12, 255 }, 0.92f * alfa));
    DrawRectangleRoundedLinesEx(caixa, 0.5f, 12, 2, Fade(COR_OURO, alfa));
    gui_textoCentralizado(gui->fontes.pequeno, gui->aviso, LARGURA_TELA / 2.0f, caixa.y + 11, 24,
                          Fade(COR_TEXTO, alfa));
}

void gui_soltarConfetes(Gui *gui, int quantidade)
{
    static const Color cores[] = {
        { 242, 193, 78, 255 }, { 46, 204, 113, 255 }, { 255, 255, 255, 255 },
        { 52, 152, 219, 255 }, { 231, 76, 60, 255 }
    };
    int i;
    Confete *confete;

    for (i = 0; i < MAX_CONFETES && quantidade > 0; i++) {
        confete = &gui->confetes[i];
        if (confete->vida > 0.0f) {
            continue;
        }
        confete->posicao = (Vector2){ (float)GetRandomValue(0, LARGURA_TELA), (float)GetRandomValue(-200, -10) };
        confete->velocidade = (Vector2){ (float)GetRandomValue(-90, 90), (float)GetRandomValue(120, 320) };
        confete->rotacao = (float)GetRandomValue(0, 360);
        confete->vida = (float)GetRandomValue(30, 55) / 10.0f;
        confete->cor = cores[GetRandomValue(0, 4)];
        quantidade--;
    }
}

void gui_atualizarConfetes(Gui *gui)
{
    int i;
    float dt = GetFrameTime();
    Confete *confete;

    for (i = 0; i < MAX_CONFETES; i++) {
        confete = &gui->confetes[i];
        if (confete->vida <= 0.0f) {
            continue;
        }
        confete->vida -= dt;
        confete->velocidade.y += 60.0f * dt;
        confete->posicao.x += (confete->velocidade.x + sinf(gui->tempo * 3.0f + (float)i) * 40.0f) * dt;
        confete->posicao.y += confete->velocidade.y * dt;
        confete->rotacao += 240.0f * dt;
        DrawRectanglePro((Rectangle){ confete->posicao.x, confete->posicao.y, 12, 7 }, (Vector2){ 6, 3.5f },
                         confete->rotacao, Fade(confete->cor, fminf(1.0f, confete->vida)));
    }
}
