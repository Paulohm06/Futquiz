/**
 * @file pergunta.h
 * @brief Estrutura Pergunta, níveis de dificuldade, categorias e funções
 *        para validar e exibir uma pergunta.
 * @author Equipe Futquiz
 */

#ifndef PERGUNTA_H
#define PERGUNTA_H

/** Tamanho máximo do enunciado (incluindo o '\0'). */
#define TAM_ENUNCIADO 256
/** Tamanho máximo de cada alternativa (incluindo o '\0'). */
#define TAM_ALTERNATIVA 128
/** Quantidade de alternativas por pergunta. */
#define NUM_ALTERNATIVAS 4
/** Quantidade de níveis de dificuldade. */
#define NUM_NIVEIS 5
/** Quantidade de categorias. */
#define NUM_CATEGORIAS 4

/**
 * @brief Níveis de dificuldade do jogo (1 a 5).
 */
typedef enum {
    NIVEL_MUITO_FACIL = 1,  /**< Perguntas 1 e 2 da partida.   */
    NIVEL_FACIL,            /**< Perguntas 3 e 4 da partida.   */
    NIVEL_MEDIO,            /**< Perguntas 5 a 8 da partida.   */
    NIVEL_DIFICIL,          /**< Perguntas 9 a 12 da partida.  */
    NIVEL_MUITO_DIFICIL     /**< Perguntas 13 a 15 da partida. */
} NivelDificuldade;

/**
 * @brief Categorias temáticas das perguntas de futebol.
 */
typedef enum {
    CATEGORIA_BRASILEIRO = 1,  /**< Futebol brasileiro.    */
    CATEGORIA_INTERNACIONAL,   /**< Futebol internacional. */
    CATEGORIA_COPA,            /**< Copa do Mundo.         */
    CATEGORIA_LIBERTADORES     /**< Copa Libertadores.     */
} Categoria;

/**
 * @brief Uma pergunta do jogo.
 */
typedef struct {
    int id;                                                /**< Identificador único.            */
    char enunciado[TAM_ENUNCIADO];                         /**< Texto da pergunta.              */
    char alternativas[NUM_ALTERNATIVAS][TAM_ALTERNATIVA];  /**< Alternativas A, B, C e D.       */
    char correta;                                          /**< Letra da alternativa correta.   */
    NivelDificuldade nivel;                                /**< Nível de dificuldade (1 a 5).   */
    Categoria categoria;                                   /**< Categoria temática.             */
} Pergunta;

/**
 * @brief Devolve o nome de um nível de dificuldade.
 * @param nivel Nível desejado.
 * @return Nome do nível ou "Desconhecido".
 */
const char *pergunta_nomeNivel(NivelDificuldade nivel);

/**
 * @brief Devolve a cor ANSI usada para destacar um nível.
 * @param nivel Nível desejado.
 * @return Código de cor ANSI.
 */
const char *pergunta_corNivel(NivelDificuldade nivel);

/**
 * @brief Devolve o nome de exibição de uma categoria.
 * @param categoria Categoria desejada.
 * @return Nome da categoria ou "Desconhecida".
 */
const char *pergunta_nomeCategoria(Categoria categoria);

/**
 * @brief Devolve o código usado no arquivo CSV para uma categoria.
 * @param categoria Categoria desejada.
 * @return Código (ex.: "COPA") ou "?".
 */
const char *pergunta_codigoCategoria(Categoria categoria);

/**
 * @brief Converte um código do CSV (ou número de 1 a 4) em categoria.
 * @param codigo Texto lido do arquivo.
 * @param categoria Endereço onde o resultado será guardado.
 * @return 1 se o código é válido, 0 caso contrário.
 */
int pergunta_categoriaPorCodigo(const char *codigo, Categoria *categoria);

/**
 * @brief Verifica se um número corresponde a um nível válido.
 * @param valor Número a verificar.
 * @return 1 se válido, 0 caso contrário.
 */
int pergunta_nivelValido(int valor);

/**
 * @brief Verifica se um número corresponde a uma categoria válida.
 * @param valor Número a verificar.
 * @return 1 se válida, 0 caso contrário.
 */
int pergunta_categoriaValida(int valor);

/**
 * @brief Quantidade mínima de perguntas por nível exigida no cadastro inicial.
 * @param nivel Nível desejado.
 * @return Quantidade mínima (20, 20, 15, 15, 10).
 */
int pergunta_minimoCadastro(NivelDificuldade nivel);

/**
 * @brief Quantidade de perguntas de um nível usadas em uma partida.
 * @param nivel Nível desejado.
 * @return Quantidade usada na partida (2, 2, 4, 4, 3).
 */
int pergunta_quantidadeNaPartida(NivelDificuldade nivel);

/**
 * @brief Verifica se todos os campos de uma pergunta são válidos.
 * @param pergunta Pergunta a validar.
 * @return 1 se válida, 0 caso contrário.
 */
int pergunta_validar(const Pergunta *pergunta);

/**
 * @brief Exibe a pergunta completa, com cores.
 * @param pergunta Pergunta a exibir.
 * @param mostrarResposta Se 1, destaca a alternativa correta.
 */
void pergunta_exibir(const Pergunta *pergunta, int mostrarResposta);

/**
 * @brief Exibe a pergunta em uma linha (para listagens em tabela).
 * @param pergunta Pergunta a exibir.
 */
void pergunta_exibirResumo(const Pergunta *pergunta);

/**
 * @brief Exibe o cabeçalho da tabela usada por pergunta_exibirResumo().
 */
void pergunta_exibirCabecalhoResumo(void);

#endif /* PERGUNTA_H */
