/**
 * @file ranking.c
 * @brief Implementação do ranking dos jogadores.
 * @author Equipe Futquiz
 */

#include "ranking.h"

#include <stdio.h>
#include <string.h>

/**
 * @brief Compara dois resultados.
 * @param a Primeiro resultado.
 * @param b Segundo resultado.
 * @return 1 se @p a deve ficar acima de @p b, 0 caso contrário.
 */
static int ficaAcima(const RegistroRanking *a, const RegistroRanking *b)
{
    if (a->premio != b->premio) {
        return a->premio > b->premio;
    }
    return a->acertos > b->acertos;
}

void ranking_inicializar(Ranking *ranking)
{
    memset(ranking, 0, sizeof(*ranking));
}

int ranking_carregar(Ranking *ranking, const char *caminho)
{
    FILE *arquivo;
    int i;

    ranking_inicializar(ranking);
    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        return 0;
    }
    if (fread(ranking, sizeof(*ranking), 1, arquivo) != 1 ||
        ranking->quantidade < 0 || ranking->quantidade > RANKING_MAX) {
        ranking_inicializar(ranking);
        fclose(arquivo);
        return 0;
    }
    for (i = 0; i < ranking->quantidade; i++) {
        ranking->registros[i].nome[TAM_NOME_JOGADOR - 1] = '\0';
    }
    fclose(arquivo);
    return 1;
}

int ranking_salvar(const Ranking *ranking, const char *caminho)
{
    FILE *arquivo = fopen(caminho, "wb");
    int ok;

    if (arquivo == NULL) {
        return 0;
    }
    ok = fwrite(ranking, sizeof(*ranking), 1, arquivo) == 1;
    return (fclose(arquivo) == 0) && ok;
}

int ranking_inserir(Ranking *ranking, const char *nome, int premio, int acertos)
{
    RegistroRanking novo;
    int posicao;

    if (premio <= 0) {
        return 0; /* quem sai com R$ 0 não entra no ranking */
    }
    memset(&novo, 0, sizeof(novo));
    strncpy(novo.nome, nome, TAM_NOME_JOGADOR - 1);
    novo.premio = premio;
    novo.acertos = acertos;

    /* Procura a posição do novo resultado. */
    posicao = 0;
    while (posicao < ranking->quantidade && !ficaAcima(&novo, &ranking->registros[posicao])) {
        posicao++;
    }
    if (posicao >= RANKING_MAX) {
        return 0;
    }
    /* Empurra os de baixo uma posição (o último sai se estiver cheio). */
    if (ranking->quantidade < RANKING_MAX) {
        ranking->quantidade++;
    }
    memmove(&ranking->registros[posicao + 1], &ranking->registros[posicao],
            (size_t)(ranking->quantidade - 1 - posicao) * sizeof(RegistroRanking));
    ranking->registros[posicao] = novo;
    return posicao + 1;
}
