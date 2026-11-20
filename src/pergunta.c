/**
 * @file pergunta.c
 * @brief Implementação das funções da estrutura Pergunta.
 * @author Equipe Futquiz
 */

#include "pergunta.h"
#include "interface.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>

/**
 * @brief Compara dois textos sem diferenciar maiúsculas de minúsculas.
 * @param a Primeiro texto.
 * @param b Segundo texto.
 * @return 1 se forem iguais, 0 caso contrário.
 */
static int textosIguaisSemCaixa(const char *a, const char *b)
{
    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }
    return *a == *b;
}

const char *pergunta_nomeNivel(NivelDificuldade nivel)
{
    switch (nivel) {
    case NIVEL_MUITO_FACIL:   return "Muito fácil";
    case NIVEL_FACIL:         return "Fácil";
    case NIVEL_MEDIO:         return "Médio";
    case NIVEL_DIFICIL:       return "Difícil";
    case NIVEL_MUITO_DIFICIL: return "Muito difícil";
    }
    return "Desconhecido";
}

const char *pergunta_corNivel(NivelDificuldade nivel)
{
    switch (nivel) {
    case NIVEL_MUITO_FACIL:   return COR_VERDE;
    case NIVEL_FACIL:         return COR_CIANO;
    case NIVEL_MEDIO:         return COR_AMARELO;
    case NIVEL_DIFICIL:       return COR_VERMELHO;
    case NIVEL_MUITO_DIFICIL: return COR_MAGENTA;
    }
    return COR_RESET;
}

const char *pergunta_nomeCategoria(Categoria categoria)
{
    switch (categoria) {
    case CATEGORIA_BRASILEIRO:    return "Futebol Brasileiro";
    case CATEGORIA_INTERNACIONAL: return "Futebol Internacional";
    case CATEGORIA_COPA:          return "Copa do Mundo";
    case CATEGORIA_LIBERTADORES:  return "Libertadores";
    }
    return "Desconhecida";
}

const char *pergunta_codigoCategoria(Categoria categoria)
{
    switch (categoria) {
    case CATEGORIA_BRASILEIRO:    return "BRASILEIRO";
    case CATEGORIA_INTERNACIONAL: return "INTERNACIONAL";
    case CATEGORIA_COPA:          return "COPA";
    case CATEGORIA_LIBERTADORES:  return "LIBERTADORES";
    }
    return "?";
}

int pergunta_categoriaPorCodigo(const char *codigo, Categoria *categoria)
{
    int numero;

    /* Aceita o número da categoria (1 a 4)... */
    if (interface_converterInteiro(codigo, &numero)) {
        if (!pergunta_categoriaValida(numero)) {
            return 0;
        }
        *categoria = (Categoria)numero;
        return 1;
    }
    /* ...ou o código em texto (BRASILEIRO, INTERNACIONAL, COPA, LIBERTADORES). */
    for (numero = 1; numero <= NUM_CATEGORIAS; numero++) {
        if (textosIguaisSemCaixa(codigo, pergunta_codigoCategoria((Categoria)numero))) {
            *categoria = (Categoria)numero;
            return 1;
        }
    }
    return 0;
}

int pergunta_nivelValido(int valor)
{
    return valor >= NIVEL_MUITO_FACIL && valor <= NIVEL_MUITO_DIFICIL;
}

int pergunta_categoriaValida(int valor)
{
    return valor >= CATEGORIA_BRASILEIRO && valor <= CATEGORIA_LIBERTADORES;
}

int pergunta_minimoCadastro(NivelDificuldade nivel)
{
    switch (nivel) {
    case NIVEL_MUITO_FACIL:   return 20;
    case NIVEL_FACIL:         return 20;
    case NIVEL_MEDIO:         return 15;
    case NIVEL_DIFICIL:       return 15;
    case NIVEL_MUITO_DIFICIL: return 10;
    }
    return 0;
}

int pergunta_quantidadeNaPartida(NivelDificuldade nivel)
{
    switch (nivel) {
    case NIVEL_MUITO_FACIL:   return 2;
    case NIVEL_FACIL:         return 2;
    case NIVEL_MEDIO:         return 4;
    case NIVEL_DIFICIL:       return 4;
    case NIVEL_MUITO_DIFICIL: return 3;
    }
    return 0;
}

int pergunta_validar(const Pergunta *pergunta)
{
    int i;

    if (pergunta->enunciado[0] == '\0') {
        return 0;
    }
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        if (pergunta->alternativas[i][0] == '\0') {
            return 0;
        }
    }
    if (pergunta->correta < 'A' || pergunta->correta > 'D') {
        return 0;
    }
    return pergunta_nivelValido((int)pergunta->nivel) && pergunta_categoriaValida((int)pergunta->categoria);
}

void pergunta_exibir(const Pergunta *pergunta, int mostrarResposta)
{
    int i;
    char letra;

    printf("  " COR_CINZA "ID %d" COR_RESET "  |  %s%s" COR_RESET "  |  " COR_AZUL "%s" COR_RESET "\n\n",
           pergunta->id, pergunta_corNivel(pergunta->nivel), pergunta_nomeNivel(pergunta->nivel),
           pergunta_nomeCategoria(pergunta->categoria));
    printf("  " COR_BRANCO "%s" COR_RESET "\n\n", pergunta->enunciado);

    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        letra = (char)('A' + i);
        if (mostrarResposta && letra == pergunta->correta) {
            printf("    " COR_VERDE "(%c) %s  <- correta" COR_RESET "\n", letra, pergunta->alternativas[i]);
        } else {
            printf("    " COR_AMARELO "(%c)" COR_RESET " %s\n", letra, pergunta->alternativas[i]);
        }
    }
    printf("\n");
}

void pergunta_exibirCabecalhoResumo(void)
{
    printf(COR_NEGRITO "  %4s  %-13s  %-21s  %-3s  %s" COR_RESET "\n", "ID", "Nível", "Categoria", "Resp", "Enunciado");
    printf(COR_CINZA "  ---------------------------------------------------------------------------------" COR_RESET "\n");
}

void pergunta_exibirResumo(const Pergunta *pergunta)
{
    printf("  %4d  %s", pergunta->id, pergunta_corNivel(pergunta->nivel));
    interface_imprimirAjustado(pergunta_nomeNivel(pergunta->nivel), 13);
    printf(COR_RESET "  ");
    interface_imprimirAjustado(pergunta_nomeCategoria(pergunta->categoria), 21);
    printf("   %c   ", pergunta->correta);
    interface_imprimirAjustado(pergunta->enunciado, 45);
    printf("\n");
}
