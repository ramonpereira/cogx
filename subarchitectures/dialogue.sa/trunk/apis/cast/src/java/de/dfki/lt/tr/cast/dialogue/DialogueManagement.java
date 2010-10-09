package de.dfki.lt.tr.cast.dialogue;

import java.util.Map;


import cast.AlreadyExistsOnWMException;
import cast.DoesNotExistOnWMException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.ManagedComponent;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import de.dfki.lt.tr.beliefs.slice.distribs.BasicProbDistribution;
import de.dfki.lt.tr.beliefs.slice.distribs.FormulaProbPair;
import de.dfki.lt.tr.beliefs.slice.distribs.FormulaValues;
import de.dfki.lt.tr.beliefs.slice.epstatus.AttributedEpistemicStatus;
import de.dfki.lt.tr.beliefs.slice.epstatus.PrivateEpistemicStatus;
import de.dfki.lt.tr.beliefs.slice.events.Event;
import de.dfki.lt.tr.beliefs.slice.intentions.CommunicativeIntention;
import de.dfki.lt.tr.beliefs.slice.intentions.Intention;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.ModalFormula;
import de.dfki.lt.tr.beliefs.slice.logicalcontent.dFormula;
import de.dfki.lt.tr.beliefs.slice.sitbeliefs.dBelief;
import de.dfki.lt.tr.dialmanagement.arch.DialogueException;
import de.dfki.lt.tr.dialmanagement.components.DialogueManager;
import de.dfki.lt.tr.dialmanagement.data.policies.PolicyAction;
import de.dfki.lt.tr.dialmanagement.utils.EpistemicObjectUtils;
import de.dfki.lt.tr.dialmanagement.utils.FormulaUtils;
import de.dfki.lt.tr.dialmanagement.utils.TextPolicyReader;
import de.dfki.lt.tr.dialmanagement.utils.XMLPolicyReader;
import de.dfki.lt.tr.dialogue.interpret.IntentionManagementConstants;
import java.util.HashMap;



/**
 * CAST wrapper for the dialogue manager.  Listens to new intentions and events inserted onto the Working Memory,
 * and compute the next appropriate action, if any is available.  
 * 
 * @author Pierre Lison
 * @version 08/07/2010
 *
 */
public class DialogueManagement extends ManagedComponent {

	// the dialogue manager
	DialogueManager manager;

	// default parameters for the dialogue manager
	String policyFile = "subarchitectures/dialogue.sa/config/policies/yr2/basicforwardpolicy.xml";


	/**
	 * Construct a new dialogue manager, with default values
	 * 
	 */
	public DialogueManagement() {
		try {
			manager = new DialogueManager(XMLPolicyReader.constructPolicy(policyFile));
		}
		catch (DialogueException e) {
			e.printStackTrace();
		} 
	} 


	/**
	 * Configuration method.  If the parameters --policy, --actions and --observations are given, 
	 * use their values as parameters to the FSA-based dialogue manager.  If these values
	 * are not given, the default ones are used
	 * 
	 */
	@Override
	public void configure(Map<String, String> _config) {
		if ((_config.containsKey("--policy"))) {
			try {
				log("Provided parameters: policy=" + _config.get("--policy"));
				policyFile = _config.get("--policy");
				manager = new DialogueManager(XMLPolicyReader.constructPolicy(policyFile));
			} catch (DialogueException e) {
				log(e.getMessage());
				e.printStackTrace();
			} 	
		}
	}



	@Override
	public void start() {

		// communicative intentions
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(CommunicativeIntention.class,  WorkingMemoryOperation.ADD),
				new WorkingMemoryChangeReceiver() {

					public void workingMemoryChanged(
							WorkingMemoryChange _wmc) {
						try {
							CommunicativeIntention initIntention = getMemoryEntry(_wmc.address, CommunicativeIntention.class);
							if (initIntention.intent.estatus instanceof AttributedEpistemicStatus) {
								newIntentionReceived(initIntention);
							}
						} catch (DoesNotExistOnWMException e) {
							e.printStackTrace();
						} catch (UnknownSubarchitectureException e) {
							e.printStackTrace();
						}
					}
				});


		// (non-communicative) intentions
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(Intention.class,  WorkingMemoryOperation.ADD),
				new WorkingMemoryChangeReceiver() {

					public void workingMemoryChanged(
							WorkingMemoryChange _wmc) {
						try {
							Intention initIntention = getMemoryEntry(_wmc.address, Intention.class);
							if (initIntention.estatus instanceof PrivateEpistemicStatus) {
								addToWorkingMemory(newDataID(), new CommunicativeIntention(initIntention));
							}
						} catch (DoesNotExistOnWMException e) {
							e.printStackTrace();
						} catch (UnknownSubarchitectureException e) {
							e.printStackTrace();
						}
						catch (AlreadyExistsOnWMException e) {
							e.printStackTrace();
						} 
					}
				}); 


		// events
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(Event.class,  WorkingMemoryOperation.ADD),
				new WorkingMemoryChangeReceiver() {

					public void workingMemoryChanged(
							WorkingMemoryChange _wmc) {
						try {
							newEventReceived(getMemoryEntry(_wmc.address, Event.class));
						} catch (DoesNotExistOnWMException e) {
							e.printStackTrace();
						} catch (UnknownSubarchitectureException e) {
							e.printStackTrace();
						}
					}
				});
	}




	/**
	 * If a new intention is added, triggers the dialogue manager to determine 
	 * the next appropriate action, if any is available.  
	 * 
	 * If an intention action is returned, create a new private intention and 
	 * inserts it into the working memory
	 * 
	 * @param intention the (attributed) intention received as observation
	 * 
	 */
	public void newIntentionReceived (CommunicativeIntention intention) {
		try {
			
			// create an "augmented" intention based on the received one
			CommunicativeIntention augmentedIntention = createAugmentedIntention(intention);
			String formAsString = FormulaUtils.getString(augmentedIntention.intent.content.get(0).postconditions);
			debug("augmented intention: " + formAsString);	

			// running the dialogue manager to select the next action
			PolicyAction action = manager.nextAction(augmentedIntention);
			log("action chosen: " + action.toString());

			// if the action is not void, adds the new intention to the WM
			if (!action.isVoid()) {
				
				// if it is a communicative intention
				if (action.getType() == PolicyAction.COMMUNICATIVE_INTENTION) {
					log("creating a new communicative intention based on the dialogue manager selection");
					CommunicativeIntention response = 
					EpistemicObjectUtils.createSimplePrivateCommunicativeIntention((action).getContent(), 1.0f); 
					addToWorkingMemory(newDataID(), response);
					log("new private communicative intention successfully added to working memory");
				}
				
				// and if it is a (non-communicative) intention
				else if (action.getType() == PolicyAction.INTENTION) {
					if (FormulaUtils.subsumes(augmentedIntention.intent.content.get(0).postconditions, action.getContent())) {
						
						// we are dealing with a simple forwarding action	
						log("simply forwarding the communicative action beyond dialogue.sa");
						addToWorkingMemory(intention.intent.id, intention.intent);
					}
					else {
						log("creating a new intention based on the dialogue manager selection");
						Intention response = 
							EpistemicObjectUtils.createSimplePrivateIntention((action).getContent(), 1.0f); 
							addToWorkingMemory(newDataID(), response);
					}
		
					log("new private intention successfully added to working memory");
				}
			}

		} catch (Exception e) {
			log(e.getMessage());
			e.printStackTrace();
		}
	}


	/**
	 * If a new event is added, triggers the dialogue manager to determine the next appropriate
	 * action, if any is available.  If an intention action is returned, create a new private intention
	 * and inserts it into the working memory
	 * 
	 * @param event the event received as observation
	 * 
	 */
	public void newEventReceived (Event event) {
		try {
			
			// running the dialogue manager to select the next action
			PolicyAction action = manager.nextAction(event);
			log("action chosen: " + action.toString());

			// if the action is not void, adds the new intention to the WM
			if (!action.isVoid()) {
				CommunicativeIntention response = 
					EpistemicObjectUtils.createSimplePrivateCommunicativeIntention((action).getContent(), 1.0f);
				addToWorkingMemory(newDataID(), response);
				log("new private intention successfully added to working memory");
			}

		} catch (DialogueException e) {
			log(e.getMessage());
			e.printStackTrace();
		} catch (AlreadyExistsOnWMException e) {
			e.printStackTrace();
		}
	}


	/**
	 * Create a new intention based on the content of the init intention, by
	 * replace pointers by the content of the pointed epistemic object
	 * 
	 * @param initIntention the initial intention
	 * @return
	 * @throws DialogueException if formatting problems
	 * @throws DoesNotExistOnWMException if the pointers relate to non-existent entities on the WM
	 */
	public CommunicativeIntention createAugmentedIntention (CommunicativeIntention initIntention) 
	throws DialogueException, DoesNotExistOnWMException {

		if (initIntention!=null && initIntention.intent.content.size() > 0)  {

			if (initIntention.intent.content.get(0).postconditions instanceof ModalFormula &&
					((ModalFormula)initIntention.intent.content.get(0).postconditions).op.equals(
							IntentionManagementConstants.beliefLinkModality)) {

				// extracting the pointer content
				String beliefID = FormulaUtils.getString(
						((ModalFormula)initIntention.intent.content.get(0).postconditions).form);

				// checking that it exists on the working memory
				if (existsOnWorkingMemory(beliefID)) {
					dBelief b = getMemoryEntry(beliefID, dBelief.class);

					debug("type of distrib: " + b.content.getClass().getCanonicalName());

					// assuming it has a basic probdistribution
					if (b.content instanceof BasicProbDistribution &&
							((BasicProbDistribution)b.content).values instanceof FormulaValues) {

						// extracting the content pairs
						HashMap<dFormula,Float> mapPairs = new HashMap<dFormula,Float>();
						for (FormulaProbPair pair : ((FormulaValues)((BasicProbDistribution)b.content).values).values) {
							FormulaProbPair newPair = new FormulaProbPair(new ModalFormula(0, 
									IntentionManagementConstants.beliefLinkModality, pair.val), pair.prob);
							mapPairs.put(newPair.val, newPair.prob);
						}

						// create a forged intention with this content
						CommunicativeIntention forgedIntention = 
							EpistemicObjectUtils.createAttributedCommunicativeIntention(mapPairs);

						return forgedIntention;
					}
				}
			}
		}
		
		// else, return the same intention
		return initIntention;
	}

}
