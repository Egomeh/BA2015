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

#define TETRIS_MAX_SCORE 1000000.0

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

    class MDPTetrisDetailedResult
    {
    public:
        MDPTetrisDetailedResult(unsigned int minScore, unsigned int maxScore, double mean, double standardDeviation)
            : m_maxScore(maxScore),
              m_minScore(minScore),
              m_mean(mean),
              m_standardDeviation(standardDeviation){};

        /* Get the values */
        double mean(void) const { return m_mean; }
        double standardDeviation(void) const { return m_standardDeviation; }
        unsigned int maxScore(void) const { return m_maxScore; }
        unsigned int minScore(void) const { return m_minScore; }
    private:
        /* Highest and lowest score of the game played */
        unsigned int m_maxScore, m_minScore;

        /* Mean score and standard deviation */
        double m_mean, m_standardDeviation;

    };


    MDPTetris(int board_width, int board_height, int nb_games,
              Game *game, GamesStatistics *stats, std::string featureFile);

    /* Propoase a starting point, so far, just 0,0,...,0 */
    SearchPointType proposeStartingPoint() const;

    /* Number of variables, which is equal to number
     * of dimensions from the feature policy */
    std::size_t numberOfVariables() const;

    /* The function for evaluating a single feature policy */
    ResultType eval(const SearchPointType &input) const;

    /* The function for evaluating a single feature policy */
    MDPTetrisDetailedResult evalDetailed(const SearchPointType &input) const;

    /* Set the game data file */
    void setGamedataFilename(std::string filename)
    { m_gamedataFilename = filename; }


    /* Change the number of games played when evaulating */
    void setNbGames(unsigned int nbGames)
    { m_nbGames = nbGames; }

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

    /* A filename to which game data is written */
    std::string m_gamedataFilename;

    /* Info about last evaluation */
    unsigned int m_lastMinScore, m_lastMaxScore;
    double m_lastStandardDeviation;

};

#endif //EXAMPLEPROJECT_MDPTETRIS_H
