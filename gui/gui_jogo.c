/**
 * @file gui_jogo.c
 * @brief Tela gráfica da partida: placar, pergunta, alternativas, escada de
 *        prêmios, ajudas (VAR, Torcida e Substituição) e revelação da resposta.
 * @author Equipe Futquiz
 */

#include "gui.h"
#include "arquivo.h"

#include <stdio.h>
#include <math.h>

/* ---- Layout da tela de jogo (janela de 1280 x 720) ---- */
#define MARGEM 24.0f
#define LARGURA_ESQUERDA 920.0f
#define Y_PERGUNTA 98.0f
#define ALTURA_PERGUNTA 168.0f
#define Y_ALTERNATIVAS 280.0f
#define ALTURA_ALTERNATIVA 62.0f
#define PASSO_ALTERNATIVA 74.0f
#define Y_BARRA 580.0f
#define ALTURA_BARRA 116.0f
#define X_PREMIOS 968.0f
#define LARGURA_PREMIOS 288.0f

/** Duração da animação do VAR (s). */
#define DURACAO_VAR 1.7f
/** Duração do suspense antes de revelar a resposta (s). */
#define DURACAO_SUSPENSE 1.4f
/** Duração da animação das barras da torcida (s). */
#define DURACAO_TORCIDA 0.9f

/** Ícones desenhados nos botões de ajuda. */
typedef enum {
    ICONE_VAR,
    ICONE_TORCIDA,
    ICONE_SUBSTITUICAO,
    ICONE_PARAR
} IconeAjuda;

/**
 * @brief Muda a etapa da rodada e zera o cronômetro da etapa.
 * @param gui Estado da interface.
 * @param etapa Nova etapa.
 */
static void mudarEtapa(Gui *gui, EtapaRodada etapa)
{
    gui->etapa = etapa;
    gui->tempoEtapa = 0.0f;
}

/**
 * @brief Partida que deve ser desenhada (a cópia congelada durante a revelação).
 * @param gui Estado da interface.
 * @return Partida a exibir.
 */
static const Partida *partidaExibida(const Gui *gui)
{
    if (gui->etapa == ETAPA_SUSPENSE || gui->etapa == ETAPA_REVELADA) {
        return &gui->exibida;
    }
    return &gui->partida;
}

/**
 * @brief Calcula o maior tamanho de fonte (até @p maximo) que cabe em uma largura.
 * @param fonte Fonte.
 * @param texto Texto.
 * @param maximo Tamanho máximo.
 * @param largura Largura disponível.
 * @return Tamanho da fonte.
 */
static float tamanhoQueCabe(Font fonte, const char *texto, float maximo, float largura)
{
    float tamanho = maximo;

    while (tamanho > 10.0f && gui_larguraTexto(fonte, texto, tamanho) > largura) {
        tamanho -= 1.0f;
    }
    return tamanho;
}

/**
 * @brief Encerra a partida: registra no ranking e vai para a tela de resultado.
 * @param gui Estado da interface.
 */
static void finalizarPartida(Gui *gui)
{
    gui->posicaoRanking = ranking_inserir(gui->ranking, gui->nome, gui->partida.premioFinal,
                                          partida_acertos(&gui->partida));
    if (gui->posicaoRanking > 0 && !ranking_salvar(gui->ranking, CAMINHO_RANKING)) {
        gui_mostrarAviso(gui, "Não foi possível salvar o ranking.");
    }
    if (gui->partida.estado == PARTIDA_CAMPEAO) {
        gui_soltarConfetes(gui, MAX_CONFETES);
    } else if (gui->posicaoRanking > 0) {
        gui_soltarConfetes(gui, 60);
    }
    if (gui->posicaoRanking > 0 || gui->partida.estado == PARTIDA_CAMPEAO) {
        gui_tocar(gui, gui->sons.torcida);
    }
    gui->tela = TELA_RESULTADO;
}

int gui_iniciarPartida(Gui *gui)
{
    if (!partida_iniciar(&gui->partida, gui->banco, gui->categoria)) {
        return 0;
    }
    gui->selecionada = -1;
    mudarEtapa(gui, ETAPA_ESCOLHENDO);
    gui_tocar(gui, gui->sons.apito);
    gui->tela = TELA_JOGO;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Desenho das partes da tela                                          */
/* ------------------------------------------------------------------ */

/**
 * @brief Desenha o placar do topo (jogador, rodada, nível e categoria).
 * @param gui Estado da interface.
 * @param partida Partida exibida.
 */
static void desenharPlacar(const Gui *gui, const Partida *partida)
{
    Rectangle area = { MARGEM, MARGEM, LARGURA_ESQUERDA, 60 };
    const Pergunta *pergunta = partida_perguntaAtual(partida);
    Color corNivel = gui_corNivel(pergunta->nivel);
    char rodada[32];
    float x;

    gui_desenharPainel(area, COR_PAINEL);
    snprintf(rodada, sizeof(rodada), "RODADA %02d/%d", partida->rodada + 1, NUM_RODADAS);

    gui_desenharBola((Vector2){ area.x + 32, area.y + 30 }, 15, gui->tempo * 40.0f);
    gui_texto(gui->fontes.titulo, gui->nome, area.x + 58, area.y + 10, 36, COR_TEXTO);
    x = area.x + 58 + gui_larguraTexto(gui->fontes.titulo, gui->nome, 36) + 18;
    DrawRectangle((int)x, (int)area.y + 14, 3, 32, COR_OURO);
    gui_texto(gui->fontes.titulo, rodada, x + 16, area.y + 10, 36, COR_OURO);

    /* Etiqueta do nível com a cor dele. */
    x = area.x + area.width - 16 - gui_larguraTexto(gui->fontes.pequeno, pergunta_nomeCategoria(pergunta->categoria), 22);
    gui_texto(gui->fontes.pequeno, pergunta_nomeCategoria(pergunta->categoria), x, area.y + 18, 22, COR_TEXTO_SUAVE);
    x -= gui_larguraTexto(gui->fontes.pequeno, pergunta_nomeNivel(pergunta->nivel), 22) + 50;
    DrawRectangleRounded((Rectangle){ x - 2, area.y + 14, gui_larguraTexto(gui->fontes.pequeno,
                         pergunta_nomeNivel(pergunta->nivel), 22) + 34, 32 }, 0.6f, 10, Fade(corNivel, 0.22f));
    DrawCircle((int)x + 10, (int)area.y + 30, 5, corNivel);
    gui_texto(gui->fontes.pequeno, pergunta_nomeNivel(pergunta->nivel), x + 20, area.y + 18, 22, corNivel);
}

/**
 * @brief Desenha o painel com o enunciado da pergunta.
 * @param gui Estado da interface.
 * @param partida Partida exibida.
 */
static void desenharPergunta(const Gui *gui, const Partida *partida)
{
    Rectangle area = { MARGEM, Y_PERGUNTA, LARGURA_ESQUERDA, ALTURA_PERGUNTA };
    const Pergunta *pergunta = partida_perguntaAtual(partida);

    gui_desenharPainel(area, COR_PAINEL);
    DrawRectangle((int)area.x, (int)area.y + 24, 5, (int)area.height - 48, gui_corNivel(pergunta->nivel));
    if (partida->rodada == NUM_RODADAS - 1) {
        gui_textoCentralizado(gui->fontes.titulo, "PERGUNTA DO MILHÃO", area.x + area.width / 2.0f,
                              area.y + 10, 24, COR_OURO);
    }
    gui_textoQuebrado(gui->fontes.texto, pergunta->enunciado,
                      (Rectangle){ area.x + 40, area.y + 30, area.width - 80, area.height - 50 },
                      36, 22, 1, COR_TEXTO);
}

/**
 * @brief Desenha uma alternativa e trata o clique nela.
 * @param gui Estado da interface.
 * @param partida Partida exibida.
 * @param indice Alternativa (0 a 3).
 * @param interativo 1 se pode ser clicada nesta etapa.
 */
static void desenharAlternativa(Gui *gui, const Partida *partida, int indice, int interativo)
{
    Rectangle area = { MARGEM, Y_ALTERNATIVAS + indice * PASSO_ALTERNATIVA, LARGURA_ESQUERDA, ALTURA_ALTERNATIVA };
    const Pergunta *pergunta = partida_perguntaAtual(partida);
    /* Durante a animação do VAR as anuladas ainda não aparecem. */
    int eliminada = partida->eliminadas[indice] && gui->etapa != ETAPA_VAR;
    int sobre = interativo && !eliminada && gui_mouseSobre(gui, area);
    int correta = (pergunta->correta - 'A') == indice;
    int selecionada = gui->selecionada == indice;
    Color fundo = COR_PAINEL;
    Color borda = COR_BORDA;
    Color corTexto = COR_TEXTO;
    char letra[2] = { (char)('A' + indice), '\0' };
    char porcentagem[8];
    float progresso;
    float larguraTexto = area.width - 96 - (partida->torcidaAtiva ? 90 : 20);

    if (sobre) {
        fundo = (Color){ 30, 64, 46, 240 };
        borda = COR_OURO;
    }
    if (selecionada && (gui->etapa == ETAPA_CONFIRMANDO || gui->etapa == ETAPA_SUSPENSE)) {
        float pulso = (gui->etapa == ETAPA_SUSPENSE) ? 0.5f + 0.5f * sinf(gui->tempo * 14.0f) : 1.0f;
        fundo = ColorLerp((Color){ 120, 70, 8, 240 }, COR_ALERTA, pulso);
        borda = COR_OURO;
    }
    if (gui->etapa == ETAPA_REVELADA) {
        if (correta) {
            fundo = (Color){ 30, 150, 80, 245 };
            borda = COR_ACERTO;
        } else if (selecionada) {
            fundo = (Color){ 170, 45, 35, 245 };
            borda = COR_ERRO;
        }
    }
    if (eliminada) {
        fundo = Fade(COR_PAINEL, 0.45f);
        corTexto = COR_DESATIVADO;
    }

    DrawRectangleRounded(area, 0.5f, 12, fundo);
    DrawRectangleRoundedLinesEx(area, 0.5f, 12, 2.0f, borda);

    /* Barra de votos da torcida, crescendo com animação. */
    if (partida->torcidaAtiva && !eliminada) {
        progresso = fminf(1.0f, gui->tempoTorcida / DURACAO_TORCIDA);
        progresso = 1.0f - powf(1.0f - progresso, 3.0f);
        DrawRectangleRounded((Rectangle){ area.x + 4, area.y + 4,
                             (area.width - 8) * partida->votosTorcida[indice] / 100.0f * progresso,
                             area.height - 8 }, 0.5f, 12, (Color){ 52, 152, 219, 70 });
        snprintf(porcentagem, sizeof(porcentagem), "%d%%",
                 (int)roundf((float)partida->votosTorcida[indice] * progresso));
        gui_texto(gui->fontes.titulo, porcentagem,
                  area.x + area.width - 20 - gui_larguraTexto(gui->fontes.titulo, porcentagem, 32),
                  area.y + 13, 32, (Color){ 120, 200, 255, 255 });
    }

    /* Letra em um círculo dourado. */
    DrawCircle((int)area.x + 38, (int)(area.y + area.height / 2), 21, eliminada ? COR_DESATIVADO : COR_OURO);
    gui_textoCentralizado(gui->fontes.titulo, letra, area.x + 38, area.y + area.height / 2 - 16, 30,
                          (Color){ 30, 22, 6, 255 });

    gui_textoQuebrado(gui->fontes.texto, pergunta->alternativas[indice],
                      (Rectangle){ area.x + 76, area.y + 6, larguraTexto, area.height - 12 }, 26, 18, 0, corTexto);

    if (eliminada) {
        Rectangle selo = { area.x + area.width - 210, area.y + 16, 190, 30 };
        DrawLineEx((Vector2){ area.x + 70, area.y + area.height / 2 },
                   (Vector2){ selo.x - 16, area.y + area.height / 2 }, 2, Fade(COR_ERRO, 0.6f));
        DrawRectangleRounded(selo, 0.5f, 10, Fade(COR_ERRO, 0.85f));
        gui_textoCentralizado(gui->fontes.titulo, "ANULADA PELO VAR", selo.x + selo.width / 2, selo.y + 3, 22,
                              COR_TEXTO);
    }

    if (sobre && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        gui_tocar(gui, gui->sons.clique);
        gui->selecionada = indice;
        mudarEtapa(gui, ETAPA_CONFIRMANDO);
    }
}

/**
 * @brief Desenha a escada de prêmios do lado direito.
 * @param gui Estado da interface.
 * @param partida Partida exibida.
 */
static void desenharEscadaPremios(const Gui *gui, const Partida *partida)
{
    Rectangle painel = { X_PREMIOS, MARGEM, LARGURA_PREMIOS, ALTURA_TELA - 2 * MARGEM };
    Rectangle linha;
    int rodada;
    char numero[8];
    char premio[32];
    Color corTexto;
    float pulso;
    /* Na revelação de um acerto, a rodada respondida já conta como conquistada. */
    int acabouDeAcertar = gui->etapa == ETAPA_REVELADA && gui->resultado != RESPOSTA_ERRADA;
    int conquistadas = partida->rodada + acabouDeAcertar;

    gui_desenharPainel(painel, COR_PAINEL);
    gui_textoCentralizado(gui->fontes.titulo, "PRÊMIOS", painel.x + painel.width / 2, painel.y + 12, 34, COR_OURO);

    for (rodada = 0; rodada < NUM_RODADAS; rodada++) {
        linha = (Rectangle){ painel.x + 12, painel.y + 62 + (NUM_RODADAS - 1 - rodada) * 39.6f, painel.width - 24, 34 };
        snprintf(numero, sizeof(numero), "%d", rodada + 1);
        partida_formatarPremio(partida_premioDaRodada(rodada), premio, sizeof(premio));

        if (rodada < conquistadas) {
            corTexto = COR_ACERTO;
        } else if (rodada == NUM_RODADAS - 1) {
            corTexto = COR_OURO;
        } else {
            corTexto = COR_TEXTO_SUAVE;
        }
        if (partida_ehMarco(rodada)) {
            DrawRectangleRounded(linha, 0.5f, 10, (Color){ 40, 90, 160, 90 });
            gui_texto(gui->fontes.pequeno, "MARCO", linha.x + 46, linha.y + 8, 16, (Color){ 140, 190, 255, 255 });
        }
        if (rodada == partida->rodada && !acabouDeAcertar && partida->estado == PARTIDA_EM_ANDAMENTO) {
            pulso = 0.85f + 0.15f * sinf(gui->tempo * 4.0f);
            DrawRectangleRounded(linha, 0.5f, 10, Fade(COR_OURO, pulso));
            corTexto = (Color){ 30, 22, 6, 255 };
        }
        gui_texto(gui->fontes.titulo, numero, linha.x + 12, linha.y + 3, 24, corTexto);
        gui_texto(gui->fontes.titulo, premio, linha.x + linha.width - 12 - gui_larguraTexto(gui->fontes.titulo,
                  premio, 24), linha.y + 3, 24, corTexto);
    }
}

/**
 * @brief Desenha o ícone de um botão de ajuda.
 * @param icone Tipo de ícone.
 * @param centro Centro do ícone.
 * @param cor Cor principal.
 */
static void desenharIcone(IconeAjuda icone, Vector2 centro, Color cor)
{
    switch (icone) {
    case ICONE_VAR:
        DrawRectangleRoundedLinesEx((Rectangle){ centro.x - 22, centro.y - 15, 44, 28 }, 0.25f, 6, 3, cor);
        DrawRectangle((int)centro.x - 2, (int)centro.y + 13, 4, 6, cor);
        DrawRectangle((int)centro.x - 12, (int)centro.y + 18, 24, 3, cor);
        DrawTriangle((Vector2){ centro.x - 5, centro.y - 8 }, (Vector2){ centro.x - 5, centro.y + 6 },
                     (Vector2){ centro.x + 8, centro.y - 1 }, cor);
        break;
    case ICONE_TORCIDA:
        DrawRectangle((int)centro.x - 20, (int)centro.y - 2, 10, 20, cor);
        DrawRectangle((int)centro.x - 5, (int)centro.y - 16, 10, 34, cor);
        DrawRectangle((int)centro.x + 10, (int)centro.y - 8, 10, 26, cor);
        break;
    case ICONE_SUBSTITUICAO:
        DrawTriangle((Vector2){ centro.x - 12, centro.y - 20 }, (Vector2){ centro.x - 22, centro.y - 4 },
                     (Vector2){ centro.x - 2, centro.y - 4 }, cor.a == 255 ? COR_ACERTO : cor);
        DrawRectangle((int)centro.x - 16, (int)centro.y - 4, 8, 22, cor.a == 255 ? COR_ACERTO : cor);
        DrawTriangle((Vector2){ centro.x + 12, centro.y + 20 }, (Vector2){ centro.x + 22, centro.y + 4 },
                     (Vector2){ centro.x + 2, centro.y + 4 }, cor.a == 255 ? COR_ERRO : cor);
        DrawRectangle((int)centro.x + 8, (int)centro.y - 18, 8, 22, cor.a == 255 ? COR_ERRO : cor);
        break;
    case ICONE_PARAR:
        DrawPoly(centro, 8, 20, 22.5f, cor);
        DrawRectangle((int)centro.x - 11, (int)centro.y - 3, 22, 6, COR_TEXTO);
        break;
    }
}

/**
 * @brief Desenha um botão de ajuda e trata o clique.
 * @param gui Estado da interface.
 * @param area Retângulo do botão.
 * @param titulo Nome da ajuda.
 * @param detalhe Texto pequeno (usos restantes).
 * @param icone Ícone.
 * @param ativo 1 se disponível.
 * @return 1 se foi clicado.
 */
static int botaoAjuda(Gui *gui, Rectangle area, const char *titulo, const char *detalhe, IconeAjuda icone, int ativo)
{
    int sobre = ativo && gui_mouseSobre(gui, area);
    Color corIcone = ativo ? (icone == ICONE_PARAR ? COR_ERRO : COR_OURO) : Fade(COR_DESATIVADO, 0.7f);
    float tamanho = tamanhoQueCabe(gui->fontes.titulo, titulo, 26, area.width - 14);

    gui_desenharPainel(area, sobre ? (Color){ 30, 64, 46, 240 } : COR_PAINEL);
    if (sobre) {
        DrawRectangleRoundedLinesEx(area, 0.25f, 10, 2, COR_OURO);
    }
    desenharIcone(icone, (Vector2){ area.x + area.width / 2, area.y + 32 }, corIcone);
    gui_textoCentralizado(gui->fontes.titulo, titulo, area.x + area.width / 2, area.y + 58, tamanho,
                          ativo ? COR_TEXTO : COR_DESATIVADO);
    gui_textoCentralizado(gui->fontes.pequeno, detalhe, area.x + area.width / 2, area.y + 90,
                          tamanhoQueCabe(gui->fontes.pequeno, detalhe, 16, area.width - 12),
                          ativo ? COR_TEXTO_SUAVE : COR_DESATIVADO);
    if (sobre && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        gui_tocar(gui, gui->sons.clique);
        return 1;
    }
    return 0;
}

/**
 * @brief Desenha uma caixa de valor (Errar, Parar ou Acertar).
 * @param gui Estado da interface.
 * @param area Retângulo.
 * @param rotulo Rótulo.
 * @param valor Valor em reais.
 * @param cor Cor do rótulo.
 */
static void caixaValor(const Gui *gui, Rectangle area, const char *rotulo, int valor, Color cor)
{
    char texto[32];
    float tamanho;

    partida_formatarPremio(valor, texto, sizeof(texto));
    tamanho = tamanhoQueCabe(gui->fontes.titulo, texto, 30, area.width - 16);
    gui_desenharPainel(area, COR_PAINEL);
    DrawRectangleRounded((Rectangle){ area.x + 10, area.y + 12, area.width - 20, 30 }, 0.5f, 10, Fade(cor, 0.2f));
    gui_textoCentralizado(gui->fontes.titulo, rotulo, area.x + area.width / 2, area.y + 14, 24, cor);
    gui_textoCentralizado(gui->fontes.titulo, texto, area.x + area.width / 2, area.y + 62, tamanho, COR_TEXTO);
}

/* ------------------------------------------------------------------ */
/* Ações da rodada                                                     */
/* ------------------------------------------------------------------ */

/**
 * @brief Usa uma ajuda e dá o retorno visual e sonoro.
 * @param gui Estado da interface.
 * @param icone Ajuda escolhida.
 */
static void usarAjuda(Gui *gui, IconeAjuda icone)
{
    if (icone == ICONE_PARAR) {
        mudarEtapa(gui, ETAPA_CONFIRMAR_PARAR);
        return;
    }
    if (!partida_ajudasLiberadas(&gui->partida)) {
        gui_mostrarAviso(gui, "Na pergunta do milhão não há ajudas!");
        return;
    }
    if (icone == ICONE_VAR) {
        if (partida_usarVar(&gui->partida)) {
            gui_tocar(gui, gui->sons.apito);
            mudarEtapa(gui, ETAPA_VAR);
        } else {
            gui_mostrarAviso(gui, "O VAR já foi usado nesta partida.");
        }
    } else if (icone == ICONE_TORCIDA) {
        if (partida_usarTorcida(&gui->partida)) {
            gui->tempoTorcida = 0.0f;
            gui_tocar(gui, gui->sons.torcida);
            gui_mostrarAviso(gui, "A torcida votou!");
        } else {
            gui_mostrarAviso(gui, "A torcida já ajudou nesta partida.");
        }
    } else if (partida_substituir(&gui->partida)) {
        gui->selecionada = -1;
        gui_tocar(gui, gui->sons.apito);
        gui_mostrarAviso(gui, "Substituição! Nova pergunta em campo.");
    } else {
        gui_mostrarAviso(gui, "Sem substituições ou sem perguntas reservas.");
    }
}

/**
 * @brief Confirma a alternativa escolhida e começa o suspense.
 * @param gui Estado da interface.
 */
static void confirmarResposta(Gui *gui)
{
    int rodada = gui->partida.rodada;

    gui->exibida = gui->partida; /* congela o que está na tela */
    gui->resultado = partida_responder(&gui->partida, (char)('A' + gui->selecionada));
    if (gui->resultado == RESPOSTA_ERRADA) {
        gui->mensagem = partida_mensagemErro();
    } else {
        gui->mensagem = partida_mensagemAcerto(rodada);
    }
    mudarEtapa(gui, ETAPA_SUSPENSE);
}

/**
 * @brief Revela a resposta depois do suspense (som e confetes).
 * @param gui Estado da interface.
 */
static void revelarResposta(Gui *gui)
{
    mudarEtapa(gui, ETAPA_REVELADA);
    if (gui->resultado == RESPOSTA_ERRADA) {
        gui_tocar(gui, gui->sons.erro);
    } else if (gui->resultado == RESPOSTA_CAMPEAO) {
        gui_tocar(gui, gui->sons.campeao);
        gui_soltarConfetes(gui, MAX_CONFETES);
    } else {
        gui_tocar(gui, gui->sons.acerto);
        if (partida_ehMarco(gui->exibida.rodada)) {
            gui_soltarConfetes(gui, 90);
        }
    }
}

/**
 * @brief Avança depois da revelação: próxima rodada ou fim de jogo.
 * @param gui Estado da interface.
 */
static void continuarAposRevelar(Gui *gui)
{
    gui->selecionada = -1;
    if (gui->resultado == RESPOSTA_CERTA) {
        mudarEtapa(gui, ETAPA_ESCOLHENDO);
    } else {
        finalizarPartida(gui);
    }
}

/* ------------------------------------------------------------------ */
/* Barra inferior (ajudas, confirmação e revelação)                    */
/* ------------------------------------------------------------------ */

/**
 * @brief Barra inferior padrão: ajudas e valores de errar/parar/acertar.
 * @param gui Estado da interface.
 */
static void barraAjudas(Gui *gui)
{
    Partida *partida = &gui->partida;
    int liberadas = partida_ajudasLiberadas(partida) && gui->etapa == ETAPA_ESCOLHENDO;
    char detalhe[32];
    float x = MARGEM;

    if (botaoAjuda(gui, (Rectangle){ x, Y_BARRA, 118, ALTURA_BARRA }, "VAR",
                   partida->varDisponivel ? "anula 2 erradas" : "usado", ICONE_VAR,
                   liberadas && partida->varDisponivel)) {
        usarAjuda(gui, ICONE_VAR);
    }
    x += 128;
    if (botaoAjuda(gui, (Rectangle){ x, Y_BARRA, 118, ALTURA_BARRA }, "TORCIDA",
                   partida->torcidaDisponivel ? "votos do público" : "usado", ICONE_TORCIDA,
                   liberadas && partida->torcidaDisponivel)) {
        usarAjuda(gui, ICONE_TORCIDA);
    }
    x += 128;
    snprintf(detalhe, sizeof(detalhe), "%d restante(s)", partida->substituicoesRestantes);
    if (botaoAjuda(gui, (Rectangle){ x, Y_BARRA, 118, ALTURA_BARRA }, "SUBSTITUIÇÃO", detalhe, ICONE_SUBSTITUICAO,
                   liberadas && partida->substituicoesRestantes > 0)) {
        usarAjuda(gui, ICONE_SUBSTITUICAO);
    }
    x += 128;
    if (botaoAjuda(gui, (Rectangle){ x, Y_BARRA, 118, ALTURA_BARRA }, "PARAR", "leva o acumulado", ICONE_PARAR,
                   gui->etapa == ETAPA_ESCOLHENDO)) {
        usarAjuda(gui, ICONE_PARAR);
    }

    caixaValor(gui, (Rectangle){ 546, Y_BARRA, 126, ALTURA_BARRA }, "ERRAR", partida_premioSeErrar(partida), COR_ERRO);
    caixaValor(gui, (Rectangle){ 682, Y_BARRA, 126, ALTURA_BARRA }, "PARAR", partida_premioSeParar(partida), COR_OURO);
    caixaValor(gui, (Rectangle){ 818, Y_BARRA, 126, ALTURA_BARRA }, "ACERTAR", partida_premioSeAcertar(partida),
               COR_ACERTO);
}

/**
 * @brief Barra "Está certo disso?" com os botões de confirmação.
 * @param gui Estado da interface.
 */
static void barraConfirmacao(Gui *gui)
{
    Rectangle area = { MARGEM, Y_BARRA, LARGURA_ESQUERDA, ALTURA_BARRA };
    char texto[64];

    gui_desenharPainel(area, COR_PAINEL_CLARO);
    snprintf(texto, sizeof(texto), "Alternativa %c. Está certo disso?", 'A' + gui->selecionada);
    gui_texto(gui->fontes.titulo, texto, area.x + 32, area.y + 32, 44, COR_OURO);
    if (gui_botao(gui, (Rectangle){ area.x + 556, area.y + 30, 200, 56 }, "SIM, CONFIRMO", BOTAO_DESTAQUE, 1) ||
        IsKeyPressed(KEY_ENTER)) {
        confirmarResposta(gui);
        return;
    }
    if (gui_botao(gui, (Rectangle){ area.x + 772, area.y + 30, 124, 56 }, "NÃO", BOTAO_PADRAO, 1) ||
        (IsKeyPressed(KEY_ESCAPE) && gui->tempoEtapa > 0.0f)) {
        gui->selecionada = -1;
        mudarEtapa(gui, ETAPA_ESCOLHENDO);
    }
}

/**
 * @brief Barra com a mensagem comemorativa (ou de erro) após a revelação.
 * @param gui Estado da interface.
 */
static void barraRevelacao(Gui *gui)
{
    Rectangle area = { MARGEM, Y_BARRA, LARGURA_ESQUERDA, ALTURA_BARRA };
    const Pergunta *pergunta = partida_perguntaAtual(&gui->exibida);
    int acertou = gui->resultado != RESPOSTA_ERRADA;
    float escala = fminf(1.0f, gui->tempoEtapa / 0.25f);
    float tamanho = tamanhoQueCabe(gui->fontes.titulo, gui->mensagem, 50, 560) * (0.6f + 0.4f * escala);
    char detalhe[160];
    char premio[32];

    gui_desenharPainel(area, acertou ? (Color){ 16, 70, 38, 240 } : (Color){ 80, 20, 16, 240 });
    gui_texto(gui->fontes.titulo, gui->mensagem, area.x + 32, area.y + 14 + (50 - tamanho) / 2, tamanho,
              acertou ? COR_OURO : COR_TEXTO);

    if (gui->resultado == RESPOSTA_CERTA) {
        partida_formatarPremio(partida_premioSeParar(&gui->partida), premio, sizeof(premio));
        snprintf(detalhe, sizeof(detalhe), "Você já garantiu %s.", premio);
    } else if (gui->resultado == RESPOSTA_CAMPEAO) {
        snprintf(detalhe, sizeof(detalhe), "Você acertou as 15 perguntas!");
    } else {
        snprintf(detalhe, sizeof(detalhe), "A certa era (%c) %s", pergunta->correta,
                 pergunta->alternativas[pergunta->correta - 'A']);
    }
    gui_textoQuebrado(gui->fontes.texto, detalhe, (Rectangle){ area.x + 32, area.y + 70, 640, 34 }, 22, 16, 0,
                      COR_TEXTO);

    if (gui->tempoEtapa > 0.4f &&
        (gui_botao(gui, (Rectangle){ area.x + area.width - 232, area.y + 30, 200, 56 },
                   gui->resultado == RESPOSTA_CERTA ? "PRÓXIMA" : "VER RESULTADO", BOTAO_DESTAQUE, 1) ||
         IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
        continuarAposRevelar(gui);
    }
}

/**
 * @brief Janela de confirmação para parar o jogo.
 * @param gui Estado da interface.
 */
static void janelaParar(Gui *gui)
{
    Rectangle janela = { (LARGURA_TELA - 560) / 2.0f, (ALTURA_TELA - 280) / 2.0f, 560, 280 };
    char texto[80];
    char premio[32];

    DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, (Color){ 0, 0, 0, 160 });
    gui_desenharPainel(janela, (Color){ 12, 26, 18, 250 });
    gui_textoCentralizado(gui->fontes.titulo, "PARAR AGORA?", janela.x + janela.width / 2, janela.y + 30, 56,
                          COR_OURO);
    partida_formatarPremio(partida_premioSeParar(&gui->partida), premio, sizeof(premio));
    snprintf(texto, sizeof(texto), "Você sai de campo levando %s.", premio);
    gui_textoCentralizado(gui->fontes.texto, texto, janela.x + janela.width / 2, janela.y + 116, 26, COR_TEXTO);

    if (gui_botao(gui, (Rectangle){ janela.x + 40, janela.y + 186, 230, 60 }, "PARAR E LEVAR", BOTAO_PERIGO, 1)) {
        partida_parar(&gui->partida);
        finalizarPartida(gui);
        return;
    }
    /* tempoEtapa > 0 evita que o mesmo ESC que abriu a janela já a feche. */
    if (gui_botao(gui, (Rectangle){ janela.x + 290, janela.y + 186, 230, 60 }, "CONTINUAR", BOTAO_PADRAO, 1) ||
        (IsKeyPressed(KEY_ESCAPE) && gui->tempoEtapa > 0.0f)) {
        mudarEtapa(gui, ETAPA_ESCOLHENDO);
    }
}

/**
 * @brief Animação do VAR revisando o lance.
 * @param gui Estado da interface.
 */
static void animacaoVar(Gui *gui)
{
    Rectangle area = { MARGEM, Y_ALTERNATIVAS - 4, LARGURA_ESQUERDA, 4 * PASSO_ALTERNATIVA - 4 };
    float varredura = fmodf(gui->tempoEtapa * 1.6f, 1.0f);
    int pontos = (int)(gui->tempoEtapa * 3.0f) % 4;
    char texto[40] = "REVISANDO O LANCE";
    int i;

    DrawRectangleRounded(area, 0.08f, 10, (Color){ 4, 10, 18, 235 });
    DrawRectangleRoundedLinesEx(area, 0.08f, 10, 3, (Color){ 52, 152, 219, 255 });
    for (i = 0; i < pontos; i++) {
        texto[17 + i] = '.';
        texto[18 + i] = '\0';
    }
    gui_textoCentralizado(gui->fontes.titulo, "VAR", area.x + area.width / 2, area.y + 50, 110,
                          (Color){ 120, 200, 255, 255 });
    gui_textoCentralizado(gui->fontes.titulo, texto, area.x + area.width / 2, area.y + 186, 34, COR_TEXTO);
    DrawRectangle((int)area.x + 8, (int)(area.y + 8 + varredura * (area.height - 16)), (int)area.width - 16, 3,
                  (Color){ 120, 200, 255, 160 });
    if (gui->tempoEtapa >= DURACAO_VAR) {
        mudarEtapa(gui, ETAPA_ESCOLHENDO);
        gui_mostrarAviso(gui, "O VAR anulou duas alternativas erradas!");
    }
}

/**
 * @brief Atalhos de teclado da etapa de escolha.
 * @param gui Estado da interface.
 */
static void tratarTeclado(Gui *gui)
{
    int i;

    if (gui->etapa != ETAPA_ESCOLHENDO && gui->etapa != ETAPA_CONFIRMANDO) {
        return;
    }
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        if (IsKeyPressed(KEY_A + i) && !gui->partida.eliminadas[i]) {
            gui_tocar(gui, gui->sons.clique);
            gui->selecionada = i;
            mudarEtapa(gui, ETAPA_CONFIRMANDO);
            return;
        }
    }
    if (gui->etapa != ETAPA_ESCOLHENDO) {
        return;
    }
    if (IsKeyPressed(KEY_ONE)) {
        usarAjuda(gui, ICONE_VAR);
    } else if (IsKeyPressed(KEY_TWO)) {
        usarAjuda(gui, ICONE_TORCIDA);
    } else if (IsKeyPressed(KEY_THREE)) {
        usarAjuda(gui, ICONE_SUBSTITUICAO);
    } else if (IsKeyPressed(KEY_ZERO) || IsKeyPressed(KEY_ESCAPE)) {
        usarAjuda(gui, ICONE_PARAR);
    }
}

void gui_telaJogo(Gui *gui)
{
    const Partida *partida;
    int interativo;
    int i;

    gui->tempoEtapa += GetFrameTime();
    gui->tempoTorcida += GetFrameTime();
    tratarTeclado(gui);
    if (gui->etapa == ETAPA_SUSPENSE && gui->tempoEtapa >= DURACAO_SUSPENSE) {
        revelarResposta(gui);
    }
    if (gui->tela != TELA_JOGO) {
        return;
    }

    partida = partidaExibida(gui);
    interativo = gui->etapa == ETAPA_ESCOLHENDO || gui->etapa == ETAPA_CONFIRMANDO;

    desenharPlacar(gui, partida);
    desenharPergunta(gui, partida);
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        desenharAlternativa(gui, partida, i, interativo);
    }
    desenharEscadaPremios(gui, partida);

    switch (gui->etapa) {
    case ETAPA_CONFIRMANDO:
        barraConfirmacao(gui);
        break;
    case ETAPA_REVELADA:
        barraRevelacao(gui);
        break;
    case ETAPA_VAR:
        barraAjudas(gui);
        animacaoVar(gui);
        break;
    case ETAPA_CONFIRMAR_PARAR:
        barraAjudas(gui);
        janelaParar(gui);
        break;
    default:
        barraAjudas(gui);
        break;
    }
}
