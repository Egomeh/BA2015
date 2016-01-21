/*!
 *
 * \brief       Implements the Cross Entropy Algorithm.
 * 
 * Christophe Thiery, Bruno Scherrer. Improvements on Learning Tetris with Cross Entropy.
 * International Computer Games Association Journal, ICGA, 2009, 32. <inria-00418930>
 * 
 *
 * \author      Jens Holm, Mathias Petræus and Mark Wulff
 * \date        January 2016
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
 #define SHARK_COMPILE_DLL
#include "CrossEntropy.h"
#include "cconfig.h"

#include <shark/Core/Exception.h>
#include <shark/Algorithms/DirectSearch/Operators/Evaluation/PenalizingEvaluator.h>
#include <shark/Algorithms/DirectSearch/FitnessExtractor.h>
#include <shark/Algorithms/DirectSearch/Operators/Selection/ElitistSelection.h>

using namespace shark;

/**
* \brief Suggest a population size of 100.
*/
unsigned CrossEntropy::suggestPopulationSize(  ) {
	/*
	 * Most papers suggests a population size of 100, thus
	 * simply choose 100.
	 */
	return 100;
}

/**
* \brief Calculates Selection Size for the supplied Population Size.
*/
unsigned int CrossEntropy::suggestSelectionSize( unsigned int populationSize ) {
	/* 
	 * Most papers says 10% of the population size is used for
	 * the new generation, thus, just take 10% of the population size.
	 */
	return (unsigned int) (populationSize / 10.0);
}

CrossEntropy::CrossEntropy()
: m_variance( 0 )
, m_counter( 0 )
, m_distribution( Normal< Rng::rng_type >( Rng::globalRng, 0, 1.0 ) )
, m_noise (boost::shared_ptr<INoiseType> (new ConstantNoise(0.0)))
{
	m_features |= REQUIRES_VALUE;
}

void CrossEntropy::read( InArchive & archive ) {
	archive >> m_numberOfVariables;
	archive >> m_selectionSize;
	archive >> m_populationSize;

	archive >> m_variance;

	archive >> m_mean;

	archive >> m_counter;
}

void CrossEntropy::write( OutArchive & archive ) const {
	archive << m_numberOfVariables;
	archive << m_selectionSize;
	archive << m_populationSize;

	archive << m_variance;

	archive << m_mean;

	archive << m_counter;
}


void CrossEntropy::init( ObjectiveFunctionType & function, SearchPointType const& p) {
	
	unsigned int populationSize = CrossEntropy::suggestPopulationSize( );
	unsigned int selectionSize = CrossEntropy::suggestSelectionSize( populationSize );
	RealVector initialVariance(p.size());

	// Most papers set the variance to 100 by default.
	for(int i = 0; i < p.size(); i++)
	{
		initialVariance(i) = 100;
	}
	init( function,
		p,
		populationSize,
		selectionSize,
		initialVariance
	);
}

/**
* \brief Initializes the algorithm for the supplied objective function.
*/
void CrossEntropy::init(
	ObjectiveFunctionType& function, 
	SearchPointType const& initialSearchPoint,
	unsigned int populationSize,
	unsigned int selectionSize,
	RealVector initialVariance
) {
	checkFeatures(function);
	function.init();
	
	m_numberOfVariables = function.numberOfVariables();
	m_populationSize = populationSize;
	m_selectionSize = static_cast<unsigned int>(::floor(selectionSize));
	m_variance = initialVariance;

	m_mean.resize( m_numberOfVariables );
	m_mean.clear();

	m_mean = initialSearchPoint;
	m_best.point = initialSearchPoint;
	m_best.value = function(initialSearchPoint);
	m_counter = 0;

}

/**
* \brief Updates the strategy parameters based on the supplied offspring population.
*/
void CrossEntropy::updateStrategyParameters( const std::vector<Individual<RealVector, double> > & offspring ) {

    /* Calculate the centroid of the offspring */
	RealVector m(m_numberOfVariables);
	for (int i = 0; i < m_numberOfVariables; i++)
	{
        m(i) = 0;
        for (int j = 0; j < offspring.size(); j++)
        {
        	m(i) += offspring[j].searchPoint()(i);
        }
        m(i) /= double(offspring.size());
	}


	// mean update
	m_mean = m;

	// Sigma update
	size_t nOffspring = offspring.size();
	double normalizationFactor = 1.0 / double(nOffspring);

	for (int j = 0; j < m_numberOfVariables; j++) {
        double innerSum = 0.0;
        for (int i = 0; i < offspring.size(); i++) {
            double diff = offspring[i].searchPoint()(j) - m(j);
            innerSum += diff * diff;
        }
        innerSum *= normalizationFactor;

        // Apply noise
        m_variance(j) = innerSum + m_noise->noiseValue(m_counter);
    }

}

/**
* \brief Executes one iteration of the algorithm.
*/
void CrossEntropy::step(ObjectiveFunctionType const& function){

	std::vector< Individual<RealVector, double> > offspring( m_populationSize );

	PenalizingEvaluator penalizingEvaluator;
	for( unsigned int i = 0; i < offspring.size(); i++ ) {
        RealVector sample(m_numberOfVariables);
        for (int j = 0; j < m_numberOfVariables; j++)
        {
            sample(j) = m_distribution(m_mean(j), m_variance(j)); // N (0, 100)
        }
		offspring[i].searchPoint() = sample;
	}

	penalizingEvaluator( function, offspring.begin(), offspring.end() );

	// Selection
	std::vector< Individual<RealVector, double> > parents( m_selectionSize );
	ElitistSelection<FitnessExtractor> selection;
	selection(offspring.begin(),offspring.end(),parents.begin(), parents.end());
	// Strategy parameter update
	m_counter++; // increase generation counter
	updateStrategyParameters( parents );

	m_best.point= parents[ 0 ].searchPoint();
	m_best.value= parents[ 0 ].unpenalizedFitness();
}

void CrossEntropy::init(ObjectiveFunctionType& function ){
	if(!(function.features() & ObjectiveFunctionType::CAN_PROPOSE_STARTING_POINT))
		throw SHARKEXCEPTION( "[AbstractSingleObjectiveOptimizer::init] Objective function does not propose a starting point");
	CrossEntropy::init(function,function.proposeStartingPoint());
}


void CrossEntropy::updateDistribution() {
	/* Not sure if we should do anything here */
	/* Always same variance, therefore no update? */
}
