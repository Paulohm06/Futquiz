/**
 * @file gui.h
 * @brief Tipos e funções compartilhadas da interface gráfica (raylib).
 *
 * A interface gráfica reaproveita os módulos do núcleo (banco, arquivo,
 * partida e ranking). Toda a informação da tela fica na struct Gui, que é
 * passada por ponteiro para as funções (sem variáveis globais).
 *
 * @author Equipe Futquiz
 */

#ifndef GUI_H
#define GUI_H

#include "raylib.h"
#include "banco.h"
#include "partida.h"
#include "ranking.h"

/** Largura da janela. */
#define LARGURA_TELA 1280
/** Altura da janela. */
#define ALTURA_TELA 720
/** Quantidade máxima de confetes na tela. */
#define MAX_CONFETES 180

/* ---- Paleta de cores ---- */
#define COR_GRAMA_CLARA   (Color){ 24, 104, 56, 255 }
#define COR_GRAMA_ESCURA  (Color){ 19, 90, 48, 255 }
#define COR_PAINEL        (Color){ 7, 18, 13, 220 }
#define COR_PAINEL_CLARO  (Color){ 20, 40, 30, 235 }
#define COR_BORDA         (Color){ 255, 255, 255, 38 }
#define COR_OURO          (Color){ 242, 193, 78, 255 }
#define COR_OURO_ESCURO   (Color){ 184, 134, 36, 255 }
#define COR_ACERTO        (Color){ 46, 204, 113, 255 }
#define COR_ERRO          (Color){ 231, 76, 60, 255 }
#define COR_ALERTA        (Color){ 243, 156, 18, 255 }
#define COR_TEXTO         (Color){ 242, 242, 236, 255 }
#define COR_TEXTO_SUAVE   (Color){ 168, 192, 176, 255 }
#define COR_DESATIVADO    (Color){ 90, 104, 96, 255 }

/**
 * @brief Telas da interface.
 */
typedef enum {
    TELA_MENU,       /**< Menu inicial.                       */
    TELA_NOME,       /**< Nome do jogador e categoria.        */
    TELA_JOGO,       /**< Partida em andamento.               */
    TELA_RESULTADO,  /**< Fim da partida.                     */
    TELA_RANKING,    /**< Ranking dos jogadores.              */
    TELA_NAVEGAR,    /**< Navegação pelas perguntas por nível. */
    TELA_SAIR        /**< Pedido para fechar o programa.      */
} Tela;

/**
 * @brief Etapas de uma rodada na tela de jogo.
 */
typedef enum {
    ETAPA_ESCOLHENDO,      /**< Jogador escolhendo uma alternativa. */
    ETAPA_CONFIRMANDO,     /**< "Está certo disso?"                 */
    ETAPA_CONFIRMAR_PARAR, /**< Confirmação de parar o jogo.        */
    ETAPA_VAR,             /**< Animação do VAR revisando o lance.  */
    ETAPA_SUSPENSE,        /**< Suspense antes de revelar.          */
    ETAPA_REVELADA         /**< Resposta revelada.                  */
} EtapaRodada;

/**
 * @brief Estilos visuais de botão.
 */
typedef enum {
    BOTAO_PADRAO,    /**< Botão escuro com borda.  */
    BOTAO_DESTAQUE,  /**< Botão dourado.           */
    BOTAO_PERIGO     /**< Botão vermelho.          */
} EstiloBotao;

/**
 * @brief Fontes carregadas.
 */
typedef struct {
    Font titulo;   /**< Anton: títulos, placar e números. */
    Font texto;    /**< Fira Sans SemiBold grande.        */
    Font pequeno;  /**< Fira Sans SemiBold pequena.       */
} Fontes;

/**
 * @brief Sons gerados por código (não precisam de arquivos).
 */
typedef struct {
    Sound clique;   /**< Clique de botão.          */
    Sound acerto;   /**< Arpejo de acerto.         */
    Sound erro;     /**< Buzina de erro.           */
    Sound apito;    /**< Apito do árbitro.         */
    Sound torcida;  /**< Barulho da torcida.       */
    Sound campeao;  /**< Fanfarra de campeão.      */
    int ativo;      /**< 1 se o áudio funcionou.   */
} Sons;

/**
 * @brief Um pedaço de confete da comemoração.
 */
typedef struct {
    Vector2 posicao;    /**< Posição na tela.        */
    Vector2 velocidade; /**< Velocidade (px/s).      */
    float rotacao;      /**< Ângulo em graus.        */
    float vida;         /**< Segundos restantes.     */
    Color cor;          /**< Cor do confete.         */
} Confete;

/**
 * @brief Todo o estado da interface gráfica.
 */
typedef struct {
    Fontes fontes;                  /**< Fontes carregadas.                     */
    Sons sons;                      /**< Sons gerados.                          */
    Tela tela;                      /**< Tela atual.                            */
    float tempo;                    /**< Tempo total desde o início (s).        */
    int mouseSobreBotao;            /**< 1 se o mouse está sobre um botão.      */

    BancoPerguntas *banco;          /**< Banco de perguntas.                    */
    Ranking *ranking;               /**< Ranking dos jogadores.                 */

    char nome[TAM_NOME_JOGADOR];    /**< Nome digitado.                         */
    int categoria;                  /**< Categoria preferida.                   */
    char aviso[128];                /**< Aviso temporário (toast).              */
    float tempoAviso;               /**< Segundos restantes do aviso.           */

    Partida partida;                /**< Partida real.                          */
    Partida exibida;                /**< Cópia exibida durante a revelação.     */
    EtapaRodada etapa;              /**< Etapa da rodada.                       */
    float tempoEtapa;               /**< Tempo desde o início da etapa (s).     */
    float tempoTorcida;             /**< Tempo desde que a torcida votou (s).   */
    int selecionada;                /**< Alternativa escolhida (0-3) ou -1.     */
    ResultadoResposta resultado;    /**< Resultado da última resposta.          */
    const char *mensagem;           /**< Mensagem comemorativa sorteada.        */
    int posicaoRanking;             /**< Posição conquistada no ranking.        */

    int nivelNavegacao;             /**< Nível escolhido na navegação.          */
    int *indicesNavegacao;          /**< Perguntas do nível (alocado).          */
    int quantidadeNavegacao;        /**< Quantidade de perguntas do nível.      */
    int atualNavegacao;             /**< Pergunta exibida.                      */
    int mostrarResposta;            /**< 1 para destacar a correta.             */

    Confete confetes[MAX_CONFETES]; /**< Confetes ativos.                       */
} Gui;

/* ---- gui_recursos.c ---- */

/**
 * @brief Carrega as fontes (com acentos) e gera os sons.
 * @param gui Estado da interface.
 */
void gui_carregarRecursos(Gui *gui);

/**
 * @brief Libera fontes e sons.
 * @param gui Estado da interface.
 */
void gui_liberarRecursos(Gui *gui);

/**
 * @brief Toca um som se o áudio estiver ativo.
 * @param gui Estado da interface.
 * @param som Som a tocar.
 */
void gui_tocar(const Gui *gui, Sound som);

/* ---- gui_desenho.c ---- */

/**
 * @brief Desenha o fundo de gramado com as linhas do campo.
 * @param gui Estado da interface.
 */
void gui_desenharFundo(const Gui *gui);

/**
 * @brief Desenha um painel arredondado translúcido.
 * @param area Retângulo do painel.
 * @param cor Cor de fundo.
 */
void gui_desenharPainel(Rectangle area, Color cor);

/**
 * @brief Escreve um texto com a fonte escolhida.
 * @param fonte Fonte.
 * @param texto Texto UTF-8.
 * @param x Posição x.
 * @param y Posição y.
 * @param tamanho Tamanho em pixels.
 * @param cor Cor.
 */
void gui_texto(Font fonte, const char *texto, float x, float y, float tamanho, Color cor);

/**
 * @brief Escreve um texto centralizado horizontalmente em @p centroX.
 * @param fonte Fonte.
 * @param texto Texto UTF-8.
 * @param centroX Centro horizontal.
 * @param y Posição y.
 * @param tamanho Tamanho em pixels.
 * @param cor Cor.
 */
void gui_textoCentralizado(Font fonte, const char *texto, float centroX, float y, float tamanho, Color cor);

/**
 * @brief Mede a largura de um texto.
 * @param fonte Fonte.
 * @param texto Texto UTF-8.
 * @param tamanho Tamanho em pixels.
 * @return Largura em pixels.
 */
float gui_larguraTexto(Font fonte, const char *texto, float tamanho);

/**
 * @brief Escreve um texto quebrando linhas para caber em uma área.
 *
 * Se não couber, o tamanho da fonte é reduzido até @p tamanhoMinimo.
 *
 * @param fonte Fonte.
 * @param texto Texto UTF-8.
 * @param area Área disponível.
 * @param tamanho Tamanho desejado.
 * @param tamanhoMinimo Menor tamanho aceito.
 * @param centralizar 1 para centralizar as linhas.
 * @param cor Cor.
 */
void gui_textoQuebrado(Font fonte, const char *texto, Rectangle area, float tamanho, float tamanhoMinimo,
                       int centralizar, Color cor);

/**
 * @brief Desenha e trata um botão.
 * @param gui Estado da interface.
 * @param area Retângulo do botão.
 * @param texto Rótulo.
 * @param estilo Estilo visual.
 * @param ativo 0 para desenhar desativado.
 * @return 1 se foi clicado neste quadro.
 */
int gui_botao(Gui *gui, Rectangle area, const char *texto, EstiloBotao estilo, int ativo);

/**
 * @brief Verifica se o mouse está sobre uma área (e marca o cursor de mão).
 * @param gui Estado da interface.
 * @param area Retângulo.
 * @return 1 se o mouse está sobre a área.
 */
int gui_mouseSobre(Gui *gui, Rectangle area);

/**
 * @brief Desenha uma bola de futebol.
 * @param centro Centro da bola.
 * @param raio Raio.
 * @param rotacao Rotação em graus.
 */
void gui_desenharBola(Vector2 centro, float raio, float rotacao);

/**
 * @brief Devolve a cor de destaque de um nível.
 * @param nivel Nível.
 * @return Cor.
 */
Color gui_corNivel(NivelDificuldade nivel);

/**
 * @brief Mostra um aviso temporário na parte de baixo da tela.
 * @param gui Estado da interface.
 * @param mensagem Texto do aviso.
 */
void gui_mostrarAviso(Gui *gui, const char *mensagem);

/**
 * @brief Desenha o aviso temporário, se houver.
 * @param gui Estado da interface.
 */
void gui_desenharAviso(Gui *gui);

/**
 * @brief Solta uma chuva de confetes.
 * @param gui Estado da interface.
 * @param quantidade Quantidade de confetes.
 */
void gui_soltarConfetes(Gui *gui, int quantidade);

/**
 * @brief Atualiza e desenha os confetes.
 * @param gui Estado da interface.
 */
void gui_atualizarConfetes(Gui *gui);

/* ---- gui_telas.c ---- */

/**
 * @brief Tela do menu inicial.
 * @param gui Estado da interface.
 */
void gui_telaMenu(Gui *gui);

/**
 * @brief Tela de nome do jogador e escolha de categoria.
 * @param gui Estado da interface.
 */
void gui_telaNome(Gui *gui);

/**
 * @brief Tela de resultado ao fim da partida.
 * @param gui Estado da interface.
 */
void gui_telaResultado(Gui *gui);

/**
 * @brief Tela do ranking.
 * @param gui Estado da interface.
 */
void gui_telaRanking(Gui *gui);

/**
 * @brief Tela de navegação das perguntas por nível.
 * @param gui Estado da interface.
 */
void gui_telaNavegar(Gui *gui);

/**
 * @brief Prepara a lista de perguntas do nível escolhido na navegação.
 * @param gui Estado da interface.
 * @param nivel Nível (1 a 5).
 */
void gui_carregarNivelNavegacao(Gui *gui, int nivel);

/* ---- gui_jogo.c ---- */

/**
 * @brief Começa uma nova partida e vai para a tela de jogo.
 * @param gui Estado da interface.
 * @return 1 se iniciou, 0 se não há perguntas suficientes.
 */
int gui_iniciarPartida(Gui *gui);

/**
 * @brief Tela da partida.
 * @param gui Estado da interface.
 */
void gui_telaJogo(Gui *gui);

#endif /* GUI_H */
