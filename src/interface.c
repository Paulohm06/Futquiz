/**
 * @file interface.c
 * @brief Implementação das funções de interface com o usuário no terminal.
 * @author Equipe Futquiz
 */

#include "interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

/** Tamanho do buffer usado para ler números e respostas curtas. */
#define TAM_BUFFER_CURTO 64

void interface_inicializar(void)
{
#ifdef _WIN32
    HANDLE saida = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;

    if (saida != INVALID_HANDLE_VALUE && GetConsoleMode(saida, &modo)) {
        SetConsoleMode(saida, modo | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
}

void interface_limparTela(void)
{
    printf("\033[2J\033[H");
    fflush(stdout);
}

void interface_beep(int vezes)
{
    int i;

    for (i = 0; i < vezes; i++) {
        putchar('\a');
    }
    fflush(stdout);
}

void interface_mostrarTitulo(const char *titulo)
{
    interface_limparTela();
    printf(COR_VERDE "  ================================================\n");
    printf("     " COR_BRANCO "F U T Q U I Z" COR_VERDE "  -  O Jogo do Milhão da Bola\n");
    printf("  ================================================" COR_RESET "\n");
    printf(COR_AMARELO "  >> %s" COR_RESET "\n\n", titulo);
}

void interface_mostrarErro(const char *mensagem)
{
    interface_beep(1);
    printf(COR_VERMELHO "  [ERRO] %s" COR_RESET "\n", mensagem);
}

void interface_mostrarSucesso(const char *mensagem)
{
    printf(COR_VERDE "  [OK] %s" COR_RESET "\n", mensagem);
}

void interface_mostrarAviso(const char *mensagem)
{
    printf(COR_AMARELO "  [AVISO] %s" COR_RESET "\n", mensagem);
}

void interface_mostrarInfo(const char *mensagem)
{
    printf(COR_CIANO "  %s" COR_RESET "\n", mensagem);
}

void interface_imprimirOpcao(int numero, const char *texto)
{
    printf("  " COR_AMARELO "[%d]" COR_RESET " %s\n", numero, texto);
}

void interface_imprimirAjustado(const char *texto, int largura)
{
    int colunas = 0;
    const unsigned char *c = (const unsigned char *)texto;

    while (*c != '\0' && colunas < largura) {
        putchar(*c);
        c++;
        /* Copia os bytes de continuação de um caractere UTF-8 (10xxxxxx). */
        while ((*c & 0xC0) == 0x80) {
            putchar(*c);
            c++;
        }
        colunas++;
    }
    while (colunas < largura) {
        putchar(' ');
        colunas++;
    }
}

int interface_lerLinha(char *destino, size_t tamanho)
{
    int caractere;

    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return 0;
    }
    /* Se a linha não coube no buffer, descarta o restante. */
    if (strchr(destino, '\n') == NULL) {
        while ((caractere = getchar()) != '\n' && caractere != EOF) {
            /* descarta */
        }
    }
    destino[strcspn(destino, "\r\n")] = '\0';
    return 1;
}

int interface_entradaEncerrada(void)
{
    return feof(stdin) != 0;
}

/**
 * @brief Remove espaços em branco do início e do fim de um texto.
 * @param texto Texto a ser modificado.
 */
static void apararEspacos(char *texto)
{
    size_t inicio = 0;
    size_t fim = strlen(texto);

    while (texto[inicio] != '\0' && isspace((unsigned char)texto[inicio])) {
        inicio++;
    }
    while (fim > inicio && isspace((unsigned char)texto[fim - 1])) {
        fim--;
    }
    memmove(texto, texto + inicio, fim - inicio);
    texto[fim - inicio] = '\0';
}

void interface_lerTexto(const char *rotulo, char *destino, size_t tamanho, int permitirVazio)
{
    for (;;) {
        printf("%s", rotulo);
        fflush(stdout);
        if (!interface_lerLinha(destino, tamanho)) {
            return;
        }
        apararEspacos(destino);
        if (destino[0] != '\0' || permitirVazio) {
            return;
        }
        interface_mostrarErro("Este campo não pode ficar vazio.");
    }
}

int interface_converterInteiro(const char *texto, int *valor)
{
    char *fim;
    long convertido;

    errno = 0;
    convertido = strtol(texto, &fim, 10);
    if (fim == texto || errno == ERANGE || convertido < INT_MIN || convertido > INT_MAX) {
        return 0;
    }
    while (isspace((unsigned char)*fim)) {
        fim++;
    }
    if (*fim != '\0') {
        return 0;
    }
    *valor = (int)convertido;
    return 1;
}

int interface_lerInteiro(const char *rotulo, int minimo, int maximo)
{
    char buffer[TAM_BUFFER_CURTO];
    char mensagem[TAM_BUFFER_CURTO * 2];
    int valor;

    for (;;) {
        printf("%s", rotulo);
        fflush(stdout);
        if (!interface_lerLinha(buffer, sizeof(buffer))) {
            return minimo;
        }
        if (interface_converterInteiro(buffer, &valor) && valor >= minimo && valor <= maximo) {
            return valor;
        }
        snprintf(mensagem, sizeof(mensagem), "Digite um número inteiro entre %d e %d.", minimo, maximo);
        interface_mostrarErro(mensagem);
    }
}

int interface_lerInteiroOpcional(const char *rotulo, int minimo, int maximo, int valorAtual)
{
    char buffer[TAM_BUFFER_CURTO];
    char mensagem[TAM_BUFFER_CURTO * 2];
    int valor;

    for (;;) {
        interface_lerTexto(rotulo, buffer, sizeof(buffer), 1);
        if (buffer[0] == '\0') {
            return valorAtual;
        }
        if (interface_converterInteiro(buffer, &valor) && valor >= minimo && valor <= maximo) {
            return valor;
        }
        snprintf(mensagem, sizeof(mensagem), "Digite um número entre %d e %d (ou ENTER para manter).",
                 minimo, maximo);
        interface_mostrarErro(mensagem);
    }
}

char interface_lerAlternativa(const char *rotulo, int permitirVazio)
{
    char buffer[TAM_BUFFER_CURTO];
    char letra;

    for (;;) {
        interface_lerTexto(rotulo, buffer, sizeof(buffer), 1);
        if (buffer[0] == '\0') {
            if (permitirVazio || interface_entradaEncerrada()) {
                return '\0';
            }
        } else {
            letra = (char)toupper((unsigned char)buffer[0]);
            if (buffer[1] == '\0' && letra >= 'A' && letra <= 'D') {
                return letra;
            }
        }
        interface_mostrarErro("Digite apenas uma letra: A, B, C ou D.");
    }
}

int interface_confirmar(const char *pergunta)
{
    char buffer[TAM_BUFFER_CURTO];
    char resposta;

    for (;;) {
        printf("%s " COR_AMARELO "(S/N)" COR_RESET ": ", pergunta);
        fflush(stdout);
        if (!interface_lerLinha(buffer, sizeof(buffer))) {
            return 0;
        }
        apararEspacos(buffer);
        resposta = (char)toupper((unsigned char)buffer[0]);
        if (buffer[0] != '\0' && buffer[1] == '\0' && (resposta == 'S' || resposta == 'N')) {
            return resposta == 'S';
        }
        interface_mostrarErro("Responda com S ou N.");
    }
}

void interface_pausar(void)
{
    char buffer[TAM_BUFFER_CURTO];

    printf(COR_CINZA "\n  Pressione ENTER para continuar..." COR_RESET);
    fflush(stdout);
    interface_lerLinha(buffer, sizeof(buffer));
}
