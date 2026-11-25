/**
 * @file menu.c
 * @brief Implementação dos menus e telas do sistema.
 * @author Equipe Futquiz
 */

#include "menu.h"
#include "interface.h"
#include "arquivo.h"
#include "jogo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

/** Quantidade de linhas exibidas por página nas listagens. */
#define LINHAS_POR_PAGINA 15
/** Tamanho dos buffers de mensagens. */
#define TAM_MENSAGEM 160

/* ---- Opções do menu principal ---- */
enum {
    OPCAO_SAIR = 0,
    OPCAO_GERENCIAR,
    OPCAO_NAVEGAR,
    OPCAO_JOGAR,
    OPCAO_RANKING,
    OPCAO_EXPORTAR,
    OPCAO_ESTATISTICAS
};

/* ---- Opções do menu de gerenciamento ---- */
enum {
    GERENCIAR_VOLTAR = 0,
    GERENCIAR_INSERIR,
    GERENCIAR_LISTAR,
    GERENCIAR_PESQUISAR,
    GERENCIAR_ALTERAR,
    GERENCIAR_EXCLUIR
};

/* ---- Opções do menu de pesquisa ---- */
enum {
    PESQUISAR_VOLTAR = 0,
    PESQUISAR_TEXTO,
    PESQUISAR_ID,
    PESQUISAR_CATEGORIA
};

/* ------------------------------------------------------------------ */
/* Funções auxiliares                                                  */
/* ------------------------------------------------------------------ */

/**
 * @brief Mostra a lista de níveis e lê a escolha do usuário.
 * @param permitirCancelar Se 1, aceita 0 para cancelar.
 * @return Nível escolhido (1 a 5) ou 0 se cancelado.
 */
static int escolherNivel(int permitirCancelar)
{
    int nivel;

    printf("\n");
    for (nivel = 1; nivel <= NUM_NIVEIS; nivel++) {
        printf("  " COR_AMARELO "[%d]" COR_RESET " %s%s" COR_RESET "\n", nivel,
               pergunta_corNivel((NivelDificuldade)nivel), pergunta_nomeNivel((NivelDificuldade)nivel));
    }
    if (permitirCancelar) {
        interface_imprimirOpcao(0, "Cancelar");
    }
    return interface_lerInteiro("\n  Nível: ", permitirCancelar ? 0 : 1, NUM_NIVEIS);
}

/**
 * @brief Mostra a lista de categorias.
 */
static void mostrarCategorias(void)
{
    int categoria;

    printf("\n");
    for (categoria = 1; categoria <= NUM_CATEGORIAS; categoria++) {
        interface_imprimirOpcao(categoria, pergunta_nomeCategoria((Categoria)categoria));
    }
}

/**
 * @brief Mostra a lista de categorias e lê a escolha do usuário.
 * @param permitirCancelar Se 1, aceita 0 para cancelar.
 * @return Categoria escolhida (1 a 4) ou 0 se cancelado.
 */
static int escolherCategoria(int permitirCancelar)
{
    mostrarCategorias();
    if (permitirCancelar) {
        interface_imprimirOpcao(0, "Cancelar");
    }
    return interface_lerInteiro("\n  Categoria: ", permitirCancelar ? 0 : 1, NUM_CATEGORIAS);
}

/**
 * @brief Lista em tabela as perguntas indicadas, com paginação.
 * @param banco Banco de perguntas.
 * @param indices Índices das perguntas a listar (NULL = todas).
 * @param quantidade Quantidade de índices.
 */
static void listarIndices(const BancoPerguntas *banco, const int *indices, int quantidade)
{
    int i;
    int indice;

    pergunta_exibirCabecalhoResumo();
    for (i = 0; i < quantidade; i++) {
        indice = (indices == NULL) ? i : indices[i];
        pergunta_exibirResumo(&banco->itens[indice]);
        if ((i + 1) % LINHAS_POR_PAGINA == 0 && i + 1 < quantidade) {
            printf(COR_CINZA "  -- %d de %d --" COR_RESET, i + 1, quantidade);
            interface_pausar();
            if (interface_entradaEncerrada()) {
                return;
            }
        }
    }
    printf(COR_CINZA "\n  Total: %d pergunta(s)." COR_RESET "\n", quantidade);
}

/**
 * @brief Navega pelas perguntas indicadas, uma por vez.
 *
 * Teclas: [P] próxima, [A] anterior, [R] mostra/esconde a resposta,
 * [V] volta ao menu.
 *
 * @param banco Banco de perguntas.
 * @param indices Índices das perguntas a navegar.
 * @param quantidade Quantidade de índices.
 * @param titulo Título exibido no topo da tela.
 */
static void navegarPerguntas(const BancoPerguntas *banco, const int *indices, int quantidade, const char *titulo)
{
    int atual = 0;
    int mostrarResposta = 0;
    char opcao[16];
    char aviso[TAM_MENSAGEM] = "";
    const Pergunta *pergunta;

    interface_beep(1);
    for (;;) {
        pergunta = &banco->itens[indices[atual]];
        interface_mostrarTitulo(titulo);
        printf("  %sPergunta %d de %d" COR_RESET "\n\n", pergunta_corNivel(pergunta->nivel), atual + 1, quantidade);
        pergunta_exibir(pergunta, mostrarResposta);

        if (aviso[0] != '\0') {
            interface_mostrarAviso(aviso);
            aviso[0] = '\0';
        }
        printf("  " COR_AMARELO "[P]" COR_RESET " Próxima   " COR_AMARELO "[A]" COR_RESET " Anterior   "
               COR_AMARELO "[R]" COR_RESET " %s resposta   " COR_AMARELO "[V]" COR_RESET " Voltar\n",
               mostrarResposta ? "Esconder" : "Mostrar");
        interface_lerTexto("\n  Opção: ", opcao, sizeof(opcao), 1);
        if (interface_entradaEncerrada()) {
            return;
        }

        switch (toupper((unsigned char)opcao[0])) {
        case 'P':
            if (atual < quantidade - 1) {
                atual++;
                mostrarResposta = 0;
            } else {
                interface_beep(1);
                snprintf(aviso, sizeof(aviso), "Esta já é a última pergunta.");
            }
            break;
        case 'A':
            if (atual > 0) {
                atual--;
                mostrarResposta = 0;
            } else {
                interface_beep(1);
                snprintf(aviso, sizeof(aviso), "Esta já é a primeira pergunta.");
            }
            break;
        case 'R':
            mostrarResposta = !mostrarResposta;
            break;
        case 'V':
            return;
        default:
            interface_beep(1);
            snprintf(aviso, sizeof(aviso), "Opção inválida. Use P, A, R ou V.");
            break;
        }
    }
}

/**
 * @brief Lê os dados de uma pergunta nova do teclado.
 * @param pergunta Pergunta que receberá os dados.
 */
static void lerDadosPergunta(Pergunta *pergunta)
{
    int i;
    char rotulo[48];

    memset(pergunta, 0, sizeof(*pergunta));
    interface_lerTexto("  Enunciado: ", pergunta->enunciado, TAM_ENUNCIADO, 0);
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        snprintf(rotulo, sizeof(rotulo), "  Alternativa %c: ", 'A' + i);
        interface_lerTexto(rotulo, pergunta->alternativas[i], TAM_ALTERNATIVA, 0);
    }
    pergunta->correta = interface_lerAlternativa("  Alternativa correta (A-D): ", 0);
    printf("\n  " COR_NEGRITO "Nível de dificuldade:" COR_RESET);
    pergunta->nivel = (NivelDificuldade)escolherNivel(0);
    printf("\n  " COR_NEGRITO "Categoria:" COR_RESET);
    pergunta->categoria = (Categoria)escolherCategoria(0);
}

/**
 * @brief Lê os novos dados de uma pergunta existente (ENTER mantém o valor).
 * @param pergunta Pergunta a editar.
 */
static void lerAlteracoesPergunta(Pergunta *pergunta)
{
    int i;
    char rotulo[64];
    char buffer[TAM_ENUNCIADO];
    char letra;

    interface_mostrarInfo("Deixe em branco e tecle ENTER para manter o valor atual.\n");

    interface_lerTexto("  Novo enunciado: ", buffer, TAM_ENUNCIADO, 1);
    if (buffer[0] != '\0') {
        strcpy(pergunta->enunciado, buffer);
    }
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        snprintf(rotulo, sizeof(rotulo), "  Nova alternativa %c: ", 'A' + i);
        interface_lerTexto(rotulo, buffer, TAM_ALTERNATIVA, 1);
        if (buffer[0] != '\0') {
            strcpy(pergunta->alternativas[i], buffer);
        }
    }
    snprintf(rotulo, sizeof(rotulo), "  Nova alternativa correta [atual %c]: ", pergunta->correta);
    letra = interface_lerAlternativa(rotulo, 1);
    if (letra != '\0') {
        pergunta->correta = letra;
    }

    printf("\n  " COR_NEGRITO "Nível:" COR_RESET);
    for (i = 1; i <= NUM_NIVEIS; i++) {
        printf("\n  " COR_AMARELO "[%d]" COR_RESET " %s", i, pergunta_nomeNivel((NivelDificuldade)i));
    }
    snprintf(rotulo, sizeof(rotulo), "\n  Novo nível [atual %d]: ", (int)pergunta->nivel);
    pergunta->nivel = (NivelDificuldade)interface_lerInteiroOpcional(rotulo, 1, NUM_NIVEIS, (int)pergunta->nivel);

    printf("\n  " COR_NEGRITO "Categoria:" COR_RESET);
    mostrarCategorias();
    snprintf(rotulo, sizeof(rotulo), "\n  Nova categoria [atual %d]: ", (int)pergunta->categoria);
    pergunta->categoria = (Categoria)interface_lerInteiroOpcional(rotulo, 1, NUM_CATEGORIAS,
                                                                    (int)pergunta->categoria);
}

/**
 * @brief Pede um ID ao usuário e devolve a pergunta correspondente.
 * @param banco Banco de perguntas.
 * @return Ponteiro para a pergunta ou NULL (cancelado ou inexistente).
 */
static Pergunta *pedirPerguntaPorId(BancoPerguntas *banco)
{
    int id;
    char mensagem[TAM_MENSAGEM];
    Pergunta *pergunta;

    id = interface_lerInteiro("  ID da pergunta (0 para cancelar): ", 0, INT_MAX);
    if (id == 0) {
        return NULL;
    }
    pergunta = banco_buscarPorId(banco, id);
    if (pergunta == NULL) {
        snprintf(mensagem, sizeof(mensagem), "Não existe pergunta com ID %d.", id);
        interface_mostrarErro(mensagem);
    }
    return pergunta;
}

/* ------------------------------------------------------------------ */
/* Telas do cadastro (CRUD)                                            */
/* ------------------------------------------------------------------ */

/**
 * @brief Tela de inserção de uma nova pergunta.
 * @param banco Banco de perguntas.
 */
static void inserirPergunta(BancoPerguntas *banco)
{
    Pergunta nova;
    char mensagem[TAM_MENSAGEM];
    int id;

    interface_mostrarTitulo("Inserir nova pergunta");
    lerDadosPergunta(&nova);
    if (!pergunta_validar(&nova)) {
        interface_mostrarErro("Dados inválidos. A pergunta não foi cadastrada.");
        interface_pausar();
        return;
    }

    interface_mostrarTitulo("Confira a nova pergunta");
    pergunta_exibir(&nova, 1);
    if (!interface_confirmar("  Confirmar cadastro?")) {
        interface_mostrarAviso("Cadastro cancelado.");
        interface_pausar();
        return;
    }

    id = banco_inserir(banco, nova);
    if (id == -1) {
        interface_mostrarErro("Memória insuficiente para cadastrar a pergunta.");
    } else {
        interface_beep(2);
        snprintf(mensagem, sizeof(mensagem), "GOOOL! Pergunta cadastrada com o ID %d.", id);
        interface_mostrarSucesso(mensagem);
    }
    interface_pausar();
}

/**
 * @brief Tela que lista todas as perguntas.
 * @param banco Banco de perguntas.
 */
static void listarPerguntas(const BancoPerguntas *banco)
{
    interface_mostrarTitulo("Todas as perguntas");
    if (banco->quantidade == 0) {
        interface_mostrarAviso("Nenhuma pergunta cadastrada.");
    } else {
        listarIndices(banco, NULL, banco->quantidade);
    }
    interface_pausar();
}

/**
 * @brief Mostra o resultado de uma pesquisa e oferece navegar por ele.
 * @param banco Banco de perguntas.
 * @param indices Resultado da pesquisa (será liberado aqui).
 * @param quantidade Quantidade de resultados.
 * @param titulo Título usado na navegação.
 */
static void mostrarResultadoPesquisa(const BancoPerguntas *banco, int *indices, int quantidade, const char *titulo)
{
    if (quantidade == 0) {
        interface_mostrarAviso("Nenhuma pergunta encontrada.");
        interface_pausar();
        return;
    }
    printf("\n");
    listarIndices(banco, indices, quantidade);
    if (interface_confirmar("\n  Deseja navegar pelos resultados?")) {
        navegarPerguntas(banco, indices, quantidade, titulo);
    }
    free(indices);
}

/**
 * @brief Submenu de pesquisa (por texto, ID ou categoria).
 * @param banco Banco de perguntas.
 */
static void menuPesquisar(BancoPerguntas *banco)
{
    int opcao;
    int quantidade;
    int *indices;
    char trecho[TAM_ENUNCIADO];
    Pergunta *pergunta;

    do {
        interface_mostrarTitulo("Pesquisar perguntas");
        interface_imprimirOpcao(PESQUISAR_TEXTO, "Por trecho do texto");
        interface_imprimirOpcao(PESQUISAR_ID, "Por ID");
        interface_imprimirOpcao(PESQUISAR_CATEGORIA, "Por categoria");
        interface_imprimirOpcao(PESQUISAR_VOLTAR, "Voltar");
        opcao = interface_lerInteiro("\n  Escolha: ", 0, 3);

        switch (opcao) {
        case PESQUISAR_TEXTO:
            interface_lerTexto("\n  Texto procurado: ", trecho, sizeof(trecho), 0);
            indices = banco_pesquisarTexto(banco, trecho, &quantidade);
            mostrarResultadoPesquisa(banco, indices, quantidade, "Resultado da pesquisa");
            break;
        case PESQUISAR_ID:
            printf("\n");
            pergunta = pedirPerguntaPorId(banco);
            if (pergunta != NULL) {
                printf("\n");
                pergunta_exibir(pergunta, 1);
            }
            interface_pausar();
            break;
        case PESQUISAR_CATEGORIA:
            opcao = escolherCategoria(1);
            if (opcao != 0) {
                indices = banco_indicesPorCategoria(banco, (Categoria)opcao, &quantidade);
                mostrarResultadoPesquisa(banco, indices, quantidade,
                                         pergunta_nomeCategoria((Categoria)opcao));
            }
            opcao = PESQUISAR_CATEGORIA; /* continua no submenu */
            break;
        default:
            break;
        }
    } while (opcao != PESQUISAR_VOLTAR && !interface_entradaEncerrada());
}

/**
 * @brief Tela de alteração de uma pergunta existente.
 * @param banco Banco de perguntas.
 */
static void alterarPergunta(BancoPerguntas *banco)
{
    Pergunta *original;
    Pergunta editada;

    interface_mostrarTitulo("Alterar pergunta");
    original = pedirPerguntaPorId(banco);
    if (original == NULL) {
        interface_pausar();
        return;
    }
    printf("\n");
    pergunta_exibir(original, 1);

    editada = *original;
    lerAlteracoesPergunta(&editada);
    if (!pergunta_validar(&editada)) {
        interface_mostrarErro("Dados inválidos. Nada foi alterado.");
        interface_pausar();
        return;
    }

    interface_mostrarTitulo("Confira a pergunta alterada");
    pergunta_exibir(&editada, 1);
    if (interface_confirmar("  Salvar alterações?")) {
        *original = editada;
        interface_beep(1);
        interface_mostrarSucesso("Pergunta alterada.");
    } else {
        interface_mostrarAviso("Alteração cancelada.");
    }
    interface_pausar();
}

/**
 * @brief Tela de exclusão de uma pergunta.
 * @param banco Banco de perguntas.
 */
static void excluirPergunta(BancoPerguntas *banco)
{
    Pergunta *pergunta;
    int id;

    interface_mostrarTitulo("Excluir pergunta");
    pergunta = pedirPerguntaPorId(banco);
    if (pergunta == NULL) {
        interface_pausar();
        return;
    }
    printf("\n");
    pergunta_exibir(pergunta, 1);
    id = pergunta->id;

    if (interface_confirmar(COR_VERMELHO "  Tem certeza que deseja excluir? (cartão vermelho!)" COR_RESET)) {
        banco_removerPorId(banco, id);
        interface_beep(1);
        interface_mostrarSucesso("Pergunta excluída.");
    } else {
        interface_mostrarAviso("Exclusão cancelada.");
    }
    interface_pausar();
}

/**
 * @brief Submenu de gerenciamento das perguntas (CRUD).
 * @param banco Banco de perguntas.
 */
static void menuGerenciar(BancoPerguntas *banco)
{
    int opcao;

    do {
        interface_mostrarTitulo("Gerenciar perguntas");
        interface_imprimirOpcao(GERENCIAR_INSERIR, "Inserir pergunta");
        interface_imprimirOpcao(GERENCIAR_LISTAR, "Listar todas");
        interface_imprimirOpcao(GERENCIAR_PESQUISAR, "Pesquisar");
        interface_imprimirOpcao(GERENCIAR_ALTERAR, "Alterar pergunta");
        interface_imprimirOpcao(GERENCIAR_EXCLUIR, "Excluir pergunta");
        interface_imprimirOpcao(GERENCIAR_VOLTAR, "Voltar");
        opcao = interface_lerInteiro("\n  Escolha: ", 0, 5);

        switch (opcao) {
        case GERENCIAR_INSERIR:   inserirPergunta(banco); break;
        case GERENCIAR_LISTAR:    listarPerguntas(banco); break;
        case GERENCIAR_PESQUISAR: menuPesquisar(banco);   break;
        case GERENCIAR_ALTERAR:   alterarPergunta(banco); break;
        case GERENCIAR_EXCLUIR:   excluirPergunta(banco); break;
        default:                  break;
        }
    } while (opcao != GERENCIAR_VOLTAR && !interface_entradaEncerrada());
}

/* ------------------------------------------------------------------ */
/* Demais telas do menu principal                                      */
/* ------------------------------------------------------------------ */

/**
 * @brief Tela para escolher um nível e navegar pelas perguntas dele.
 * @param banco Banco de perguntas.
 */
static void navegarPorNivel(const BancoPerguntas *banco)
{
    int nivel;
    int quantidade;
    int *indices;
    char titulo[TAM_MENSAGEM];

    interface_mostrarTitulo("Navegar por nível de dificuldade");
    nivel = escolherNivel(1);
    if (nivel == 0) {
        return;
    }
    indices = banco_indicesPorNivel(banco, (NivelDificuldade)nivel, &quantidade);
    if (quantidade == 0) {
        interface_mostrarAviso("Não há perguntas cadastradas neste nível.");
        interface_pausar();
        return;
    }
    snprintf(titulo, sizeof(titulo), "Nível %d: %s", nivel, pergunta_nomeNivel((NivelDificuldade)nivel));
    navegarPerguntas(banco, indices, quantidade, titulo);
    free(indices);
}

/**
 * @brief Tela de exportação das perguntas para um arquivo CSV.
 * @param banco Banco de perguntas.
 */
static void exportarPerguntas(const BancoPerguntas *banco)
{
    char caminho[200];
    char mensagem[TAM_MENSAGEM + 200];

    interface_mostrarTitulo("Exportar perguntas para CSV");
    printf("  Arquivo de destino (ENTER = %s): ", CAMINHO_EXPORTACAO);
    interface_lerTexto("", caminho, sizeof(caminho), 1);
    if (caminho[0] == '\0') {
        strcpy(caminho, CAMINHO_EXPORTACAO);
    }
    if (arquivo_exportarCSV(banco, caminho)) {
        interface_beep(1);
        snprintf(mensagem, sizeof(mensagem), "%d pergunta(s) exportada(s) para %s.", banco->quantidade, caminho);
        interface_mostrarSucesso(mensagem);
    } else {
        snprintf(mensagem, sizeof(mensagem), "Não foi possível criar o arquivo %s.", caminho);
        interface_mostrarErro(mensagem);
    }
    interface_pausar();
}

/**
 * @brief Tela com a quantidade de perguntas por nível e por categoria.
 * @param banco Banco de perguntas.
 */
static void mostrarEstatisticas(const BancoPerguntas *banco)
{
    int nivel;
    int categoria;
    int total;
    int minimo;

    interface_mostrarTitulo("Estatísticas do banco de perguntas");
    printf(COR_NEGRITO "  %-15s %10s %10s" COR_RESET "\n", "Nível", "Cadastradas", "Mínimo");
    for (nivel = 1; nivel <= NUM_NIVEIS; nivel++) {
        total = banco_contarPorNivel(banco, (NivelDificuldade)nivel);
        minimo = pergunta_minimoCadastro((NivelDificuldade)nivel);
        printf("  %s", pergunta_corNivel((NivelDificuldade)nivel));
        interface_imprimirAjustado(pergunta_nomeNivel((NivelDificuldade)nivel), 15);
        printf(COR_RESET " %10d %10d  %s\n", total, minimo,
               total >= minimo ? COR_VERDE "OK" COR_RESET : COR_VERMELHO "abaixo do mínimo" COR_RESET);
    }

    printf("\n" COR_NEGRITO "  %-23s %10s" COR_RESET "\n", "Categoria", "Perguntas");
    for (categoria = 1; categoria <= NUM_CATEGORIAS; categoria++) {
        printf("  ");
        interface_imprimirAjustado(pergunta_nomeCategoria((Categoria)categoria), 23);
        printf(" %10d\n", banco_contarPorCategoria(banco, (Categoria)categoria));
    }
    printf("\n  " COR_BRANCO "Total: %d pergunta(s)" COR_RESET "\n", banco->quantidade);
    interface_pausar();
}

/* ------------------------------------------------------------------ */
/* Menu principal                                                      */
/* ------------------------------------------------------------------ */

void menu_executar(BancoPerguntas *banco, Ranking *ranking)
{
    int opcao;

    do {
        interface_mostrarTitulo("Menu principal");
        interface_imprimirOpcao(OPCAO_GERENCIAR, "Gerenciar perguntas");
        interface_imprimirOpcao(OPCAO_NAVEGAR, "Navegar por nível de dificuldade");
        interface_imprimirOpcao(OPCAO_JOGAR, COR_VERDE "Jogar partida" COR_RESET);
        interface_imprimirOpcao(OPCAO_RANKING, "Ranking dos craques");
        interface_imprimirOpcao(OPCAO_EXPORTAR, "Exportar perguntas para CSV");
        interface_imprimirOpcao(OPCAO_ESTATISTICAS, "Estatísticas do banco");
        interface_imprimirOpcao(OPCAO_SAIR, "Sair e salvar");
        opcao = interface_lerInteiro("\n  Escolha: ", 0, OPCAO_ESTATISTICAS);

        switch (opcao) {
        case OPCAO_GERENCIAR:    menuGerenciar(banco);       break;
        case OPCAO_NAVEGAR:      navegarPorNivel(banco);     break;
        case OPCAO_JOGAR:        jogo_executarPartida(banco, ranking); break;
        case OPCAO_RANKING:      jogo_mostrarRanking(ranking); break;
        case OPCAO_EXPORTAR:     exportarPerguntas(banco);   break;
        case OPCAO_ESTATISTICAS: mostrarEstatisticas(banco); break;
        default:                 break;
        }
    } while (opcao != OPCAO_SAIR && !interface_entradaEncerrada());
}
