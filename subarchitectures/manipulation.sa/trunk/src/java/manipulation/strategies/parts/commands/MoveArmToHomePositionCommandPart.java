package manipulation.strategies.parts.commands;

import java.util.Observable;
import java.util.Observer;

import manipulation.commandWatcher.CommandWatcher;
import manipulation.core.share.Manipulator;
import manipulation.core.share.exceptions.ManipulatorException;
import manipulation.runner.cogx.CogXRunner;
import manipulation.slice.FarArmMovementCommand;
import manipulation.slice.LinearBaseMovementApproachCommand;
import manipulation.slice.LinearGraspApproachCommand;
import manipulation.slice.ManipulationCommand;
import manipulation.slice.ManipulationCommandStatus;
import manipulation.slice.MoveArmToHomePositionCommand;
import manipulation.slice.PutDownCommand;
import manipulation.slice.SimulateGraspCommand;
import manipulation.slice.StopCommand;
import manipulation.strategies.CommandExecution;
import manipulation.strategies.Strategy;
import manipulation.strategies.parts.StrategyPart;

import org.apache.log4j.Logger;

/**
 * defines a behaviour to reach a position in front of an object with the arm
 * 
 * @author ttoenige
 * 
 */
public class MoveArmToHomePositionCommandPart extends StrategyPart implements
		Observer {

	private Logger logger = Logger.getLogger(this.getClass());

	public MoveArmToHomePositionCommandPart(Manipulator manipulator,
			Strategy globalStrategy) {
		setManipulator(manipulator);
		setGlobalStrategy(globalStrategy);
		setPartName(PartName.MOVE_ARM_TO_HOME_POSITION_COMMAND_PART);
	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void execute() {
		logger.debug("execute: " + this.getClass());

		getManipulator().getWatcher().addObserver(this);

		try {
			getManipulator().getArmConnector().goHome();
		} catch (ManipulatorException e1) {
			logger.error(e1);
		}

		synchronized (this) {
			try {
				wait();
			} catch (InterruptedException e) {
				logger.error(e);
			}
		}

		logger.debug("we go on!");
		changeToNextPart();

	}

	/**
	 * {@inheritDoc}
	 */
	@Override
	public void changeToNextPart() {
		getManipulator().getWatcher().deleteObserver(this);

		getGlobalStrategy().setNextPart(
				getGlobalStrategy().getPart(getNextPartName()));
	}

	/**
	 * 
	 * {@inheritDoc}
	 */
	@Override
	public void update(Observable observable, Object arg) {
		if (observable instanceof CommandWatcher) {
			if (arg instanceof FarArmMovementCommand) {
				logger.info("far arm movement command");

				ManipulationCommand currentCom = ((CommandExecution) getGlobalStrategy())
						.getCurrentCommand();
				currentCom.status = ManipulationCommandStatus.COMMANDFAILED;
				((CogXRunner) (getManipulator().getRunner()))
						.updateWorkingMemoryCommand(getManipulator()
								.getWatcher().getCurrentCommandAddress(),
								currentCom);

				setNextPartName(PartName.FAR_ARM_MOVEMENT_COMMAND_PART);
				((CommandExecution) getGlobalStrategy())
						.setCurrentCommand((ManipulationCommand) arg);
				synchronized (this) {
					notifyAll();
				}
			} else if (arg instanceof PutDownCommand) {
				logger.info("put down command");

				ManipulationCommand currentCom = ((CommandExecution) getGlobalStrategy())
						.getCurrentCommand();
				currentCom.status = ManipulationCommandStatus.COMMANDFAILED;
				((CogXRunner) (getManipulator().getRunner()))
						.updateWorkingMemoryCommand(getManipulator()
								.getWatcher().getCurrentCommandAddress(),
								currentCom);

				setNextPartName(PartName.PUT_DOWN_COMMAND_PART);
				((CommandExecution) getGlobalStrategy())
						.setCurrentCommand((ManipulationCommand) arg);
				synchronized (this) {
					notifyAll();
				}
			} else if (arg instanceof LinearGraspApproachCommand) {
				logger.info("linear grasp approach command");

				ManipulationCommand currentCom = ((CommandExecution) getGlobalStrategy())
						.getCurrentCommand();
				currentCom.status = ManipulationCommandStatus.COMMANDFAILED;
				((CogXRunner) (getManipulator().getRunner()))
						.updateWorkingMemoryCommand(getManipulator()
								.getWatcher().getCurrentCommandAddress(),
								currentCom);

				setNextPartName(PartName.LINEAR_GRASP_APPROACH_COMMAND_PART);
				((CommandExecution) getGlobalStrategy())
						.setCurrentCommand((ManipulationCommand) arg);
				synchronized (this) {
					notifyAll();
				}
			} else if (arg instanceof SimulateGraspCommand) {
				logger.info("simulate grasp command");

				ManipulationCommand currentCom = ((CommandExecution) getGlobalStrategy())
						.getCurrentCommand();
				currentCom.status = ManipulationCommandStatus.COMMANDFAILED;
				((CogXRunner) (getManipulator().getRunner()))
						.updateWorkingMemoryCommand(getManipulator()
								.getWatcher().getCurrentCommandAddress(),
								currentCom);

				setNextPartName(PartName.SIMULATE_GRASP_COMMAND_PART);
				((CommandExecution) getGlobalStrategy())
						.setCurrentCommand((ManipulationCommand) arg);
				synchronized (this) {
					notifyAll();
				}
			} else if (arg instanceof LinearBaseMovementApproachCommand) {
				logger.info("linear base movement approach command");

				ManipulationCommand currentCom = ((CommandExecution) getGlobalStrategy())
						.getCurrentCommand();
				currentCom.status = ManipulationCommandStatus.COMMANDFAILED;
				((CogXRunner) (getManipulator().getRunner()))
						.updateWorkingMemoryCommand(getManipulator()
								.getWatcher().getCurrentCommandAddress(),
								currentCom);

				setNextPartName(PartName.LINEAR_BASE_MOVEMENT_APPROACH_COMMAND_PART);
				((CommandExecution) getGlobalStrategy())
						.setCurrentCommand((ManipulationCommand) arg);
				synchronized (this) {
					notifyAll();
				}
			} else if (arg instanceof StopCommand) {
				logger.info("stop command");

				ManipulationCommand currentCom = ((CommandExecution) getGlobalStrategy())
						.getCurrentCommand();
				currentCom.status = ManipulationCommandStatus.COMMANDFAILED;
				((CogXRunner) (getManipulator().getRunner()))
						.updateWorkingMemoryCommand(getManipulator()
								.getWatcher().getCurrentCommandAddress(),
								currentCom);

				setNextPartName(PartName.STOP_COMMAND_PART);
				((CommandExecution) getGlobalStrategy())
						.setCurrentCommand((ManipulationCommand) arg);
				synchronized (this) {
					notifyAll();
				}
			} else if (arg instanceof MoveArmToHomePositionCommand) {
				logger.info("move arm to home position command");

				ManipulationCommand currentCom = ((CommandExecution) getGlobalStrategy())
						.getCurrentCommand();
				currentCom.status = ManipulationCommandStatus.COMMANDFAILED;
				((CogXRunner) (getManipulator().getRunner()))
						.updateWorkingMemoryCommand(getManipulator()
								.getWatcher().getCurrentCommandAddress(),
								currentCom);

				setNextPartName(PartName.MOVE_ARM_TO_HOME_POSITION_COMMAND_PART);
				((CommandExecution) getGlobalStrategy())
						.setCurrentCommand((ManipulationCommand) arg);
				synchronized (this) {
					notifyAll();
				}
			}
		}
	}
}
