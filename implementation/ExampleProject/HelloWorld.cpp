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
    std::string start_policy = MDPTETRIS_DATA_PATH("starting_policy00.dat");

    /* File path for the pieces data file, see note above. */
    std::string piece_file = MDPTETRIS_DATA_PATH("STpieces4.dat");

    Game *game = new_game(0, 10, 20, 0, piece_file.c_str(), NULL);
    GamesStatistics *stats = games_statistics_new("stat.dat", 10, NULL);


    shark::CMA cma;

    MDPTetris objFun(10,20, 10, game, stats, start_policy);

    cma.init(objFun);

    cma.setSigma(10);


    int t = 1;
    double bestScore = 0.0;
    
    while (cma.solution().value > 0.0 && t < 50000)
    {
        cma.step(objFun);
        t += cma.lambda();
        if (10000000.0 - cma.solution().value > bestScore)
        {
            bestScore = 10000000.0 - cma.solution().value;
            std::cout << "Best score: " << bestScore << std::endl;
        }
    }

}





