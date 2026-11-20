# Futquiz

Jogo do Milhão com tema de futebol, em C, para Fundamentos de Programação 2 (UTFPR Apucarana, 2026/2).

## Como compilar e executar

```bash
make        # compila
make run    # compila e executa
make clean  # apaga objetos e executável
make reset  # apaga dados/perguntas.bin (a próxima execução relê o CSV)
make docs   # gera a documentação Doxygen em docs/
```

Execute sempre a partir da pasta do projeto (os caminhos dos dados são relativos).
No Windows use MinGW (`mingw32-make`).

## Organização

| Arquivo | Responsabilidade |
|---|---|
| `src/main.c` | Carrega os dados, roda o menu e salva ao sair |
| `src/pergunta.c/.h` | Estrutura `Pergunta`, níveis, categorias, validação e exibição |
| `src/banco.c/.h` | Vetor dinâmico de perguntas (inserir, buscar, alterar, excluir, filtrar) |
| `src/arquivo.c/.h` | CSV inicial, arquivo binário e exportação CSV |
| `src/interface.c/.h` | Cores ANSI, beeps, leitura segura e mensagens |
| `src/menu.c/.h` | Menus e telas do sistema |
| `dados/perguntas.csv` | 80 perguntas iniciais (20/20/15/15/10) |

## Dados

- **Primeira execução:** lê `dados/perguntas.csv`.
- **Ao sair:** salva tudo em `dados/perguntas.bin`.
- **Demais execuções:** lê e salva apenas o binário.
- **Exportar:** gera um CSV no mesmo formato do inicial (pode ser relido pelo sistema).

Formato do CSV:

```
nivel,categoria,enunciado,alternativa_a,alternativa_b,alternativa_c,alternativa_d,correta
```

- `nivel`: 1 (muito fácil) a 5 (muito difícil)
- `categoria`: `BRASILEIRO`, `INTERNACIONAL`, `COPA` ou `LIBERTADORES`
- Textos com vírgula devem ficar entre aspas. Linhas iniciadas por `#` são comentários.

## Próximas etapas (parte criativa)

- Partida de 15 rodadas seguindo a progressão de níveis (2, 2, 4, 4, 3)
- Ajudas: VAR (elimina duas alternativas erradas) e Ajuda da Torcida (porcentagens)
- Ranking dos jogadores
- Sons e mensagens comemorativas
- Interface gráfica com raylib
