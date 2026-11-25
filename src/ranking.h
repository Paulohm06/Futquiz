/**
 * @file ranking.h
 * @brief Ranking dos 10 jogadores que mais avançaram, salvo em arquivo binário.
 * @author Equipe Futquiz
 */

#ifndef RANKING_H
#define RANKING_H

/** Quantidade de posições do ranking. */
#define RANKING_MAX 10
/** Tamanho máximo do nome do jogador (incluindo o '\0'). */
#define TAM_NOME_JOGADOR 24
/** Arquivo binário do ranking. */
#define CAMINHO_RANKING "dados/ranking.bin"

/**
 * @brief Um registro do ranking.
 */
typedef struct {
    char nome[TAM_NOME_JOGADOR];  /**< Nome do jogador.            */
    int premio;                   /**< Prêmio levado, em reais.    */
    int acertos;                  /**< Perguntas acertadas (0-15). */
} RegistroRanking;

/**
 * @brief Ranking ordenado do maior para o menor prêmio.
 */
typedef struct {
    RegistroRanking registros[RANKING_MAX]; /**< Registros em ordem.        */
    int quantidade;                         /**< Quantidade de registros.   */
} Ranking;

/**
 * @brief Deixa o ranking vazio.
 * @param ranking Ranking a inicializar.
 */
void ranking_inicializar(Ranking *ranking);

/**
 * @brief Carrega o ranking do arquivo (se não existir, fica vazio).
 * @param ranking Ranking de destino.
 * @param caminho Caminho do arquivo binário.
 * @return 1 se carregou do arquivo, 0 se começou vazio.
 */
int ranking_carregar(Ranking *ranking, const char *caminho);

/**
 * @brief Salva o ranking no arquivo binário.
 * @param ranking Ranking a salvar.
 * @param caminho Caminho do arquivo binário.
 * @return 1 se salvou, 0 em caso de erro.
 */
int ranking_salvar(const Ranking *ranking, const char *caminho);

/**
 * @brief Tenta inserir um resultado no ranking, mantendo a ordem
 *        (maior prêmio primeiro; em empate, mais acertos).
 * @param ranking Ranking.
 * @param nome Nome do jogador.
 * @param premio Prêmio levado.
 * @param acertos Perguntas acertadas.
 * Resultados com prêmio zero não entram no ranking.
 *
 * @return Posição conquistada (1 a 10) ou 0 se não entrou.
 */
int ranking_inserir(Ranking *ranking, const char *nome, int premio, int acertos);

#endif /* RANKING_H */
