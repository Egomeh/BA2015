//
// Created by jens on 10/22/15.
//

#ifndef EXAMPLEPROJECT_MDPTETRIS_H
#define EXAMPLEPROJECT_MDPTETRIS_H

#include <iostream>
#include <typeinfo>
#include <string>

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
              std::string feature_file, std::string piece_file);

private:

    /* The struct from the mdptetris
     * library that contains features of attention
     */
    FeaturePolicy m_featurePolicy;

    /* The struct that holds info about the
     * tetris game
     */
    Game *tetris_game;

    //! Degrees of freedom
    std::size_t m_dimensions;

};

#endif //EXAMPLEPROJECT_MDPTETRIS_H
