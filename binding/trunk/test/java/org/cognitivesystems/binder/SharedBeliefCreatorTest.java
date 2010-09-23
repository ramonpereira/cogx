package org.cognitivesystems.binder;

import java.util.LinkedList;

import cast.AlreadyExistsOnWMException;
import cast.DoesNotExistOnWMException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.ManagedComponent;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import de.dfki.lt.tr.beliefs.data.Belief;
import de.dfki.lt.tr.beliefs.data.IndependentDistribution;
import de.dfki.lt.tr.beliefs.data.specificproxies.FormulaDistribution;
import de.dfki.lt.tr.beliefs.data.specificproxies.IndependentDistributionBelief;
import de.dfki.lt.tr.beliefs.data.specificproxies.IndependentFormulaDistributionsBelief;
import de.dfki.lt.tr.beliefs.slice.epstatus.AttributedEpistemicStatus;
import de.dfki.lt.tr.beliefs.slice.epstatus.SharedEpistemicStatus;
import de.dfki.lt.tr.beliefs.slice.sitbeliefs.dBelief;


/**
 * Test to see if the SharedBeliefCreator component works correctly
 * 
 * @author Pierre Lison (plison@dfki.de)
 * @version 23/10/2010
 *
 */
public class SharedBeliefCreatorTest extends ManagedComponent{

	
	boolean detectedSharedBelief = false;
	 
	/*
	 * Starting up the CAST component by registering filters on the insertion or modification
	 * of attributed beliefs
	 * 
	 * @see cast.architecture.abstr.WorkingMemoryReaderProcess#start()
	 */
	@Override
	public void start() {

			addChangeFilter(
					ChangeFilterFactory.createLocalTypeFilter(dBelief.class,  WorkingMemoryOperation.ADD),
					new WorkingMemoryChangeReceiver() {

						public void workingMemoryChanged(
								WorkingMemoryChange _wmc) {
							try {
								dBelief belief = getMemoryEntry(_wmc.address, dBelief.class);
								if (belief.estatus instanceof SharedEpistemicStatus) {
									detectedSharedBelief = true;
								}
							} catch (DoesNotExistOnWMException e) {
								e.printStackTrace();
							} catch (UnknownSubarchitectureException e) {
								e.printStackTrace();
							}
						}
					});
	

	}
	
	@Override
	public void run() {
		
		dBelief privateBelief = createPrivateBelief("red");
		addBeliefToWM (privateBelief);
		
		dBelief attributedBelief = createAttributedBelief("red");
		addBeliefToWM (attributedBelief);
	
		sleepComponent(100);
		
		if (detectedSharedBelief) {
			log("WHOOOHOO, SHARED BELIEF IS CREATED!!");
		}
		else {
			log("SORRY, NO SHARED BELIEF ADDED IN WM");
		}
	}
	
	
	
	private dBelief createPrivateBelief (String colourValue) {

		IndependentFormulaDistributionsBelief<dBelief> beliefTest = 
			IndependentFormulaDistributionsBelief.create(dBelief.class);
					
		FormulaDistribution colours = FormulaDistribution.create();	
		colours.addAll(new Object[][] { { colourValue, 0.8 } });

		beliefTest.getContent().put("colour", colours);
		
		return beliefTest.get();
	}
	
	
	private dBelief createAttributedBelief  (String colourValue) {

		IndependentFormulaDistributionsBelief<dBelief> beliefTest = 
			IndependentFormulaDistributionsBelief.create(dBelief.class);
					
		FormulaDistribution colours = FormulaDistribution.create();	
		colours.addAll(new Object[][] { { colourValue, 0.8 } });

		beliefTest.getContent().put("colour", colours);
		
		String attributingAgent = "self";
		LinkedList<String> attributedAgents = new LinkedList<String>();
		attributedAgents.add("human");
		beliefTest.setAttributed(attributingAgent, attributedAgents);
		
		return beliefTest.get();
	}
	
	
	
	private void addBeliefToWM (dBelief b) {
		
		try {
			addToWorkingMemory(newDataID(), b);	
		} 
		catch (AlreadyExistsOnWMException e) {
			e.printStackTrace();
		}
	}
}
