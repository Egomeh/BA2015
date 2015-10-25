//
// Created by jens on 10/22/15.
//

#ifndef EXAMPLEPROJECT_MDPTETRIS_H
#define EXAMPLEPROJECT_MDPTETRIS_H

#include <iostream>
#include <typeinfo>
#include <string>
#include <limits>

#include <shark/ObjectiveFunctions/AbstractObjectiveFunction.h>


extern "C"{
#include "feature_functions.h"
#include "feature_policy.h"
#include "games_statistics.h"
#include "game.h"
#include "cmaes_interface.h"
#include "common_parameters.h"
#include "random.h"
};

/*
 * Class for playing Tetris and evaluating
 * agent performance.
 */
class MDPTetris : public shark::SingleObjectiveFunction {

public:
    MDPTetris(int board_width, int board_height, int nb_games,
              Game *game, GamesStatistics *stats, std::string featureFile);

    /* Propoase a starting point, so far, just 0,0,...,0 */
    SearchPointType proposeStartingPoint() const;

    /* Number of variables, which is equal to number
     * of dimensions from the feature policy */
    std::size_t numberOfVariables() const;

    /* The function for evaluating a single feature policy */
    ResultType eval(const SearchPointType &input) const;

private:

    /* The struct from the mdptetris
     * library that contains features of attention
     */
    FeaturePolicy m_featurePolicy;

    //! Degrees of freedom
    std::size_t m_dimensions;

    /* The file that holds the pieces' information */
    std::string m_pieceFile;

    int m_boardWidth, m_boardHeight, m_nbGames;

    /* Game object to use for evaluation */
    Game *m_game;

    /* Statistics object to store game stats */
    GamesStatistics *m_stats;

};

#endif //EXAMPLEPROJECT_MDPTETRIS_H
