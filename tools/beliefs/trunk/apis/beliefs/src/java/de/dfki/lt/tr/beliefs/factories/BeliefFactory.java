/**
 * 
 */
package de.dfki.lt.tr.beliefs.factories;

import de.dfki.lt.tr.beliefs.data.GenericBelief;
import de.dfki.lt.tr.beliefs.data.abstractproxies.AbstractProxyFactory;
import de.dfki.lt.tr.beliefs.data.abstractproxies.ProxyFactory;
import de.dfki.lt.tr.beliefs.data.genericproxies.Content;
import de.dfki.lt.tr.beliefs.slice.distribs.ProbDistribution;
import de.dfki.lt.tr.beliefs.slice.epstatus.PrivateEpistemicStatus;
import de.dfki.lt.tr.beliefs.slice.framing.SpatioTemporalFrame;
import de.dfki.lt.tr.beliefs.slice.framing.TemporalInterval;
import de.dfki.lt.tr.beliefs.slice.history.AbstractBeliefHistory;
import de.dfki.lt.tr.beliefs.slice.sitbeliefs.dBelief;

/**
 * @author marc
 * 
 */
public class BeliefFactory extends AbstractProxyFactory<GenericBelief<?, ?>> {

	// public static <T2 extends dBelief> CondIndependentFormulaBeliefProxy<T2>
	// create(Class<? extends T2> type, String beliefType, String id) throws
	// InstantiationException, IllegalAccessException {
	// BeliefFactory<T2> fact = new BeliefFactory<T2>(type);
	// T2 belief = type.newInstance();
	// belief.id=id;
	// belief.content = new CondIndependentDistribs(new HashMap<String,
	// ProbDistribution>());
	// belief.estatus = new PrivateEpistemicStatus("robot");
	// belief.frame = new SpatioTemporalFrame("here", new TemporalInterval());
	// belief.hist = new AbstractBeliefHistory();
	// belief.type = beliefType;
	// return fact.cr eate(belief);
	// }

	public <T extends dBelief, C extends Content<?>> GenericBelief<T, C> create(
			Class<? extends T> type, ProxyFactory<? extends C> fact, Ice.Object o) {
		return GenericBelief.create(type, fact, o);
	}
	
	public <T extends dBelief, C extends Content<?>> GenericBelief<T, C> create(
			Class<? extends T> type, ProxyFactory<? extends C> fact,
			String beliefType, String id) throws InstantiationException,
			IllegalAccessException {
		T belief = type.newInstance();
		belief.id = id;
		belief.content = new ProbDistribution();
		belief.estatus = new PrivateEpistemicStatus("robot");
		belief.frame = new SpatioTemporalFrame("here", new TemporalInterval());
		belief.hist = new AbstractBeliefHistory();
		belief.type = beliefType;
		return create(type, fact, belief);
	}

	@Override
	public GenericBelief<?, ?> create(Ice.Object pd) {
		return GenericBelief.create(dBelief.class, new ContentFactory(), pd);
	}

}
