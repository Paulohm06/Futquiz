/**
 * @file partida.h
 * @brief Regras de uma partida do Futquiz (baseadas no Show do Milhão do SBT):
 *        sorteio das 15 perguntas, prêmios, marcos de segurança e ajudas
 *        (VAR, Ajuda da Torcida e Substituição).
 *
 * Este módulo não desenha nada na tela: ele só guarda o estado do jogo.
 * Assim a mesma lógica é usada pela versão de terminal e pela versão gráfica.
 *
 * @author Equipe Futquiz
 */

#ifndef PARTIDA_H
#define PARTIDA_H

#include <stddef.h>
#include "banco.h"

/** Quantidade de rodadas (perguntas) de uma partida. */
#define NUM_RODADAS 15
/** Quantidade de substituições (pulos) por partida, como no SBT. */
#define MAX_SUBSTITUICOES 3
/** Valor usado para "qualquer categoria". */
#define CATEGORIA_TODAS 0

/**
 * @brief Situação da partida.
 */
typedef enum {
    PARTIDA_EM_ANDAMENTO, /**< Jogador ainda está respondendo.  */
    PARTIDA_PAROU,        /**< Jogador decidiu parar.           */
    PARTIDA_ERROU,        /**< Jogador errou uma pergunta.      */
    PARTIDA_CAMPEAO       /**< Jogador acertou a pergunta final. */
} EstadoPartida;

/**
 * @brief Resultado de uma resposta.
 */
typedef enum {
    RESPOSTA_INVALIDA,  /**< Letra inválida ou eliminada pelo VAR. */
    RESPOSTA_CERTA,     /**< Acertou e segue para a próxima.       */
    RESPOSTA_ERRADA,    /**< Errou: fim de jogo.                   */
    RESPOSTA_CAMPEAO    /**< Acertou a pergunta do milhão.         */
} ResultadoResposta;

/**
 * @brief Estado completo de uma partida.
 */
typedef struct {
    const BancoPerguntas *banco;                        /**< Banco de onde vêm as perguntas.          */
    int perguntas[NUM_RODADAS];                         /**< Índice (no banco) da pergunta de cada rodada. */
    int usadas[NUM_RODADAS + MAX_SUBSTITUICOES];        /**< Índices já sorteados (para não repetir). */
    int quantidadeUsadas;                               /**< Quantidade de índices em @ref usadas.   */
    int rodada;                                         /**< Rodada atual (0 a 14).                  */
    int categoriaPreferida;                             /**< Categoria escolhida ou CATEGORIA_TODAS. */
    EstadoPartida estado;                               /**< Situação da partida.                    */
    int premioFinal;                                    /**< Prêmio levado ao terminar.              */
    int varDisponivel;                                  /**< 1 se o VAR ainda não foi usado.         */
    int torcidaDisponivel;                              /**< 1 se a torcida ainda não foi usada.     */
    int substituicoesRestantes;                         /**< Pulos que ainda restam.                 */
    int eliminadas[NUM_ALTERNATIVAS];                   /**< 1 = alternativa anulada pelo VAR.       */
    int torcidaAtiva;                                   /**< 1 se há votos da torcida nesta rodada.  */
    int votosTorcida[NUM_ALTERNATIVAS];                 /**< Porcentagem de cada alternativa.        */
} Partida;

/**
 * @brief Verifica se o banco tem perguntas suficientes para uma partida.
 * @param banco Banco de perguntas.
 * @return 1 se é possível jogar, 0 caso contrário.
 */
int partida_bancoSuficiente(const BancoPerguntas *banco);

/**
 * @brief Começa uma nova partida sorteando as 15 perguntas.
 *
 * Se houver uma categoria preferida, as perguntas dela são sorteadas
 * primeiro; se faltarem, o sorteio completa com outras categorias.
 *
 * @param partida Partida a iniciar.
 * @param banco Banco de perguntas.
 * @param categoriaPreferida Categoria (1 a 4) ou CATEGORIA_TODAS.
 * @return 1 se iniciou, 0 se o banco não tem perguntas suficientes.
 */
int partida_iniciar(Partida *partida, const BancoPerguntas *banco, int categoriaPreferida);

/**
 * @brief Devolve a pergunta da rodada atual.
 * @param partida Partida em andamento.
 * @return Ponteiro para a pergunta.
 */
const Pergunta *partida_perguntaAtual(const Partida *partida);

/**
 * @brief Nível de dificuldade usado em uma rodada (2, 2, 4, 4 e 3 perguntas).
 * @param rodada Rodada de 0 a 14.
 * @return Nível da rodada.
 */
NivelDificuldade partida_nivelDaRodada(int rodada);

/**
 * @brief Indica se a rodada é um marco de segurança (rodadas 5 e 10).
 * @param rodada Rodada de 0 a 14.
 * @return 1 se é marco, 0 caso contrário.
 */
int partida_ehMarco(int rodada);

/**
 * @brief Prêmio ganho ao acertar a pergunta de uma rodada.
 * @param rodada Rodada de 0 a 14.
 * @return Valor em reais.
 */
int partida_premioDaRodada(int rodada);

/**
 * @brief Prêmio que o jogador leva se parar agora.
 * @param partida Partida em andamento.
 * @return Valor em reais.
 */
int partida_premioSeParar(const Partida *partida);

/**
 * @brief Prêmio que o jogador passa a ter se acertar a pergunta atual.
 * @param partida Partida em andamento.
 * @return Valor em reais.
 */
int partida_premioSeAcertar(const Partida *partida);

/**
 * @brief Prêmio que o jogador leva se errar a pergunta atual.
 *
 * Errando, ele cai para o último marco de segurança alcançado.
 * Na pergunta do milhão, errar zera o prêmio (regra do SBT).
 *
 * @param partida Partida em andamento.
 * @return Valor em reais.
 */
int partida_premioSeErrar(const Partida *partida);

/**
 * @brief Responde a pergunta atual.
 * @param partida Partida em andamento.
 * @param letra Letra escolhida (A a D).
 * @return Resultado da resposta.
 */
ResultadoResposta partida_responder(Partida *partida, char letra);

/**
 * @brief Encerra a partida levando o prêmio acumulado.
 * @param partida Partida em andamento.
 */
void partida_parar(Partida *partida);

/**
 * @brief Indica se ajudas podem ser usadas (não valem na pergunta do milhão).
 * @param partida Partida em andamento.
 * @return 1 se pode, 0 caso contrário.
 */
int partida_ajudasLiberadas(const Partida *partida);

/**
 * @brief Usa o VAR: anula duas alternativas erradas.
 * @param partida Partida em andamento.
 * @return 1 se usou, 0 se não estava disponível.
 */
int partida_usarVar(Partida *partida);

/**
 * @brief Usa a Ajuda da Torcida: gera porcentagens de votos.
 *
 * Quanto mais difícil a pergunta, menos a torcida acerta.
 *
 * @param partida Partida em andamento.
 * @return 1 se usou, 0 se não estava disponível.
 */
int partida_usarTorcida(Partida *partida);

/**
 * @brief Usa uma Substituição: troca a pergunta por outra do mesmo nível.
 * @param partida Partida em andamento.
 * @return 1 se trocou, 0 se não havia substituição ou pergunta reserva.
 */
int partida_substituir(Partida *partida);

/**
 * @brief Quantidade de perguntas acertadas até agora.
 * @param partida Partida.
 * @return Número de acertos.
 */
int partida_acertos(const Partida *partida);

/**
 * @brief Formata um valor em reais (ex.: "R$ 1.000.000").
 * @param valor Valor inteiro.
 * @param destino Buffer de destino.
 * @param tamanho Tamanho do buffer.
 */
void partida_formatarPremio(int valor, char *destino, size_t tamanho);

/**
 * @brief Sorteia uma mensagem comemorativa para um acerto.
 * @param rodada Rodada que acabou de ser acertada.
 * @return Texto da mensagem.
 */
const char *partida_mensagemAcerto(int rodada);

/**
 * @brief Sorteia uma mensagem para um erro.
 * @return Texto da mensagem.
 */
const char *partida_mensagemErro(void);

#endif /* PARTIDA_H */
