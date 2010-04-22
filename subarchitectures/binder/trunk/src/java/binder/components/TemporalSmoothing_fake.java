package binder.components;

import java.util.LinkedList;
import java.util.List;

import beliefmodels.arch.BeliefException;
import beliefmodels.autogen.beliefs.Belief;
import beliefmodels.autogen.beliefs.MultiModalBelief;
import beliefmodels.autogen.beliefs.StableBelief;
import beliefmodels.autogen.beliefs.TemporalUnionBelief;
import beliefmodels.autogen.distribs.FeatureValueProbPair;
import beliefmodels.autogen.featurecontent.PointerValue;
import beliefmodels.autogen.history.CASTBeliefHistory;
import beliefmodels.builders.StableBeliefBuilder;
import beliefmodels.builders.TemporalUnionBuilder;
import beliefmodels.utils.FeatureContentUtils;
import binder.abstr.BeliefWriter;
import binder.arch.BindingWorkingMemory;
import cast.AlreadyExistsOnWMException;
import cast.DoesNotExistOnWMException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryAddress;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTData;

public class TemporalSmoothing_fake extends BeliefWriter {

	
	@Override
	public void start() {
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(TemporalUnionBelief.class,
						WorkingMemoryOperation.ADD), new WorkingMemoryChangeReceiver() {
					public void workingMemoryChanged(WorkingMemoryChange _wmc) {
						
						try {
							CASTData<TemporalUnionBelief> beliefData = getMemoryEntryWithData(_wmc.address, TemporalUnionBelief.class);
							
							StableBelief stableBelief = StableBeliefBuilder.createnewStableBelief(beliefData.getData(), _wmc.address, newDataID());

							updatePointersInCurrentBelief(stableBelief);
							updatePointersInOtherBeliefs(stableBelief);
								
							addOffspringToTStableBelief(beliefData.getData(), 
									new WorkingMemoryAddress(stableBelief.id, BindingWorkingMemory.BINDER_SA));	

							insertBeliefInWM(stableBelief);
								
						}	
			
						 catch (DoesNotExistOnWMException e) {
								e.printStackTrace();
							}
						 catch (UnknownSubarchitectureException e) {	
							e.printStackTrace();
						} 
						 catch (BeliefException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							} catch (AlreadyExistsOnWMException e) {
							// TODO Auto-generated catch block
							e.printStackTrace();
						}
					}
				}
		);
		
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(TemporalUnionBelief.class,
						WorkingMemoryOperation.OVERWRITE), new WorkingMemoryChangeReceiver() {
					public void workingMemoryChanged(WorkingMemoryChange _wmc) {
						
						try {
							CASTData<TemporalUnionBelief> beliefData = getMemoryEntryWithData(_wmc.address, TemporalUnionBelief.class);

							List<WorkingMemoryAddress> offspring = ((CASTBeliefHistory)beliefData.getData().hist).offspring;
							log("number of offspring for : " + beliefData.getData().id + ": "+ offspring.size());

							for (WorkingMemoryAddress child : offspring) {
								if (existsOnWorkingMemory(child)) {
									StableBelief childBelief = getMemoryEntry(child, StableBelief.class);
									childBelief =StableBeliefBuilder.createnewStableBelief(beliefData.getData(), _wmc.address, childBelief.id);
									updateBeliefOnWM(childBelief);
								}
							}
							
						}	

						catch (Exception e) {
							e.printStackTrace();
						} 
					}
				}
		);
		
		addChangeFilter(
				ChangeFilterFactory.createLocalTypeFilter(TemporalUnionBelief.class,
						WorkingMemoryOperation.DELETE), new WorkingMemoryChangeReceiver() {
					public void workingMemoryChanged(WorkingMemoryChange _wmc) {
						
						try {
							CASTData<TemporalUnionBelief> beliefData = getMemoryEntryWithData(_wmc.address, TemporalUnionBelief.class);

							List<WorkingMemoryAddress> offspring = ((CASTBeliefHistory)beliefData.getData().hist).offspring;

							for (WorkingMemoryAddress child : offspring) {
								if (existsOnWorkingMemory(child)) {
									deleteBeliefOnWM(child.id);
								}
							}
							
						}	

						catch (Exception e) {
							e.printStackTrace();
						} 
					}
				}
		);
	}
	

	private void addOffspringToTStableBelief (TemporalUnionBelief tunionBelief, WorkingMemoryAddress addressStableBelief) {

		if (tunionBelief.hist != null && tunionBelief.hist instanceof CASTBeliefHistory) {
			((CASTBeliefHistory)tunionBelief.hist).offspring.add(addressStableBelief);
		}
		else {
			log("WARNING: offspring of tunion is ill-formed");
			tunionBelief.hist = new CASTBeliefHistory(new LinkedList<WorkingMemoryAddress>(), new LinkedList<WorkingMemoryAddress>());
			((CASTBeliefHistory)tunionBelief.hist).offspring.add(addressStableBelief);
		}
	}


	private void updatePointersInCurrentBelief (Belief newBelief) {
		try {

			for (FeatureValueProbPair pointer : FeatureContentUtils.getAllPointerValuesInBelief(newBelief)) {

				WorkingMemoryAddress point = ((PointerValue)pointer.val).beliefId ;
				if (existsOnWorkingMemory(point)) {
				Belief belief = getMemoryEntry(point, Belief.class);
				if (((CASTBeliefHistory)belief.hist).offspring.size() > 0) {
					((PointerValue)pointer.val).beliefId = ((CASTBeliefHistory)belief.hist).offspring.get(0);	
				}
				}
				else {
					log("warning: belief " + point.id + " does not exist yet on WM");
				}
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}

	}


	private void updatePointersInOtherBeliefs (Belief newBelief) {
		try {

			if (newBelief.hist != null && newBelief.hist instanceof CASTBeliefHistory) {

				for (WorkingMemoryAddress ancestor: ((CASTBeliefHistory)newBelief.hist).ancestors) {

					CASTData<StableBelief>[] stables = getWorkingMemoryEntries(StableBelief.class);

					for (int i = 0 ; i < stables.length ; i++ ) {
						StableBelief stable = stables[i].getData();
						for (FeatureValueProbPair pointerValueInUnion : FeatureContentUtils.getAllPointerValuesInBelief(stable)) {
							PointerValue val = (PointerValue)pointerValueInUnion.val;
							if (val.beliefId.equals(ancestor)) {
								((PointerValue)pointerValueInUnion.val).beliefId = 
									new WorkingMemoryAddress(newBelief.id, BindingWorkingMemory.BINDER_SA);
								updateBeliefOnWM(stable);
							}
						}
					}
				} 
			}


		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
}
