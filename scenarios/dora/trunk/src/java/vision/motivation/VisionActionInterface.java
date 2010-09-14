/**
 * 
 */
package vision.motivation;

import java.util.Hashtable;
import java.util.Map;
import java.util.Set;

import VisionData.DetectionCommand;
import VisionData.ForegroundedModel;
import VisionData.PeopleDetectionCommand;
import cast.CASTException;
import cast.architecture.ManagedComponent;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryAddress;
import execution.slice.TriBool;
import execution.slice.actions.BackgroundModels;
import execution.slice.actions.DetectObjects;
import execution.slice.actions.DetectPeople;
import execution.slice.actions.ForegroundModels;
import execution.slice.actions.RecogniseForegroundedModels;
import execution.util.BlockingActionExecutor;
import execution.util.ComponentActionFactory;
import execution.util.LocalActionStateManager;
import execution.util.NonBlockingCompleteOnDeleteExecutor;

/**
 * Component to listen to planner actions the trigger the vision sa as
 * appropriate. Must be run from the vision sa.
 * 
 * @author nah
 * 
 */
public class VisionActionInterface extends ManagedComponent {

	/**
	 * An action executor to handle object detection.
	 * 
	 * @author nah
	 */
	private class DetectObjectsActionExecutor extends
			NonBlockingCompleteOnDeleteExecutor<DetectObjects> implements
			WorkingMemoryChangeReceiver {

		private String[] m_labels;

		public DetectObjectsActionExecutor(ManagedComponent _component) {
			super(_component, DetectObjects.class);
		}

		@Override
		public boolean acceptAction(DetectObjects _action) {
			DetectObjects action = _action;
			m_labels = action.labels;
			return true;
		}

		@Override
		public void executeAction() {
			DetectionCommand cmd = new DetectionCommand(m_labels);
			addThenCompleteOnDelete(new WorkingMemoryAddress(newDataID(),
					getSubarchitectureID()), cmd);
		}

	}

	/**
	 * An action executor to handle people detection.
	 * 
	 * @author nah
	 */
	private class DetectPeopleActionExecutor extends
			NonBlockingCompleteOnDeleteExecutor<DetectPeople> implements
			WorkingMemoryChangeReceiver {

		public DetectPeopleActionExecutor(ManagedComponent _component) {
			super(_component, DetectPeople.class);
		}

		@Override
		public boolean acceptAction(DetectPeople _action) {
			return true;
		}

		@Override
		public void executeAction() {
			println("execute action");
			PeopleDetectionCommand cmd = new PeopleDetectionCommand();
			addThenCompleteOnDelete(new WorkingMemoryAddress(newDataID(),
					getSubarchitectureID()), cmd);
		}

	}

	public static class ForegroundModelExecutor extends
			BlockingActionExecutor<ForegroundModels> {

		public ForegroundModelExecutor(ManagedComponent _component) {
			super(_component, ForegroundModels.class);
		}

		@Override
		protected VisionActionInterface getComponent() {
			return (VisionActionInterface) super.getComponent();
		}
		
		@Override
		public TriBool execute() {

			String[] models = getAction().models;
			for (String model : models) {
				if (!getComponent().m_foregroundedModels.contains(model)) {
					ForegroundedModel foreground = new ForegroundedModel(model);
					WorkingMemoryAddress addr = new WorkingMemoryAddress(
							getComponent().newDataID(), getComponent().getSubarchitectureID());
					try {
						getComponent().addToWorkingMemory(addr, foreground);
						getComponent().m_foregroundedModels.put(model, addr);
						getComponent().log("Foregrounded model: " + model);

					} catch (CASTException e) {
						getComponent().logException(e);
						return TriBool.TRIFALSE;
					}
				} else {
					getComponent().log(
							"Not foregrounding already foregrounded model: "
									+ model);
				}
			}

			return TriBool.TRITRUE;
		}
	}

	
	
	
	public static class BackgroundModelExecutor extends
			BlockingActionExecutor<BackgroundModels> {

		public BackgroundModelExecutor(ManagedComponent _component) {
			super(_component, BackgroundModels.class);
		}

		@Override
		protected VisionActionInterface getComponent() {
			return (VisionActionInterface) super.getComponent();
		}
		
		@Override
		public TriBool execute() {

			String[] models = getAction().models;
			for (String model : models) {
				WorkingMemoryAddress addr = getComponent().m_foregroundedModels.remove(model);
				if (addr != null) {
					try {
						getComponent().deleteFromWorkingMemory(addr);
						getComponent().log("Backgrounded model: " + model);
					} catch (CASTException e) {
						getComponent().logException(e);
						return TriBool.TRIFALSE;
					}
				} else {
					getComponent().log(
							"Not backgrounding non-foregrounded model: "
									+ model);
				}
			}

			return TriBool.TRITRUE;
		}
	}

	/**
	 * An action executor to handle object detection.
	 * 
	 * @author nah
	 */
	public static class RecogniseForegroundedModelsExecutor extends
			NonBlockingCompleteOnDeleteExecutor<RecogniseForegroundedModels> implements
			WorkingMemoryChangeReceiver {


		public RecogniseForegroundedModelsExecutor(ManagedComponent _component) {
			super(_component, RecogniseForegroundedModels.class);
		}

		@Override
		protected VisionActionInterface getComponent() {
			return (VisionActionInterface) super.getComponent();
		}
		@Override
		public boolean acceptAction(RecogniseForegroundedModels _action) {
			return true;
		}

		@Override
		public void executeAction() {
			Set<String> foregroundedModels = getComponent().m_foregroundedModels.keySet();
			DetectionCommand cmd = new DetectionCommand(new String[foregroundedModels.size()]);
			cmd.labels = foregroundedModels.toArray(cmd.labels);
			addThenCompleteOnDelete(new WorkingMemoryAddress(getComponent().newDataID(),
					getComponent().getSubarchitectureID()), cmd);
		}

	}
	
	private LocalActionStateManager m_actionStateManager;

	@Override
	protected void configure(Map<String, String> _config) {
	}

	private final Hashtable<String, WorkingMemoryAddress> m_foregroundedModels;

	public VisionActionInterface() {
		m_foregroundedModels = new Hashtable<String, WorkingMemoryAddress>();
	}

	@Override
	protected void start() {
		m_actionStateManager = new LocalActionStateManager(this);
		
		m_actionStateManager.registerActionType(DetectObjects.class,
				new ComponentActionFactory<DetectObjectsActionExecutor>(this,
						DetectObjectsActionExecutor.class));
	
		m_actionStateManager.registerActionType(DetectPeople.class,
				new ComponentActionFactory<DetectPeopleActionExecutor>(this,
						DetectPeopleActionExecutor.class));


		m_actionStateManager.registerActionType(ForegroundModels.class,
				new ComponentActionFactory<ForegroundModelExecutor>(this,
						ForegroundModelExecutor.class));

		m_actionStateManager.registerActionType(BackgroundModels.class,
				new ComponentActionFactory<BackgroundModelExecutor>(this,
						BackgroundModelExecutor.class));

		m_actionStateManager.registerActionType(
				RecogniseForegroundedModels.class, new ComponentActionFactory<RecogniseForegroundedModelsExecutor>(this, RecogniseForegroundedModelsExecutor.class));

	}

}
