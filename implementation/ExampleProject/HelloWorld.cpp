//===========================================================================
/*!
 * 
 *
 * \brief       Example program for hierarchical clustering.
 * 
 * 
 *
 * \author      T. Glasmachers
 * \date        2011
 *
 *
 * \par Copyright 1995-2015 Shark Development Team
 * 
 * <BR><HR>
 * This file is part of Shark.
 * <http://image.diku.dk/shark/>
 * 
 * Shark is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Shark is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with Shark.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
//===========================================================================

#include <shark/Core/Shark.h>
#include <shark/Algorithms/DirectSearch/CMA.h>
#include <iostream>
#include <string>
#include <ctime>

#include "MDPTetris.h"

extern "C"
{
#include "feature_functions.h"
#include "feature_policy.h"
#include "games_statistics.h"
#include "game.h"
#include "cmaes_interface.h"
#include "common_parameters.h"
#include "random.h"
#include "cconfig.h"
}

int main( int argc, char ** argv ) 
{


    /* init this at first, just to make sure! */
    /* Initialize the gls random generator (42 is arbitrary) */
    int seed;
    seed = (int) time(0);
    initialize_random_generator( seed );

    /* File path for the starting policy
     * NOTE: this can be done much nicer with
     * boost!
     * */
    std::string start_policy = std::string(MDPTETRIS_DATA_FOLDER)
                                + std::string("/starting_policy00.dat");

    /* File path for the pieces data file, see note above. */
    std::string piece_file = std::string(MDPTETRIS_DATA_FOLDER)
                               + std::string("/pieces4.dat");

    shark::CMA cma;

    MDPTetris objFun(10,20, 10, start_policy, piece_file);

    cma.init(objFun);
    cma.setSigma(0.1);


    int t = 1;
    double bestScore = 0.0;


    while (cma.solution().value > 0.0 && t < 1000)
    {
        cma.step(objFun);
        if (10000000.0 - cma.solution().value > bestScore)
        {
            bestScore = 10000000.0 - cma.solution().value;
            std::cout << "Best score: " << bestScore << std::endl;
        }
    }

    return 0;

    std::cout << MDPTETRIS_DATA_FOLDER << std::endl;

    shark::Shark::info( std::cout );

    /* Board dimensions */
    int board_width;
    board_width = 10;
    int board_height;
    board_height = 20;

    /* number of games to play */
    int nb_games;
    nb_games = 1;

    /* The filename of the feature file */
    std::string feature_file_name;
    feature_file_name = std::string(MDPTETRIS_DATA_FOLDER) +
                                        std::string("/features/bertsekas_initial.dat");

    /* filename to load the piece information */
    std::string piece_file_str;
    piece_file_str = std::string(MDPTETRIS_DATA_FOLDER) +
                                       std::string("/pieces4.dat");

    /* The filename of the statistics file */
    std::string statistics_file_name;
    statistics_file_name = "stat.dat";

    /* The struct that holds the policy */
    FeaturePolicy feature_policy;

    /* The game object/struct */
    Game            *game;

    /* The struct holding stats about the game */
    GamesStatistics *stats;

    /* Load in the sample feature policy from pre-made file */
    load_feature_policy(feature_file_name.c_str(), &feature_policy);
    features_initialize(&feature_policy);

    /* Init the game and game statistics */
    game = new_game(0, board_width, board_height, 0, piece_file_str.c_str(), NULL);
    stats = games_statistics_new(statistics_file_name.c_str(), nb_games, NULL);


    /* run the game and see the score! */
    double points;
    points = feature_policy_play_games(&feature_policy, nb_games, game, stats, 0);

    /* Print the resulting score */
    std::cout << "seed: " << seed << ", " << "points: " << points << std::endl;
}





