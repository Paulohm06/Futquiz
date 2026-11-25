/**
 * @file jogo.h
 * @brief Telas de terminal da partida e do ranking.
 * @author Equipe Futquiz
 */

#ifndef JOGO_H
#define JOGO_H

#include "banco.h"
#include "ranking.h"

/**
 * @brief Joga uma partida completa no terminal e registra o resultado no ranking.
 * @param banco Banco de perguntas.
 * @param ranking Ranking (é salvo em arquivo ao fim da partida).
 */
void jogo_executarPartida(const BancoPerguntas *banco, Ranking *ranking);

/**
 * @brief Mostra o ranking no terminal.
 * @param ranking Ranking a exibir.
 */
void jogo_mostrarRanking(const Ranking *ranking);

#endif /* JOGO_H */
