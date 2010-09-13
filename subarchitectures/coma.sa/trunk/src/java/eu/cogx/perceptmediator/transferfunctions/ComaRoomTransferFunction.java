/**
 * 
 */
package eu.cogx.perceptmediator.transferfunctions;

import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;

import cast.architecture.ManagedComponent;
import cast.cdl.WorkingMemoryAddress;
import cast.cdl.WorkingMemoryChange;

import comadata.ComaRoom;

import de.dfki.lt.tr.beliefs.data.formulas.Formula;
import de.dfki.lt.tr.beliefs.data.formulas.IntFormula;
import de.dfki.lt.tr.beliefs.util.BeliefException;
import eu.cogx.perceptmediator.transferfunctions.abstr.SimpleDiscreteTransferFunction;

/**
 * @author marc
 * 
 */
public class ComaRoomTransferFunction extends
		SimpleDiscreteTransferFunction<ComaRoom> {
	
	public ComaRoomTransferFunction(ManagedComponent component) {
		super(component, Logger.getLogger(ComaRoomTransferFunction.class));
		// TODO Auto-generated constructor stub
	}

	@Override
	protected Map<String, Formula> getFeatureValueMapping(
			WorkingMemoryChange wmc, ComaRoom from) throws BeliefException {
		assert (from != null);
		Map<String, Formula> result = new HashMap<String, Formula>();
		result.put("RoomId", IntFormula.create((int) from.roomId)
				.getAsFormula());
		// BoolFormula isExplored =
		// BoolFormula.create(from.status==PlaceStatus.TRUEPLACE);
		// result.put("placestatus",
		// PropositionFormula.create(from.status.name()).getAsFormula());
		// result.put("explored", isExplored.getAsFormula());
		return result;
	}

}
