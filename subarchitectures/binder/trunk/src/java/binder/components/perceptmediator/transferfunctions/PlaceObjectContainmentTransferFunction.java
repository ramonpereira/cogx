/**
 * 
 */
package binder.components.perceptmediator.transferfunctions;

import java.util.HashMap;
import java.util.Map;

import org.apache.log4j.Logger;

import cast.architecture.ManagedComponent;
import cast.cdl.WorkingMemoryAddress;
import castutils.castextensions.WMView;

import SpatialProperties.PlaceContainmentObjectProperty;
import SpatialProperties.IntegerValue;
import beliefmodels.arch.BeliefException;
import beliefmodels.autogen.beliefs.PerceptBelief;
import beliefmodels.autogen.featurecontent.FeatureValue;
import beliefmodels.autogen.featurecontent.featurenames.FeatPlaceId;
import beliefmodels.builders.FeatureValueBuilder;
import binder.components.perceptmediator.transferfunctions.abstr.DependentDiscreteTransferFunction;
import binder.components.perceptmediator.transferfunctions.abstr.SimpleDiscreteTransferFunction;
import binder.components.perceptmediator.transferfunctions.helpers.PlaceMatchingFunction;

/**
 * @author marc
 *
 */
public class PlaceObjectContainmentTransferFunction extends DependentDiscreteTransferFunction<PlaceContainmentObjectProperty, PerceptBelief> {

	public PlaceObjectContainmentTransferFunction(ManagedComponent component, WMView<PerceptBelief> allBeliefs) {
		super(component, allBeliefs, Logger.getLogger(PlaceObjectContainmentTransferFunction.class));
	}

	@Override
	protected
	Map<String, FeatureValue> getFeatureValueMapping(PlaceContainmentObjectProperty from) throws BeliefException, InterruptedException {
		assert(from != null);
		Map<String, FeatureValue> result = new HashMap<String, FeatureValue>();

		result.put("Label", FeatureValueBuilder.createNewStringValue(from.label));

		WorkingMemoryAddress wmaPlace = getReferredBelief(new PlaceMatchingFunction(
				((IntegerValue) from.mapValue).value));
		result.put(FeatPlaceId.value, FeatureValueBuilder
				.createNewStringValue(wmaPlace.id));

		
		return result;
	}


}
