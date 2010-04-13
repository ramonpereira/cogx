package binder.components;

import java.util.HashMap;
import java.util.Vector;

import beliefmodels.arch.BeliefException;
import beliefmodels.autogen.beliefs.PerceptBelief;
import beliefmodels.autogen.beliefs.PerceptUnionBelief;
import beliefmodels.autogen.distribs.BasicProbDistribution;
import beliefmodels.builders.BeliefContentBuilder;
import beliefmodels.builders.PerceptUnionBuilder;
import beliefmodels.utils.DistributionUtils;
import binder.abstr.MarkovLogicComponent;
import binder.arch.BindingWorkingMemory;
import binder.utils.MLNGenerator;
import cast.AlreadyExistsOnWMException;
import cast.DoesNotExistOnWMException;
import cast.SubarchitectureComponentException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTData;


/**
 * Perceptual grouping operation, responsible for merging percept beliefs from different modalities into
 * percept union beliefs. 
 * 
 * The component continuously monitors changes on the binder working memory, and triggers its internal 
 * inference mechanism (based on a Markov Logic Network) when a percept is being inserted, updated or
 * deleted. The final outcome of this inference is the creation of new percept union beliefs which are
 * then inserted onto the working memory, as well as the associated update of the existing percept union
 * beliefs.
 * 
 * 
 * 
 * NOTE: 
 * - still need to check subarchitecture consistency
 * - still need to add filters
 * - only perform updates on existing unions when change is significant
 * - need to add functionality for percept updates or deletions
 * - remove the testing stuff and have a proper, separate tester class
 * - actually build the union content
 * 
 * @author plison
 *
 */
public class PerceptualGrouping_MLN extends MarkovLogicComponent {

	String MLNFile = markovlogicDir + "grouping.mln";
	String resultsFile = markovlogicDir + "unions.results";

	
	/**
	 * Add a change filter on the insertion of new percept beliefs on the binder working memory
	 */
	@Override
	public void start() {
		
		insertExistingUnionsForTesting();
		
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(PerceptBelief.class,
						WorkingMemoryOperation.ADD), new WorkingMemoryChangeReceiver() {
					public void workingMemoryChanged(WorkingMemoryChange _wmc) {	
						try {
							CASTData<PerceptBelief> beliefData = getMemoryEntryWithData(_wmc.address, PerceptBelief.class);	
							
							log("received a new percept: " + beliefData.getID());
							performPerceptualGrouping (beliefData.getData());
							log("perceptual grouping operation on percept " + beliefData.getID() + " now finished");
						}	
			
						 catch (DoesNotExistOnWMException e) {
								e.printStackTrace();
							}
						 catch (UnknownSubarchitectureException e) {	
							e.printStackTrace();
						} 	 
					}
				}
		);
	}

	
	/**
	 * Temporary method to add new unions
	 */
	private void insertExistingUnionsForTesting() {
		try {
			PerceptUnionBelief u1 = new PerceptUnionBelief();
			u1.content = BeliefContentBuilder.createNewDistributionWithExistDep(0.9f, new BasicProbDistribution());
			u1.id = newDataID();
			PerceptUnionBelief u2 = new PerceptUnionBelief();
			u2.content = BeliefContentBuilder.createNewDistributionWithExistDep(0.8f, new BasicProbDistribution());
			u2.id = newDataID();	
			PerceptUnionBelief u3 = new PerceptUnionBelief();
			u3.content = BeliefContentBuilder.createNewDistributionWithExistDep(0.05f, new BasicProbDistribution());
			u3.id = newDataID();
			addToWorkingMemory(u1.id, u1);
			addToWorkingMemory(u2.id, u2);
			addToWorkingMemory(u3.id, u3);
		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}

	
	
	/**
	 * Perform the perceptual grouping operation for the given belief, and subsequently update
	 * the working memory with new percept unions (and possibly also with updates on existing ones)
	 * 
	 * @param percept the new percept which was inserted
	 */
	public void performPerceptualGrouping(PerceptBelief percept) {
	
		log("now starting perceptual grouping...");

		// extract the unions already existing in the binder WM
		HashMap<String, PerceptUnionBelief> existingUnions = extractExistingUnions();
		
		// Create identifiers for each possible new union		
		HashMap<String,String> unionsMapping = new HashMap<String,String>();
		for (String existingUnionId : existingUnions.keySet()) {
			String newUnionId = newDataID();
			unionsMapping.put(newUnionId, existingUnionId);
		}
			
		// Write the markov logic network to a file
		MLNGenerator.writeMLNFile(percept, existingUnions.values(), unionsMapping, MLNFile);
		
		// run the alchemy inference
		HashMap<String,Float> inferenceResults = runAlchemyInference(MLNFile, resultsFile);
		
		// create the new unions given the inference results
		Vector<PerceptUnionBelief> newUnions = createNewUnions(percept, inferenceResults);

		// and add them to the working memory
		addNewUnionToWM(percept, newUnions);

		// modify the existence probabilities of the existing unions
		modifyExistingUnions(percept, existingUnions, unionsMapping, inferenceResults);
		
		// and update them on the working memory
		updateExistingUnionsInWM(existingUnions);
	}


	/**
	 * Update unions already existing on the binder working memory with updated content (mostly with
	 * updated existence probability)
	 * 
	 * @param existingUnions the unions to update on the working memory
	 * @pre the unions must already be present on the working memory, with the same identifier
	 */
	private void updateExistingUnionsInWM(HashMap<String, PerceptUnionBelief> existingUnions) {

		try {
			for (String unionId: existingUnions.keySet()) {
				updateBeliefOnWM(existingUnions.get(unionId));
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		} 
	}

	
	/**
	 * Add new unions to the binder working memory
	 * 
	 * @param b temporary parameter
	 * @param newUnions the set of new unions to add on the working memory
	 * @pre the unions (or at least their identifier) must not be already be present on 
	 * 		the working memory 
	 */
	private void addNewUnionToWM(PerceptBelief b, Vector<PerceptUnionBelief> newUnions) {
		try {
			PerceptUnionBelief union = PerceptUnionBuilder.createNewSingleUnionBelief(b, newDataID());
			insertBeliefInWM(union);
		}
		catch (BeliefException e) {
			e.printStackTrace();
		} catch (AlreadyExistsOnWMException e) {
			e.printStackTrace();
		}

	}


	/**
	 * Create a set of new percept union beliefs from the inference results, associated with the 
	 * original percept
	 * @param percept
	 * @param linkToExistingUnions
	 * @param inferenceResults
	 * @return
	 */
	private Vector<PerceptUnionBelief> createNewUnions(
			PerceptBelief percept, 
			HashMap<String,Float> inferenceResults) {

		// extract the existence probability of the percept
		float perceptExistProb = DistributionUtils.getExistenceProbability(percept);

		for (String id : inferenceResults.keySet()) {
			float prob = perceptExistProb * inferenceResults.get(id);
			log("prob of " + id + ": " + prob);
		}

		return new Vector<PerceptUnionBelief>();
	}
	
	
	/**
	 * Modify the existence probability of existing unions, based on the inference results, 
	 * the newly inserted percept, the set of existing unions, and the mapping between the new 
	 * and the already existing unions
	 * 
	 * @param percept the percept
	 * @param existingUnions the set of existing unions
	 * @param linkToExistingUnions mapping from new unions to existing unions they include
	 * @param inferenceResults the inference results
	 */
	private void modifyExistingUnions (
			PerceptBelief percept, 
			HashMap<String,PerceptUnionBelief> existingUnions, 
			HashMap<String,String> linkToExistingUnions,
			HashMap<String,Float> inferenceResults) {
		
		// extract the existence probability of the percept
		float perceptExistProb = DistributionUtils.getExistenceProbability(percept);
		
		for (String newUnionId: linkToExistingUnions.keySet()) {
			PerceptUnionBelief associatedExistingUnion = existingUnions.get(linkToExistingUnions.get(newUnionId));
			float unionCurrentExistProb = DistributionUtils.getExistenceProbability(associatedExistingUnion);
			float unionNewExistProb = (unionCurrentExistProb * (1-perceptExistProb)) + 
				(perceptExistProb * (1 - inferenceResults.get(newUnionId)) * unionCurrentExistProb);
			log("new prob for " +  linkToExistingUnions.get(newUnionId) + ": " + unionNewExistProb);
			
			DistributionUtils.setExistenceProbability(associatedExistingUnion, unionNewExistProb);
		}
		
	}
	
	
	/**
	 * Exact the set of existing unions from the binder working memory
	 * 
	 * @return the set of existing unions (as a mapping from identifier to objects)
	 */
	private HashMap<String, PerceptUnionBelief> extractExistingUnions() {

		HashMap<String, PerceptUnionBelief> existingunions = new HashMap<String, PerceptUnionBelief>();

		try {
			CASTData<PerceptUnionBelief>[] unions;

			unions = getWorkingMemoryEntries(BindingWorkingMemory.BINDER_SA, PerceptUnionBelief.class);

			for (int i = (unions.length - 1) ; i >= 0 ; i--) {
				existingunions.put(unions[i].getData().id, unions[i].getData());
			}
		}
		catch (UnknownSubarchitectureException e) {
			log("Problem with architecture name!");
		}
		catch (SubarchitectureComponentException e) {
			e.printStackTrace();
		}
		return existingunions;
	}
	
	
	
	
}
