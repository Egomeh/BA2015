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
        MDPTetrisDetailedResult(
                unsigned int minScore,
                unsigned int maxScore,
                double mean,
                double standardDeviation,
                std::vector<unsigned int> scores,
                std::vector<int> policy,
                std::vector<double> weights)
            : m_maxScore(maxScore),
              m_minScore(minScore),
              m_mean(mean),
              m_standardDeviation(standardDeviation),
              m_scores(scores),
              m_policy(policy),
              m_weights(weights){};

        /* Get the values */
        double mean(void) const { return m_mean; }
        double standardDeviation(void) const { return m_standardDeviation; }
        unsigned int maxScore(void) const { return m_maxScore; }
        unsigned int minScore(void) const { return m_minScore; }
        std::vector<unsigned int> scores(void) const {return m_scores; }
        std::vector<int> policy(void) const { return m_policy; }
        std::vector<double> weights(void) const { return m_weights; }

        /* Pretty print scores */
        std::string printScores(std::string sep)
        {
            std::stringstream s;
            for (int i = 0; i < m_scores.size()-1; i++)
            {
                s << m_scores[i] << sep;
            }
            s << m_scores[m_scores.size()-1];
            return s.str();
        }

        /* Pretty print feature ID's */
        std::string printPolicy(std::string sep)
        {
            std::stringstream s;
            for (int i = 0; i < m_policy.size()-1; i++)
            {
                s << m_policy[i] << sep;
            }
            s << m_policy[m_policy.size()-1];
            return s.str();
        }

        /* Pretty print weights */
        std::string printWeights(std::string sep)
        {
            std::stringstream s;
            for (int i = 0; i < m_weights.size()-1; i++)
            {
                s << m_weights[i] << sep;
            }
            s << m_weights[m_weights.size()-1];
            return s.str();
        }




    private:
        /* Highest and lowest score of the game played */
        unsigned int m_maxScore, m_minScore;

        /* Mean score and standard deviation */
        double m_mean, m_standardDeviation;

        /* Score data */
        std::vector<unsigned int> m_scores;

        /* Policy used */
        std::vector<int> m_policy;

        /* Weights of the policy */
        std::vector<double> m_weights;
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

    /* Enable or disable length penalizing */
    void enableLengthPenalty(bool _enable)
    { m_penalizeLength = _enable; }

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

    /* Penalize length of a vector, to 
     * prevent searches stratching far 
     * from the origin.
     */
    bool m_penalizeLength = false;

};

#endif //EXAMPLEPROJECT_MDPTETRIS_H
