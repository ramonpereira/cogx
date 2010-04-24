package motivation.tests;

import motivation.util.PlaceUnionEventRelation;
import castutils.castextensions.CausalEventMonitor;
import castutils.castextensions.WMEventQueue;
import SpatialData.Place;
import beliefmodels.autogen.beliefs.Belief;

import cast.architecture.ChangeFilterFactory;
import cast.architecture.ManagedComponent;
import cast.cdl.WorkingMemoryChange;

public class PropagationTest extends ManagedComponent {
	WMEventQueue eq;
	CausalEventMonitor<Place, Belief> cer;

	/*
	 * (non-Javadoc)
	 * 
	 * @see cast.core.CASTComponent#runComponent()
	 */
	@Override
	protected void runComponent() {
		// TODO Auto-generated method stub
		while (isRunning()) {
			WorkingMemoryChange event;
			try {
				event = eq.take();
				log("received relevant change... waiting for propagation now");
				cer.waitForPropagation(event.address);
				log("propagation finished");
				log("cer.getPendingChanges().size()="
						+ cer.getPendingChanges().size());
				//sleepComponent(3000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cast.core.CASTComponent#start()
	 */
	@Override
	protected void start() {
		cer = new PlaceUnionEventRelation(this);
		cer.start();
		eq = new WMEventQueue();

		addChangeFilter(
				ChangeFilterFactory.createGlobalTypeFilter(Place.class), eq);
		super.start();
	}

}
