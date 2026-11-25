# Futquiz

Jogo do Milhão com tema de futebol, em C, para Fundamentos de Programação 2 (UTFPR Apucarana, 2026/2).

O projeto tem duas versões que usam o mesmo núcleo e os mesmos arquivos de dados:

- **Terminal** (`futquiz`): cadastro completo das perguntas, navegação por nível, partida, ranking e exportação.
- **Gráfica** (`futquiz_gui`, raylib): partida com animações e sons, ranking e navegação por nível.

## Como compilar e executar

```bash
make            # compila a versão de terminal e a gráfica (a gráfica precisa da raylib)
make run        # compila e executa; o menu tem a opção "Abrir versão gráfica"
make gui        # compila só a versão gráfica
make run-gui    # compila e executa direto a versão gráfica
make clean      # apaga objetos e executáveis
make reset      # apaga dados/perguntas.bin e dados/ranking.bin
make docs       # gera a documentação Doxygen em docs/
```

Execute sempre a partir da pasta do projeto (os caminhos de dados e fontes são relativos).

### raylib

- **Linux:** instale a raylib 5.5 (ou compile do GitHub) e rode `make gui`.
  Se ela não estiver instalada no sistema: `make gui RAYLIB_PATH=/caminho/da/raylib`.
- **Windows:** use o instalador oficial da raylib (que traz o w64devkit) e rode, no terminal do w64devkit:
  `make gui RAYLIB_PATH=C:/raylib/raylib`

## Organização

| Pasta / arquivo | Responsabilidade |
|---|---|
| `src/pergunta.c/.h` | Estrutura `Pergunta`, níveis, categorias, validação e exibição |
| `src/banco.c/.h` | Vetor dinâmico de perguntas (inserir, buscar, alterar, excluir, filtrar) |
| `src/arquivo.c/.h` | CSV inicial, arquivo binário e exportação CSV |
| `src/partida.c/.h` | Regras da partida: sorteio, prêmios, marcos e ajudas (sem desenho) |
| `src/ranking.c/.h` | Top 10 dos jogadores em arquivo binário |
| `src/interface.c/.h` | Terminal: cores ANSI, beeps, leitura segura e mensagens |
| `src/menu.c/.h`, `src/jogo.c/.h`, `src/main.c` | Telas e entrada da versão de terminal |
| `gui/` | Versão gráfica: recursos (fontes e sons), desenho, telas e partida |
| `assets/fontes/` | Fontes Anton e Fira Sans (licença OFL, arquivos incluídos) |
| `dados/perguntas.csv` | 80 perguntas iniciais (20/20/15/15/10) |

## Regras da partida

Baseadas no Show do Milhão do SBT, com a estrutura de 15 perguntas pedida no enunciado:

| Rodadas | Nível | Prêmios |
|---|---|---|
| 1 e 2 | Muito fácil | R$ 1.000 e R$ 2.000 |
| 3 e 4 | Fácil | R$ 3.000 e R$ 4.000 |
| 5 a 8 | Médio | R$ 5.000 (**1º marco**) a R$ 30.000 |
| 9 a 12 | Difícil | R$ 40.000, R$ 50.000 (**2º marco**), R$ 100.000 e R$ 200.000 |
| 13 a 15 | Muito difícil | R$ 300.000, R$ 500.000 e R$ 1.000.000 |

- **Acertar:** avança para o próximo prêmio.
- **Parar:** leva o que já ganhou.
- **Errar:** cai para o último marco de segurança alcançado.
- **Pergunta do milhão:** não vale ajuda e errar zera o prêmio (como no SBT).

### Ajudas

- **VAR** (1 vez): anula duas alternativas erradas.
- **Ajuda da Torcida** (1 vez): mostra a porcentagem de votos do público. Quanto mais difícil a pergunta, menos a torcida acerta.
- **Substituição** (3 vezes): troca a pergunta por outra do mesmo nível (os "pulos" do SBT).

### Outras funcionalidades

- Categoria preferida (Brasileiro, Internacional, Copa do Mundo, Libertadores). Se faltar pergunta da categoria em algum nível, o sorteio completa com outras.
- Ranking dos 10 jogadores que mais avançaram (`dados/ranking.bin`), salvo ao fim de cada partida.
- Mensagens comemorativas ("GOOOOL!", "Na trave!"...). No terminal tocam beeps. Na versão gráfica, os sons são gerados por código: apito, acerto, buzina de erro, torcida e fanfarra de campeão.

### Controles da versão gráfica

- **Mouse:** tudo é clicável.
- **Teclado:** A a D escolhem a alternativa, ENTER confirma, 1 = VAR, 2 = Torcida, 3 = Substituição, 0 ou ESC = Parar.
- **Na navegação por nível:** 1 a 5 trocam o nível, as setas navegam e R mostra a resposta.

## Dados

- **Primeira execução:** lê `dados/perguntas.csv`.
- **Ao sair:** salva tudo em `dados/perguntas.bin`.
- **Demais execuções:** lê e salva apenas o binário.
- **Exportar:** gera um CSV no mesmo formato do inicial.

Formato do CSV:

```
nivel,categoria,enunciado,alternativa_a,alternativa_b,alternativa_c,alternativa_d,correta
```

- `nivel`: 1 (muito fácil) a 5 (muito difícil)
- `categoria`: `BRASILEIRO`, `INTERNACIONAL`, `COPA` ou `LIBERTADORES`
- Textos com vírgula devem ficar entre aspas. Linhas iniciadas por `#` são comentários.
