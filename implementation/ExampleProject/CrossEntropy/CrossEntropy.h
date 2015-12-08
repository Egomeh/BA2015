//===========================================================================
/*!
 *  REWRITE ALL OF THIS!
 *
 * \brief       Implements the most recent version of the non-elitist CMA-ES.
 * 
 * Hansen, N. The CMA Evolution Startegy: A Tutorial, June 28, 2011
 * and the eqation numbers refer to this publication (retrieved April 2014).
 * 
 *
 * \author      Thomas Voss and Christian Igel
 * \date        April 2014
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


#ifndef SHARK_ALGORITHMS_DIRECT_SEARCH_CROSSENTROPY_H
#define SHARK_ALGORITHMS_DIRECT_SEARCH_CROSSENTROPY_H

#include <shark/Core/DLLSupport.h>
#include <shark/Algorithms/AbstractSingleObjectiveOptimizer.h>
#include <shark/Statistics/Distributions/MultiVariateNormalDistribution.h>
#include <shark/Algorithms/DirectSearch/Individual.h>


class Test{

};

namespace shark {
	/**
	* \brief Implements the CMA-ES.
	*
	*  The algorithm is described in
	*
	*  Hansen, N., S. Kern (2004). Evaluating the CMA Evolution Strategy
	*  on Multimodal Test Functions. In Proceedings of the Eighth
	*  International Conference on Parallel Problem Solving from Nature
	*  (PPSN VIII), pp. 282-291, LNCS, Springer-Verlag
	*/
	class CrossEntropy : public AbstractSingleObjectiveOptimizer<RealVector >
	{
	public:
		/**
		 * \brief Mdoels the noise when sampling
		 */
		enum SamplingNoise {
			NONE    = 0,
			CONSTANT = 1,
			LINEAR_DECREASING = 2
		};

		/**
		* \brief Default c'tor.
		*/
		SHARK_EXPORT_SYMBOL CrossEntropy();

		/// \brief From INameable: return the class name.
		std::string name() const
		{ return "Cross Entropy"; }

		/**
		* \brief Calculates the center of gravity of the given population \f$ \in \mathbb{R}^d\f$.
		*
		*
		*/
		template<typename Container, typename Extractor>
		RealVector weightedSum( const Container & container, const RealVector & weights, const Extractor & e ) {

			RealVector result( m_numberOfVariables, 0. );

			for( unsigned int j = 0; j < container.size(); j++ )
				result += weights( j ) * e( container[j] );

			return( result );
		}

		/**
		* \brief Calculates lambda for the supplied dimensionality n.
		*/
		SHARK_EXPORT_SYMBOL static unsigned suggestLambda( unsigned int dimension ) ;

		/**
		* \brief Calculates mu for the supplied lambda and the recombination strategy.
		*/
		SHARK_EXPORT_SYMBOL static unsigned suggestMu( unsigned int lambda ) ;

		void read( InArchive & archive );
		void write( OutArchive & archive ) const;

		using AbstractSingleObjectiveOptimizer<RealVector >::init;
		/**
		* \brief Initializes the algorithm for the supplied objective function.
		*/
		SHARK_EXPORT_SYMBOL void init( ObjectiveFunctionType& function, SearchPointType const& p);

        /**
         * \brief Inits the cross entropy with only with the function
         */
        SHARK_EXPORT_SYMBOL void init( ObjectiveFunctionType& function );

		/**
		* \brief Initializes the algorithm for the supplied objective function.
		*/
		SHARK_EXPORT_SYMBOL void init(
			ObjectiveFunctionType& function,
			SearchPointType const& initialSearchPoint,
			unsigned int lambda,
			unsigned int mu,
			RealVector initialSigma
		);

		/**
		* \brief Executes one iteration of the algorithm.
		*/
		SHARK_EXPORT_SYMBOL void step(ObjectiveFunctionType const& function);



        /**
         * \brief Update the multivariateNormalDistributiopon accoring to internal sigma
         */
        void updateDistribution();

		/** \brief Accesses the current step size. */
		RealVector sigma() const {
			return m_sigma;
		}

		/** \brief Accesses the current step size. */
		void setSigma(RealVector sigma) {
			m_sigma = sigma;
		}

        /** \brief set all sigma values with single double */
        void setSigma(double sigma){
            for(int i = 0; i < m_sigma.size(); i++)
                m_sigma(i) = sigma;
            updateDistribution();

        }

        /** \brief Access the type of noise used when sampling offspring */
        SamplingNoise& samplingNoiseType(){
            return m_samplingNoiseType;
        }

        /** \brief Unmutable access the type of noise used when sampling offspring */
        const SamplingNoise & samplingNoiseType() const{
            return m_samplingNoiseType;
        }

        /** \brief Set the type of noise to when sampling ofspring */
        void setSamplingNoisetype( SamplingNoise samplingNoiseType ) {
            m_samplingNoiseType = samplingNoiseType;
        }

		/** \brief Access sampling noise term */
		double & SamplingNoiseTerm(){
			return m_samplingNoise;
		}

		/** \brief Unmutable access sampling noise term */
		const double & SamplingNoiseTerm() const{
			return m_samplingNoise;
		}

		/** \brief Accesses the current population mean. */
		RealVector const& mean() const {
			return m_mean;
		}

		/**
		 * \brief Returns a const reference to the lower bound on sigma times smallest eigenvalue.
		 */
		const double & lowerBound() const {
			return m_lowerBound;
		}

		/**
		 * \brief Returns a mutable reference to the lower bound on sigma times smallest eigenvalue.
		 */
		double& lowerBound() {
			return m_lowerBound;
		}

		/**
		 * \brief Returns the size of the parent population \f$\mu\f$.
		 */
		unsigned int mu() const {
			return m_mu;
		}

		/**
		 * \brief Returns a mutable reference to the size of the parent population \f$\mu\f$.
		 */
		unsigned int& mu(){
			return m_mu;
		}

        /**
		 * \brief Returns a immutable reference to the size of the offspring population \f$\mu\f$.
		 */
		unsigned int lambda()const{
			return m_lambda;
		}

		/**
		 * \brief Returns a mutable reference to the size of the offspring population \f$\mu\f$.
		 */
		unsigned int & lambda(){
			return m_lambda;
		}


	protected:
		/**
		* \brief Updates the strategy parameters based on the supplied offspring population.
		*/
		SHARK_EXPORT_SYMBOL void updateStrategyParameters( const std::vector<Individual<RealVector, double, RealVector> > & offspring ) ;

		std::size_t m_numberOfVariables; ///< Stores the dimensionality of the search space.
		unsigned int m_mu; ///< The size of the parent population.
		unsigned int m_lambda; ///< The size of the offspring population, needs to be larger than mu.

		double m_samplingNoise;     // Noise to add diversity to sample parametre;
		RealVector m_sigma; // Variace for sample parameters

		SamplingNoise m_samplingNoiseType; ///< Stores the type of sampling noise.


		double m_lowerBound;

		RealVector m_mean;

		unsigned m_counter; ///< counter for generations

	};
}

#endif
