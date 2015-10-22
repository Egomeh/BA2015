//
// Created by jens on 10/22/15.
//

#include "MDPTetris.h"

MDPTetris::MDPTetris(int board_width, int board_height, int nb_games,
                     std::string feature_file, std::string piece_file) {

    /* Load the feature policy */
    load_feature_policy(feature_file.c_str(), &m_featurePolicy);
    features_initialize(&m_featurePolicy);

    /* set the dimension of the search
     * pace to the number of features
     * to optimize for
     */
    m_dimensions = m_featurePolicy.nb_features;

}
