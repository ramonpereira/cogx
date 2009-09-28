/**
 * 
 */
package motivation.components.generators;

import motivation.factories.MotiveFactory;
import motivation.slice.CategorizePlaceMotive;
import motivation.slice.Motive;
import SpatialData.Place;
import SpatialData.PlaceStatus;
import cast.CASTException;
import cast.DoesNotExistOnWMException;
import cast.PermissionException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTUtils;

/**
 * @author marc
 *
 */
public class CategorizePlaceGenerator extends AbstractMotiveGenerator {

	/* (non-Javadoc)
	 * @see motivation.generators.Generator#updateMotive(cast.cdl.WorkingMemoryAddress, cast.cdl.WorkingMemoryAddress)
	 */
	@Override
	protected boolean checkMotive(Motive motive) {
		try {
			Place source = getMemoryEntry(motive.referenceEntry, Place.class);
			
			// if it is a yet unexplored one...			
			log("there is a place to be checked, created " + Long.toString(motive.created.s-getCASTTime().s) + " seconds ago");

			if (source.status == PlaceStatus.TRUEPLACE) {
				log("  it's a true place, so it should be considered as a motive to be categorized");
				write(motive);
				return true;
			}
			else {
				log("  turn out this place is not a trueplace, so it should be no motive then");
				remove(motive);
			}
		} catch (DoesNotExistOnWMException e) {
			e.printStackTrace();
		} catch (UnknownSubarchitectureException e) {
			e.printStackTrace();
		} catch (PermissionException e) {
			e.printStackTrace();
		}
		return false;
	}

	/* (non-Javadoc)
	 * @see cast.core.CASTComponent#start()
	 */
	@Override
	protected void start() {
		super.start();
		WorkingMemoryChangeReceiver receiver = new WorkingMemoryChangeReceiver() {
			
			@Override
			public void workingMemoryChanged(WorkingMemoryChange _wmc)
					throws CASTException {
				debug(CASTUtils.toString(_wmc));
				// create a new motive from this node...
				CategorizePlaceMotive newMotive = MotiveFactory.createCategorizePlaceMotive(_wmc.address);
				Place p;
				try {
					p = getMemoryEntry(_wmc.address, Place.class);
					newMotive.placeID=p.id;
				} catch (CASTException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				} 
				checkMotive(newMotive);
				
			}
		};
		addChangeFilter(ChangeFilterFactory.createGlobalTypeFilter(Place.class,
				WorkingMemoryOperation.ADD), receiver);
	}

	/* (non-Javadoc)
	 * @see cast.core.CASTComponent#stop()
	 */
	@Override
	protected void stop() {
		// TODO Auto-generated method stub
		super.stop();
	}
	
	

}
