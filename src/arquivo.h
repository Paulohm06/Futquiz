/**
 * @file arquivo.h
 * @brief Leitura e gravação do banco de perguntas em arquivos
 *        (CSV inicial, binário de trabalho e exportação CSV).
 * @author Equipe Futquiz
 */

#ifndef ARQUIVO_H
#define ARQUIVO_H

#include "banco.h"

/** CSV com as perguntas iniciais (lido apenas na primeira execução). */
#define CAMINHO_CSV_INICIAL "dados/perguntas.csv"
/** Arquivo binário lido e salvo nas demais execuções. */
#define CAMINHO_BINARIO "dados/perguntas.bin"
/** Nome padrão do arquivo de exportação. */
#define CAMINHO_EXPORTACAO "dados/exportacao.csv"

/**
 * @brief Verifica se um arquivo existe e pode ser aberto para leitura.
 * @param caminho Caminho do arquivo.
 * @return 1 se existe, 0 caso contrário.
 */
int arquivo_existe(const char *caminho);

/**
 * @brief Carrega perguntas de um arquivo CSV.
 *
 * Formato de cada linha (a primeira linha pode ser o cabeçalho):
 * nivel,categoria,enunciado,alternativa_a,alternativa_b,alternativa_c,alternativa_d,correta
 *
 * Campos com vírgula devem estar entre aspas. Linhas vazias ou iniciadas
 * por '#' são ignoradas. Linhas inválidas geram aviso e são puladas.
 *
 * @param banco Banco de destino.
 * @param caminho Caminho do CSV.
 * @return Quantidade de perguntas carregadas ou -1 se não abrir o arquivo.
 */
int arquivo_carregarCSV(BancoPerguntas *banco, const char *caminho);

/**
 * @brief Salva todo o banco em um arquivo binário.
 * @param banco Banco a salvar.
 * @param caminho Caminho do arquivo binário.
 * @return 1 se salvou, 0 em caso de erro.
 */
int arquivo_salvarBinario(const BancoPerguntas *banco, const char *caminho);

/**
 * @brief Carrega o banco a partir de um arquivo binário.
 * @param banco Banco de destino (deve estar vazio).
 * @param caminho Caminho do arquivo binário.
 * @return Quantidade de perguntas carregadas ou -1 se o arquivo for inválido.
 */
int arquivo_carregarBinario(BancoPerguntas *banco, const char *caminho);

/**
 * @brief Exporta o banco para CSV no mesmo formato do arquivo inicial.
 * @param banco Banco a exportar.
 * @param caminho Caminho do arquivo CSV de saída.
 * @return 1 se exportou, 0 em caso de erro.
 */
int arquivo_exportarCSV(const BancoPerguntas *banco, const char *caminho);

#endif /* ARQUIVO_H */
