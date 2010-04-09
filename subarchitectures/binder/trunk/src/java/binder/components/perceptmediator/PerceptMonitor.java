/**
 * 
 */
package binder.components.perceptmediator;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.FutureTask;

import beliefmodels.arch.BeliefException;
import beliefmodels.autogen.beliefs.PerceptBelief;
import binder.arch.BinderException;
import binder.autogen.perceptmanagement.PerceptBeliefMaps;
import binder.components.perceptmediator.transferfunctions.TransferFunction;
import cast.CASTException;
import cast.DoesNotExistOnWMException;
import cast.SubarchitectureComponentException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.ManagedComponent;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryAddress;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTUtils;
import castutils.castextensions.CASTHelper;
import castutils.castextensions.WMEventQueue;

/**
 * @author marc
 * 
 */
public class PerceptMonitor<T extends Ice.ObjectImpl> extends CASTHelper
		implements Runnable {

	public class PerceptChangeReceiver implements WorkingMemoryChangeReceiver {
		/**
		 * @param beliefAddress
		 */
		public PerceptChangeReceiver(WorkingMemoryAddress beliefAddress,
				WorkingMemoryAddress reference) {
			super();
			this.beliefAddress = beliefAddress;
			component.addChangeFilter(ChangeFilterFactory.createAddressFilter(
					reference, WorkingMemoryOperation.OVERWRITE), this);
			component.addChangeFilter(ChangeFilterFactory.createAddressFilter(
					reference, WorkingMemoryOperation.DELETE), this);
		}

		WorkingMemoryAddress beliefAddress;

		/*
		 * (non-Javadoc)
		 * 
		 * @see
		 * cast.architecture.WorkingMemoryChangeReceiver#workingMemoryChanged
		 * (cast.cdl.WorkingMemoryChange)
		 */
		@Override
		public void workingMemoryChanged(WorkingMemoryChange _wmc) {
			log("one reference with id " + _wmc.address.subarchitecture + "::"
					+ _wmc.address.id
					+ " has changed, need to update belief with OP "
					+ _wmc.operation.name());
			if (_wmc.operation == WorkingMemoryOperation.OVERWRITE)
				overwritten(_wmc);
			else if (_wmc.operation == WorkingMemoryOperation.DELETE)
				deleted(_wmc);
		}

		public void overwritten(WorkingMemoryChange _wmc) {
			try {
				final PerceptBelief belief = component.getMemoryEntry(
						beliefAddress, PerceptBelief.class);
				final T percept = component.getMemoryEntry(_wmc.address, type);
				// we do the rest asynchronously
				FutureTask<?> f = new FutureTask<PerceptBelief>(
						new Callable<PerceptBelief>() {
							@Override
							public PerceptBelief call() throws Exception {
								transferFunction.transform(percept, belief);
								// register listener on percept changes
								component.overwriteWorkingMemory(beliefAddress,
										belief);
								return belief;
							}
						});

				log("spawn FutureTask to do the rest");
				executor.execute(f);

				transferFunction.transform(percept, belief);
			} catch (CASTException e) {
				logger.error("CAST exception on overwritten", e);
			} catch (BinderException e) {
				logger.error("BinderException on overwritten", e);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

		public void deleted(WorkingMemoryChange _wmc) {
			try {
				log("source percept deleted");
				component.deleteFromWorkingMemory(beliefAddress);
				component.removeChangeFilter(this);
				PerceptBeliefMaps pbm = PerceptBeliefManager
						.readMaps(component);
				pbm.percept2Belief.remove(_wmc.address);
				PerceptBeliefManager.commitMaps(component, pbm);

			} catch (DoesNotExistOnWMException e1) {
				logger.warn(
						"DoesNotExistOnWMException on deleted... don't worry",
						e1);
			} catch (CASTException e1) {
				logger.warn("some CASTexception... should be too serious here",
						e1);
			}

		}
	}

	private static final int MAX_THREADS = 15;

	WMEventQueue entryQueue;
	Class<T> type;
	TransferFunction<T, PerceptBelief> transferFunction;
	ExecutorService executor;

	/**
	 * @param c
	 */
	public PerceptMonitor(ManagedComponent c, Class<T> type,
			TransferFunction<T, PerceptBelief> transferFunction) {
		super(c);
		this.type = type;
		this.transferFunction = transferFunction;
		entryQueue = new WMEventQueue();
		executor = Executors.newFixedThreadPool(MAX_THREADS);
	}

	@Override
	public void run() {
		log("register ADD listener for type " + type.getSimpleName());
		component.addChangeFilter(ChangeFilterFactory.createTypeFilter(type,
				WorkingMemoryOperation.ADD), entryQueue);
		try {
			while (true) {
				final WorkingMemoryChange ev = entryQueue.take();
				log("new percept: src=" + ev.src);
				final T percept = component.getMemoryEntry(ev.address, type);

				// here we can do some checks to decide if we want that percept
				// be made into a belief

				final PerceptBelief belief = transferFunction.createBelief(
						component.newDataID(), CASTUtils.getTimeServer()
								.getCASTTime());
				final WorkingMemoryAddress beliefWMA = new WorkingMemoryAddress(
						belief.id, component.getSubarchitectureID());

				// we do the rest asynchronously
				FutureTask<?> f = new FutureTask<PerceptBelief>(
						new Callable<PerceptBelief>() {
							@Override
							public PerceptBelief call() throws Exception {
								transferFunction.transform(percept, belief);
								// register listener on percept changes
								new PerceptChangeReceiver(beliefWMA, ev.address);
								component.addToWorkingMemory(belief.id, belief);

								// remember mapping from percept to belief
								log("remember mapping "
										+ CASTUtils.toString(ev.address)
										+ " => "
										+ CASTUtils.toString(beliefWMA));
								PerceptBeliefMaps pbm = PerceptBeliefManager
										.readMaps(component);
								pbm.percept2Belief.put(ev.address, beliefWMA);
								PerceptBeliefManager.commitMaps(component, pbm);
								return belief;
							}
						});

				log("spawn FutureTask to do the rest");
				executor.execute(f);

			}
		} catch (InterruptedException e) {
			logger.warn("interrupted in PerceptMonitor::run: ", e);
		} catch (CASTException e) {
			logger.error("in PerceptMonitor::run: ", e);
		} catch (BinderException e) {
			logger.error("in PerceptMonitor::run: ", e);
		} catch (BeliefException e) {
			logger.error("in PerceptMonitor::run: ", e);
		} finally {
			try {
				component.removeChangeFilter(entryQueue);
			} catch (SubarchitectureComponentException e) {
				logger.error("while removing change filter: ", e);
			}
			logger.debug("thread ended");
		}
	}

}
