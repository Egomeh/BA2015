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
#include <iostream>
#include <string>
#include <ctime>

extern "C"
{
#include "feature_functions.h"
#include "feature_policy.h"
#include "games_statistics.h"
#include "game.h"
#include "cmaes_interface.h"
#include "common_parameters.h"
#include "random.h"
}

int main( int argc, char ** argv ) 
{
  shark::Shark::info( std::cout );

  /* Board dimensions */ 
  int board_width  = 10;
  int board_height = 20;

  /* number of games to play */
  int nb_games = 1;

  /* Initialize the gls random generator (42 is arbitrary) */
  int seed = time(0);
  initialize_random_generator( seed );

  /* The filename of the feature file */ 
  std::string feature_file_name = "dellacherie_initial.dat";

  /* The filename of the statistics file */ 
  std::string statistics_file_name = "stat.dat";

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
  game = new_game(0, board_width, board_height, 0, "pieces4.dat", NULL);
  stats = games_statistics_new(statistics_file_name.c_str(), nb_games, NULL);

  
  /* run the game and see the score! */
  double points = feature_policy_play_games(&feature_policy, nb_games, game, stats, 0);

  /* Print the resulting score */
  std::cout << "seed: " << seed << ", " << "points: " << points << std::endl;


}





