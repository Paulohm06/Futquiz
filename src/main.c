/**
 * @file main.c
 * @brief Ponto de entrada do Futquiz: carrega os dados, executa o menu
 *        principal e salva tudo no arquivo binário ao sair.
 * @author Equipe Futquiz
 */

#include "interface.h"
#include "banco.h"
#include "arquivo.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Carrega as perguntas: do binário se existir, senão do CSV inicial.
 * @param banco Banco de destino (vazio).
 */
static void carregarDados(BancoPerguntas *banco)
{
    char mensagem[160];
    int quantidade;

    interface_mostrarTitulo("Carregando o banco de perguntas");

    if (arquivo_existe(CAMINHO_BINARIO)) {
        quantidade = arquivo_carregarBinario(banco, CAMINHO_BINARIO);
        if (quantidade >= 0) {
            snprintf(mensagem, sizeof(mensagem), "%d pergunta(s) carregada(s) de %s.", quantidade, CAMINHO_BINARIO);
            interface_mostrarSucesso(mensagem);
            interface_pausar();
            return;
        }
        interface_mostrarErro("Arquivo binário inválido. Tentando o CSV inicial...");
        banco_liberar(banco);
    }

    /* Primeira execução (ou binário corrompido): lê o CSV. */
    quantidade = arquivo_carregarCSV(banco, CAMINHO_CSV_INICIAL);
    if (quantidade < 0) {
        snprintf(mensagem, sizeof(mensagem), "Não foi possível abrir %s. O banco começará vazio.",
                 CAMINHO_CSV_INICIAL);
        interface_mostrarErro(mensagem);
    } else {
        snprintf(mensagem, sizeof(mensagem), "Primeira execução: %d pergunta(s) carregada(s) de %s.",
                 quantidade, CAMINHO_CSV_INICIAL);
        interface_mostrarSucesso(mensagem);
    }
    interface_pausar();
}

/**
 * @brief Função principal.
 * @return EXIT_SUCCESS se os dados foram salvos, EXIT_FAILURE caso contrário.
 */
int main(void)
{
    BancoPerguntas banco;
    int salvou;

    interface_inicializar();
    banco_inicializar(&banco);
    carregarDados(&banco);

    menu_executar(&banco);

    salvou = arquivo_salvarBinario(&banco, CAMINHO_BINARIO);
    if (salvou) {
        interface_mostrarSucesso("Dados salvos em " CAMINHO_BINARIO ". Até a próxima partida!");
    } else {
        interface_mostrarErro("Falha ao salvar " CAMINHO_BINARIO ". As alterações foram perdidas.");
    }
    banco_liberar(&banco);
    return salvou ? EXIT_SUCCESS : EXIT_FAILURE;
}
