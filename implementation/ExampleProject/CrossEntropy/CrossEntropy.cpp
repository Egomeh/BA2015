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
unsigned int CrossEntropy::suggestMu( unsigned int lambda, RecombinationType recomb) {
	switch( recomb ) {
		case EQUAL:         
			return lambda / 4;
		case LINEAR:        
			return lambda / 2; 
		case SUPERLINEAR:   
			return lambda / 2;
	}
	return 0;
}

CrossEntropy::CrossEntropy()
:m_recombinationType( SUPERLINEAR )
, m_sigma( 0 )
, m_zeta( 0 )
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
	archive >> m_recombinationType;
	archive >> m_lowerBound;

	archive >> m_sigma;

	archive >> m_mean;
	archive >> m_weights;

	archive >> m_mutationDistribution;

	archive >> m_counter;
}

void CrossEntropy::write( OutArchive & archive ) const {
	archive << m_numberOfVariables;
	archive << m_mu;
	archive << m_lambda;
	
	archive << m_recombinationType;
	archive << m_lowerBound;

	archive << m_sigma;

	archive << m_mean;
	archive << m_weights;

	archive << m_mutationDistribution;

	archive << m_counter;
}


void CrossEntropy::init( ObjectiveFunctionType & function, SearchPointType const& p) {
	
	unsigned int lambda = CrossEntropy::suggestLambda( p.size() );
	unsigned int mu = CrossEntropy::suggestMu(  lambda, m_recombinationType );
	RealVector initialSigma(lambda);
	for(int i = 0; i < lambda; i++)
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
	RealVector initialSigma,
	const boost::optional< RealMatrix > & initialCovarianceMatrix
) {
	checkFeatures(function);
	function.init();
	
	m_numberOfVariables = function.numberOfVariables();
	m_lambda = lambda;
	m_mu = static_cast<unsigned int>(::floor(mu));
	m_sigma = initialSigma;

	m_mean.resize( m_numberOfVariables );
	m_mutationDistribution.resize( m_numberOfVariables );
	m_mean.clear();
	if(initialCovarianceMatrix){
		m_mutationDistribution.covarianceMatrix() = *initialCovarianceMatrix;
		m_mutationDistribution.update();
	}
		
	//weighting of the k-best individuals
	m_weights.resize(m_mu);
	switch (m_recombinationType) {
	case EQUAL:
		for (unsigned int i = 0; i < m_mu; i++)
			m_weights(i) = 1;
		break;
	case LINEAR:
		for (unsigned int i = 0; i < m_mu; i++)
			m_weights(i) = mu-i;
		break;
	case SUPERLINEAR:
		for (unsigned int i = 0; i < m_mu; i++)
			m_weights(i) = ::log(mu + 0.5) - ::log(1. + i); // eq. (45)
		break;
	}
	m_weights /= sum(m_weights); // eq. (45)
	//m_muEff = 1. / sum(sqr(m_weights)); // equal to sum(m_weights)^2 / sum(sqr(m_weights))

	// Step size control
	//m_cSigma = (m_muEff + 2.)/(m_numberOfVariables + m_muEff + 5.); // eq. (46)
	//m_dSigma = 1. + 2. * std::max(0., ::sqrt((m_muEff-1.)/(m_numberOfVariables+1)) - 1.) + m_cSigma; // eq. (46)

	//m_cC = (4. + m_muEff / m_numberOfVariables) / (m_numberOfVariables + 4. +  2 * m_muEff / m_numberOfVariables); // eq. (47)
	//m_c1 = 2 / (sqr(m_numberOfVariables + 1.3) + m_muEff); // eq. (48)
	double alphaMu = 2.;
	//m_cMu = std::min(1. - m_c1, alphaMu * (m_muEff - 2. + 1./m_muEff) / (sqr(m_numberOfVariables + 2) + alphaMu * m_muEff / 2)); // eq. (49)

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

	/* TODO: Do not use weighted sum, instead use simple for loop! */
	int nDims = offspring[0].searchPoint().size();
	RealVector m(nDims);// = weightedSum( offspring, m_weights, PointExtractor() );
	for (int i = 0; i < nDims; i++)
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
	double normalizationFactor = 1.0 / nOffspring;

    Normal< Rng::rng_type > normal( Rng::globalRng, 0, m_samplingNoise );
    std::cout << m_samplingNoise;

	for (int j = 0; j < m_mean.size(); j++)
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
            m_sigma(j) = std::sqrt(innerSum);
        }
        else if ( m_samplingNoiseType == CONSTANT )
        {
            m_sigma(j) = std::sqrt(innerSum) + m_samplingNoise; // Add uniform noise here
        }
        else if ( m_samplingNoiseType == LINEAR_DECREASING )
        {
            m_sigma(j) = std::sqrt(innerSum) + std::max( (5.0 - m_counter/10.0) , 0.0); // Should not be hard coded
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

	PenalizingEvaluator penalizingEvaluator;
	for( unsigned int i = 0; i < offspring.size(); i++ ) {
        Normal< Rng::rng_type > normal( Rng::globalRng, 0, 1.0f );
        RealVector sample(m_numberOfVariables);
        for (int j = 0; j < m_numberOfVariables; j++)
        {
            sample(j) = std::sqrt(m_sigma(j)) * normal();
        }
		//MultiVariateNormalDistribution::result_type sample = m_mutationDistribution();
		//offspring[i].chromosome() = sample.second;
		offspring[i].searchPoint() = m_mean + sample;
	}
	penalizingEvaluator( function, offspring.begin(), offspring.end() );

	// Selection
	std::vector< Individual<RealVector, double, RealVector> > parents( m_mu );
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
	// Update covariance matrix of distribution
	// such that it becomes diag(sigma)
	RealMatrix & updateCMatrix = m_mutationDistribution.covarianceMatrix();
	for (int i = 0; i < updateCMatrix.size1(); i++)
	{
		updateCMatrix(i,i) = m_sigma(i);
	}
	m_mutationDistribution.update();
}
