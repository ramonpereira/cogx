package motivation.components.generators;

import java.util.Map;

import motivation.slice.ExploreMotive;
import motivation.slice.MotivePriority;
import motivation.slice.MotiveStatus;
import SpatialData.Place;
import SpatialData.PlaceStatus;
import autogen.Planner.Goal;
import cast.cdl.WorkingMemoryAddress;
import cast.core.CASTUtils;
import de.dfki.lt.tr.beliefs.data.CASTIndependentFormulaDistributionsBelief;
import de.dfki.lt.tr.beliefs.data.specificproxies.FormulaDistribution;
import eu.cogx.beliefs.slice.GroundedBelief;
import eu.cogx.perceptmediator.components.AssociatedBorderPropertyMediator;
import eu.cogx.perceptmediator.components.AssociatedSpacePropertyMediator;
import eu.cogx.perceptmediator.transferfunctions.abstr.SimpleDiscreteTransferFunction;

public class PlaceBeliefGoalGenerator extends
		AbstractBeliefMotiveGenerator<ExploreMotive, GroundedBelief> {

	private static final String PLACETYPE = SimpleDiscreteTransferFunction
			.getBeliefTypeFromCastType(CASTUtils.typeName(Place.class));
	private static final int MAX_EXECUTION_TIME = 60 * 5;
	private static final int MAX_PLANNING_TIME = 30;

	/**
	 * the maximum costs to drop we assign if information gain is really high
	 * (~1) in seconds
	 */
	private static final double MAX_COSTS_TO_DROP = 50;

	/**
	 * normalize borderproperties // normalization: Kristoffer Sjöö
	 * (21.10.2009): the maximum border value should be around(*) 2*pi*d/s where
	 * d is the maximum allowed scan range (currently 5m) and s is the grid cell
	 * size (currently 0.1m, I think). Free space max is pi*d2/s2.
	 * 
	 * 
	 * 
	 */
	final double borderNormalizeFactor = 2 * Math.PI * (5 / 0.1);
	/**
	 * normalize space property factor
	 * 
	 */
	final double spaceNormalizeFactor = Math.PI
			* (Math.pow(5.0, 2) / Math.pow(0.1, 2));
	private double m_spaceMeasureConstant;
	private double m_borderMeasureConstant;
	private double m_constantGain;

	public PlaceBeliefGoalGenerator() {
		super(PLACETYPE, ExploreMotive.class, GroundedBelief.class);
	}

	@Override
	protected ExploreMotive checkForAddition(WorkingMemoryAddress adr,
			GroundedBelief newEntry) {
		assert (newEntry.type.equals(PLACETYPE));
		log("checkForAddition(): check belief " + newEntry.id + " for addition");
		CASTIndependentFormulaDistributionsBelief<GroundedBelief> belief = CASTIndependentFormulaDistributionsBelief
				.create(GroundedBelief.class, newEntry);
		// get the most likely status
		boolean isExplored = belief.getContent().get("placestatus")
				.getDistribution().getMostLikely().getProposition()
				.equalsIgnoreCase(PlaceStatus.TRUEPLACE.name());
		log("checkForAddition(): placestatus="
				+ belief.getContent().get("placestatus").getDistribution()
						.getMostLikely().getProposition());
		// if that is a place holder
		if (!isExplored) {
			log("place is not yet explored, so it is a goal");
			ExploreMotive result = new ExploreMotive();
			result.created = getCASTTime();
			result.correspondingUnion = "";
			result.maxExecutionTime = MAX_EXECUTION_TIME;
			result.maxPlanningTime = MAX_PLANNING_TIME;
			result.priority = MotivePriority.UNSURFACE;
			result.referenceEntry = adr;
			result.status = MotiveStatus.UNSURFACED;
			fillValues(belief, result);
			return result;
		}
		return null;
	}

	@Override
	protected ExploreMotive checkForUpdate(GroundedBelief newEntry,
			ExploreMotive motive) {
		assert (newEntry.type.equals(PLACETYPE));
		log("check goal " + CASTUtils.toString(motive.thisEntry)
				+ " for update");
		CASTIndependentFormulaDistributionsBelief<GroundedBelief> belief = CASTIndependentFormulaDistributionsBelief
				.create(GroundedBelief.class, newEntry);
		boolean isExplored = belief.getContent().get("placestatus")
				.getDistribution().getMostLikely().getProposition()
				.equalsIgnoreCase(PlaceStatus.TRUEPLACE.name());
		// if that is a place holder
		if (!isExplored) {
			log("place is not yet explored, so it is a goal");
			fillValues(belief, motive);
			return motive;
		} else {
			return null;
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see cast.core.CASTComponent#configure(java.util.Map)
	 */
	@Override
	protected void configure(Map<String, String> config) {
		m_spaceMeasureConstant = 0.0;
		m_borderMeasureConstant = 0.9;
		m_constantGain = 0.1;
	}

	private void fillValues(
			CASTIndependentFormulaDistributionsBelief<GroundedBelief> belief,
			ExploreMotive motive) {
		motive.updated = getCASTTime();
		// initially this costs are taken as -1, corresponding to an ultimate
		// goal.
		motive.costs = -1;
		FormulaDistribution borderFeature = belief.getContent().get(
				AssociatedBorderPropertyMediator.ASSOCIATEDBORDER_FEATURENAME);
		FormulaDistribution spaceFeature = belief.getContent().get(
				AssociatedSpacePropertyMediator.ASSOCIATEDSPACE_FEATURENAME);
		if (borderFeature != null && spaceFeature != null) {
			log("we have geometrical features that we can use to compute the gain");
			double border = borderFeature.getDistribution().getMostLikely()
					.getDouble()
					/ borderNormalizeFactor;
			double space = spaceFeature.getDistribution().getMostLikely()
					.getDouble()
					/ spaceNormalizeFactor;
			log("  border=" + border);
			log("  space=" + space);
			motive.informationGain = (space * m_spaceMeasureConstant)
					+ (border * m_borderMeasureConstant) + m_constantGain;
			log("  gain=" + motive.informationGain);
		}

		motive.goal = new Goal(computeImportance(motive), "(= (placestatus '"
				+ belief.getId() + "') trueplace)", false);
		log("updated goal to " + motive.goal.goalString);
	}

	float computeImportance(ExploreMotive m) {
		if (m.informationGain < 0)
			return -1.0f;
		else
			return (float) (m.informationGain * MAX_COSTS_TO_DROP);
	}

}
