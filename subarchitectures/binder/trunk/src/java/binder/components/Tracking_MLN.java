package binder.components;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.Map.Entry;

import beliefmodels.arch.BeliefException;
import beliefmodels.autogen.beliefs.Belief;
import beliefmodels.autogen.beliefs.MultiModalBelief;
import beliefmodels.autogen.beliefs.PerceptBelief;
import beliefmodels.autogen.beliefs.PerceptUnionBelief;
import beliefmodels.autogen.beliefs.TemporalUnionBelief;
import beliefmodels.autogen.history.CASTBeliefHistory;
import beliefmodels.builders.PerceptUnionBuilder;
import beliefmodels.builders.TemporalUnionBuilder;
import beliefmodels.utils.DistributionUtils;
import binder.abstr.MarkovLogicComponent;
import binder.arch.BindingWorkingMemory;
import binder.ml.MLException;
import binder.utils.MLNGenerator;
import binder.utils.MLNPreferences;
import cast.SubarchitectureComponentException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryAddress;
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
 * - still need to check subarchitecture consistency OK
 * - still need to add filters OK
 * - only perform updates on existing unions when change is significant OK
 * - need to add functionality for percept updates or deletions OK?
 * - remove the testing stuff and have a proper, separate tester class  OK
 * - actually build the union content OK
 * - change the belief history to have only cast values OK
 * - testing, defensive programming, check null values and pre/post conditions
 * - when constructing a new belief, propagate the pointers correctly OK
 * - have proper logging functionality
 * - implement the same kind of functionality for tracking 
 * - send examples of test cases to Sergio OK
 * - do the test with percepts instead of unions as inputs OK
 * 
 * @author plison
 *
 */ 
public class Tracking_MLN extends MarkovLogicComponent<MultiModalBelief> {


	public Tracking_MLN() {
		super(new MultiModalBelief());
		MLNFile = markovlogicDir + "tracking.mln";
		resultsFile = markovlogicDir + "tracking.results";
	}

	/**
	 * Perform the tracking operation for the given belief, and subsequently update
	 * the working memory with new temporal unions (and possibly also with updates on existing ones)
	 * 
	 * @param mmbelief the new mmbelief which was inserted
	 */
	public void performTracking(MultiModalBelief mmbelief, WorkingMemoryAddress mmbeliefWMAddress) throws BeliefException {

		log("now starting tracking...");

		// extract the unions already existing in the binder WM
		HashMap<String, Belief> existingUnions = extractExistingUnions();

		HashMap<String, Belief> relevantUnions = selectRelevantUnions(existingUnions, mmbelief);

		if (relevantUnions.size() > 0) {
			// Create identifiers for each possible new union		
			HashMap<String,String> unionsMapping = new HashMap<String,String>();
			for (String existingUnionId : relevantUnions.keySet()) {
				String newUnionId = newDataID();
				unionsMapping.put(newUnionId, existingUnionId);
			}
			log("newly created union ids: " + unionsMapping.keySet().toString());

			String newSingleUnionId = newDataID();
			//	unionsMapping.put("P", newSingleUnionId);

			// Write the markov logic network to a file
			try {
				MLNPreferences prefs = new MLNPreferences();
				prefs.setFile_correlations(MLNPreferences.markovlogicDir + "tracking/similarities.mln");
				prefs.setFile_predicates(MLNPreferences.markovlogicDir + "tracking/correlations_predicates.mln");
				MLNGenerator gen = new MLNGenerator(prefs);
				gen.writeMLNFile(mmbelief, relevantUnions.values(), unionsMapping, newSingleUnionId, MLNFile);
			} catch (MLException e1) {
				e1.printStackTrace();
			}

			// run the alchemy inference
			try { 
				HashMap<String,Float> inferenceResults = runAlchemyInference(MLNFile, resultsFile);

				log("filtering inference results to keep only the " + maxAlternatives + " best alternatives");
				HashMap<String,Float> filteredInferenceResults = filterInferenceResults(inferenceResults, maxAlternatives);

				// create the new unions given the inference results
				Vector<Belief> newUnions = createNewUnions(mmbelief, mmbeliefWMAddress, relevantUnions,
						unionsMapping, newSingleUnionId, filteredInferenceResults);

				// and add them to the working memory
				for (Belief newUnion : newUnions) {
					if (DistributionUtils.getExistenceProbability(newUnion) > lowestProbThreshold)  {
						insertBeliefInWM(newUnion);
						log("inserting belief " + newUnion.id + " on WM");
					}
					else {
						log ("Belief " + newUnion.id + " has probability " + DistributionUtils.getExistenceProbability(newUnion) +
						", which is lower than the minimum threshold.  Not inserting");
					}
				}

				// modify the existence probabilities of the existing unions
				List<Belief> unionsToUpdate = getExistingUnionsToUpdate(mmbelief, relevantUnions, unionsMapping, filteredInferenceResults);

				// and update them on the working memory
				for (Belief unionToUpdate : unionsToUpdate) {
					if (DistributionUtils.getExistenceProbability(unionToUpdate) > lowestProbThreshold)  {
						log("updating belief " + unionToUpdate.id + " on WM");
						updateBeliefOnWM(unionToUpdate);
					}
					else  {
						log("deleting belief: " + unionToUpdate.id + " on WM");
						deleteBeliefOnWM(unionToUpdate.id);
					}
				}

			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}
		else {
			try {
				log("no relevant union to group with mmbelief " + mmbelief.id + " has been found");
				TemporalUnionBelief union = TemporalUnionBuilder.createNewSingleUnionBelief(mmbelief, mmbeliefWMAddress, newDataID());
				if (DistributionUtils.getExistenceProbability(union) > lowestProbThreshold)  {
					insertBeliefInWM(union);
					log("inserting belief " + union.id + " on WM");
				}
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}
	}


	/**
	 * Create a set of new mmbelief union beliefs from the inference results, associated with the 
	 * original mmbelief
	 * @param mmbelief
	 * @param linkToExistingUnions
	 * @param inferenceResults
	 * @return
	 */
	private Vector<Belief> createNewUnions(
			MultiModalBelief mmbelief,
			WorkingMemoryAddress mmbeliefWMAddress,
			HashMap<String,Belief> existingUnions,
			HashMap<String,String> unionsMapping,
			String newSingleUnionId,
			HashMap<String,Float> inferenceResults) throws BeliefException {

		// extract the existence probability of the mmbelief
		float mmbeliefExistProb = DistributionUtils.getExistenceProbability(mmbelief);

		Vector<Belief> newUnions = new Vector<Belief>();
		for (String id : unionsMapping.keySet()) {

			if (!inferenceResults.containsKey(id)) {
				throw new BeliefException("ERROR, id " + id + " is not in inferenceResults.  inferenceResults = " + inferenceResults.keySet().toString());
			}

			else if (!existingUnions.containsKey(unionsMapping.get(id))) {
				throw new BeliefException("ERROR, existing union id " + unionsMapping.get(id) + " is not in existingUnions");
			}

			float prob = mmbeliefExistProb * inferenceResults.get(id);
			log("computed probability for new mmbelief union " + id + ": " + prob);

			Belief existingUnion = existingUnions.get(unionsMapping.get(id)); 
			//	try {
			List<WorkingMemoryAddress> addresses = new LinkedList<WorkingMemoryAddress>();
			addresses.add(mmbeliefWMAddress);
			addresses.add(new WorkingMemoryAddress(existingUnion.id, BindingWorkingMemory.BINDER_SA));
			TemporalUnionBelief newUnion = TemporalUnionBuilder.createNewDoubleUnionBelief(mmbelief, addresses, existingUnion, prob, id);
			newUnions.add(newUnion);
			//	}
			/**	catch (BeliefException e) {
				e.printStackTrace();
			} */
		} 

		if (!inferenceResults.containsKey(newSingleUnionId)) {
			throw new BeliefException("ERROR, id " + newSingleUnionId + " is not in inferenceResults");
		}
		TemporalUnionBelief newSingleUnion = 
			TemporalUnionBuilder.createNewSingleUnionBelief(mmbelief, mmbeliefWMAddress, inferenceResults.get(newSingleUnionId), newSingleUnionId);

		newUnions.add(newSingleUnion);

		return newUnions;
	}


	/**
	 * Modify the existence probability of existing unions, based on the inference results, 
	 * the newly inserted mmbelief, the set of existing unions, and the mapping between the new 
	 * and the already existing unions
	 * 
	 * @param mmbelief the mmbelief
	 * @param existingUnions the set of existing unions
	 * @param linkToExistingUnions mapping from new unions to existing unions they include
	 * @param inferenceResults the inference results
	 */
	private List<Belief> getExistingUnionsToUpdate (
			MultiModalBelief mmbelief, 
			HashMap<String,Belief> existingUnions, 
			HashMap<String,String> linkToExistingUnions,
			HashMap<String,Float> inferenceResults) {

		List<Belief> unionsToUpdate = new LinkedList<Belief>();

		// extract the existence probability of the mmbelief
		float mmbeliefExistProb = DistributionUtils.getExistenceProbability(mmbelief);

		for (String newUnionId: linkToExistingUnions.keySet()) {
			Belief existingUnion = existingUnions.get(linkToExistingUnions.get(newUnionId));
			float unionCurrentExistProb = DistributionUtils.getExistenceProbability(existingUnion);
			float unionNewExistProb = (unionCurrentExistProb * (1-mmbeliefExistProb)) + 
			(mmbeliefExistProb * (1 - inferenceResults.get(newUnionId)) * unionCurrentExistProb);
			log("new prob for " +  linkToExistingUnions.get(newUnionId) + ": " + unionNewExistProb);

			try {
				DistributionUtils.setExistenceProbability(existingUnion, unionNewExistProb);
			}
			catch (Exception e) {
				e.printStackTrace();
			}
			if (Math.abs(unionNewExistProb - unionCurrentExistProb) > minProbDifferenceForUpdate) {
				unionsToUpdate.add(existingUnion);
				log("Existing union " + existingUnion.id + " going from existence probability " + unionCurrentExistProb + 
						" to probabibility " + unionNewExistProb + ", update necessary");
			}
		}
		return unionsToUpdate;
	}  

	private HashMap<String,Belief> selectRelevantUnions (HashMap<String,Belief> existingUnions, 
			MultiModalBelief mmbelief) throws BeliefException {

		HashMap<String,Belief> relevantUnions = new HashMap<String,Belief>();

		if (((CASTBeliefHistory)mmbelief.hist).ancestors.size() == 0) {
			throw new BeliefException ("ERROR: mmbelief history contains 0 element");
		}


		for (String mmbeliefOrigin : getOriginSubarchitectures(mmbelief)) {

			for (String existingUnionId : existingUnions.keySet()) {

				Belief existingUnion = existingUnions.get(existingUnionId);
				List<String> existinUnionOrigins = getOriginSubarchitectures(existingUnion);

				if (existinUnionOrigins.contains(mmbeliefOrigin)) {
					relevantUnions.put(existingUnionId, existingUnion);
				}

			}
		}


		return relevantUnions;
	}

	/**
	 * Exact the set of existing unions from the binder working memory
	 * 
	 * @return the set of existing unions (as a mapping from identifier to objects)
	 */
	private HashMap<String, Belief> extractExistingUnions() {

		HashMap<String, Belief> existingunions = new HashMap<String, Belief>();

		try {
			CASTData<TemporalUnionBelief>[] unions;

			unions = getWorkingMemoryEntries(BindingWorkingMemory.BINDER_SA, TemporalUnionBelief.class);

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


	private void deleteAllStableBeliefAttachedToUnion (WorkingMemoryAddress unionAddress) {

		try {
			CASTData<MultiModalBelief>[] mmBeliefs;

			mmBeliefs = getWorkingMemoryEntries(BindingWorkingMemory.BINDER_SA, MultiModalBelief.class);

			for (int i = (mmBeliefs.length - 1) ; i >= 0 ; i--) {
				MultiModalBelief mmbelief = mmBeliefs[i].getData();
				if (mmbelief != null && mmbelief.hist != null && mmbelief.hist instanceof CASTBeliefHistory) {
					if (((CASTBeliefHistory)mmbelief.hist).ancestors.contains(unionAddress)) {
						deleteBeliefOnWM (mmBeliefs[i].getID());
					}
				}
			}
		}
		catch (UnknownSubarchitectureException e) {
			log("Problem with architecture name!");
		}
		catch (SubarchitectureComponentException e) {
			e.printStackTrace();
		}
	}



	@Override
	public void workingMemoryChangeDelete(WorkingMemoryAddress WMAddress) {
		deleteAllStableBeliefAttachedToUnion(WMAddress);
	}



	@Override
	public void workingMemoryChangeInsert(Belief belief, WorkingMemoryAddress WMAddress) throws BeliefException {
		performTracking((MultiModalBelief)belief, WMAddress);
	}


}
