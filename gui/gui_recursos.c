/**
 * @file gui_recursos.c
 * @brief Carregamento das fontes e geração dos sons por código
 *        (síntese simples de ondas, sem arquivos de áudio).
 * @author Equipe Futquiz
 */

#include "gui.h"

#include <math.h>
#include <stdlib.h>

/** Taxa de amostragem dos sons gerados. */
#define TAXA_AMOSTRAGEM 44100
/** Valor de pi. */
#define PI_F 3.14159265f

/** Formas de onda disponíveis para os tons. */
typedef enum {
    ONDA_SENO,
    ONDA_QUADRADA,
    ONDA_SERRA
} FormaOnda;

/**
 * @brief Carrega uma fonte com os caracteres acentuados do português.
 * @param caminho Caminho do arquivo .ttf.
 * @param tamanho Tamanho base em pixels.
 * @return Fonte carregada (ou a fonte padrão se falhar).
 */
static Font carregarFonte(const char *caminho, int tamanho)
{
    int codigos[95 + 224];
    int quantidade = 0;
    int codigo;
    Font fonte;

    /* ASCII imprimível + Latin-1 + Latin Extended-A (á, ç, ã, º, Ş...). */
    for (codigo = 32; codigo <= 126; codigo++) {
        codigos[quantidade++] = codigo;
    }
    for (codigo = 160; codigo <= 383; codigo++) {
        codigos[quantidade++] = codigo;
    }
    fonte = LoadFontEx(caminho, tamanho, codigos, quantidade);
    if (!IsFontValid(fonte)) {
        TraceLog(LOG_WARNING, "Fonte %s não encontrada, usando a padrão.", caminho);
        return GetFontDefault();
    }
    GenTextureMipmaps(&fonte.texture);
    SetTextureFilter(fonte.texture, TEXTURE_FILTER_TRILINEAR);
    return fonte;
}

/**
 * @brief Libera uma fonte, exceto a padrão da raylib.
 * @param fonte Fonte a liberar.
 */
static void liberarFonte(Font fonte)
{
    if (fonte.texture.id != GetFontDefault().texture.id) {
        UnloadFont(fonte);
    }
}

/**
 * @brief Soma um tom com envelope (ataque rápido e decaimento) ao buffer.
 * @param buffer Amostras de áudio.
 * @param total Quantidade de amostras do buffer.
 * @param inicio Início do tom em segundos.
 * @param duracao Duração em segundos.
 * @param frequencia Frequência em Hz.
 * @param volume Volume de 0 a 1.
 * @param forma Forma de onda.
 */
static void adicionarTom(float *buffer, int total, float inicio, float duracao, float frequencia,
                         float volume, FormaOnda forma)
{
    int primeira = (int)(inicio * TAXA_AMOSTRAGEM);
    int quantidade = (int)(duracao * TAXA_AMOSTRAGEM);
    int i;
    float t;
    float fase;
    float onda;
    float envelope;

    for (i = 0; i < quantidade && primeira + i < total; i++) {
        t = (float)i / TAXA_AMOSTRAGEM;
        fase = fmodf(t * frequencia, 1.0f);
        if (forma == ONDA_QUADRADA) {
            onda = fase < 0.5f ? 1.0f : -1.0f;
        } else if (forma == ONDA_SERRA) {
            onda = 2.0f * fase - 1.0f;
        } else {
            onda = sinf(2.0f * PI_F * fase);
        }
        envelope = fminf(1.0f, t / 0.005f) * expf(-3.0f * t / duracao);
        buffer[primeira + i] += onda * volume * envelope;
    }
}

/**
 * @brief Soma ruído filtrado (som de torcida) ao buffer.
 * @param buffer Amostras de áudio.
 * @param total Quantidade de amostras.
 * @param volume Volume de 0 a 1.
 */
static void adicionarTorcida(float *buffer, int total, float volume)
{
    int i;
    float filtrado = 0.0f;
    float t;
    float duracao = (float)total / TAXA_AMOSTRAGEM;
    float envelope;
    float ruido;

    for (i = 0; i < total; i++) {
        t = (float)i / TAXA_AMOSTRAGEM;
        ruido = (float)rand() / (float)RAND_MAX * 2.0f - 1.0f;
        filtrado += 0.07f * (ruido - filtrado); /* filtro passa-baixa */
        envelope = fminf(1.0f, t / 0.5f) * fminf(1.0f, (duracao - t) / 0.8f);
        envelope *= 0.75f + 0.25f * sinf(2.0f * PI_F * 1.3f * t); /* "ondas" da torcida */
        buffer[i] += filtrado * 3.0f * volume * envelope;
    }
}

/**
 * @brief Converte amostras em um Sound da raylib e libera o buffer.
 * @param buffer Amostras (de -1 a 1), liberadas aqui.
 * @param total Quantidade de amostras.
 * @return Som carregado.
 */
static Sound criarSom(float *buffer, int total)
{
    Wave onda;
    short *dados = MemAlloc((unsigned int)(total * (int)sizeof(short)));
    int i;
    float amostra;
    Sound som;

    for (i = 0; i < total; i++) {
        amostra = fmaxf(-1.0f, fminf(1.0f, buffer[i]));
        dados[i] = (short)(amostra * 32000.0f);
    }
    free(buffer);
    onda.frameCount = (unsigned int)total;
    onda.sampleRate = TAXA_AMOSTRAGEM;
    onda.sampleSize = 16;
    onda.channels = 1;
    onda.data = dados;
    som = LoadSoundFromWave(onda);
    UnloadWave(onda);
    return som;
}

/**
 * @brief Aloca um buffer de áudio zerado.
 * @param segundos Duração em segundos.
 * @param total Endereço onde será guardada a quantidade de amostras.
 * @return Buffer alocado.
 */
static float *novoBuffer(float segundos, int *total)
{
    *total = (int)(segundos * TAXA_AMOSTRAGEM);
    return calloc((size_t)*total, sizeof(float));
}

/**
 * @brief Gera o apito do árbitro (tom agudo com trinado).
 * @return Som do apito.
 */
static Sound gerarApito(void)
{
    int total;
    int i;
    float t;
    float frequencia;
    float fase = 0.0f;
    float envelope;
    float *buffer = novoBuffer(0.75f, &total);

    for (i = 0; i < total; i++) {
        t = (float)i / TAXA_AMOSTRAGEM;
        frequencia = 2900.0f + 140.0f * sinf(2.0f * PI_F * 30.0f * t);
        fase += frequencia / TAXA_AMOSTRAGEM;
        envelope = fminf(1.0f, t / 0.02f) * fminf(1.0f, (0.75f - t) / 0.1f);
        buffer[i] = sinf(2.0f * PI_F * fase) * 0.22f * envelope;
    }
    return criarSom(buffer, total);
}

/**
 * @brief Gera a buzina de erro (serra descendente).
 * @return Som de erro.
 */
static Sound gerarErro(void)
{
    int total;
    int i;
    float t;
    float fase = 0.0f;
    float envelope;
    float *buffer = novoBuffer(0.7f, &total);

    for (i = 0; i < total; i++) {
        t = (float)i / TAXA_AMOSTRAGEM;
        fase += (200.0f - 90.0f * t) / TAXA_AMOSTRAGEM;
        envelope = fminf(1.0f, t / 0.01f) * fminf(1.0f, (0.7f - t) / 0.15f);
        buffer[i] = (2.0f * fmodf(fase, 1.0f) - 1.0f) * 0.3f * envelope;
    }
    return criarSom(buffer, total);
}

void gui_carregarRecursos(Gui *gui)
{
    int total;
    float *buffer;
    static const float notasAcerto[] = { 523.25f, 659.25f, 783.99f, 1046.50f };
    static const float notasCampeao[] = { 392.00f, 523.25f, 659.25f, 783.99f, 659.25f, 783.99f };
    int i;

    gui->fontes.titulo = carregarFonte("assets/fontes/Anton-Regular.ttf", 112);
    gui->fontes.texto = carregarFonte("assets/fontes/FiraSans-SemiBold.ttf", 56);
    gui->fontes.pequeno = carregarFonte("assets/fontes/FiraSans-SemiBold.ttf", 26);

    InitAudioDevice();
    gui->sons.ativo = IsAudioDeviceReady();
    if (!gui->sons.ativo) {
        return;
    }

    buffer = novoBuffer(0.05f, &total);
    adicionarTom(buffer, total, 0.0f, 0.05f, 1400.0f, 0.35f, ONDA_SENO);
    gui->sons.clique = criarSom(buffer, total);

    buffer = novoBuffer(0.7f, &total);
    for (i = 0; i < 4; i++) {
        adicionarTom(buffer, total, 0.09f * (float)i, 0.35f, notasAcerto[i], 0.16f, ONDA_QUADRADA);
        adicionarTom(buffer, total, 0.09f * (float)i, 0.35f, notasAcerto[i] * 2.0f, 0.10f, ONDA_SENO);
    }
    gui->sons.acerto = criarSom(buffer, total);

    gui->sons.erro = gerarErro();
    gui->sons.apito = gerarApito();

    buffer = novoBuffer(2.2f, &total);
    adicionarTorcida(buffer, total, 0.5f);
    gui->sons.torcida = criarSom(buffer, total);

    buffer = novoBuffer(3.0f, &total);
    adicionarTorcida(buffer, total, 0.35f);
    for (i = 0; i < 6; i++) {
        adicionarTom(buffer, total, 0.16f * (float)i, 0.3f, notasCampeao[i], 0.15f, ONDA_QUADRADA);
    }
    adicionarTom(buffer, total, 1.0f, 1.6f, 1046.50f, 0.18f, ONDA_QUADRADA);
    adicionarTom(buffer, total, 1.0f, 1.6f, 783.99f, 0.12f, ONDA_SENO);
    adicionarTom(buffer, total, 1.0f, 1.6f, 523.25f, 0.12f, ONDA_SENO);
    gui->sons.campeao = criarSom(buffer, total);
}

void gui_liberarRecursos(Gui *gui)
{
    if (gui->sons.ativo) {
        UnloadSound(gui->sons.clique);
        UnloadSound(gui->sons.acerto);
        UnloadSound(gui->sons.erro);
        UnloadSound(gui->sons.apito);
        UnloadSound(gui->sons.torcida);
        UnloadSound(gui->sons.campeao);
        CloseAudioDevice();
    }
    liberarFonte(gui->fontes.titulo);
    liberarFonte(gui->fontes.texto);
    liberarFonte(gui->fontes.pequeno);
}

void gui_tocar(const Gui *gui, Sound som)
{
    if (gui->sons.ativo) {
        PlaySound(som);
    }
}
