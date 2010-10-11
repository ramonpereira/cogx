// =================================================================                                                        
// Copyright (C) 2009-2011 Pierre Lison (plison@dfki.de)                                                                
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


package de.dfki.lt.tr.dialmanagement.utils;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Comparator;

import cast.DoesNotExistOnWMException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ManagedComponent;
import cast.cdl.WorkingMemoryAddress;

import de.dfki.lt.tr.beliefs.slice.distribs.BasicProbDistribution;
import de.dfki.lt.tr.beliefs.slice.distribs.CondIndependentDistribs;
import de.dfki.lt.tr.beliefs.slice.distribs.FormulaProbPair;
import de.dfki.lt.tr.beliefs.slice.distribs.FormulaValues;
import de.dfki.lt.tr.beliefs.slice.epstatus.AttributedEpistemicStatus;
import de.dfki.lt.tr.beliefs.slice.epstatus.EpistemicStatus;
import de.dfki.lt.tr.beliefs.slice.epstatus.PrivateEpistemicStatus;
import de.dfki.lt.tr.beliefs.slice.events.Event;
import de.dfki.lt.tr.beliefs.slice.framing.SpatioTemporalFrame;
import de.dfki.lt.tr.beliefs.slice.framing.TemporalInterval;
import de.dfki.lt.tr.beliefs.slice.intentions.CommunicativeIntention;
import de.dfki.lt.tr.beliefs.slice.intentions.Intention;
import de.dfki.lt.tr.beliefs.slice.intentions.IntentionalContent;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.BinaryOp;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.ComplexFormula;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.ModalFormula;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.PointerFormula;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.UnknownFormula;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.dFormula;
import de.dfki.lt.tr.beliefs.slice.sitbeliefs.dBelief;
import de.dfki.lt.tr.dialmanagement.arch.DialogueException;
import de.dfki.lt.tr.dialogue.interpret.IntentionManagementConstants;

/**
 * Utility functions for creating and manipulating epistemic objects 
 * (beliefs, events, intentions)
 *  
 * @author Pierre Lison (plison@dfki.de)
 * @version 07/10/2010
 *
 */
 
public class EpistemicObjectUtils {

	// logging and debugging
	public static boolean LOGGING = true;
	public static boolean DEBUG = false;
	
	// incremental counter for forging identifiers
	static int incrCounter = 0;

	
	public static final SpatioTemporalFrame curFrame = new SpatioTemporalFrame ("here", new TemporalInterval(),1.0f);
	public static final List<String> robotAgent = Arrays.asList("robot");
	public static final List<String> humanAgent = Arrays.asList("human");	
	public static final EpistemicStatus privateStatus = new PrivateEpistemicStatus("robot");
	public static final EpistemicStatus attributedStatus = new AttributedEpistemicStatus(robotAgent.get(0), humanAgent);
	
		
	/**
	 * Extract the intentional content of a formula with two modal operators <pre> and <post>
	 * 
	 * @param fullFormula the formula containing the two modal formulae
	 * @param agents the agents of the intentional content
	 * @param prob the probability
	 * @return the resulting intentional content
	 */
	public static IntentionalContent createIntentionalContent (dFormula fullFormula, List<String> agents, float prob) {
		
		dFormula precondition = new UnknownFormula(0);
		dFormula postcondition = fullFormula;
		if (fullFormula instanceof ComplexFormula) {
			
			for (dFormula subFormula : ((ComplexFormula)fullFormula).forms) {
				
				if (subFormula instanceof ModalFormula && ((ModalFormula)subFormula).op.equals("pre")) {
					precondition = ((ModalFormula)subFormula).form;
				}
				if (subFormula instanceof ModalFormula && ((ModalFormula)subFormula).op.equals("post")) {
					postcondition = ((ModalFormula)subFormula).form;
				}
			}
		}
		return new IntentionalContent (agents, precondition, postcondition, prob);
	}
	

	/**
	 * Create a new event with a list of alternative event descriptions, with associated
	 * probabilities
	 * 
	 * @param events a hashmap mapping each event description to its probability value
	 * @return the newly constructed event
	 * @throws DialogueException if descriptions are ill-formed
	 */
	public static Event createEvent (HashMap<dFormula,Float> events) throws DialogueException {
		
	    SpatioTemporalFrame frame = new SpatioTemporalFrame ("here", new TemporalInterval(),1.0f);
		PrivateEpistemicStatus priv = new PrivateEpistemicStatus ("robot");
		
		List<FormulaProbPair> content = new LinkedList<FormulaProbPair>();
		
		for (dFormula formula: events.keySet()) {
			content.add((new FormulaProbPair(formula, events.get(formula))));
		}
		return new Event(frame, priv, forgeNewId(), new BasicProbDistribution("content", new FormulaValues(content)));
	}
	
	
	/**
	 * Create a new simple event with a unique event description and associated probability
	 * 
	 * @param event the event description
	 * @param prob the probability
	 * @return the newly constructed event 
	 * @throws DialogueException if the event description is ill-formed
	 */
	public static Event createSimpleEvent (dFormula event, float prob) throws DialogueException {
		HashMap<dFormula,Float> events = new HashMap<dFormula,Float>();
		events.put(event, prob);
		return createEvent(events);
	}
	
	
	public static Event createSimpleEvent (String eventstr, float prob) throws DialogueException {
		HashMap<dFormula,Float> events = new HashMap<dFormula,Float>();
		events.put(FormulaUtils.constructFormula(eventstr), prob);
		return createEvent(events);
	}
	

	/**
	 * Sort the list of <formula,prob> pairs by decreasing probability (i.e. the pair
	 * with the highest probability comes first, then the second, und so weiter)
	 * 
	 * @param initContent the initial, unsorted list
	 * @return the sorted list
	 */
	public static List<FormulaProbPair> sortFormulaValues (List<FormulaProbPair> formProbPairs) {
		
		Collections.sort(formProbPairs, new FormulaProbPairsComparator());
		return formProbPairs;
	}
	
	
	
	/**
	 * Transform an intentional content into a single formula, with two modal
	 * formulae <pre> and <post>
	 * 
	 * @param content the intentional content
	 * @return the resulting formula
	 */
	public static dFormula translateIntoFormula (IntentionalContent content) {
		
		if (!(content.preconditions instanceof UnknownFormula)) {
			List<dFormula> formulae = new LinkedList<dFormula>();
			formulae.add(new ModalFormula(0, "pre", content.preconditions));
			formulae.add(new ModalFormula(0, "post", content.postconditions));
			return new ComplexFormula(0,formulae, BinaryOp.conj);
		}
		else {
			return new ModalFormula(0, "post", content.postconditions);
		}
		
	}
	
	
	
	
	/**
	 * Create a copy of the communicative intention
	 * 
	 * @param initCI the comm. intention to copy
	 * @return the copied communicative intention
	 * @throws DialogueException if something went wrong in the copy
	 */
	public static CommunicativeIntention copy (CommunicativeIntention initCI) throws DialogueException {
		return new CommunicativeIntention(copy(initCI.intent))	;	
	}
	
	
	/**
	 * Create a copy of the intention
	 * 
	 * @param initIntention the intention to copy
	 * @return the copied intention
	 * @throws DialogueException
	 */
	public static Intention copy (Intention initIntention) throws DialogueException {
		
		List<IntentionalContent> newContent = new LinkedList<IntentionalContent>();
		for (IntentionalContent existingIntent : initIntention.content) {
			dFormula copiedPreconditions = FormulaUtils.copy(existingIntent.preconditions);
			dFormula copiedPostconditions = FormulaUtils.copy(existingIntent.postconditions);
			newContent.add(new IntentionalContent (existingIntent.agents, copiedPreconditions,copiedPostconditions, existingIntent.probValue));
		}
		return new Intention(initIntention.frame, initIntention.estatus, initIntention.id, initIntention.content);
	}
	
	
	
	
	/**
	 * Expand the intentional content -- i.e. if the formula of the postcondition contains
	 * a pointer formula, replace this pointer by the value of the epistemic object
	 * being pointed at
	 * 
	 * @param initCI the initial communicative intention
	 * @param component the CAST component to retrieve object from memory
	 * @return the expanded intentional content (as a copy)
	 * 
	 * @throws DialogueException
	 * @throws DoesNotExistOnWMException
	 * @throws UnknownSubarchitectureException
	 */
	public static CommunicativeIntention expandCommunicativeIntention  
		(CommunicativeIntention initCI, ManagedComponent component) 
	throws DialogueException, DoesNotExistOnWMException, UnknownSubarchitectureException  {
		
		CommunicativeIntention newCI = copy (initCI);
		debug("communicative intention " + initCI.intent.id + " successfully copied");
		
		for (IntentionalContent alternativeContent : newCI.intent.content) {			
			alternativeContent.postconditions = expandFormula(alternativeContent.postconditions, component);		
			debug("expanded postcondition: " + FormulaUtils.getString(alternativeContent.postconditions));
		}
			
		return newCI;
	}

	
	
	/**
	 * Extract the content of the belief and express it as a single ComplexFormula
	 * 
	 * TODO: take the ambiguity into account

	 * @param b the belief
	 * @return the complex formula representing the belief
	 * @throws DialogueException
	 */
	private static ComplexFormula getBeliefContent (dBelief b) throws DialogueException {
		
		debug("type of distrib: " + b.content.getClass().getCanonicalName());
		
		if (b.content instanceof CondIndependentDistribs) {
			
			List<dFormula> beliefFormulae = new LinkedList<dFormula>();
			
			for (String key : ((CondIndependentDistribs)b.content).distribs.keySet()) {
				if (((CondIndependentDistribs)b.content).distribs.get(key) instanceof BasicProbDistribution) {
					if (((BasicProbDistribution)((CondIndependentDistribs)b.content).
							distribs.get(key)).values instanceof FormulaValues) {
						
						// here we only take the first value
						if (((FormulaValues)((BasicProbDistribution)((CondIndependentDistribs)
								b.content).distribs.get(key)).values).values.size() > 0) {
							dFormula keyValue = ((FormulaValues)((BasicProbDistribution)
									((CondIndependentDistribs)b.content).distribs.get(key)).values).values.get(0).val;
							ModalFormula replacementFormula = new ModalFormula(0, key, keyValue);
							debug("adding formula: " + FormulaUtils.getString(replacementFormula));
							beliefFormulae.add(replacementFormula);
						}
					}
				}
			}
			
			ComplexFormula completeFormula = new ComplexFormula(0, beliefFormulae, BinaryOp.conj);
			debug("complete replacement formula: " + FormulaUtils.getString(completeFormula)); 
			return completeFormula;
		}
		else if (b.content instanceof BasicProbDistribution) {
						
			if  (((BasicProbDistribution)b.content).values instanceof FormulaValues) {
				
				// here again, only take the first value
				if (((FormulaValues)((BasicProbDistribution)b.content).values).values.size() > 0) {
					dFormula form = ((FormulaValues)((BasicProbDistribution)b.content).values).values.get(0).val;
					if (form instanceof ComplexFormula) {
						return (ComplexFormula)form;
					}
					else {
						List<dFormula> forms = new LinkedList<dFormula>();
						forms.add(form);
						return new ComplexFormula(0, forms, BinaryOp.conj) ;
					}
				}
			}
		}
		throw new DialogueException("WARNING: belief content of " + b.id + " could not be extracted");
	}
	
		
	/**
	 * Expand the given formula (if the formula contains a pointer, replaces it by
	 * the content of the epistemic object being pointed at)
	 * 
	 * @param form the formula to expand
	 * @param component the CAST component to extract the pointed-to object
	 * @return the expanded formula
	 * 
	 * @throws UnknownSubarchitectureException
	 * @throws DoesNotExistOnWMException
	 * @throws DialogueException
	 */
	private static dFormula expandFormula (dFormula form, ManagedComponent component)
		throws UnknownSubarchitectureException, DoesNotExistOnWMException, DialogueException {
		
		if (form instanceof ComplexFormula) {
			List<dFormula> newSubFormulae = new LinkedList<dFormula>();
			for (dFormula existingSubFormula : ((ComplexFormula)form).forms) {
				newSubFormulae.add(expandFormula(existingSubFormula,component));
			}
			return new ComplexFormula(0, newSubFormulae, ((ComplexFormula)form).op);
		}
		
		else if (form instanceof ModalFormula) {
			return new ModalFormula(0, ((ModalFormula)form).op, expandFormula(((ModalFormula)form).form, component));
		}
		
		else if (form instanceof PointerFormula) {
			debug("found pointer: " + FormulaUtils.getString(form));
			WorkingMemoryAddress WMPointer= ((PointerFormula)form).pointer;
			if (WMPointer != null && component.existsOnWorkingMemory(WMPointer)) {			
				try {
				dBelief b = component.getMemoryEntry(WMPointer, dBelief.class);
				ComplexFormula expandedFormula =  getBeliefContent(b);
				expandedFormula.forms.add(new ModalFormula(0, "ref", form));
				return expandedFormula;
				}
				catch (ClassCastException e) {
					e.printStackTrace();
				}
			}
		}
		
		return form;
	}
	
	
	/**
	 * Forge a new identifier
	 * @return
	 */
	public static String forgeNewId () {
		incrCounter++;
		return "id" + incrCounter;
	}
	

	/**
	 * Logging
	 * @param s
	 */
	private static void log (String s) {
		if (LOGGING) {
			System.out.println("[epobjectutils] " + s);
		}
	}

	/**
	 * Debugging
	 * @param s
	 */
	private static void debug (String s) {
		if (DEBUG) {
			System.out.println("[epobjectutils] " + s);
		}
	}
 }


/**
 * Comparator for intentional content
 * 
 * @author Pierre Lison (plison@dfki.de)
 * @version 04/07/2010
 * 
 */
final class IntentionalContentComparator implements Comparator<IntentionalContent> {

	
	/**
	 * Returns a positive value if o2 > o1, 0 if o2 == o1, and a negative value if o2 < 01
	 * 
	 */
	@Override
	public int compare(IntentionalContent o1, IntentionalContent o2) {
		if (o1 != null && o2 != null) {
			return (int)(o2.probValue - o1.probValue)*10000 ;
		}
		return 0;
	}
	
}



/**
 * Comparator for intentional content
 * 
 * @author Pierre Lison (plison@dfki.de)
 * @version 04/07/2010
 * 
 */
final class FormulaProbPairsComparator implements Comparator<FormulaProbPair> {

	/**
	 * Returns a positive value if o2 > o1, 0 if o2 == o1, and a negative value if o2 < 01
	 * 
	 */
	@Override
	public int compare(FormulaProbPair o1, FormulaProbPair o2) {
		if (o1 != null && o2 != null) {
			return (int)(o2.prob - o1.prob)*10000 ;
		}
		return 0;
	}
	
	


}
