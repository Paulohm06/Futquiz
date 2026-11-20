/**
 * @file menu.h
 * @brief Menus do sistema: menu principal, cadastro de perguntas,
 *        pesquisa, navegação por nível, exportação e estatísticas.
 * @author Equipe Futquiz
 */

#ifndef MENU_H
#define MENU_H

#include "banco.h"

/**
 * @brief Executa o menu principal até o usuário escolher sair.
 * @param banco Banco de perguntas carregado.
 */
void menu_executar(BancoPerguntas *banco);

#endif /* MENU_H */
