/**
 * 
 */
package motivation.components.managers;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import com.sun.org.apache.bcel.internal.generic.INSTANCEOF;

import motivation.slice.ExploreMotive;
import motivation.slice.Motive;
import motivation.slice.PlanProxy;
import motivation.util.GoalTranslator;
import IceInternal.Instance;
import autogen.Planner.Completion;
import autogen.Planner.PlanningTask;
import cast.AlreadyExistsOnWMException;
import cast.CASTException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryAddress;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;

/**
 * @author marc
 * 
 */
public class PlanAllManager extends MotiveManager {

	Set<Motive> managedMotives;

	/**
	 * @param specificType
	 */
	public PlanAllManager() {
		super(ExploreMotive.class);
		managedMotives = Collections.synchronizedSet(new HashSet<Motive>());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * motivation.components.managers.MotiveManager#manageMotive(motivation.
	 * slice.Motive)
	 */
	@Override
	protected void manageMotive(Motive motive) {
		log("have a new motive to manage... type is "
				+ Motive.class.getSimpleName());
		if (motive instanceof ExploreMotive) {
			log("have some explore motive around");
			managedMotives.add(motive);
			log("size of managedMotives is " + managedMotives.size());
			// create a new goal
			// ask the planner if this new goal can be added
			// hand this goal to execution
			if (managedMotives.size() == 1) {
				sleepComponent(2000);
				generatePlan();
			}
		} else {
			log("some motive we cannot yet handle");
		}
	}

	/**
	 * Create task with non-crashy default values.
	 * 
	 * @return
	 */
	private PlanningTask newPlanningTask() {
		return new PlanningTask(0, null, null, null, null, Completion.PENDING,
				Completion.PENDING);
	}

	/**
	 * 
	 */
	private void generatePlan() {
		String id = newDataID();

		PlanningTask plan = newPlanningTask();

		String goalString = "";//"(and ";
		for (Motive m : managedMotives) {
			if (m instanceof ExploreMotive) {
				goalString=goalString + GoalTranslator.motive2PlannerGoal((ExploreMotive) m, getOriginMap());
				break; // TODO: just take the first goal for now and skip the rest
			}
			else
				goalString=goalString + GoalTranslator.motive2PlannerGoal((ExploreMotive) m, getOriginMap());
		}
		goalString=goalString+")";
		log("generated goal string: "+goalString);
		plan.goal=goalString;
		//plan.goal = "(forall (?p - place) (= (explored ?p) true))";

		addChangeFilter(ChangeFilterFactory.createIDFilter(id,
				WorkingMemoryOperation.OVERWRITE),
				new WorkingMemoryChangeReceiver() {

					public void workingMemoryChanged(WorkingMemoryChange _wmc)
							throws CASTException {
						planGenerated(_wmc.address, getMemoryEntry(
								_wmc.address, PlanningTask.class));

					}
				});

		try {
			addToWorkingMemory(id, plan);
		} catch (AlreadyExistsOnWMException e) {
			e.printStackTrace();
		}
	}

	private void planGenerated(WorkingMemoryAddress _wma,
			PlanningTask _planningTask) {

		if (_planningTask.planningStatus == Completion.SUCCEEDED) {
			log("planning succeeeeeeeded "
					+ _planningTask.planningStatus.name());
			String id = newDataID();
			PlanProxy pp = new PlanProxy();
			pp.planAddress = _wma;
			try {
				addToWorkingMemory(id, pp);
			} catch (AlreadyExistsOnWMException e) {
				e.printStackTrace();
			}
		} else if (_planningTask.planningStatus == Completion.FAILED) {
			println("planning failed: " + _planningTask.planningStatus + " "
					+ _planningTask.goal);
		} else {
			log("planning status " + _planningTask.planningStatus.name());
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * motivation.components.managers.MotiveManager#retractMotive(motivation
	 * .slice.Motive)
	 */
	@Override
	protected void retractMotive(Motive motive) {
		log("someone decided this motive has to be retracted... type is "
				+ motive.getClass().getSimpleName());
		if (managedMotives.isEmpty()) { // nothing to be done anymore

		}

	}

}
