/**
 * @file interface.h
 * @brief Funções de interface com o usuário no terminal: cores, beeps,
 *        leitura segura de dados e mensagens padronizadas.
 * @author Equipe Futquiz
 */

#ifndef INTERFACE_H
#define INTERFACE_H

#include <stddef.h>

/* ---- Códigos ANSI de cores e estilos ---- */
#define COR_RESET      "\033[0m"
#define COR_NEGRITO    "\033[1m"
#define COR_VERMELHO   "\033[1;31m"
#define COR_VERDE      "\033[1;32m"
#define COR_AMARELO    "\033[1;33m"
#define COR_AZUL       "\033[1;34m"
#define COR_MAGENTA    "\033[1;35m"
#define COR_CIANO      "\033[1;36m"
#define COR_BRANCO     "\033[1;37m"
#define COR_CINZA      "\033[0;90m"

/**
 * @brief Prepara o terminal (no Windows ativa cores ANSI e UTF-8).
 */
void interface_inicializar(void);

/**
 * @brief Limpa a tela e posiciona o cursor no topo.
 */
void interface_limparTela(void);

/**
 * @brief Emite um ou mais beeps no terminal.
 * @param vezes Quantidade de beeps.
 */
void interface_beep(int vezes);

/**
 * @brief Limpa a tela e mostra o cabeçalho do jogo com um título.
 * @param titulo Título da tela atual.
 */
void interface_mostrarTitulo(const char *titulo);

/**
 * @brief Mostra uma mensagem de erro em vermelho, com beep.
 * @param mensagem Texto da mensagem.
 */
void interface_mostrarErro(const char *mensagem);

/**
 * @brief Mostra uma mensagem de sucesso em verde.
 * @param mensagem Texto da mensagem.
 */
void interface_mostrarSucesso(const char *mensagem);

/**
 * @brief Mostra uma mensagem de aviso em amarelo.
 * @param mensagem Texto da mensagem.
 */
void interface_mostrarAviso(const char *mensagem);

/**
 * @brief Mostra uma mensagem informativa em ciano.
 * @param mensagem Texto da mensagem.
 */
void interface_mostrarInfo(const char *mensagem);

/**
 * @brief Imprime um item de menu no formato "[numero] texto".
 * @param numero Número da opção.
 * @param texto Descrição da opção.
 */
void interface_imprimirOpcao(int numero, const char *texto);

/**
 * @brief Imprime um texto UTF-8 ocupando exatamente @p largura colunas
 *        (corta se for maior e completa com espaços se for menor).
 * @param texto Texto a imprimir.
 * @param largura Número de colunas.
 */
void interface_imprimirAjustado(const char *texto, int largura);

/**
 * @brief Lê uma linha do teclado, descartando o excesso e o '\n'.
 * @param destino Buffer de destino.
 * @param tamanho Tamanho do buffer.
 * @return 1 se leu algo, 0 se a entrada terminou (EOF).
 */
int interface_lerLinha(char *destino, size_t tamanho);

/**
 * @brief Indica se a entrada padrão foi encerrada (EOF).
 * @return 1 se encerrada, 0 caso contrário.
 */
int interface_entradaEncerrada(void);

/**
 * @brief Lê um texto com rótulo, removendo espaços das pontas.
 * @param rotulo Texto exibido antes da leitura.
 * @param destino Buffer de destino.
 * @param tamanho Tamanho do buffer.
 * @param permitirVazio Se 0, repete a leitura até receber algo.
 */
void interface_lerTexto(const char *rotulo, char *destino, size_t tamanho, int permitirVazio);

/**
 * @brief Converte um texto em inteiro, validando o conteúdo.
 * @param texto Texto a converter.
 * @param valor Endereço onde o resultado será guardado.
 * @return 1 se a conversão foi válida, 0 caso contrário.
 */
int interface_converterInteiro(const char *texto, int *valor);

/**
 * @brief Lê um inteiro dentro de um intervalo, repetindo em caso de erro.
 * @param rotulo Texto exibido antes da leitura.
 * @param minimo Menor valor aceito.
 * @param maximo Maior valor aceito.
 * @return Valor lido (ou @p minimo se a entrada terminar).
 */
int interface_lerInteiro(const char *rotulo, int minimo, int maximo);

/**
 * @brief Lê um inteiro opcional: ENTER vazio mantém o valor atual.
 * @param rotulo Texto exibido antes da leitura.
 * @param minimo Menor valor aceito.
 * @param maximo Maior valor aceito.
 * @param valorAtual Valor devolvido se o usuário não digitar nada.
 * @return Valor lido ou @p valorAtual.
 */
int interface_lerInteiroOpcional(const char *rotulo, int minimo, int maximo, int valorAtual);

/**
 * @brief Lê a letra de uma alternativa (A, B, C ou D).
 * @param rotulo Texto exibido antes da leitura.
 * @param permitirVazio Se 1, ENTER vazio devolve '\0'.
 * @return Letra maiúscula lida ou '\0'.
 */
char interface_lerAlternativa(const char *rotulo, int permitirVazio);

/**
 * @brief Faz uma pergunta de sim ou não.
 * @param pergunta Texto da pergunta.
 * @return 1 para sim, 0 para não.
 */
int interface_confirmar(const char *pergunta);

/**
 * @brief Pausa até o usuário pressionar ENTER.
 */
void interface_pausar(void);

#endif /* INTERFACE_H */
