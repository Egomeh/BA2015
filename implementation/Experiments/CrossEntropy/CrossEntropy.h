//===========================================================================
/*!
 *
 * \brief       Implements the Cross Entropy Algorithm.
 * 
 * Hansen, N. The CMA Evolution Startegy: A Tutorial, June 28, 2011
 * and the eqation numbers refer to this publication (retrieved April 2014).
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
//===========================================================================


#ifndef SHARK_ALGORITHMS_DIRECT_SEARCH_CROSSENTROPY_H
#define SHARK_ALGORITHMS_DIRECT_SEARCH_CROSSENTROPY_H

#include <shark/Core/DLLSupport.h>
#include <shark/Algorithms/AbstractSingleObjectiveOptimizer.h>
#include <shark/Statistics/Distributions/MultiVariateNormalDistribution.h>
#include <shark/Algorithms/DirectSearch/Individual.h>

#include <boost/shared_ptr.hpp>

namespace shark {
	/**
	* \brief Implements the Cross-Entropy method.
	*
	*  The algorithm is described in "Improvements on Learning Tetris with Cross Entropy"
	*
	*  Find the real reference and add it.
	*/
	class CrossEntropy : public AbstractSingleObjectiveOptimizer<RealVector >
	{
	public:
		/**
		 * \brief Models the noise when sampling
		 */
		enum SamplingNoise {
			NONE    = 0,
			CONSTANT = 1,
			LINEAR_DECREASING = 2
		};

		/**
		 * \brief Interface class for noise type.
		 */
		class INoiseType {
		public:
			double noiseValue (int t) const; 
		};

		/**
		 * \brief Smart pointer for noise type.
		 */
		typedef boost::shared_ptr<INoiseType> StrongNoisePtr;

		/**
		 * \brief Constant noise term z_t = noise.
		 */
        class ConstantNoise : public INoiseType {
		public:
			ConstantNoise ( double noise ) { m_noise = noise; };
			double noiseValue (int t) const { return std::max(m_noise, 0.0); } 
		private:
			double m_noise;
		};

		/**
		 * \brief Linear noise term z_t = a + t / b.
		 */
		class LinearNoise : public INoiseType {
		public:
			LinearNoise ( double a, double b ) { m_a = a; m_b = b; };
			double noiseValue (int t) const { return std::max(m_a + (t * m_b), 0.0); } 
		private:
			double m_a, m_b;
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

		/** \brief Accesses the current population mean. */
		RealVector const& mean() const {
			return m_mean;
		}

		/**
		 * \brief Returns the size of the parent population \f$\mu\f$.
		 */
		unsigned int selectionSize() const {
			return m_selectionSize;
		}

		/**
		 * \brief Returns a mutable reference to the size of the parent population \f$\mu\f$.
		 */
		unsigned int& selectionSize(){
			return m_selectionSize;
		}

                /**
		 * \brief Returns a immutable reference to the size of the offspring population \f$\mu\f$.
		 */
		unsigned int populationSize()const{
			return m_populationSize;
		}

		/**
		 * \brief Returns a mutable reference to the size of the offspring population \f$\mu\f$.
		 */
		unsigned int & populationSize(){
			return m_populationSize;
		}


	protected:
		/**
		* \brief Updates the strategy parameters based on the supplied offspring population.
		*/
		SHARK_EXPORT_SYMBOL void updateStrategyParameters( const std::vector<Individual<RealVector, double, RealVector> > & offspring ) ;

		std::size_t m_numberOfVariables; ///< Stores the dimensionality of the search space.
		unsigned int m_selectionSize; ///< Number of vectors chosen when updating distribution parameters.
		unsigned int m_populationSize; ///< Number of vectors sampled in a generation.

		RealVector m_sigma; ///< Variace for sample parameters

		StrongNoisePtr m_noise; ///< Noise type to apply in update of distribution parameters.

		RealVector m_mean; ///< The mean of the population.

		unsigned m_counter; ///< counter for generations

	};
}

#endif
