/**
 * @file partida.c
 * @brief Implementação das regras de uma partida do Futquiz.
 * @author Equipe Futquiz
 */

#include "partida.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Rodada da pergunta do milhão (última, índice 14). */
#define RODADA_FINAL (NUM_RODADAS - 1)

/**
 * @brief Verifica se um índice do banco já foi sorteado nesta partida.
 * @param partida Partida.
 * @param indice Índice no banco.
 * @return 1 se já foi usado, 0 caso contrário.
 */
static int jaUsada(const Partida *partida, int indice)
{
    int i;

    for (i = 0; i < partida->quantidadeUsadas; i++) {
        if (partida->usadas[i] == indice) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Verifica se uma pergunta pode ser sorteada.
 * @param partida Partida.
 * @param indice Índice no banco.
 * @param nivel Nível exigido.
 * @param categoria Categoria exigida ou CATEGORIA_TODAS.
 * @return 1 se é candidata, 0 caso contrário.
 */
static int ehCandidata(const Partida *partida, int indice, NivelDificuldade nivel, int categoria)
{
    const Pergunta *pergunta = &partida->banco->itens[indice];

    return pergunta->nivel == nivel &&
           (categoria == CATEGORIA_TODAS || (int)pergunta->categoria == categoria) &&
           !jaUsada(partida, indice);
}

/**
 * @brief Sorteia entre as candidatas de um nível e categoria.
 * @param partida Partida.
 * @param nivel Nível exigido.
 * @param categoria Categoria exigida ou CATEGORIA_TODAS.
 * @return Índice sorteado ou -1 se não houver candidatas.
 */
static int sortearComFiltro(const Partida *partida, NivelDificuldade nivel, int categoria)
{
    int i;
    int total = 0;
    int escolhida;

    for (i = 0; i < partida->banco->quantidade; i++) {
        total += ehCandidata(partida, i, nivel, categoria);
    }
    if (total == 0) {
        return -1;
    }
    escolhida = rand() % total;
    for (i = 0; i < partida->banco->quantidade; i++) {
        if (ehCandidata(partida, i, nivel, categoria)) {
            if (escolhida == 0) {
                return i;
            }
            escolhida--;
        }
    }
    return -1;
}

/**
 * @brief Sorteia uma pergunta inédita do nível, preferindo a categoria escolhida.
 * @param partida Partida.
 * @param nivel Nível exigido.
 * @return Índice sorteado (já marcado como usado) ou -1 se não houver.
 */
static int sortearPergunta(Partida *partida, NivelDificuldade nivel)
{
    int indice = -1;

    if (partida->quantidadeUsadas >= NUM_RODADAS + MAX_SUBSTITUICOES) {
        return -1;
    }
    if (partida->categoriaPreferida != CATEGORIA_TODAS) {
        indice = sortearComFiltro(partida, nivel, partida->categoriaPreferida);
    }
    if (indice == -1) {
        indice = sortearComFiltro(partida, nivel, CATEGORIA_TODAS);
    }
    if (indice != -1) {
        partida->usadas[partida->quantidadeUsadas++] = indice;
    }
    return indice;
}

/**
 * @brief Limpa o VAR e a torcida ao trocar de pergunta.
 * @param partida Partida.
 */
static void limparAjudasDaRodada(Partida *partida)
{
    memset(partida->eliminadas, 0, sizeof(partida->eliminadas));
    memset(partida->votosTorcida, 0, sizeof(partida->votosTorcida));
    partida->torcidaAtiva = 0;
}

/**
 * @brief Encerra a partida com um estado e um prêmio.
 * @param partida Partida.
 * @param estado Estado final.
 * @param premio Prêmio levado.
 */
static void encerrar(Partida *partida, EstadoPartida estado, int premio)
{
    partida->estado = estado;
    partida->premioFinal = premio;
}

int partida_bancoSuficiente(const BancoPerguntas *banco)
{
    int nivel;

    for (nivel = 1; nivel <= NUM_NIVEIS; nivel++) {
        if (banco_contarPorNivel(banco, (NivelDificuldade)nivel) <
            pergunta_quantidadeNaPartida((NivelDificuldade)nivel)) {
            return 0;
        }
    }
    return 1;
}

int partida_iniciar(Partida *partida, const BancoPerguntas *banco, int categoriaPreferida)
{
    int rodada;

    if (!partida_bancoSuficiente(banco)) {
        return 0;
    }
    memset(partida, 0, sizeof(*partida));
    partida->banco = banco;
    partida->categoriaPreferida = categoriaPreferida;
    partida->estado = PARTIDA_EM_ANDAMENTO;
    partida->varDisponivel = 1;
    partida->torcidaDisponivel = 1;
    partida->substituicoesRestantes = MAX_SUBSTITUICOES;

    for (rodada = 0; rodada < NUM_RODADAS; rodada++) {
        partida->perguntas[rodada] = sortearPergunta(partida, partida_nivelDaRodada(rodada));
        if (partida->perguntas[rodada] == -1) {
            return 0;
        }
    }
    return 1;
}

const Pergunta *partida_perguntaAtual(const Partida *partida)
{
    return &partida->banco->itens[partida->perguntas[partida->rodada]];
}

NivelDificuldade partida_nivelDaRodada(int rodada)
{
    if (rodada < 2) {
        return NIVEL_MUITO_FACIL;   /* rodadas 1 e 2   */
    }
    if (rodada < 4) {
        return NIVEL_FACIL;         /* rodadas 3 e 4   */
    }
    if (rodada < 8) {
        return NIVEL_MEDIO;         /* rodadas 5 a 8   */
    }
    if (rodada < 12) {
        return NIVEL_DIFICIL;       /* rodadas 9 a 12  */
    }
    return NIVEL_MUITO_DIFICIL;     /* rodadas 13 a 15 */
}

int partida_ehMarco(int rodada)
{
    return rodada == 4 || rodada == 9; /* rodadas 5 e 10 */
}

int partida_premioDaRodada(int rodada)
{
    static const int premios[NUM_RODADAS] = {
        1000, 2000, 3000, 4000, 5000,
        10000, 20000, 30000, 40000, 50000,
        100000, 200000, 300000, 500000, 1000000
    };

    if (rodada < 0 || rodada >= NUM_RODADAS) {
        return 0;
    }
    return premios[rodada];
}

int partida_premioSeParar(const Partida *partida)
{
    return partida_premioDaRodada(partida->rodada - 1);
}

int partida_premioSeAcertar(const Partida *partida)
{
    return partida_premioDaRodada(partida->rodada);
}

int partida_premioSeErrar(const Partida *partida)
{
    int rodada;

    if (partida->rodada == RODADA_FINAL) {
        return 0;
    }
    for (rodada = partida->rodada - 1; rodada >= 0; rodada--) {
        if (partida_ehMarco(rodada)) {
            return partida_premioDaRodada(rodada);
        }
    }
    return 0;
}

ResultadoResposta partida_responder(Partida *partida, char letra)
{
    int alternativa = letra - 'A';
    const Pergunta *pergunta;

    if (partida->estado != PARTIDA_EM_ANDAMENTO || alternativa < 0 || alternativa >= NUM_ALTERNATIVAS ||
        partida->eliminadas[alternativa]) {
        return RESPOSTA_INVALIDA;
    }
    pergunta = partida_perguntaAtual(partida);
    if (letra != pergunta->correta) {
        encerrar(partida, PARTIDA_ERROU, partida_premioSeErrar(partida));
        return RESPOSTA_ERRADA;
    }
    if (partida->rodada == RODADA_FINAL) {
        encerrar(partida, PARTIDA_CAMPEAO, partida_premioDaRodada(RODADA_FINAL));
        return RESPOSTA_CAMPEAO;
    }
    partida->rodada++;
    limparAjudasDaRodada(partida);
    return RESPOSTA_CERTA;
}

void partida_parar(Partida *partida)
{
    if (partida->estado == PARTIDA_EM_ANDAMENTO) {
        encerrar(partida, PARTIDA_PAROU, partida_premioSeParar(partida));
    }
}

int partida_ajudasLiberadas(const Partida *partida)
{
    return partida->estado == PARTIDA_EM_ANDAMENTO && partida->rodada != RODADA_FINAL;
}

int partida_usarVar(Partida *partida)
{
    int erradas[NUM_ALTERNATIVAS];
    int quantidade = 0;
    int i;
    int sorteada;
    const Pergunta *pergunta;

    if (!partida->varDisponivel || !partida_ajudasLiberadas(partida)) {
        return 0;
    }
    pergunta = partida_perguntaAtual(partida);
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        if ('A' + i != pergunta->correta) {
            erradas[quantidade++] = i;
        }
    }
    /* Sorteia qual das 3 erradas continua na tela; as outras 2 são anuladas. */
    sorteada = rand() % quantidade;
    for (i = 0; i < quantidade; i++) {
        if (i != sorteada) {
            partida->eliminadas[erradas[i]] = 1;
        }
    }
    partida->varDisponivel = 0;
    return 1;
}

int partida_usarTorcida(Partida *partida)
{
    /* Porcentagem base de acerto da torcida por nível (muito fácil -> muito difícil). */
    static const int baseAcerto[NUM_NIVEIS] = {75, 62, 50, 38, 28};
    int pesos[NUM_ALTERNATIVAS] = {0};
    int somaPesos = 0;
    int restante;
    int distribuido = 0;
    int ultima = -1;
    int correta;
    int i;
    const Pergunta *pergunta;

    if (!partida->torcidaDisponivel || !partida_ajudasLiberadas(partida)) {
        return 0;
    }
    pergunta = partida_perguntaAtual(partida);
    correta = pergunta->correta - 'A';

    /* A alternativa correta recebe a base do nível, com variação de +-12. */
    partida->votosTorcida[correta] = baseAcerto[pergunta->nivel - 1] + rand() % 25 - 12;
    restante = 100 - partida->votosTorcida[correta];

    /* O restante é dividido aleatoriamente entre as erradas não anuladas. */
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        if (i != correta && !partida->eliminadas[i]) {
            pesos[i] = 1 + rand() % 100;
            somaPesos += pesos[i];
            ultima = i;
        }
    }
    if (ultima == -1) {
        partida->votosTorcida[correta] = 100;
    } else {
        for (i = 0; i < NUM_ALTERNATIVAS; i++) {
            if (pesos[i] > 0 && i != ultima) {
                partida->votosTorcida[i] = restante * pesos[i] / somaPesos;
                distribuido += partida->votosTorcida[i];
            }
        }
        partida->votosTorcida[ultima] = restante - distribuido; /* fecha 100% */
    }
    partida->torcidaAtiva = 1;
    partida->torcidaDisponivel = 0;
    return 1;
}

int partida_substituir(Partida *partida)
{
    int nova;

    if (partida->substituicoesRestantes <= 0 || !partida_ajudasLiberadas(partida)) {
        return 0;
    }
    nova = sortearPergunta(partida, partida_nivelDaRodada(partida->rodada));
    if (nova == -1) {
        return 0;
    }
    partida->perguntas[partida->rodada] = nova;
    partida->substituicoesRestantes--;
    limparAjudasDaRodada(partida);
    return 1;
}

int partida_acertos(const Partida *partida)
{
    return partida->rodada + (partida->estado == PARTIDA_CAMPEAO ? 1 : 0);
}

void partida_formatarPremio(int valor, char *destino, size_t tamanho)
{
    char digitos[16];
    char invertido[24];
    int tamanhoDigitos;
    int posicao = 0;
    int i;

    snprintf(digitos, sizeof(digitos), "%d", valor);
    tamanhoDigitos = (int)strlen(digitos);
    /* Monta de trás para frente colocando um ponto a cada 3 dígitos. */
    for (i = tamanhoDigitos - 1; i >= 0; i--) {
        invertido[posicao++] = digitos[i];
        if ((tamanhoDigitos - i) % 3 == 0 && i > 0) {
            invertido[posicao++] = '.';
        }
    }
    invertido[posicao] = '\0';
    for (i = 0; i < posicao / 2; i++) {
        char temporario = invertido[i];
        invertido[i] = invertido[posicao - 1 - i];
        invertido[posicao - 1 - i] = temporario;
    }
    snprintf(destino, tamanho, "R$ %s", invertido);
}

const char *partida_mensagemAcerto(int rodada)
{
    static const char *mensagens[] = {
        "GOOOOL!", "Que golaço!", "Bola na rede!", "Chutou, é gol!",
        "Que pintura!", "Acertou no ângulo!", "Tabelinha perfeita!", "É craque!"
    };
    const int quantidade = (int)(sizeof(mensagens) / sizeof(mensagens[0]));

    if (rodada == 4) {
        return "Classificado! Primeiro marco de segurança garantido!";
    }
    if (rodada == 9) {
        return "Está na final! Segundo marco de segurança garantido!";
    }
    if (rodada == RODADA_FINAL) {
        return "É CAMPEÃO! Um milhão no bolso!";
    }
    return mensagens[rand() % quantidade];
}

const char *partida_mensagemErro(void)
{
    static const char *mensagens[] = {
        "Na trave!", "Cartão vermelho!", "Impedido!", "Pra fora!", "Gol contra!"
    };
    const int quantidade = (int)(sizeof(mensagens) / sizeof(mensagens[0]));

    return mensagens[rand() % quantidade];
}
