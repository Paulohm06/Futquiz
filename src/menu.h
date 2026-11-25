/**
 * @file menu.h
 * @brief Menus do sistema: menu principal, cadastro de perguntas,
 *        pesquisa, navegação por nível, partida, ranking, exportação e estatísticas.
 * @author Equipe Futquiz
 */

#ifndef MENU_H
#define MENU_H

#include "banco.h"
#include "ranking.h"

/**
 * @brief Executa o menu principal até o usuário escolher sair.
 * @param banco Banco de perguntas carregado.
 * @param ranking Ranking dos jogadores.
 */
void menu_executar(BancoPerguntas *banco, Ranking *ranking);

#endif /* MENU_H */
