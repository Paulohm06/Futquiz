/**
 * @file arquivo.c
 * @brief Implementação da leitura e gravação de arquivos CSV e binário.
 * @author Equipe Futquiz
 */

#include "arquivo.h"
#include "interface.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

/** Tamanho máximo de uma linha do CSV. */
#define TAM_LINHA_CSV 2048
/** Tamanho máximo de um campo do CSV. */
#define TAM_CAMPO_CSV TAM_ENUNCIADO
/** Quantidade de campos de cada linha do CSV. */
#define NUM_CAMPOS_CSV 8
/** Assinatura gravada no início do arquivo binário. */
#define ASSINATURA_BINARIO "FTQZ"
/** Versão do formato binário (mude se alterar a struct Pergunta). */
#define VERSAO_BINARIO 1

/**
 * @brief Cabeçalho gravado no início do arquivo binário.
 */
typedef struct {
    char assinatura[4];  /**< Sempre "FTQZ".                     */
    int versao;          /**< Versão do formato.                 */
    int quantidade;      /**< Quantidade de perguntas gravadas.  */
    int proximoId;       /**< Próximo ID livre do banco.         */
} CabecalhoBinario;

/** Posição de cada campo dentro de uma linha do CSV. */
enum {
    CAMPO_NIVEL = 0,
    CAMPO_CATEGORIA,
    CAMPO_ENUNCIADO,
    CAMPO_ALTERNATIVA_A,
    CAMPO_CORRETA = CAMPO_ALTERNATIVA_A + NUM_ALTERNATIVAS
};

int arquivo_existe(const char *caminho)
{
    FILE *arquivo = fopen(caminho, "rb");

    if (arquivo == NULL) {
        return 0;
    }
    fclose(arquivo);
    return 1;
}

/**
 * @brief Copia um texto com limite de tamanho, sempre terminando em '\0'.
 * @param destino Buffer de destino.
 * @param origem Texto de origem.
 * @param tamanho Tamanho do buffer de destino.
 */
static void copiarTexto(char *destino, const char *origem, size_t tamanho)
{
    strncpy(destino, origem, tamanho - 1);
    destino[tamanho - 1] = '\0';
}

/**
 * @brief Remove espaços em branco do início e do fim de um campo.
 * @param texto Texto a modificar.
 */
static void apararCampo(char *texto)
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

/**
 * @brief Separa uma linha CSV em campos, respeitando aspas.
 *
 * Dentro de aspas, a vírgula faz parte do texto e "" vira uma aspa.
 *
 * @param linha Linha lida do arquivo.
 * @param campos Matriz que recebe os campos.
 * @param maxCampos Quantidade máxima de campos.
 * @return Quantidade de campos lidos ou -1 se houver campos demais.
 */
static int separarCampos(const char *linha, char campos[][TAM_CAMPO_CSV], int maxCampos)
{
    int quantidade = 0;
    size_t posicao = 0;
    int entreAspas = 0;
    const char *c;

    campos[0][0] = '\0';
    for (c = linha; *c != '\0'; c++) {
        if (entreAspas) {
            if (*c == '"' && c[1] == '"') {
                if (posicao < TAM_CAMPO_CSV - 1) {
                    campos[quantidade][posicao++] = '"';
                }
                c++;
            } else if (*c == '"') {
                entreAspas = 0;
            } else if (posicao < TAM_CAMPO_CSV - 1) {
                campos[quantidade][posicao++] = *c;
            }
        } else if (*c == '"') {
            entreAspas = 1;
        } else if (*c == ',') {
            campos[quantidade][posicao] = '\0';
            quantidade++;
            if (quantidade >= maxCampos) {
                return -1;
            }
            posicao = 0;
        } else if (posicao < TAM_CAMPO_CSV - 1) {
            campos[quantidade][posicao++] = *c;
        }
    }
    campos[quantidade][posicao] = '\0';
    return quantidade + 1;
}

/**
 * @brief Converte os campos de uma linha do CSV em uma Pergunta.
 * @param campos Campos já separados.
 * @param pergunta Pergunta de destino.
 * @return 1 se todos os campos forem válidos, 0 caso contrário.
 */
static int camposParaPergunta(char campos[][TAM_CAMPO_CSV], Pergunta *pergunta)
{
    int i;
    int nivel;
    Categoria categoria;

    memset(pergunta, 0, sizeof(*pergunta));
    if (!interface_converterInteiro(campos[CAMPO_NIVEL], &nivel) || !pergunta_nivelValido(nivel)) {
        return 0;
    }
    if (!pergunta_categoriaPorCodigo(campos[CAMPO_CATEGORIA], &categoria)) {
        return 0;
    }
    pergunta->nivel = (NivelDificuldade)nivel;
    pergunta->categoria = categoria;
    copiarTexto(pergunta->enunciado, campos[CAMPO_ENUNCIADO], TAM_ENUNCIADO);
    for (i = 0; i < NUM_ALTERNATIVAS; i++) {
        copiarTexto(pergunta->alternativas[i], campos[CAMPO_ALTERNATIVA_A + i], TAM_ALTERNATIVA);
    }
    if (strlen(campos[CAMPO_CORRETA]) != 1) {
        return 0;
    }
    pergunta->correta = (char)toupper((unsigned char)campos[CAMPO_CORRETA][0]);
    return pergunta_validar(pergunta);
}

int arquivo_carregarCSV(BancoPerguntas *banco, const char *caminho)
{
    FILE *arquivo;
    char linha[TAM_LINHA_CSV];
    char campos[NUM_CAMPOS_CSV][TAM_CAMPO_CSV];
    char mensagem[128];
    char *inicio;
    int numeroLinha = 0;
    int carregadas = 0;
    int numCampos;
    int nivelTeste;
    int i;
    Pergunta pergunta;

    arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        return -1;
    }
    while (fgets(linha, sizeof(linha), arquivo) != NULL) {
        numeroLinha++;
        linha[strcspn(linha, "\r\n")] = '\0';
        inicio = linha;

        /* Pula o BOM do UTF-8, se o editor tiver gravado. */
        if (numeroLinha == 1 && (unsigned char)inicio[0] == 0xEF &&
            (unsigned char)inicio[1] == 0xBB && (unsigned char)inicio[2] == 0xBF) {
            inicio += 3;
        }
        if (inicio[0] == '\0' || inicio[0] == '#') {
            continue;
        }
        numCampos = separarCampos(inicio, campos, NUM_CAMPOS_CSV);
        for (i = 0; i < numCampos; i++) {
            apararCampo(campos[i]);
        }
        /* Primeira linha sem número no primeiro campo = cabeçalho. */
        if (numeroLinha == 1 && numCampos > 0 && !interface_converterInteiro(campos[0], &nivelTeste)) {
            continue;
        }
        if (numCampos != NUM_CAMPOS_CSV || !camposParaPergunta(campos, &pergunta)) {
            snprintf(mensagem, sizeof(mensagem), "Linha %d do CSV inválida e ignorada.", numeroLinha);
            interface_mostrarAviso(mensagem);
            continue;
        }
        if (banco_inserir(banco, pergunta) == -1) {
            interface_mostrarErro("Memória insuficiente ao carregar o CSV.");
            break;
        }
        carregadas++;
    }
    fclose(arquivo);
    return carregadas;
}

int arquivo_salvarBinario(const BancoPerguntas *banco, const char *caminho)
{
    FILE *arquivo;
    CabecalhoBinario cabecalho;
    char caminhoTemporario[256];
    int ok;

    /* Grava primeiro em um arquivo temporário para não perder os dados
       antigos caso algo dê errado no meio da gravação. */
    snprintf(caminhoTemporario, sizeof(caminhoTemporario), "%s.tmp", caminho);
    arquivo = fopen(caminhoTemporario, "wb");
    if (arquivo == NULL) {
        return 0;
    }
    memcpy(cabecalho.assinatura, ASSINATURA_BINARIO, sizeof(cabecalho.assinatura));
    cabecalho.versao = VERSAO_BINARIO;
    cabecalho.quantidade = banco->quantidade;
    cabecalho.proximoId = banco->proximoId;

    ok = fwrite(&cabecalho, sizeof(cabecalho), 1, arquivo) == 1;
    if (ok && banco->quantidade > 0) {
        ok = fwrite(banco->itens, sizeof(Pergunta), (size_t)banco->quantidade, arquivo)
             == (size_t)banco->quantidade;
    }
    if (fclose(arquivo) != 0) {
        ok = 0;
    }
    if (!ok) {
        remove(caminhoTemporario);
        return 0;
    }
    remove(caminho); /* no Windows, rename falha se o destino existir */
    return rename(caminhoTemporario, caminho) == 0;
}

int arquivo_carregarBinario(BancoPerguntas *banco, const char *caminho)
{
    FILE *arquivo;
    CabecalhoBinario cabecalho;
    Pergunta pergunta;
    int i;
    int k;

    arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        return -1;
    }
    if (fread(&cabecalho, sizeof(cabecalho), 1, arquivo) != 1 ||
        memcmp(cabecalho.assinatura, ASSINATURA_BINARIO, sizeof(cabecalho.assinatura)) != 0 ||
        cabecalho.versao != VERSAO_BINARIO || cabecalho.quantidade < 0) {
        fclose(arquivo);
        return -1;
    }
    for (i = 0; i < cabecalho.quantidade; i++) {
        if (fread(&pergunta, sizeof(pergunta), 1, arquivo) != 1) {
            fclose(arquivo);
            banco_liberar(banco);
            return -1;
        }
        /* Garante que os textos terminam em '\0' mesmo com arquivo corrompido. */
        pergunta.enunciado[TAM_ENUNCIADO - 1] = '\0';
        for (k = 0; k < NUM_ALTERNATIVAS; k++) {
            pergunta.alternativas[k][TAM_ALTERNATIVA - 1] = '\0';
        }
        if (!pergunta_validar(&pergunta) || banco_inserir(banco, pergunta) == -1) {
            fclose(arquivo);
            banco_liberar(banco);
            return -1;
        }
    }
    fclose(arquivo);
    if (cabecalho.proximoId > banco->proximoId) {
        banco->proximoId = cabecalho.proximoId;
    }
    return banco->quantidade;
}

/**
 * @brief Escreve um campo de texto no CSV, entre aspas e com "" para aspas.
 * @param arquivo Arquivo de saída.
 * @param texto Texto do campo.
 */
static void escreverCampoCSV(FILE *arquivo, const char *texto)
{
    fputc('"', arquivo);
    for (; *texto != '\0'; texto++) {
        if (*texto == '"') {
            fputc('"', arquivo);
        }
        fputc(*texto, arquivo);
    }
    fputc('"', arquivo);
}

int arquivo_exportarCSV(const BancoPerguntas *banco, const char *caminho)
{
    FILE *arquivo;
    const Pergunta *pergunta;
    int i;
    int k;

    arquivo = fopen(caminho, "w");
    if (arquivo == NULL) {
        return 0;
    }
    fprintf(arquivo, "nivel,categoria,enunciado,alternativa_a,alternativa_b,alternativa_c,alternativa_d,correta\n");
    for (i = 0; i < banco->quantidade; i++) {
        pergunta = &banco->itens[i];
        fprintf(arquivo, "%d,%s,", (int)pergunta->nivel, pergunta_codigoCategoria(pergunta->categoria));
        escreverCampoCSV(arquivo, pergunta->enunciado);
        for (k = 0; k < NUM_ALTERNATIVAS; k++) {
            fputc(',', arquivo);
            escreverCampoCSV(arquivo, pergunta->alternativas[k]);
        }
        fprintf(arquivo, ",%c\n", pergunta->correta);
    }
    return fclose(arquivo) == 0;
}
