/**
 * @file jogo.c
 * @brief Implementação das telas de terminal da partida e do ranking.
 * @author Equipe Futquiz
 */

#include "jogo.h"
#include "partida.h"
#include "interface.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/** Tamanho dos buffers de mensagens. */
#define TAM_MENSAGEM 160
/** Largura máxima da barra de votos da torcida. */
#define LARGURA_BARRA 40

/**
 * @brief Escreve um prêmio de forma curta (1k, 50k, 1M).
 * @param valor Valor em reais.
 * @param destino Buffer de destino.
 * @param tamanho Tamanho do buffer.
 */
static void premioCurto(int valor, char *destino, size_t tamanho)
{
    if (valor >= 1000000) {
        snprintf(destino, tamanho, "%dM", valor / 1000000);
    } else {
        snprintf(destino, tamanho, "%dk", valor / 1000);
    }
}

/**
 * @brief Mostra a escada de prêmios com a rodada atual destacada.
 * @param partida Partida em andamento.
 */
static void mostrarEscadaPremios(const Partida *partida)
{
    int rodada;
    char texto[16];

    printf("  ");
    for (rodada = 0; rodada < NUM_RODADAS; rodada++) {
        premioCurto(partida_premioDaRodada(rodada), texto, sizeof(texto));
        if (rodada < partida->rodada) {
            printf(COR_VERDE "%s" COR_RESET, texto);
        } else if (rodada == partida->rodada) {
            printf("\033[30;43m %s " COR_RESET, texto);
        } else if (partida_ehMarco(rodada)) {
            printf(COR_AZUL "%s" COR_RESET, texto);
        } else {
            printf(COR_CINZA "%s" COR_RESET, texto);
        }
        printf(partida_ehMarco(rodada) ? COR_AZUL " | " COR_RESET : " ");
    }
    printf("\n  " COR_CINZA "(verde = conquistado, azul = marco de segurança)" COR_RESET "\n\n");
}

/**
 * @brief Mostra as alternativas, os anulados pelo VAR e os votos da torcida.
 * @param partida Partida em andamento.
 * @param pergunta Pergunta atual.
 */
static void mostrarAlternativas(const Partida *partida, const Pergunta *pergunta)
{
    int i;
    int j;
    int tamanhoBarra;

    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        if (partida->eliminadas[i]) {
            printf("    " COR_CINZA "(%c) ---- anulada pelo VAR ----" COR_RESET "\n", 'A' + i);
        } else {
            printf("    " COR_AMARELO "(%c)" COR_RESET " %s\n", 'A' + i, pergunta->alternativas[i]);
        }
    }
    if (!partida->torcidaAtiva) {
        return;
    }
    printf("\n  " COR_CIANO "A torcida votou:" COR_RESET "\n");
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        if (partida->eliminadas[i]) {
            continue;
        }
        tamanhoBarra = partida->votosTorcida[i] * LARGURA_BARRA / 100;
        printf("    (%c) " COR_CIANO, 'A' + i);
        for (j = 0; j < tamanhoBarra; j++) {
            putchar('#');
        }
        printf(COR_RESET " %d%%\n", partida->votosTorcida[i]);
    }
}

/**
 * @brief Mostra os valores de errar, parar e acertar (como no SBT).
 * @param partida Partida em andamento.
 */
static void mostrarValores(const Partida *partida)
{
    char errar[32];
    char parar[32];
    char acertar[32];

    partida_formatarPremio(partida_premioSeErrar(partida), errar, sizeof(errar));
    partida_formatarPremio(partida_premioSeParar(partida), parar, sizeof(parar));
    partida_formatarPremio(partida_premioSeAcertar(partida), acertar, sizeof(acertar));
    printf("\n  " COR_VERMELHO "ERRAR: %s" COR_RESET "   " COR_AMARELO "PARAR: %s" COR_RESET
           "   " COR_VERDE "ACERTAR: %s" COR_RESET "\n", errar, parar, acertar);
}

/**
 * @brief Mostra as opções de ajuda e de ação da rodada.
 * @param partida Partida em andamento.
 */
static void mostrarOpcoes(const Partida *partida)
{
    int liberadas = partida_ajudasLiberadas(partida);

    printf("\n  " COR_NEGRITO "[A-D]" COR_RESET " Responder   ");
    printf("%s[1] VAR" COR_RESET "   ", (liberadas && partida->varDisponivel) ? COR_AMARELO : COR_CINZA);
    printf("%s[2] Torcida" COR_RESET "   ", (liberadas && partida->torcidaDisponivel) ? COR_AMARELO : COR_CINZA);
    printf("%s[3] Substituição (%d)" COR_RESET "   ",
           (liberadas && partida->substituicoesRestantes > 0) ? COR_AMARELO : COR_CINZA,
           partida->substituicoesRestantes);
    printf(COR_VERMELHO "[0] Parar" COR_RESET "\n");
    if (!liberadas) {
        printf("  " COR_MAGENTA "Pergunta do milhão: as ajudas não valem e errar zera o prêmio!" COR_RESET "\n");
    }
}

/**
 * @brief Desenha a tela completa de uma rodada.
 * @param partida Partida em andamento.
 * @param nome Nome do jogador.
 */
static void desenharRodada(const Partida *partida, const char *nome)
{
    const Pergunta *pergunta = partida_perguntaAtual(partida);
    char titulo[TAM_MENSAGEM];

    snprintf(titulo, sizeof(titulo), "Partida de %s", nome);
    interface_mostrarTitulo(titulo);
    mostrarEscadaPremios(partida);
    printf("  %sRodada %d de %d  |  %s" COR_RESET "  |  " COR_AZUL "%s" COR_RESET "\n\n",
           pergunta_corNivel(pergunta->nivel), partida->rodada + 1, NUM_RODADAS,
           pergunta_nomeNivel(pergunta->nivel), pergunta_nomeCategoria(pergunta->categoria));
    printf("  " COR_BRANCO "%s" COR_RESET "\n\n", pergunta->enunciado);
    mostrarAlternativas(partida, pergunta);
    mostrarValores(partida);
    mostrarOpcoes(partida);
}

/**
 * @brief Mostra o resultado de uma resposta.
 * @param partida Partida (já atualizada pela resposta).
 * @param resultado Resultado devolvido por partida_responder().
 * @param pergunta Pergunta respondida.
 * @param rodadaRespondida Rodada em que a resposta foi dada.
 */
static void mostrarResultadoResposta(const Partida *partida, ResultadoResposta resultado,
                                     const Pergunta *pergunta, int rodadaRespondida)
{
    char premio[32];
    char mensagem[TAM_MENSAGEM];

    printf("\n");
    if (resultado == RESPOSTA_ERRADA) {
        interface_beep(3);
        printf("  " COR_VERMELHO "%s" COR_RESET " A resposta certa era " COR_VERDE "(%c) %s" COR_RESET "\n",
               partida_mensagemErro(), pergunta->correta, pergunta->alternativas[pergunta->correta - 'A']);
        return;
    }
    interface_beep(resultado == RESPOSTA_CAMPEAO ? 5 : 2);
    printf("  " COR_VERDE "%s" COR_RESET "\n", partida_mensagemAcerto(rodadaRespondida));
    if (resultado == RESPOSTA_CERTA) {
        partida_formatarPremio(partida_premioSeParar(partida), premio, sizeof(premio));
        snprintf(mensagem, sizeof(mensagem), "Você já garantiu %s.", premio);
        interface_mostrarInfo(mensagem);
    }
}

/**
 * @brief Pede o nome do jogador e a categoria preferida.
 * @param nome Buffer que recebe o nome.
 * @param tamanho Tamanho do buffer.
 * @return Categoria escolhida ou CATEGORIA_TODAS.
 */
static int prepararJogador(char *nome, size_t tamanho)
{
    int categoria;

    interface_mostrarTitulo("Nova partida");
    interface_lerTexto("  Nome do jogador: ", nome, tamanho, 0);
    printf("\n  " COR_NEGRITO "Escolha a categoria preferida:" COR_RESET "\n\n");
    interface_imprimirOpcao(CATEGORIA_TODAS, "Todas as categorias");
    for (categoria = 1; categoria <= NUM_CATEGORIAS; categoria++) {
        interface_imprimirOpcao(categoria, pergunta_nomeCategoria((Categoria)categoria));
    }
    return interface_lerInteiro("\n  Categoria: ", 0, NUM_CATEGORIAS);
}

/**
 * @brief Trata o uso de uma ajuda escolhida pelo jogador.
 * @param partida Partida em andamento.
 * @param opcao Caractere digitado ('1', '2' ou '3').
 * @param aviso Buffer para mensagem de retorno.
 * @param tamanhoAviso Tamanho do buffer de aviso.
 */
static void usarAjuda(Partida *partida, char opcao, char *aviso, size_t tamanhoAviso)
{
    int usou = 0;

    if (!partida_ajudasLiberadas(partida)) {
        snprintf(aviso, tamanhoAviso, "Na pergunta do milhão não há ajudas.");
    } else if (opcao == '1') {
        usou = partida_usarVar(partida);
        snprintf(aviso, tamanhoAviso, usou ? "O VAR revisou o lance e anulou duas alternativas!"
                                           : "O VAR já foi usado nesta partida.");
    } else if (opcao == '2') {
        usou = partida_usarTorcida(partida);
        snprintf(aviso, tamanhoAviso, usou ? "A torcida fez barulho! Veja os votos."
                                           : "A torcida já ajudou nesta partida.");
    } else {
        usou = partida_substituir(partida);
        snprintf(aviso, tamanhoAviso, usou ? "Substituição feita: nova pergunta em campo!"
                                           : "Sem substituições ou sem perguntas reservas.");
    }
    interface_beep(1);
}

/**
 * @brief Mostra o fim da partida e registra no ranking.
 * @param partida Partida encerrada.
 * @param nome Nome do jogador.
 * @param ranking Ranking.
 */
static void finalizarPartida(const Partida *partida, const char *nome, Ranking *ranking)
{
    char premio[32];
    char mensagem[TAM_MENSAGEM];
    int posicao;

    partida_formatarPremio(partida->premioFinal, premio, sizeof(premio));
    interface_mostrarTitulo("Fim de jogo");
    if (partida->estado == PARTIDA_CAMPEAO) {
        printf(COR_AMARELO "      *  *  *   C A M P E Ã O   *  *  *\n" COR_RESET "\n");
    }
    printf("  Jogador: " COR_BRANCO "%s" COR_RESET "\n", nome);
    printf("  Acertos: " COR_BRANCO "%d de %d" COR_RESET "\n", partida_acertos(partida), NUM_RODADAS);
    printf("  Prêmio:  " COR_VERDE "%s" COR_RESET "\n\n", premio);

    posicao = ranking_inserir(ranking, nome, partida->premioFinal, partida_acertos(partida));
    if (posicao > 0) {
        interface_beep(2);
        snprintf(mensagem, sizeof(mensagem), "Você entrou no ranking em %dº lugar!", posicao);
        interface_mostrarSucesso(mensagem);
        if (!ranking_salvar(ranking, CAMINHO_RANKING)) {
            interface_mostrarErro("Não foi possível salvar o ranking.");
        }
    } else {
        interface_mostrarInfo("Não foi dessa vez que você entrou no ranking.");
    }
    interface_pausar();
}

void jogo_executarPartida(const BancoPerguntas *banco, Ranking *ranking)
{
    Partida partida;
    char nome[TAM_NOME_JOGADOR];
    char entrada[16];
    char aviso[TAM_MENSAGEM] = "";
    char opcao;
    int categoria;
    int rodadaRespondida;
    const Pergunta *pergunta;
    ResultadoResposta resultado;

    if (!partida_bancoSuficiente(banco)) {
        interface_mostrarTitulo("Nova partida");
        interface_mostrarErro("Não há perguntas suficientes em todos os níveis para uma partida.");
        interface_pausar();
        return;
    }
    categoria = prepararJogador(nome, sizeof(nome));
    if (interface_entradaEncerrada() || !partida_iniciar(&partida, banco, categoria)) {
        return;
    }
    interface_beep(1);

    while (partida.estado == PARTIDA_EM_ANDAMENTO) {
        desenharRodada(&partida, nome);
        if (aviso[0] != '\0') {
            printf("\n");
            interface_mostrarAviso(aviso);
            aviso[0] = '\0';
        }
        interface_lerTexto("\n  Sua jogada: ", entrada, sizeof(entrada), 1);
        if (interface_entradaEncerrada()) {
            partida_parar(&partida);
            break;
        }
        opcao = (char)toupper((unsigned char)entrada[0]);

        if (opcao >= 'A' && opcao <= 'D' && entrada[1] == '\0') {
            if (partida.eliminadas[opcao - 'A']) {
                snprintf(aviso, sizeof(aviso), "Essa alternativa foi anulada pelo VAR.");
                continue;
            }
            if (!interface_confirmar("  Está certo disso?")) {
                continue;
            }
            pergunta = partida_perguntaAtual(&partida);
            rodadaRespondida = partida.rodada;
            resultado = partida_responder(&partida, opcao);
            mostrarResultadoResposta(&partida, resultado, pergunta, rodadaRespondida);
            interface_pausar();
        } else if ((opcao == '1' || opcao == '2' || opcao == '3') && entrada[1] == '\0') {
            usarAjuda(&partida, opcao, aviso, sizeof(aviso));
        } else if (opcao == '0' && entrada[1] == '\0') {
            if (interface_confirmar("  Quer mesmo parar e levar o que já ganhou?")) {
                partida_parar(&partida);
            }
        } else {
            interface_beep(1);
            snprintf(aviso, sizeof(aviso), "Jogada inválida. Use A, B, C, D, 1, 2, 3 ou 0.");
        }
    }
    finalizarPartida(&partida, nome, ranking);
}

void jogo_mostrarRanking(const Ranking *ranking)
{
    int i;
    char premio[32];
    const char *cor;

    interface_mostrarTitulo("Ranking dos craques");
    if (ranking->quantidade == 0) {
        interface_mostrarAviso("Ninguém jogou ainda. Seja o primeiro!");
        interface_pausar();
        return;
    }
    printf(COR_NEGRITO "  %-4s %-24s %8s %16s" COR_RESET "\n", "Pos", "Jogador", "Acertos", "Prêmio");
    for (i = 0; i < ranking->quantidade; i++) {
        partida_formatarPremio(ranking->registros[i].premio, premio, sizeof(premio));
        cor = (i == 0) ? COR_AMARELO : (i == 1) ? COR_BRANCO : (i == 2) ? COR_VERMELHO : COR_RESET;
        printf("  %s%2dº  ", cor, i + 1);
        interface_imprimirAjustado(ranking->registros[i].nome, 24);
        printf(" %8d %16s" COR_RESET "\n", ranking->registros[i].acertos, premio);
    }
    interface_pausar();
}
