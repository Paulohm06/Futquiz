/**
 * @file banco.c
 * @brief Implementação do banco de perguntas em vetor dinâmico.
 * @author Equipe Futquiz
 */

#include "banco.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/** Capacidade inicial do vetor quando a primeira pergunta é inserida. */
#define CAPACIDADE_INICIAL 16

/**
 * @brief Muda a capacidade do vetor de perguntas.
 * @param banco Banco a redimensionar.
 * @param novaCapacidade Nova quantidade de posições.
 * @return 1 se deu certo, 0 se faltou memória.
 */
static int redimensionar(BancoPerguntas *banco, int novaCapacidade)
{
    Pergunta *novoVetor = realloc(banco->itens, (size_t)novaCapacidade * sizeof(Pergunta));

    if (novoVetor == NULL) {
        return 0;
    }
    banco->itens = novoVetor;
    banco->capacidade = novaCapacidade;
    return 1;
}

void banco_inicializar(BancoPerguntas *banco)
{
    banco->itens = NULL;
    banco->quantidade = 0;
    banco->capacidade = 0;
    banco->proximoId = 1;
}

void banco_liberar(BancoPerguntas *banco)
{
    free(banco->itens);
    banco_inicializar(banco);
}

int banco_inserir(BancoPerguntas *banco, Pergunta pergunta)
{
    int novaCapacidade;

    if (pergunta.id > 0 && banco_indicePorId(banco, pergunta.id) != -1) {
        pergunta.id = 0; /* ID repetido: gera um novo. */
    }
    /* Vetor cheio: dobra a capacidade. */
    if (banco->quantidade == banco->capacidade) {
        novaCapacidade = (banco->capacidade == 0) ? CAPACIDADE_INICIAL : banco->capacidade * 2;
        if (!redimensionar(banco, novaCapacidade)) {
            return -1;
        }
    }
    if (pergunta.id <= 0) {
        pergunta.id = banco->proximoId;
    }
    if (pergunta.id >= banco->proximoId) {
        banco->proximoId = pergunta.id + 1;
    }
    banco->itens[banco->quantidade] = pergunta;
    banco->quantidade++;
    return pergunta.id;
}

int banco_removerPorId(BancoPerguntas *banco, int id)
{
    int indice = banco_indicePorId(banco, id);

    if (indice == -1) {
        return 0;
    }
    /* Desloca as perguntas seguintes uma posição para trás. */
    memmove(&banco->itens[indice], &banco->itens[indice + 1],
            (size_t)(banco->quantidade - indice - 1) * sizeof(Pergunta));
    banco->quantidade--;

    /* Se sobrou muito espaço (menos de 1/4 usado), reduz pela metade. */
    if (banco->capacidade > CAPACIDADE_INICIAL && banco->quantidade < banco->capacidade / 4) {
        redimensionar(banco, banco->capacidade / 2); /* falha aqui não é grave */
    }
    return 1;
}

int banco_indicePorId(const BancoPerguntas *banco, int id)
{
    int i;

    for (i = 0; i < banco->quantidade; i++) {
        if (banco->itens[i].id == id) {
            return i;
        }
    }
    return -1;
}

Pergunta *banco_buscarPorId(BancoPerguntas *banco, int id)
{
    int indice = banco_indicePorId(banco, id);

    return (indice == -1) ? NULL : &banco->itens[indice];
}

int banco_contarPorNivel(const BancoPerguntas *banco, NivelDificuldade nivel)
{
    int i;
    int total = 0;

    for (i = 0; i < banco->quantidade; i++) {
        if (banco->itens[i].nivel == nivel) {
            total++;
        }
    }
    return total;
}

int banco_contarPorCategoria(const BancoPerguntas *banco, Categoria categoria)
{
    int i;
    int total = 0;

    for (i = 0; i < banco->quantidade; i++) {
        if (banco->itens[i].categoria == categoria) {
            total++;
        }
    }
    return total;
}

/**
 * @brief Aloca um vetor de índices do tamanho do banco.
 * @param banco Banco de referência.
 * @return Vetor alocado ou NULL se o banco estiver vazio ou faltar memória.
 */
static int *alocarIndices(const BancoPerguntas *banco)
{
    if (banco->quantidade == 0) {
        return NULL;
    }
    return malloc((size_t)banco->quantidade * sizeof(int));
}

/**
 * @brief Ajusta o vetor de resultados: libera se estiver vazio.
 * @param indices Vetor de índices.
 * @param quantidade Quantidade encontrada.
 * @return O próprio vetor ou NULL se a quantidade for zero.
 */
static int *finalizarIndices(int *indices, int quantidade)
{
    if (quantidade == 0) {
        free(indices);
        return NULL;
    }
    return indices;
}

int *banco_indicesPorNivel(const BancoPerguntas *banco, NivelDificuldade nivel, int *quantidade)
{
    int i;
    int *indices = alocarIndices(banco);

    *quantidade = 0;
    if (indices == NULL) {
        return NULL;
    }
    for (i = 0; i < banco->quantidade; i++) {
        if (banco->itens[i].nivel == nivel) {
            indices[(*quantidade)++] = i;
        }
    }
    return finalizarIndices(indices, *quantidade);
}

int *banco_indicesPorCategoria(const BancoPerguntas *banco, Categoria categoria, int *quantidade)
{
    int i;
    int *indices = alocarIndices(banco);

    *quantidade = 0;
    if (indices == NULL) {
        return NULL;
    }
    for (i = 0; i < banco->quantidade; i++) {
        if (banco->itens[i].categoria == categoria) {
            indices[(*quantidade)++] = i;
        }
    }
    return finalizarIndices(indices, *quantidade);
}

/**
 * @brief Verifica se @p texto contém @p trecho, ignorando maiúsculas/minúsculas.
 * @param texto Texto onde procurar.
 * @param trecho Texto procurado.
 * @return 1 se contém, 0 caso contrário.
 */
static int contemTextoSemCaixa(const char *texto, const char *trecho)
{
    size_t i;
    size_t j;
    size_t tamanhoTexto = strlen(texto);
    size_t tamanhoTrecho = strlen(trecho);

    if (tamanhoTrecho == 0) {
        return 1;
    }
    for (i = 0; i + tamanhoTrecho <= tamanhoTexto; i++) {
        for (j = 0; j < tamanhoTrecho; j++) {
            if (tolower((unsigned char)texto[i + j]) != tolower((unsigned char)trecho[j])) {
                break;
            }
        }
        if (j == tamanhoTrecho) {
            return 1;
        }
    }
    return 0;
}

int *banco_pesquisarTexto(const BancoPerguntas *banco, const char *trecho, int *quantidade)
{
    int i;
    int k;
    int encontrou;
    int *indices = alocarIndices(banco);

    *quantidade = 0;
    if (indices == NULL) {
        return NULL;
    }
    for (i = 0; i < banco->quantidade; i++) {
        encontrou = contemTextoSemCaixa(banco->itens[i].enunciado, trecho);
        for (k = 0; k < NUM_ALTERNATIVAS && !encontrou; k++) {
            encontrou = contemTextoSemCaixa(banco->itens[i].alternativas[k], trecho);
        }
        if (encontrou) {
            indices[(*quantidade)++] = i;
        }
    }
    return finalizarIndices(indices, *quantidade);
}
