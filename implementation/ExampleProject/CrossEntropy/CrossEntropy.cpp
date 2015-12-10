/*!
 *
 * \brief       Implements the most recent version of the non-elitist CMA-ES.
 * 
 * The algorithm is described in
 * Hansen, N. The CMA Evolution Startegy: A Tutorial, June 28, 2011
 * and the eqation numbers refer to this publication (retrieved April 2014).
 * 
 *
 * \author      Thomas Voss and Christian Igel
 * \date        April 2014
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
 #define SHARK_COMPILE_DLL
#include "CrossEntropy.h"
#include "cconfig.h"

#include <shark/Core/Exception.h>
#include <shark/Algorithms/DirectSearch/Operators/Evaluation/PenalizingEvaluator.h>
#include <shark/Algorithms/DirectSearch/FitnessExtractor.h>
#include <shark/Algorithms/DirectSearch/Operators/Selection/ElitistSelection.h>

using namespace shark;

//Functors used by the Cross Entropy

namespace{
	struct PointExtractor {
		template<typename T>
		const RealVector & operator()( const T & t ) const {
			return t.searchPoint();
		}
	};

	struct StepExtractor {
		template<typename T>
		const RealVector & operator()( const T & t ) const {
			return  t.chromosome();
		}
	};
	
	double chi( unsigned int n ) {
		return( std::sqrt( static_cast<double>( n ) )*(1. - 1./(4.*n) + 1./(21.*n*n)) );
	}

}

/**
* \brief Calculates lambda for the supplied dimensionality n.
*/
unsigned CrossEntropy::suggestLambda( unsigned int dimension ) {
	unsigned lambda = unsigned( 4. + ::floor( 3. * ::log( static_cast<double>( dimension ) ) ) ); // eq. (44)
	// heuristic for small search spaces
	lambda = std::max<unsigned int>( 5, std::min( lambda, dimension ) );
	return lambda;
}

/**
* \brief Calculates mu for the supplied lambda and the recombination strategy.
*/
unsigned int CrossEntropy::suggestMu( unsigned int lambda ) {
	/* Most papers says 10% of the population size is used for
	 * the new generation, thus, just take 10% of the lambda.
	 * */
	return (unsigned int) (lambda / 10.0);
}

CrossEntropy::CrossEntropy()
: m_sigma( 0 )
, m_samplingNoise( 0 )
, m_samplingNoiseType( NONE )
, m_lowerBound( 1E-20)
, m_counter( 0 ) {
	m_features |= REQUIRES_VALUE;
}

void CrossEntropy::read( InArchive & archive ) {
	archive >> m_numberOfVariables;
	archive >> m_mu;
	archive >> m_lambda;
	archive >> m_lowerBound;

	archive >> m_sigma;

	archive >> m_mean;

	archive >> m_counter;
}

void CrossEntropy::write( OutArchive & archive ) const {
	archive << m_numberOfVariables;
	archive << m_mu;
	archive << m_lambda;

	archive << m_lowerBound;

	archive << m_sigma;

	archive << m_mean;


	archive << m_counter;
}


void CrossEntropy::init( ObjectiveFunctionType & function, SearchPointType const& p) {
	
	unsigned int lambda = CrossEntropy::suggestLambda( p.size() );
	unsigned int mu = CrossEntropy::suggestMu(  lambda );
	RealVector initialSigma(p.size());
	for(int i = 0; i < p.size(); i++)
	{
		initialSigma(i) = 100;
	}
	init( function,
		p,
		lambda,
		mu,
		initialSigma
	);
}

/**
* \brief Initializes the algorithm for the supplied objective function.
*/
void CrossEntropy::init(
	ObjectiveFunctionType& function, 
	SearchPointType const& initialSearchPoint,
	unsigned int lambda, 
	unsigned int mu,
	RealVector initialSigma
) {
	checkFeatures(function);
	function.init();
	
	m_numberOfVariables = function.numberOfVariables();
	m_lambda = lambda;
	m_mu = static_cast<unsigned int>(::floor(mu));
	m_sigma = initialSigma;

	m_mean.resize( m_numberOfVariables );
	m_mean.clear();

	m_mean = initialSearchPoint;
	m_best.point = initialSearchPoint;
	m_best.value = function(initialSearchPoint);

	m_lowerBound = 1E-20;
	m_counter = 0;
}

/**
* \brief Updates the strategy parameters based on the supplied offspring population.
*/
void CrossEntropy::updateStrategyParameters( const std::vector<Individual<RealVector, double, RealVector> > & offspring ) {

	RealVector m(m_numberOfVariables);// = weightedSum( offspring, m_weights, PointExtractor() );
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
	unsigned int nOffspring = offspring.size();
	double normalizationFactor = 1.0 / double(nOffspring);

	for (int j = 0; j < m_numberOfVariables; j++)
	{
        double innerSum = 0.0;
		for (int i = 0; i < offspring.size(); i++)
		{
            double temp = offspring[i].searchPoint()(j) - m(j);
			innerSum += temp * temp;
		}
        innerSum *= normalizationFactor;

        // Different noise types
        if ( m_samplingNoiseType == NONE )
        {
            m_sigma(j) = innerSum;
        }
        else if ( m_samplingNoiseType == CONSTANT )
        {
            m_sigma(j) = innerSum + m_samplingNoise; // Add uniform noise here
        }
        else if ( m_samplingNoiseType == LINEAR_DECREASING )
        {
            m_sigma(j) = innerSum + std::max( (5.0 - m_counter/10.0) , 0.0); // Should not be hard coded
        }
	}

    //updateDistribution();


	//m_mutationDistribution.update();

    //std::cout << m_mutationDistribution.covarianceMatrix() << std::endl;
    //std::cout << m_mean << std::endl;

}

/**
* \brief Executes one iteration of the algorithm.
*/
void CrossEntropy::step(ObjectiveFunctionType const& function){

	std::vector< Individual<RealVector, double, RealVector> > offspring( m_lambda );

	std::cout << m_sigma << std::endl;

	PenalizingEvaluator penalizingEvaluator;
	for( unsigned int i = 0; i < offspring.size(); i++ ) {
        RealVector sample(m_numberOfVariables);
        for (int j = 0; j < m_numberOfVariables; j++)
        {
            Normal< Rng::rng_type > normal( Rng::globalRng, 0, m_sigma(j) );
            sample(j) = normal(); // N (0, 100)
        }
		//MultiVariateNormalDistribution::result_type sample = m_mutationDistribution();
		//offspring[i].chromosome() = sample.second;
		offspring[i].searchPoint() = m_mean + sample;
	}
	penalizingEvaluator( function, offspring.begin(), offspring.end() );

	// Selection
	std::vector< Individual<RealVector, double> > parents( m_mu );
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
}
