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


package de.dfki.lt.tr.dialmanagement.components;

import java.util.Collection;
import java.util.List;

import de.dfki.lt.tr.beliefs.slice.distribs.BasicProbDistribution;
import de.dfki.lt.tr.beliefs.slice.distribs.FormulaProbPair;
import de.dfki.lt.tr.beliefs.slice.distribs.FormulaValues;
import de.dfki.lt.tr.beliefs.slice.events.Event;
import de.dfki.lt.tr.beliefs.slice.intentions.CommunicativeIntention;
import de.dfki.lt.tr.beliefs.slice.intentions.IntentionalContent;
import de.dfki.lt.tr.dialmanagement.arch.DialogueException;
import de.dfki.lt.tr.dialmanagement.data.DialoguePolicy;
import de.dfki.lt.tr.dialmanagement.data.Observation;
import de.dfki.lt.tr.dialmanagement.data.PolicyAction;
import de.dfki.lt.tr.dialmanagement.data.PolicyEdge;
import de.dfki.lt.tr.dialmanagement.data.PolicyNode;
import de.dfki.lt.tr.dialmanagement.utils.PolicyUtils;


/**
 * A simple, FSA-based dialogue manager.  The manager revolves around a dialogue policy and
 * a pointer to the current node in the policy.  The traversal of the policy is implemented
 * with the nexAction method, which takes an observation as argument, and jumps to the
 * appropriate next action.
 * 
 * @author Pierre Lison (plison@dfki.de)
 * @version 3/10/2010
 *
 */
public class DialogueManager {

	// logging and debugging
	public static boolean LOGGING = true;
	public static boolean DEBUG = true;

	// the dialogue policy
	DialoguePolicy policy;

	// the current action node
	PolicyNode curNode;

	/**
	 * Create a new dialogue manager based on a dialogue policy
	 * 
	 * @param policy the dialogue policy (typically extracted from a configuration file)
	 */
	public DialogueManager(DialoguePolicy policy) {
		this.policy = policy;
		curNode = policy.getInitNode();
	}



	/**
	 * Jumps to the next appropriate action given the intention provided as argument.  
	 * The method extracts all alternative intentional contents to create an observation,
	 * and then checks whether a policy edge matching this observation can be found
	 * 
	 * If one exists, the method assigns curNode to be the node pointed by the edge,
	 * and returns it. Otherwise, a void action is returned.
	 *   
	 * @param intention the intention
	 * @return the next action if one is available, or else a void action 
	 * @throws DialogueException if the content of the intention or the dialogue policy
	 *         is ill-formatted
	 */
	public PolicyAction nextAction(CommunicativeIntention intention) throws DialogueException {

		Observation obs = new Observation(Observation.INTENTION);
		for (IntentionalContent icontent : intention.intent.content) {
			obs.addAlternative(icontent.postconditions, icontent.probValue);
		}

		return nextAction(obs);
	}




	/**
	 * Jumps to the next appropriate action given the event provided as argument.  
	 * The method extracts all alternative event descriptions to create an observation,
	 * and then checks whether a policy edge matching this observation can be found
	 * 
	 * If one exists, the method assigns curNode to be the node pointed by the edge,
	 * and returns it. Otherwise, a void action is returned.
	 *   
	 * @param event the event
	 * @return the next action if one is available, or else a void action 
	 * @throws DialogueException if the content of the event or the dialogue policy
	 *         is ill-formatted
	 */
	public PolicyAction nextAction(Event event) throws DialogueException {

		// we assume here that the event content is a BasicProbDistribution
		Observation obs = new Observation(Observation.EVENT);
		if (event.content instanceof BasicProbDistribution && 
				((BasicProbDistribution)event.content).values instanceof FormulaValues) {
			for (FormulaProbPair pair : ((FormulaValues)((BasicProbDistribution)event.content).values).values) {
				obs.addAlternative(pair.val, pair.prob);
			}
		}

		return nextAction(obs);
	} 



	/**
	 * Jumps to the next appropriate action in the policy given a particular observation. 
	 * Two separate cases are distinguished:
	 * - if the dialogue policy has an outgoing observation edge starting from curNode and
	 *   whose content matches the content in obs, then assigns curNode to be the node pointed by 
	 *   the edge, and returns it
	 * - if no such observation edge is available from curNode, then returns a void action,
	 *   and keep the curNode as it is
	 * 
	 * @param obs the observation           
	 * @return if a node jump is available at the current node with the given observation, returns
	 *         the next action.   Else, returns a void action         
	 * @throws DialogueException exception thrown if the policy or the observation is ill-formed
	 */
	public PolicyAction nextAction (Observation obs) throws DialogueException {

		// get matching edges
		Collection<PolicyEdge> matchingEdges = curNode.getMatchingEdges(obs);
		
		// sort the edges by preferential order
		List<PolicyEdge> sortedEdges = PolicyUtils.sortEdges(matchingEdges);
		if (sortedEdges.size() > 0) {

			// select the best edge
			PolicyEdge selectedEdge = sortedEdges.get(0);
			curNode = selectedEdge.getOutgoingAction();
			
			// if the outgoing action is underspecified, fill the arguments
			if (curNode.getAction().isUnderspecified()) {
				curNode.getAction().fillActionArguments(PolicyUtils.extractFilledArguments(obs, selectedEdge));
			} 
			
			// and return the action to perform
			return curNode.getAction();
		}

		debug("Warning: observation " + obs.toString() + " not applicable from node " + curNode.getId());
		debug("Policy: " + policy.toString());
	
		// else, return a void action
		return new PolicyAction();
	}



	/**
	 * Returns the action encapsulated in the current node
	 * 
	 * @return the current action
	 */
	public PolicyAction getCurrentAction () {
		return curNode.getAction();
	}


	/**
	 * Returns true if the manager reached a final state in the dialogue policy, 
	 * false otherwise
	 * 
	 * @return true if the policy is finished, false otherwise
	 */
	public boolean isFinished() {
		return policy.isFinalNode(curNode);
	}

	/**
	 * Returns true if the manager is set on the initial state of the dialogue
	 * policy, false otherwise
	 * 
	 * @return true if the policy is starting, false otherwise
	 */
	public boolean isStarting() {
		return policy.isInitNode(curNode);
	}



	/**
	 * Logging
	 * @param s
	 */
	private static void log (String s) {
		if (LOGGING) {
			System.out.println("[dialmanager] " + s);
		}
	}

	/**
	 * Debugging
	 * @param s
	 */
	private static void debug (String s) {
		if (DEBUG) {
			System.out.println("[dialmanager] " + s);
		}
	}
}
