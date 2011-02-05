package verbalisation;

import java.util.HashMap;
import java.util.Map;

import motivation.slice.CategorizeRoomMotive;
import motivation.slice.ExploreMotive;
import motivation.slice.Motive;
import motivation.slice.MotiveStatus;
import motivation.slice.PatrolMotive;
import SpatialData.NavCommand;
import SpatialData.Place;
import SpatialData.PlaceStatus;
import SpatialData.ProcessViewPointCommand;
import SpatialData.ViewPointGenerationCommand;
import SpatialProperties.GatewayPlaceProperty;
import SpatialProperties.ObjectPlaceProperty;
import VisionData.DetectionCommand;
import VisionData.VisualObject;
import cast.CASTException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.ManagedComponent;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTUtils;
import castutils.castextensions.Accessor;
import de.dfki.lt.hfc.operators.FProduct;

/**
 * - found new place N - found new placeholder N - found new room N - seen
 * object O - room N is categorised as a C - found a doorway - going to place N
 * - going room categorise room b - activated goals G - planning for active
 * goals - planning succeeded/failed - executing plan - executing next action -
 * action succeeded/failed - plan execution succeeded/failed
 * 
 * 
 * subarchitectures/comsys/grammars/contentPlanning/doc/cogx-testbed.xml
 * 
 * @author nah
 * 
 */
public class DoraVerbalisation extends ManagedComponent implements
		WorkingMemoryChangeReceiver {

	private final TextGenerator<NavCommand> NAV_CMD_GENERATOR = new TextGenerator<NavCommand>() {

		@Override
		public String toText(NavCommand _i) {

			switch (_i.cmd) {
			case GOTOPLACE:
				PlaceStatus status = m_placeIDToStatus.get(_i.destId[0]);
				if (status == null) {
					return "";
				} else if (status == PlaceStatus.PLACEHOLDER) {
					return "Exploring place " + _i.destId[0];
				} else {
					return "Going to place " + _i.destId[0];
				}
			case TURNTO:
				return "Turning";

			default:
				// return "I'm doing something, but I don't want talk about it";
				return "";
			}
		}
	};

	private static final TextGenerator<Motive> MOTIVE_ACTIVATED = new TextGenerator<Motive>() {
		@Override
		public String toText(Motive _i) {

			if (_i instanceof ExploreMotive) {
				return "Activated motive to explore hypothesis for Place "
						+ ((ExploreMotive) _i).placeID;
			} else if (_i instanceof CategorizeRoomMotive) {
				return "Activated motive to determine category of room "
						+ ((CategorizeRoomMotive) _i).roomId;
			} else if (_i instanceof PatrolMotive) {
				return "Activated motive to patrol place "
						+ ((PatrolMotive) _i).placeID;
			}
			return "";
		}
	};

	private static final String[] DETECTION_COMMAND_SAYINGS = {
			"Looking here.", "Having a look-see", "What have we here" };

	private static final TextGenerator<DetectionCommand> DETECTION_COMMAND_GENERATOR = new TextGenerator<DetectionCommand>() {
		private int m_sayingIndex = 0;

		@Override
		public String toText(DetectionCommand _i) {
			if (m_sayingIndex == DETECTION_COMMAND_SAYINGS.length) {
				m_sayingIndex = 0;
			}
			return DETECTION_COMMAND_SAYINGS[m_sayingIndex++];
		}
	};

//	private static final TextGenerator<ObjectPlaceProperty> OBJECT_PROPERTY_GENERATOR = new TextGenerator<ObjectPlaceProperty>() {
//		@Override
//		public String toText(ObjectPlaceProperty _i) {
//			return "Found an object of type "
//					+ ((SpatialProperties.StringValue) _i.mapValue).value;
//		}
//	};

	private static final TextGenerator<ViewPointGenerationCommand> VIEW_POINT_CMD_GENERATOR = new TextGenerator<ViewPointGenerationCommand>() {
		@Override
		public String toText(ViewPointGenerationCommand _i) {
			return "Generating view points for object " + _i.label;
		}
	};

	private static final TextGenerator<ProcessViewPointCommand> PROCESS_SINGLE_VIEWPOINT_GENERATOR = new TextGenerator<ProcessViewPointCommand>() {

		@Override
		public String toText(ProcessViewPointCommand _i) {
			StringBuffer sb = new StringBuffer(
					"Processing view point to look for");
			for (String model : _i.objectModels) {
				sb.append(" a ");
				sb.append(model);
			}
			return sb.toString();
		}
	};

	private static final TextGenerator<VisualObject> VISUAL_OBJECT_GENERATOR = new TextGenerator<VisualObject>() {
		@Override
		public String toText(VisualObject _i) {
			if (_i.detectionConfidence > 0.08) {
				return "That looks like " + _i.identLabels[0];
			} else {
				return "";
			}
		}
	};

	private static Accessor<Motive, MotiveStatus> MOTIVE_STATUS_ACCESSOR = new Accessor<Motive, MotiveStatus>() {
		@Override
		public MotiveStatus access(Motive _entry) {
			return _entry.status;
		}
	};

	private static Accessor<Place, PlaceStatus> PLACE_STATUS_ACCESSOR = new Accessor<Place, PlaceStatus>() {
		@Override
		public PlaceStatus access(Place _entry) {
			return _entry.status;
		}
	};

	private static TextGenerator<Place> NEW_PLACE_GENERATOR = new TextGenerator<Place>() {

		@Override
		public String toText(Place _i) {

			StringBuilder sb = new StringBuilder();
			if (_i.status == PlaceStatus.PLACEHOLDER) {
				sb.append("I have a hypothesis that Place ");
				sb.append(_i.id);
				sb.append("exists");
			} else {
				sb.append("Created Place ");
				sb.append(_i.id);
			}
			return sb.toString();
		}
	};

	private static TextGenerator<Place> PLACE_EXPLORED_GENERATOR = new TextGenerator<Place>() {
		@Override
		public String toText(Place _i) {
			StringBuilder sb = new StringBuilder();
			sb.append("Created Place ");
			sb.append(_i.id);
			sb.append(" from an existing hypothesis");
			return sb.toString();
		}
	};

	private static TextGenerator<Place> PLACE_EXPLORATION_FAILED_GENERATOR = new TextGenerator<Place>() {
		@Override
		public String toText(Place _i) {
			StringBuilder sb = new StringBuilder();
			sb.append("Hypothesis for Place ");
			sb.append(_i.id);
			sb.append(" was not valid");
			return sb.toString();
		}
	};

	private final VerbalisationFacade m_verbals;

	private final HashMap<Long, PlaceStatus> m_placeIDToStatus;

	public DoraVerbalisation() {
		m_verbals = new VerbalisationFacade(this);
		m_placeIDToStatus = new HashMap<Long, PlaceStatus>();
	}

	@Override
	public void configure(Map<String, String> _config) {
		m_verbals.configure(_config);
	}

	public void start() {

		// say stuff...

		// when all motives are activated
		// m_verbals.verbaliseOnStateTransition(Motive.class,
		// MOTIVE_STATUS_ACCESSOR, MotiveStatus.SURFACED,
		// MotiveStatus.ACTIVE, MOTIVE_ACTIVATED);

		// when places are created
		// m_verbals.verbaliseOnAddition(Place.class, NEW_PLACE_GENERATOR);

		// when places are explored
		// m_verbals.verbaliseOnStateTransition(Place.class,
		// PLACE_STATUS_ACCESSOR, PlaceStatus.PLACEHOLDER,
		// PlaceStatus.TRUEPLACE, PLACE_EXPLORED_GENERATOR);
		//
		// // when places are deleted (i.e. exploration could be carried out)
		// m_verbals.verbaliseOnDeletion(Place.class,
		// PLACE_EXPLORATION_FAILED_GENERATOR);

		// // when navigation is told to move the robot
		m_verbals.verbaliseOnAddition(NavCommand.class, NAV_CMD_GENERATOR);

		// // when plan execution is triggered
		// m_verbals.verbaliseCannedTextOnAddition(PlanProxy.class,
		// "Starting plan execution.");

		// when a recognition command is triggered�
		// m_verbals.verbaliseOnAddition(DetectionCommand.class,
		// DETECTION_COMMAND_GENERATOR);

//		// when an object is added to the spatial model -> this is once per
//		// object class in place
//		m_verbals.verbaliseOnAddition(ObjectPlaceProperty.class,
//				OBJECT_PROPERTY_GENERATOR);

		// when an object is recognised at all -> is this every positive
		// recognition result
		m_verbals.verbaliseOnAddition(VisualObject.class,
				VISUAL_OBJECT_GENERATOR);

		// m_verbals.verbaliseOnOverwrite(ComaRoom.class,
		// new RoomCategoryTextGenerator());

		m_verbals.verbaliseCannedTextOnAddition(GatewayPlaceProperty.class,
				"Ah ha. Looks like there is a door here");

		// when spatial is asked for new viewpoints
		m_verbals.verbaliseOnAddition(ViewPointGenerationCommand.class,
				VIEW_POINT_CMD_GENERATOR);

		// when spatial is asked for new viewpoints
		m_verbals.verbaliseOnAddition(ProcessViewPointCommand.class,
				PROCESS_SINGLE_VIEWPOINT_GENERATOR);

		addChangeFilter(
				ChangeFilterFactory.createGlobalTypeFilter(Place.class), this);
		addChangeFilter(
				ChangeFilterFactory.createGlobalTypeFilter(Place.class), this);
	}

	public void runComponent() {
		m_verbals.verbaliseCannedText("my name is dora.");
	}

	@Override
	public void workingMemoryChanged(WorkingMemoryChange _wmc)
			throws CASTException {
		// stop me making silly mistakes
		assert (_wmc.type.equals(CASTUtils.typeName(Place.class)));

		if (_wmc.operation != WorkingMemoryOperation.DELETE) {
			Place place = getMemoryEntry(_wmc.address, Place.class);
			m_placeIDToStatus.put(place.id, place.status);
		}

	}

}
