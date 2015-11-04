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

    features_initialize(&attemptPolicy);

    /* run the game and see the score! */
    double points;
    GamesStatistics *stats = games_statistics_new(NULL, m_nbGames, NULL);
    points = feature_policy_play_games(&attemptPolicy, m_nbGames, m_game, stats, 0);

    /* This was for debugging, remove later or use fgor ref.
    std::cout << "weight set: " << input << " " << stats->best_mean << std::endl;
    std::cout << "scores ";
    for (int i = 0; i < m_nbGames; i++)
    {
        std::cout << stats->scores[i] << ",";
    }
    std::cout << std::endl;
     */


    games_statistics_end_episode(stats, NULL);
    games_statistics_free(stats);

    return 10000000.0 - points;
}
