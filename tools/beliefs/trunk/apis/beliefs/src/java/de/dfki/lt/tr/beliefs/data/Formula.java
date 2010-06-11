// =================================================================
// Copyright (C) 2010 DFKI GmbH Talking Robots 
// Geert-Jan M. Kruijff (gj@dfki.de)
//                                                                                                                          
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License 
// as published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.
//                                                                                                                          
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//                                                                                                                          
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
// =================================================================

// =================================================================
// PACKAGE DEFINITION 
package de.dfki.lt.tr.beliefs.data;

// Belief API slice
import de.dfki.lt.tr.beliefs.data.abstractproxies.Proxy;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.ElementaryFormula;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.dFormula;
import de.dfki.lt.tr.beliefs.util.BeliefInvalidQueryException;

/**
 * The <tt>Formula</tt> class implements the basic structure for building up
 * content.
 * 
 * 
 * @author Geert-Jan M. Kruijff (gj@dfki.de), Marc Hanheide (marc@hanheide.de)
 * @started 100521
 * @version 100521
 */

public class Formula<T extends dFormula> extends Proxy<T> {

	protected float _probability;

	/**
	 * Object is created from underlying slice-based datastructure, and a given
	 * probability.
	 */
	public Formula(Class<T> type, T formula) {
		super(type, formula);

	} // end constructor

	public Formula(Class<T> class1, T val, float prob) {
		super(class1, val);
		setProbability(prob);
	}

	/**
	 * Returns the identifier of the formula. By default this is set to -1, upon
	 * initialization.
	 * 
	 * @return int The identifier of the formula
	 * @throws BeliefNotInitializedException
	 *             If the formula has not been initialized
	 */

	public int getId()  {
		return _content.id;
	} // end getId

	/**
	 * Sets the identifier of the formula.
	 * 
	 * @param id
	 *            The identifier (represented as an integer) for the formula
	 * @throws BeliefNotInitializedException
	 *             If the formula has not been initialized
	 */

	public void setId(int id)  {
		_content.id = id;
	} // end setId

	/**
	 * Returns whether the formula is a proposition (or not)
	 * 
	 * @return boolean True if the formula is a proposition
	 * @throws BeliefNotInitializedException
	 *             If the formula has not been initialized
	 */

	public boolean isProposition() {
		return (_content instanceof ElementaryFormula);
	} // end isProposition

	/**
	 * Returns the proposition for a formula, provided the formula is
	 * propositional
	 * 
	 * @return String The proposition of the formula
	 * @throws BeliefNotInitializedException
	 *             If the formula is not initialized
	 * @throws BeliefInvalidQueryException
	 *             If the formula is not of type proposition
	 */

	public String getProposition() throws
			BeliefInvalidQueryException {
		if (!(_content instanceof ElementaryFormula)) {
			throw new BeliefInvalidQueryException(
					"Cannot query [proposition] for formula: Formula not of type proposition");
		} else {
			return ((ElementaryFormula) _content).prop;
		}
	} // end getProposition

	/**
	 * Sets the probability for an instantiated formula
	 * 
	 * @param prob
	 *            The probability
	 * @throws BeliefNotInitializedException
	 *             If the formula is not initialized
	 * @throws BeliefInvalidOperationException
	 *             If the formula has not been instantiated
	 */

	public void setProbability(float prob) {
		_probability = prob;
	} // end setProbability

	/**
	 * Returns the probability for an instantiated formula
	 * 
	 * @return float The probability of the formula
	 * @throws BeliefInvalidOperationException
	 *             If the formula is not instantiated
	 * @throws BeliefNotInitializedException
	 *             If the formula is not initialized
	 */

	public float getProbability() {
		return _probability;
	} // end setProbability

	// /**
	// * Returns the formula and probability as a pair
	// *
	// * @return FormulaProbPair The slice-based datastructure of a formula and
	// * its associated probability
	// * @throws BeliefNotInitializedException
	// * If the formula is not instantiated
	// * @throws BeliefInvalidOperationException
	// * If the probability or the formula is not initialized
	// */
	//
	// public FormulaProbPair getAsPair() throws BeliefNotInitializedException,
	// BeliefInvalidOperationException {
	// if (_probability == -1) {
	// throw new BeliefInvalidOperationException(
	// "Cannot create formula,probability pair for non-initialized probability");
	// }
	// return new FormulaProbPair(_content, _probability);
	// } // end getAsPair

} // end class
