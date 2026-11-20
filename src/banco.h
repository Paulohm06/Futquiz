/**
 * @file banco.h
 * @brief Banco de perguntas: vetor dinâmico que cresce e diminui conforme
 *        as perguntas são inseridas e excluídas (CRUD).
 * @author Equipe Futquiz
 */

#ifndef BANCO_H
#define BANCO_H

#include "pergunta.h"

/**
 * @brief Coleção dinâmica de perguntas.
 */
typedef struct {
    Pergunta *itens;   /**< Vetor alocado dinamicamente.            */
    int quantidade;    /**< Quantidade de perguntas em uso.          */
    int capacidade;    /**< Quantidade de posições alocadas.         */
    int proximoId;     /**< ID que será dado à próxima pergunta nova. */
} BancoPerguntas;

/**
 * @brief Inicializa um banco vazio.
 * @param banco Banco a inicializar.
 */
void banco_inicializar(BancoPerguntas *banco);

/**
 * @brief Libera a memória do banco e o deixa vazio.
 * @param banco Banco a liberar.
 */
void banco_liberar(BancoPerguntas *banco);

/**
 * @brief Insere uma pergunta no banco, aumentando o vetor se preciso.
 *        Se pergunta.id <= 0, um novo ID é gerado automaticamente.
 * @param banco Banco de destino.
 * @param pergunta Pergunta a inserir (copiada para o banco).
 * @return ID da pergunta inserida ou -1 em caso de erro.
 */
int banco_inserir(BancoPerguntas *banco, Pergunta pergunta);

/**
 * @brief Remove uma pergunta pelo ID, diminuindo o vetor se sobrar espaço.
 * @param banco Banco de origem.
 * @param id ID da pergunta.
 * @return 1 se removeu, 0 se o ID não existe.
 */
int banco_removerPorId(BancoPerguntas *banco, int id);

/**
 * @brief Procura a posição de uma pergunta no vetor pelo ID.
 * @param banco Banco onde procurar.
 * @param id ID procurado.
 * @return Índice no vetor ou -1 se não encontrar.
 */
int banco_indicePorId(const BancoPerguntas *banco, int id);

/**
 * @brief Procura uma pergunta pelo ID.
 * @param banco Banco onde procurar.
 * @param id ID procurado.
 * @return Ponteiro para a pergunta ou NULL se não encontrar.
 */
Pergunta *banco_buscarPorId(BancoPerguntas *banco, int id);

/**
 * @brief Conta quantas perguntas existem em um nível.
 * @param banco Banco a consultar.
 * @param nivel Nível desejado.
 * @return Quantidade de perguntas.
 */
int banco_contarPorNivel(const BancoPerguntas *banco, NivelDificuldade nivel);

/**
 * @brief Conta quantas perguntas existem em uma categoria.
 * @param banco Banco a consultar.
 * @param categoria Categoria desejada.
 * @return Quantidade de perguntas.
 */
int banco_contarPorCategoria(const BancoPerguntas *banco, Categoria categoria);

/**
 * @brief Monta um vetor com os índices das perguntas de um nível.
 * @param banco Banco a consultar.
 * @param nivel Nível desejado.
 * @param quantidade Endereço onde será guardada a quantidade encontrada.
 * @return Vetor alocado (liberar com free) ou NULL se nada for encontrado.
 */
int *banco_indicesPorNivel(const BancoPerguntas *banco, NivelDificuldade nivel, int *quantidade);

/**
 * @brief Monta um vetor com os índices das perguntas de uma categoria.
 * @param banco Banco a consultar.
 * @param categoria Categoria desejada.
 * @param quantidade Endereço onde será guardada a quantidade encontrada.
 * @return Vetor alocado (liberar com free) ou NULL se nada for encontrado.
 */
int *banco_indicesPorCategoria(const BancoPerguntas *banco, Categoria categoria, int *quantidade);

/**
 * @brief Pesquisa perguntas cujo enunciado ou alternativas contenham um
 *        trecho de texto (sem diferenciar maiúsculas de minúsculas).
 * @param banco Banco a consultar.
 * @param trecho Texto procurado.
 * @param quantidade Endereço onde será guardada a quantidade encontrada.
 * @return Vetor alocado (liberar com free) ou NULL se nada for encontrado.
 */
int *banco_pesquisarTexto(const BancoPerguntas *banco, const char *trecho, int *quantidade);

#endif /* BANCO_H */
