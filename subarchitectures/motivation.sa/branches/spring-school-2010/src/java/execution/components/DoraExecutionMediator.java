package execution.components;

import java.util.List;

import autogen.Planner.Action;
import beliefmodels.autogen.beliefs.Belief;
import beliefmodels.autogen.featurecontent.FeatureValue;
import beliefmodels.autogen.featurecontent.PointerValue;
import beliefmodels.autogen.featurecontent.StringValue;
import beliefmodels.autogen.featurecontent.IntegerValue;
import cast.CASTException;
import castutils.facades.BinderFacade;
import execution.slice.ActionExecutionException;
import execution.slice.actions.DetectObjects;
import execution.slice.actions.DetectPeople;
import execution.slice.actions.ExplorePlace;
import execution.slice.actions.GoToPlace;
import execution.slice.actions.ComsysTestFeatureValue;
import execution.slice.actions.ComsysQueryFeature;
import execution.util.ActionConverter;

/**
 * Execution mediator specifically for Dora/Spring School.
 * 
 * @author nah
 * 
 */
public class DoraExecutionMediator extends PlanExecutionMediator implements
		ActionConverter {
	private final BinderFacade m_binderFacade;
	private static final String[] DEFAULT_LABELS = { "record1", "record2",
			"record3", "record4" };

	public DoraExecutionMediator() {
		m_binderFacade = new BinderFacade(this);
	}

	@Override
	protected void start() {
		super.start();
		m_binderFacade.start();
	}

	private String plannerLiteralToWMID(String _string) {
		return _string.substring(_string.indexOf("_") + 1).replace("__", ":");
	}

	/**
	 * Does the system specific work of converting a planning action into real
	 * system stuff.
	 * 
	 * @param _plannedAction
	 * @return
	 * @throws CASTException
	 */
	public execution.slice.Action toSystemAction(Action _plannedAction)
			throws CASTException {
		if (_plannedAction.name.equals("move")) {
			assert _plannedAction.arguments.length == 2 : "move action arity is expected to be 2";

			GoToPlace act = newActionInstance(GoToPlace.class);
			String placeUnionID = ((PointerValue) _plannedAction.arguments[1]).beliefId.id;
			Belief placeUnion = m_binderFacade.getBelief(placeUnionID);

			if (placeUnion == null) {
				throw new ActionExecutionException(
						"No union for place union id: " + placeUnionID);
			}

			List<FeatureValue> placeIDFeatures = m_binderFacade
					.getFeatureValue(placeUnion, "PlaceId");
			if (placeIDFeatures.isEmpty()) {
				throw new ActionExecutionException(
						"No PlaceId features for union id: " + placeUnionID);

			}
			IntegerValue placeID = (IntegerValue) placeIDFeatures.get(0);
			act.placeID = placeID.val;
			return act;
			// } else if (_plannedAction.name.equals("categorize_room")) {
			// assert _plannedAction.arguments.length == 2 :
			// "categorize_room action arity is expected to be 2";
			// String roomUnionID =
			// plannerLiteralToWMID(_plannedAction.arguments[1]);
			// // ok, we have to do some binder lookups here to find all places
			// // that belong to the room:
			// // 1. lookup the union for the room
			// // 2. find all RelationProxies that have this room union as a
			// source
			// // ("contains" relations)
			// // 3. read the target of these relations and check if they have a
			// // "place_id"
			// // 4. add these place_ids to the Action arguments
			// Union roomUnion = m_binderFacade.getUnion(roomUnionID);
			// Set<Long> placeIDs = new HashSet<Long>();
			// log("look at roomUnion:");
			// for (Proxy p : roomUnion.includedProxies) {
			// // check if it is room
			//
			// if (m_binderFacade.getFeatureValue(p, "roomId") != null) {
			// Map<WorkingMemoryAddress, RelationProxy> relMap = m_binderFacade
			// .findRelationBySrc(p.entityID);
			// for (RelationProxy rp : relMap.values()) {
			// Proxy placeProxy = m_binderFacade
			// .getProxy(((AddressValue) rp.target.alternativeValues[0]).val);
			// List<FeatureValue> features = m_binderFacade
			// .getFeatureValue(placeProxy, "place_id");
			// if (!features.isEmpty()) {
			// long placeId = Long
			// .parseLong(((StringValue) features.get(0)).val);
			// log("  related to this room is place_id " + placeId);
			// placeIDs.add(new Long(placeId));
			// }
			//
			// }
			// break;
			// }
			// }
			// ActiveVisualSearch avs =
			// newActionInstance(ActiveVisualSearch.class);
			// // TODO: what is expected here???
			// avs.placeIDs = new long[placeIDs.size()];
			// int count = 0;
			// for (Long o : placeIDs)
			// avs.placeIDs[count++] = o.longValue();
			// return avs;
		} else if (_plannedAction.name.equals("explore_place")) {
			return new ExplorePlace();
		} else if (_plannedAction.name.equals("look-for-object")) {
			assert _plannedAction.arguments.length == 1 : "look-for-object action arity is expected to be 1";

			DetectObjects act = newActionInstance(DetectObjects.class);
			act.labels = DEFAULT_LABELS;
			return act;
		} else if (_plannedAction.name.equals("look-for-people")) {
			assert _plannedAction.arguments.length == 1 : "look-for-people action arity is expected to be 1";

			DetectPeople act = newActionInstance(DetectPeople.class);
			return act;
		} else if (_plannedAction.name.equals("ask-for-feature")) {
			assert _plannedAction.arguments.length == 3 : "ask-for-feature action arity is expected to be 3";
			String beliefID = ((PointerValue) _plannedAction.arguments[1]).beliefId.id;
			String featureID = "";
			Belief belief = m_binderFacade.getBelief(beliefID);
			// TODO: implement this action
			throw(new ActionExecutionException(_plannedAction.name + " not yet implemented."));
			//AskFeature act = newActionInstance(AskFeature.class);
			//return act;
		} else if (_plannedAction.name.equals("ask-for-placename")) {
			assert _plannedAction.arguments.length == 2 : "ask-for-feature action arity is expected to be 2";
			String beliefID =  ((PointerValue) _plannedAction.arguments[1]).beliefId.id;
			String featureID = "name";
			Belief belief = m_binderFacade.getBelief(beliefID);
			ComsysQueryFeature act = newActionInstance(ComsysQueryFeature.class);
            act.beliefID = beliefID;
            act.featureID = featureID;
			return act;
		} else if (_plannedAction.name.equals("verify-placename")) {
			assert _plannedAction.arguments.length == 3 : "ask-for-feature action arity is expected to be 2";
			String beliefID =  ((PointerValue) _plannedAction.arguments[1]).beliefId.id;
			String featureID = "name";
			Belief belief = m_binderFacade.getBelief(beliefID);
			ComsysTestFeatureValue act = newActionInstance(ComsysTestFeatureValue.class);
            act.beliefID = beliefID;
            act.featureType = featureID;
            act.featureValue = _plannedAction.arguments[2];
			return act;
		}

		throw new ActionExecutionException("No conversion available for: "
				+ _plannedAction.fullName);
	}

	@Override
	public ActionConverter getActionConverter() {

		return this;
	}

}
