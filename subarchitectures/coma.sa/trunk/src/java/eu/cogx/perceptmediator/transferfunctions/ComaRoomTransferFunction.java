/**
 * 
 */
package eu.cogx.perceptmediator.transferfunctions;

import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;

import SpatialProbabilities.JointProbabilityValue;

import cast.architecture.ManagedComponent;
import cast.cdl.WorkingMemoryChange;
import castutils.castextensions.IceXMLSerializer;

import comadata.ComaRoom;

import de.dfki.lt.tr.beliefs.data.CASTIndependentFormulaDistributionsBelief;
import de.dfki.lt.tr.beliefs.data.formulas.Formula;
import de.dfki.lt.tr.beliefs.data.formulas.IntFormula;
import de.dfki.lt.tr.beliefs.data.specificproxies.FormulaDistribution;
import de.dfki.lt.tr.beliefs.data.specificproxies.IndependentFormulaDistributions;
import de.dfki.lt.tr.beliefs.util.BeliefException;
import eu.cogx.beliefs.slice.PerceptBelief;
import eu.cogx.perceptmediator.transferfunctions.abstr.SimpleDiscreteTransferFunction;

/**
 * @author marc
 * 
 */
public class ComaRoomTransferFunction extends
		SimpleDiscreteTransferFunction<ComaRoom, PerceptBelief> {

	public static final String ROOM_ID = "RoomId";

	public ComaRoomTransferFunction(ManagedComponent component) {
		super(component, Logger.getLogger(ComaRoomTransferFunction.class),
				PerceptBelief.class);
		// TODO Auto-generated constructor stub
	}

	@Override
	protected Map<String, Formula> getFeatureValueMapping(
			WorkingMemoryChange wmc, ComaRoom from) throws BeliefException {
		assert (from != null);
		Map<String, Formula> result = new HashMap<String, Formula>();
		result
				.put(ROOM_ID, IntFormula.create((int) from.roomId)
						.getAsFormula());

		// BoolFormula isExplored =
		// BoolFormula.create(from.status==PlaceStatus.TRUEPLACE);
		// result.put("placestatus",
		// PropositionFormula.create(from.status.name()).getAsFormula());
		// result.put("explored", isExplored.getAsFormula());
		return result;
	}

	@Override
	protected void fillBelief(
			CASTIndependentFormulaDistributionsBelief<PerceptBelief> belief,
			WorkingMemoryChange wmc, ComaRoom from) {
		super.fillBelief(belief, wmc, from);
		if (from.categories.massFunction == null) {
			component.getLogger().info(
					"Coma room without a category yet, not mediating!");
			return;
		}
		logger.info("fill belief with coma categories");
		IndependentFormulaDistributions distr = belief.getContent();
		FormulaDistribution fd = FormulaDistribution.create();
		for (JointProbabilityValue jp : from.categories.massFunction) {
			String value = ((SpatialProbabilities.StringRandomVariableValue) (jp.variableValues[0])).value;
			logger.info("adding " + value + " (" + jp.probability + ")");
			fd.add(value, jp.probability);
		}
		assert (fd.size() > 0);
		distr.put("category", fd);
	}

}
