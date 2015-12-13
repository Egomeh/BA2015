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
#include <shark/ObjectiveFunctions/AbstractObjectiveFunction.h>
#include <iostream>
#include <string>
#include <ctime>

#include "cconfig.h"
#include "MDPTetris.h"
#include "CrossEntropy.h"


#include <shark/Statistics/Distributions/MultiVariateNormalDistribution.h>




int main()
{
    std::string piecesFile = MDPTETRIS_DATA_PATH("pieces4.dat");
    std::string start_policy = MDPTETRIS_DATA_PATH("starting_policy01.dat");

    int seed = (int) time(0);
    initialize_random_generator( seed );
    shark::Rng::seed( seed );

    int nbGames = 1;

    Game *game = new_game(0, 10, 20, 0, piecesFile.c_str(), NULL);
    GamesStatistics *stats = games_statistics_new(NULL, nbGames, NULL);

    MDPTetris objFun(10,20, nbGames, game, stats, start_policy );

    shark::RealVector test_vector(objFun.numberOfVariables());

    double arr[] = {-4.49139,-6.73538,-10.8526,-9.02891,-10.4329,-11.5992,
                    -8.19691,-5.73886,-13.5224,-10.209,-3.90456,-6.10412,
                    -6.16699,-6.72412,-7.56298,-12.3828,-7.39287,-7.43948,
                    -8.03732,-5.00829,-6.39778,-26.0587};
    for (int i = 0; i < 22; i++)
    {
        test_vector(i) = arr[i];
    }
    for(double i = 0.1; i < 10.0; i += 0.1)
    {
        shark::RealVector scaled(test_vector.size());
        for (int j = 0; j < test_vector.size(); j++)
        {
            scaled(j) = test_vector(j) * i;
        }
        initialize_random_generator( seed );
        std::cout << i << "," << TETRIS_MAX_SCORE - objFun.eval(scaled) << std::endl;
    }

    std::cout << test_vector << std::endl;

}





