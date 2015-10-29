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

#define OPT_SEED               "-seed"
#define OPT_START_POL_FILE     "-startPolicy"
#define OPT_PIECE_FILE         "-pieceFile"
#define OPT_OPTIMIZER          "-optimizer"

void useCMA(std::string startPolicyFile,
            std::string piecesFile,
            std::string statsFile,
            unsigned int nbGames,
            unsigned int boardWidth,
            unsigned int boardHeight,
            int randomSeed,
            double initialSigma,
            unsigned int maxIterations,
            std::ostream & out)
{
    out << "Running CMA-ES with following configurations" << std::endl;
    out << "Start policy       : " << startPolicyFile << std::endl;
    out << "Pieces             : " << piecesFile << std::endl;
    out << "Stats file         : " << statsFile << std::endl;
    out << "Game evaluatioons  : " << nbGames << std::endl;
    out << "Game board with    : " << boardWidth << std::endl;
    out << "Game board height  : " << boardHeight << std::endl;
    out << "Random seed        : " << randomSeed << std::endl;
    out << "initialSigma       : " << initialSigma << std::endl;
    out << "MaxIterations      : " << maxIterations << std::endl;

    initialize_random_generator( randomSeed );
    shark::Rng::seed( randomSeed );
    shark::CMA cma;

    Game *game = new_game(0, 10, 20, 0, piecesFile.c_str(), NULL);
    GamesStatistics *stats = games_statistics_new(statsFile.c_str(), 10, NULL);

    MDPTetris objFun(10,20, 10, game, stats, startPolicyFile);

    cma.init(objFun);

    cma.setSigma(initialSigma);


    int t = 1;
    double bestScore = 0.0;

    while (cma.solution().value > 0.0 && t < maxIterations)
    {
        cma.step(objFun);
        t += cma.lambda();
        if (10000000.0 - cma.solution().value > bestScore)
        {
            bestScore = 10000000.0 - cma.solution().value;
            _DUMP(bestScore);
            _DUMP(t);
        }
    }


}


void useCE(std::string startPolicyFile,
           std::string piecesFile,
           std::string statsFile,
           unsigned int nbGames,
           unsigned int boardWidth,
           unsigned int boardHeight,
           int randomSeed,
           double initialSigma,
           unsigned int maxIterations,
           std::ostream & out)
{
    out << "Running Cross Entropy with following configurations" << std::endl;
    out << "Start policy       : " << startPolicyFile << std::endl;
    out << "Pieces             : " << piecesFile << std::endl;
    out << "Stats file         : " << statsFile << std::endl;
    out << "Game evaluatioons  : " << nbGames << std::endl;
    out << "Game board with    : " << boardWidth << std::endl;
    out << "Game board height  : " << boardHeight << std::endl;
    out << "Random seed        : " << randomSeed << std::endl;
    out << "initialSigma       : " << initialSigma << std::endl;
    out << "MaxIterations      : " << maxIterations << std::endl;

    initialize_random_generator( randomSeed );
    shark::Rng::seed( randomSeed );
    shark::CrossEntropy ce;

    Game *game = new_game(0, 10, 20, 0, piecesFile.c_str(), NULL);
    GamesStatistics *stats = games_statistics_new(statsFile.c_str(), 10, NULL);

    MDPTetris objFun(10,20, 10, game, stats, startPolicyFile);

    ce.init(objFun);

    // Still need to set the initial sigma vector


    int t = 1;
    double bestScore = 0.0;

    while (ce.solution().value > 0.0 && t < maxIterations)
    {
        ce.step(objFun);
        t += ce.lambda();
        if (10000000.0 - ce.solution().value > bestScore)
        {
            bestScore = 10000000.0 - ce.solution().value;
            _DUMP(bestScore);
            _DUMP(t);
        }
    }


}

int main( int argc, char ** argv ) 
{

    std::map<std::string, std::string> options;
    for (int i = 1; i < argc; i++)
    {
        std::string opt       = argv[i];
        std::string delimiter = "=";

        size_t del_pos =  opt.find(delimiter);
        if (del_pos == std::string::npos || del_pos+1 == opt.size())
        {
            std::cerr << "Ill posed command line argument: " << argv[i] << std::endl;
            return 64;
        }

        std::string key       = opt.substr(0, opt.find(delimiter));
        std::string value     = opt.substr(opt.find(delimiter)+1, opt.size());
        options[key] = value;
        std::cout << "Registering option " << key << " : " << value << std::endl;
    }


    /* Set the random seed for the gsl random generator */
    int seed;
    if (options.count(OPT_SEED) == 1)
    {
       seed = atoi ( options[OPT_SEED].c_str() );
    }
    else
    {
        seed = (int) time(0);
    }


    /* Set the initial policy file */
    std::string start_policy;
    if (options.count(OPT_START_POL_FILE) == 1)
    {
        start_policy = options[OPT_START_POL_FILE];
    }
    else
    {
        start_policy = MDPTETRIS_DATA_PATH("starting_policy00.dat");
    }


    /* Set the initial policy file */
    std::string piece_file;
    if (options.count(OPT_PIECE_FILE) == 1)
    {
        piece_file = options[OPT_PIECE_FILE];
    }
    else
    {
        piece_file = MDPTETRIS_DATA_PATH("STpieces4.dat");
    }

    std::string statFile = "stat.dat";

    unsigned int nbGames = 10;
    unsigned int boardWidth = 10;
    unsigned int boardHeight = 20;
    double initialSigma = 1.0;
    unsigned int maxIterations = 5000;

    if ( options.count(OPT_OPTIMIZER) == 1 )
    {
        if ( options[OPT_OPTIMIZER].compare("cma") == 0 )
        {
            useCMA(
                    start_policy,
                    piece_file,
                    statFile,
                    nbGames,
                    boardWidth,
                    boardHeight,
                    seed,
                    initialSigma,
                    maxIterations,
                    std::cout
            );
        }
        else if ( options[OPT_OPTIMIZER].compare("ce") == 0 )
        {
            useCE(
                    start_policy,
                    piece_file,
                    statFile,
                    nbGames,
                    boardWidth,
                    boardHeight,
                    seed,
                    initialSigma,
                    maxIterations,
                    std::cout
            );
        }
    }
    else
    {
        std::cerr << "Optimizer not recognized!" << std::endl;
        return 64;
    }

}





