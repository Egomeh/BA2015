//
// Created by jens on 10/22/15.
//

#include "MDPTetris.h"

MDPTetris::MDPTetris(int board_width, int board_height, int nb_games,
                     Game *game, GamesStatistics * stats, std::string feature_file) {

    /* Load the feature policy */
    load_feature_policy(feature_file.c_str(), &m_featurePolicy);
    features_initialize(&m_featurePolicy);

    /* Store the location of the piece file */
    //m_pieceFile = piece_file;

    /* Store board height and width and number of games to play */
    m_boardHeight = board_height;
    m_boardWidth  = board_width;
    m_nbGames     = nb_games;

    /* set the dimension of the search
     * pace to the number of features
     * to optimize for
     */
    m_dimensions = m_featurePolicy.nb_features;

    /* Allow proposal of starting point in search space */
    m_features |= CAN_PROPOSE_STARTING_POINT;

    m_game = game;
    m_stats = stats;

}

shark::blas::vector<double> MDPTetris::proposeStartingPoint() const {

    /* Create the search point  */
    SearchPointType startingPoint(m_dimensions);

    /* Start the search at what was defined
     * in the feature file
     * */
    for (std::size_t i = 0; i != m_dimensions; i++)
    {
        startingPoint(i) = m_featurePolicy.features->weight;
    }

    /* return the point */
    return startingPoint;
}

std::size_t MDPTetris::numberOfVariables() const {

    /* Variables to adjust are just one per feature */
    return m_dimensions;
}

double MDPTetris::eval(const SearchPointType &input) const {

    m_evaluationCounter++;

    //std::cout << "evaluation on: " << input << std::endl;

    /* init a game and statistics object */
    //Game *game = new_game(0, m_boardWidth, m_boardHeight, 0, m_pieceFile.c_str(), NULL);
    //GamesStatistics *stats = games_statistics_new("stat.dat", m_nbGames, NULL);

    FeaturePolicy attemptPolicy = m_featurePolicy;

    for (std::size_t i = 0; i < m_dimensions; i++)
    {
        attemptPolicy.features[i].weight = input(i);
    }

    /* MDPTetris specific */
    features_initialize(&attemptPolicy);

    /* run the game and see the score! */
    double points;
    GamesStatistics *stats = games_statistics_new(NULL, m_nbGames, NULL);

             /* MDPTetris function for playing tetris:
                attemptPolicy: policy to use when playing.
                m_nbGames    : How many games to play.
                m_game       : The game object, holding board dimensions etc.
                stats        : The object to hold game statistics.
              */
    points = feature_policy_play_games(&attemptPolicy, m_nbGames, m_game, stats, 0);

    /* Store the results about the game */
    if (m_gamedataFilename.size() > 0)
    {
        std::ofstream fs;
        fs.open (m_gamedataFilename.c_str(), std::ios::app);

        /* Do not report each individual
        fs << "weights:" << input << ":";
        for (int i = 0; i < m_nbGames; i++)
        {
            fs << stats->scores[i];
            if (i < m_nbGames-1)
            {
                fs << ",";
            };
        }
        fs << std::endl;
        fs.close();
         */
    }


    games_statistics_end_episode(stats, NULL);
    games_statistics_free(stats);

    return TETRIS_MAX_SCORE - points;
}

MDPTetris::MDPTetrisDetailedResult MDPTetris::evalDetailed(const shark::blas::vector<double> &input) const {
    m_evaluationCounter++;

    //std::cout << "evaluation on: " << input << std::endl;

    /* init a game and statistics object */
    //Game *game = new_game(0, m_boardWidth, m_boardHeight, 0, m_pieceFile.c_str(), NULL);
    //GamesStatistics *stats = games_statistics_new("stat.dat", m_nbGames, NULL);

    FeaturePolicy attemptPolicy = m_featurePolicy;
    std::vector<int> policy;
    std::vector<double> weights;

    for (std::size_t i = 0; i < m_dimensions; i++)
    {
        attemptPolicy.features[i].weight = input(i);
        policy.push_back(attemptPolicy.features[i].feature_id);
        weights.push_back(input(i));
    }

    /* MDPTetris specific */
    features_initialize(&attemptPolicy);

    /* run the game and see the score! */
    double points;
    GamesStatistics *stats = games_statistics_new(NULL, m_nbGames, NULL);

    /* MDPTetris function for playing tetris:
       attemptPolicy: policy to use when playing.
       m_nbGames    : How many games to play.
       m_game       : The game object, holding board dimensions etc.
       stats        : The object to hold game statistics.
     */
    points = feature_policy_play_games(&attemptPolicy, m_nbGames, m_game, stats, 0);

    /* Store the results about the game */
    if (m_gamedataFilename.size() > 0)
    {
        std::ofstream fs;
        fs.open (m_gamedataFilename.c_str(), std::ios::app);
    }

    /* Calculate the details of the games played */
    unsigned int minScore, maxScore, sum;
    std::vector<unsigned int> scores;
    double mean, stdDiv;
    sum = 0;
    for (int i = 0; i < m_nbGames; i++)
    {
        unsigned int curScore = stats->scores[i];
        /* For the first score, just set the min and max */
        if (i==0)
            minScore = maxScore = curScore;

        /* Record if higher or lower bound are found */
        if (curScore < minScore)
            minScore = curScore;

        /* Record if higher or lower bound are found */
        if (curScore > maxScore)
            maxScore = curScore;

        /* add score to the sum */
        sum += curScore;

        scores.push_back(curScore);
    }

    /* Sort the scores, such that median is easy to find */
    std::sort(scores.begin(), scores.end());

    /* calc the mean */
    mean = double(sum) / m_nbGames;

    /* Calc standard deviation */
    double varianceTemp = 0.0;
    for (int i = 0; i < m_nbGames; i++)
    {
        unsigned int curScore = stats->scores[i];

        /* Distance to the mean */
        double d = (double)curScore - points;
        varianceTemp += d*d;
    }
    varianceTemp = varianceTemp / (double) (m_nbGames-1);
    double stdDeviation = std::sqrt(varianceTemp);

    MDPTetrisDetailedResult result(minScore, maxScore, points, stdDeviation, scores, policy, weights);


    games_statistics_end_episode(stats, NULL);
    games_statistics_free(stats);

    return result;

}
