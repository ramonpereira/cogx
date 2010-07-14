
// =================================================================                                                        
// Copyright (C) 2009-2011 Pierre Lison (pierre.lison@dfki.de)                                                                
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


package de.dfki.lt.tr.dialmanagement.data.observations;

import de.dfki.lt.tr.beliefs.slice.logicalcontent.UnderspecifiedFormula;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.UnknownFormula;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.dFormula;
import de.dfki.lt.tr.dialmanagement.arch.DialogueException;
import de.dfki.lt.tr.dialmanagement.utils.FormulaUtils;

/**
 * Observation of an intention, described by a formula associated with a probability
 * value (or a range of probability values)
 * 
 * @author Pierre Lison (plison@dfki.de)
 * @version 03/07/2010
 *
 */
public class IntentionObservation extends AbstractObservation {

	// the formula describing the intention
	protected dFormula intentFormula;

	
	/**
	 * Create a new intention observation given a propositional modal formula and a range 
	 * of possible probability values
	 * 
	 * @param intentFormula the formula describing the intention
	 * @param minProb the minimum probability
	 * @param maxProb the maximum probability
	 * @throws DialogueException if the inputs are ill-defined
	 */
	public IntentionObservation (dFormula intentFormula, float minProb, float maxProb) throws DialogueException {	
		super(minProb,maxProb);
		if (intentFormula == null) {
			throw new DialogueException ("ERROR: formula for the intention is null");
		}
		this.intentFormula = intentFormula;
		System.out.println("probs: " + this.minProb + " " + this.maxProb);
	} 
	

	/**
	 * Create a new intention observation given a propositional modal formula and a specific
	 * probability value
	 * 
	 * @param intentFormula  the formula describing the intention
	 * @param prob the probability for the intention
	 * @throws DialogueException if the inputs are ill-defined
	 */
	public IntentionObservation(dFormula intentFormula, float prob) throws DialogueException {
		this(intentFormula,prob,prob);
	}
	
	

	/**
	 * Returns the formula contained in the observation
	 * @return the formula
	 */
	public dFormula getFormula () {
		return intentFormula;
	}
	
	/**
	 * Returns a text representation of the observation
	 */
	public String toString() {
		return "I[" + FormulaUtils.getString(intentFormula) 
		+ "]" + " (" + minProb + ", " + maxProb + ")";
	}
	

	/**
	 * Returns true if the formula is underspecified, false otherwise
	 * 
	 * @return true if the formula is underspecified, false otherwise
	 */
	@Override
	public boolean isUnderspecified() {
		return (intentFormula instanceof UnderspecifiedFormula);
	}
	
	
	/**
	 * Returns true if the formula is unknown, false otherwise
	 * 
	 * @return true if the formula is unknown, false otherwise
	 */
	@Override
	public boolean isUnknown() {
		return (intentFormula instanceof UnknownFormula);
	}
	
	
	
	/**
	 * Returns true if the current object is equivalent to the one passed as argument.
	 * By equivalency, we mean that (1) the formulae must be equal, and that (2) the 
	 * (min,max) range of probability values specified in the current object must be 
	 * contained in the (min,max) probability range of obj. 
	 * 
	 * obj the object to compare
	 * 
	 */
	@Override
	public boolean equals(Object obj) {
		
		if (obj instanceof IntentionObservation && obj != null) {
			IntentionObservation intentObj = (IntentionObservation) obj;
			
			System.out.println(FormulaUtils.isEqualTo(intentFormula, intentObj.getFormula()) && 
						minProb >= intentObj.getMinProb() && maxProb <= intentObj.getMaxProb());
			
				if (FormulaUtils.isEqualTo(intentFormula, intentObj.getFormula()) && 
						minProb >= intentObj.getMinProb() && maxProb <= intentObj.getMaxProb()) {
					return true;
				}
			
		}
		return false;
	}
	
	
}
