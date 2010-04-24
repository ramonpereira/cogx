package binder.components;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Vector;

import beliefmodels.arch.BeliefException;
import beliefmodels.autogen.beliefs.Belief;
import beliefmodels.autogen.beliefs.MultiModalBelief;
import beliefmodels.autogen.beliefs.TemporalUnionBelief;
import beliefmodels.autogen.distribs.BasicProbDistribution;
import beliefmodels.autogen.distribs.FeatureValueProbPair;
import beliefmodels.autogen.featurecontent.PointerValue;
import beliefmodels.autogen.history.CASTBeliefHistory;
import beliefmodels.builders.TemporalUnionBuilder;
import beliefmodels.utils.DistributionUtils;
import beliefmodels.utils.FeatureContentUtils;
import binder.abstr.MarkovLogicComponent;
import binder.arch.BindingWorkingMemory;
import cast.SubarchitectureComponentException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryAddress;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTData;

/**
 * Perceptual grouping operation, responsible for merging percept beliefs from
 * different modalities into percept union beliefs.
 * 
 * The component continuously monitors changes on the binder working memory, and
 * triggers its internal inference mechanism (based on a Markov Logic Network)
 * when a percept is being inserted, updated or deleted. The final outcome of
 * this inference is the creation of new percept union beliefs which are then
 * inserted onto the working memory, as well as the associated update of the
 * existing percept union beliefs.
 * 
 * 
 * 
 * NOTE: - still need to check subarchitecture consistency OK - still need to
 * add filters OK - only perform updates on existing unions when change is
 * significant OK - need to add functionality for percept updates or deletions
 * OK? - remove the testing stuff and have a proper, separate tester class OK -
 * actually build the union content OK - change the belief history to have only
 * cast values OK - testing, defensive programming, check null values and
 * pre/post conditions - when constructing a new belief, propagate the pointers
 * correctly OK - have proper logging functionality - implement the same kind of
 * functionality for tracking - send examples of test cases to Sergio OK - do
 * the test with percepts instead of unions as inputs OK
 * 
 * @author plison
 * 
 */
public class Tracking_MLN extends MarkovLogicComponent<MultiModalBelief> {

	
	public Tracking_MLN() {
		super(new MultiModalBelief());
		MLNFile = markovlogicDir + "tracking.mln";
		resultsFile = markovlogicDir + "tracking.results";
		correlationsFile = markovlogicDir + "tracking/similarities.mln";
		predicatesFile = markovlogicDir
				+ "tracking/correlations_predicates.mln";
	}

	

	/**
	 * Add a change filter on the insertion of new percept beliefs on the binder working memory
	 */
	public void start() {
		// Insertion
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(MultiModalBelief.class,
						WorkingMemoryOperation.ADD), new WorkingMemoryChangeReceiver() {
					@SuppressWarnings("unchecked")
					public void workingMemoryChanged(WorkingMemoryChange _wmc) {	
						try {
							CASTData<MultiModalBelief> beliefData = getMemoryEntryWithData(_wmc.address, MultiModalBelief.class);

							log("received a new percept: " + beliefData.getID());
							performInference(beliefData.getData(), _wmc.address);
							log("grouping operation on percept " + beliefData.getID() + " now finished");
						}
						catch (Exception e) {
							e.printStackTrace();
						}
					}
				}
		);

		// Deletion
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(MultiModalBelief.class,
						WorkingMemoryOperation.DELETE), new WorkingMemoryChangeReceiver() {
					public void workingMemoryChanged(WorkingMemoryChange _wmc) {	
						try {
							workingMemoryChangeDelete(_wmc.address);
						}
						catch (Exception e) {
							e.printStackTrace();
						}
					}
				}
		);

		// Update
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(MultiModalBelief.class,
						WorkingMemoryOperation.OVERWRITE), new WorkingMemoryChangeReceiver() {
					@SuppressWarnings("unchecked")
					public void workingMemoryChanged(WorkingMemoryChange _wmc) {	
						try {
							workingMemoryChangeDelete(_wmc.address);
							CASTData<MultiModalBelief> beliefData = getMemoryEntryWithData(_wmc.address, MultiModalBelief.class);

							if (!beliefData.getID().equals(beliefUpdateToIgnore)) {
							log("received a new percept: " + beliefData.getID());
							MultiModalBelief belief = beliefData.getData();
							performInference(belief, _wmc.address);
							log("grouping operation on percept " + beliefData.getID() + " now finished");
							}	
						}
						catch (Exception e) {
							e.printStackTrace();
						}
					}
				}
		);
	}

	

	/**
	 * Create a set of new mmbelief union beliefs from the inference results,
	 * associated with the original mmbelief
	 * 
	 * @param mmbelief
	 * @param linkToExistingUnions
	 * @param inferenceResults
	 * @return
	 */
	@Override
	protected Vector<Belief> createNewUnions(MultiModalBelief mmbelief,
			WorkingMemoryAddress mmbeliefWMAddress,
			Map<String, Belief> relevantUnions,
			Map<String, String> unionsMapping, String newSingleUnionId,
			Map<String, Float> inferenceResults) throws BeliefException {

		// extract the existence probability of the mmbelief
		float mmbeliefExistProb = DistributionUtils
				.getExistenceProbability(mmbelief);
		Vector<Belief> newUnions = new Vector<Belief>();
		for (String id : unionsMapping.keySet()) {

			if (!inferenceResults.containsKey(id)) {
				throw new BeliefException("ERROR, id " + id
						+ " is not in inferenceResults.  inferenceResults = "
						+ inferenceResults.keySet().toString());
			}

			else if (!relevantUnions.containsKey(unionsMapping.get(id))) {
				throw new BeliefException("ERROR, existing union id "
						+ unionsMapping.get(id) + " is not in existingUnions");
			}

			float prob = mmbeliefExistProb * inferenceResults.get(id);
			log("computed probability for new mmbelief union " + id + ": "
					+ prob);

			Belief existingUnion = relevantUnions.get(unionsMapping.get(id));
			// try {
			List<WorkingMemoryAddress> addresses = new LinkedList<WorkingMemoryAddress>();
			addresses.add(mmbeliefWMAddress);
			addresses.add(new WorkingMemoryAddress(existingUnion.id,
					BindingWorkingMemory.BINDER_SA));
			TemporalUnionBelief newUnion = TemporalUnionBuilder
					.createNewDoubleUnionBelief(mmbelief, addresses,
							existingUnion, prob, id);
			newUnions.add(newUnion);
			// }
			/**
			 * catch (BeliefException e) { e.printStackTrace(); }
			 */
		}

		if (!inferenceResults.containsKey(newSingleUnionId)) {
			throw new BeliefException("ERROR, id " + newSingleUnionId
					+ " is not in inferenceResults");
		}
		TemporalUnionBelief newSingleUnion = TemporalUnionBuilder
				.createNewSingleUnionBelief(mmbelief, mmbeliefWMAddress,
						inferenceResults.get(newSingleUnionId)
								* mmbeliefExistProb, newSingleUnionId);

		newUnions.add(newSingleUnion);

		return newUnions;
	}

	@Override
	protected void updatePointersInOtherBeliefs(Belief newBelief) {
		try {

			if (newBelief.hist != null
					&& newBelief.hist instanceof CASTBeliefHistory) {

				for (WorkingMemoryAddress ancestor : ((CASTBeliefHistory) newBelief.hist).ancestors) {

					CASTData<TemporalUnionBelief>[] tunions = getWorkingMemoryEntries(TemporalUnionBelief.class);

					for (int i = 0; i < tunions.length; i++) {
						TemporalUnionBelief tunion = tunions[i].getData();
						for (FeatureValueProbPair pointerValueInUnion : FeatureContentUtils
								.getAllPointerValuesInBelief(tunion)) {
							PointerValue val = (PointerValue) pointerValueInUnion.val;
							if (val.beliefId.equals(ancestor)) {
								Belief ancestorBelief = getMemoryEntry(
										ancestor, Belief.class);
								if (!ancestorBelief.getClass().equals(
										TemporalUnionBelief.class)) {
									((PointerValue) pointerValueInUnion.val).beliefId = new WorkingMemoryAddress(
											newBelief.id,
											BindingWorkingMemory.BINDER_SA);
								} else {
									// dirty hacks here...

									// here, reducing the probability of the
									// existing link
									float oldProb = pointerValueInUnion.prob;
									pointerValueInUnion.prob = (DistributionUtils
											.getExistenceProbability(ancestorBelief) - DistributionUtils
											.getExistenceProbability(newBelief))
											/ DistributionUtils
													.getExistenceProbability(ancestorBelief);

									log("link from "
											+ tunion.id
											+ " to "
											+ ((PointerValue) pointerValueInUnion.val).beliefId.id
											+ " has prob. reduced from "
											+ oldProb + " to "
											+ pointerValueInUnion.prob);

									BasicProbDistribution pointerDistrib = FeatureContentUtils
											.getFeatureForFeatureValueInBelief(
													tunion,
													pointerValueInUnion.val);

									if (pointerDistrib != null) {
										// and adding a second link as well
										FeatureContentUtils
												.addAnotherValueInBasicProbDistribution(
														pointerDistrib,
														new FeatureValueProbPair(
																new PointerValue(
																		new WorkingMemoryAddress(
																				newBelief.id,
																				BindingWorkingMemory.BINDER_SA)),
																DistributionUtils
																		.getExistenceProbability(newBelief)
																		/ DistributionUtils
																				.getExistenceProbability(ancestorBelief)));

										log("creation of new link from "
												+ tunion.id
												+ " to "
												+ newBelief.id
												+ " with prob. "
												+ DistributionUtils
														.getExistenceProbability(newBelief)
												/ DistributionUtils
														.getExistenceProbability(ancestorBelief));

									}
								}
								updateBeliefOnWM(tunion);
							}
						}
					}
				}
			}

		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	@Override
	public void workingMemoryChangeDelete(WorkingMemoryAddress WMAddress) {
		deleteAllMultiModalBeliefAttachedToUnion(WMAddress);
	}

	/**
	 * Exact the set of existing unions from the binder working memory
	 * 
	 * @return the set of existing unions (as a mapping from identifier to
	 *         objects)
	 */
	protected HashMap<String, Belief> extractExistingUnions() {

		HashMap<String, Belief> existingunions = new HashMap<String, Belief>();

		try {
			CASTData<TemporalUnionBelief>[] unions;

			unions = getWorkingMemoryEntries(BindingWorkingMemory.BINDER_SA,
					TemporalUnionBelief.class);

			for (int i = (unions.length - 1); i >= 0; i--) {
				existingunions.put(unions[i].getData().id, unions[i].getData());
			}
		} catch (UnknownSubarchitectureException e) {
			log("Problem with architecture name!");
		} catch (SubarchitectureComponentException e) {
			e.printStackTrace();
		}
		return existingunions;
	}

	@Override
	protected Belief createNewSingleUnionBelief(MultiModalBelief belief,
			WorkingMemoryAddress beliefWMAddress) throws BeliefException {
		TemporalUnionBelief union = TemporalUnionBuilder.createNewSingleUnionBelief(belief, beliefWMAddress, newDataID());
		return union;
	}
}
