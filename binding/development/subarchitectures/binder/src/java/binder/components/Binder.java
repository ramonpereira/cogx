package binder.components;

import java.util.Enumeration;
import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import binder.autogen.core.AlternativeUnionConfigurations;
import binder.autogen.core.Feature;
import binder.autogen.core.FeatureValue;
import binder.autogen.core.PerceivedEntity;
import binder.autogen.core.ProbabilityDistribution;
import binder.autogen.core.Proxy;
import binder.autogen.core.Union;
import binder.autogen.core.UnionConfiguration;
import binder.autogen.distributions.combined.CombinedProbabilityDistribution;
import binder.autogen.distributions.combined.OperationType;
import binder.autogen.distributions.discrete.DiscreteProbabilityDistribution;
import binder.autogen.featvalues.StringValue;
import binder.bayesiannetwork.BayesianNetworkManager;
import binder.utils.GradientDescent;
import binder.utils.ProbDistribUtils;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.ManagedComponent;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTData;

/**
 * The core binding algorithm - it takes proxies as inputs, and generate
 * a distribution of possible union configurations (sequences of unions)
 * for the proxies, given a predefined Bayesian Network specifying
 * the correlations between features
 * 
 * @author Pierre Lison
 * @version 31/08/2008
 * 
 */

public class Binder extends ManagedComponent  {

	// The Bayesian network manager
	BayesianNetworkManager BNManager;

	// Constant controlling the greediness of the binder
	public float ALPHA_CONST = 0.2f;

	// Prior probability of the existence of a proxy
	public float PRIOR_PEXISTS = 0.4f;

	// whether to perform incremental or full rebinding
	public boolean incrementalBinding = true;

	// Filtering parameters: maximum number of union configurations
	// to keep in the binder at a given time
	public int NB_CONFIGURATIONS_TO_KEEP = 1;

	// The union configurations computed for the current state 
	// of the binder WM (modulo filtering)
	Vector<UnionConfiguration> currentUnionConfigurations ;

	
	
	/** 
	 * Add filters on proxy insertions, modifications (overwrite) and deletions,
	 * and initialise the binder
	 * 
	 */
	
	@Override
	public void start() {

		// Proxy insertion
		addChangeFilter(ChangeFilterFactory.createGlobalTypeFilter(Proxy.class,
				WorkingMemoryOperation.ADD), new WorkingMemoryChangeReceiver() {

			public void workingMemoryChanged(WorkingMemoryChange _wmc) {
				try {
					proxyInsertion(_wmc);
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			} 
		});

		// Proxy modification
		addChangeFilter(ChangeFilterFactory.createGlobalTypeFilter(Proxy.class,
				WorkingMemoryOperation.OVERWRITE), new WorkingMemoryChangeReceiver() {

			public void workingMemoryChanged(WorkingMemoryChange _wmc) {
				try {
					proxyUpdate(_wmc);
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			} 
		});
		
		// Proxy deletion
		addChangeFilter(ChangeFilterFactory.createGlobalTypeFilter(Proxy.class,
				WorkingMemoryOperation.DELETE), new WorkingMemoryChangeReceiver() {

			public void workingMemoryChanged(WorkingMemoryChange _wmc) {
				try {
					proxyDeletion(_wmc);
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			} 
		});

		// Initialisation stuff
		BNManager = new BayesianNetworkManager();
		initializeUnionConfigurations();

		log("Binding Monitor successfully started");
	}


	/**
	 * (re)initialize the binder with a single union configuration
	 */
	
	public void initializeUnionConfigurations () {
		currentUnionConfigurations = new Vector<UnionConfiguration>();
		UnionConfiguration initialConfig = new UnionConfiguration();
		initialConfig.includedUnions = new Union[0];
		currentUnionConfigurations.add(initialConfig);
	}


	
	/**
	 * Get all the proxies included in a set of perceptual entities (in case the entity
	 * is a proxy, it is simply added, and in case it is an union, the set of all included
	 * proxies is added to the resulting set)
	 * @param includedEntities the set of perceptual entities
	 * @return the resulting set of proxies
	 */
	private Vector<Proxy> getProxies (Vector<PerceivedEntity> includedEntities) {
		Vector<Proxy> includedProxies = new Vector<Proxy>();
		for (Enumeration<PerceivedEntity> en = includedEntities.elements() ; en.hasMoreElements() ;) {
			PerceivedEntity entity = en.nextElement();
			if (entity.getClass().equals(Proxy.class)) {
				includedProxies.add((Proxy)entity);
			}
			else if (entity.getClass().equals(Union.class)) {
				Union includedUnion = (Union)entity;
				for (int i = 0 ; i < includedUnion.includedProxies.length ; i++) {
					includedProxies.add(includedUnion.includedProxies[i]);
				}
			}
		}
		return includedProxies;
	}
	

	private float computeProbExists (Vector<PerceivedEntity> includedEntities) {
		float probExists = PRIOR_PEXISTS;
		float probNotExists = (1 - PRIOR_PEXISTS);

		for (Enumeration<PerceivedEntity> e = includedEntities.elements(); e.hasMoreElements();) {
			PerceivedEntity prox = e.nextElement();
			probExists = probExists * prox.probExists; 
			probNotExists = probNotExists * (1- prox.probExists); 
		}	
		
		probExists = probExists / ((float) Math.pow(PRIOR_PEXISTS,(includedEntities.size())));
		probNotExists = probNotExists / ((float) Math.pow((1-PRIOR_PEXISTS),(includedEntities.size())));
		
		float normConstant = 1.0f / (probExists + probNotExists);
		probExists = normConstant * probExists;
		return probExists;
	}
	
	
	
	private Vector<Feature> getFeatures (Vector<PerceivedEntity> includedEntities) {
		Vector<Feature> features = new Vector<Feature>();

		for (Enumeration<PerceivedEntity> e = includedEntities.elements(); e.hasMoreElements();) {
			PerceivedEntity prox = e.nextElement();
			for (int i = 0; i < prox.features.length ; i++) {
				Feature feat = new Feature();
				feat.featlabel = prox.features[i].featlabel;
				feat.alternativeValues = new FeatureValue[prox.features[i].alternativeValues.length];
				for (int j =0; j < prox.features[i].alternativeValues.length ; j++) {
					feat.alternativeValues[j] = new FeatureValue();
					if (prox.features[i].alternativeValues[j].getClass().equals(StringValue.class)) {
						feat.alternativeValues[j] = 
							new StringValue(prox.features[i].alternativeValues[j].independentProb,
								((StringValue)prox.features[i].alternativeValues[j]).val);
					}
				}
				features.add(feat);
			}
		}
		return features;
	}
	
	
	/**
	 * Construct a new union based on the merge of several perceptual entities
	 * (which may be proxies or unions themselves)
	 * 
	 * @param includedEntities as vector of entities (with size > 0 )
	 * @return the new union
	 */
	
	public Union constructNewUnion(Vector<PerceivedEntity> includedEntities) {
		//	log("***** Constructing a new union ****");
		
		// Create a new union with a new data ID
		Union union = new Union();
		union.entityID = newDataID();
	
		// Specify the proxies included in the union
		Vector<Proxy> includedProxies = getProxies(includedEntities);
		union.includedProxies = new Proxy[includedProxies.size()];
		union.includedProxies = includedProxies.toArray(union.includedProxies);

		// Compute the existence probability of the union
		union.probExists = computeProbExists(includedEntities);

		// Extract the possible features for the union
		Vector<Feature> features = getFeatures(includedEntities);
		union.features = new Feature[features.size()];
		union.features = features.toArray(union.features);

		// Finally, compute the union distribution
		if (includedEntities.size() > 1) {
			union.distribution = computeUnionDistribution(union);
		}
		else {
			union.distribution = includedEntities.elementAt(0).distribution;
		}

		return union;
	}


	/** 
	 * Compute the probability distribution of the union
	 * @param union
	 * @return
	 */
	private ProbabilityDistribution computeUnionDistribution(Union union) {

		// Compute the prior distribution for the union
		DiscreteProbabilityDistribution priorDistrib =  BNManager.getPriorDistribution(union.features);

//		log("Maximum for prior distribution of the union: " + GradientDescent.getMaximum(priorDistrib));

		Vector<CombinedProbabilityDistribution> proxiesDistrib = 
			new Vector<CombinedProbabilityDistribution>();

		for (int i = 0 ; i < union.includedProxies.length ; i++) {
			Proxy proxy = union.includedProxies[i];

			//		log("Maximum for observation-driven distribution of the proxy " + i +  ": "
			// + GradientDescent.getMaximum(proxy.distribution));

			DiscreteProbabilityDistribution priorDistribForProxy = BNManager.getPriorDistribution(proxy);
			
			//		log("Maximum for prior distribution of the proxy " + i +  
			// ": " + GradientDescent.getMaximum(priorDistribForProxy));

			CombinedProbabilityDistribution finalProxyDistrib = new CombinedProbabilityDistribution();
			finalProxyDistrib.opType = OperationType.MULTIPLIED;
			finalProxyDistrib.distributions = new DiscreteProbabilityDistribution[2];
			finalProxyDistrib.distributions[0] = proxy.distribution;
			finalProxyDistrib.distributions[1] = 
				ProbDistribUtils.invertDistribution(priorDistribForProxy);
			finalProxyDistrib.distributions[1] = 
				ProbDistribUtils.multiplyDistributionWithConstantValue
				((DiscreteProbabilityDistribution)finalProxyDistrib.distributions[1], ALPHA_CONST);
	
			//		log("Maximum for final distribution of the proxy " + 
			// i +  ": " + GradientDescent.getMaximum(finalProxyDistrib));

			proxiesDistrib.add(finalProxyDistrib);
		}

		CombinedProbabilityDistribution finalDistrib = new CombinedProbabilityDistribution();
		finalDistrib.opType = OperationType.MULTIPLIED;
		finalDistrib.distributions = new ProbabilityDistribution[proxiesDistrib.size() + 1];

		finalDistrib.distributions[0] = 
			ProbDistribUtils.multiplyDistributionWithConstantValue(priorDistrib, ALPHA_CONST);

		for (int i = 1 ; i < proxiesDistrib.size() + 1 ; i++ ) {
			finalDistrib.distributions[i] = proxiesDistrib.elementAt(i-1);
		}

		//		log("Maximum for final distribution of the union: " + GradientDescent.getMaximum(finalDistrib));

		// finalDistrib = ProbDistribUtils.normaliseDistribution(finalDistrib, 1.0f);
		return finalDistrib;
	}




	public Union getInitialUnion(Proxy proxy) {
		Vector<PerceivedEntity> curProxyV = new Vector<PerceivedEntity>();
		curProxyV.add(proxy);
		Union union = constructNewUnion(curProxyV);
		return union;
	} 


	@Override
	public void configure(Map<String, String> _config) {
		if (_config.containsKey("--alpha")) {
			ALPHA_CONST = Float.parseFloat(_config.get("--alpha"));
		} 
		if (_config.containsKey("--incremental")) {
			incrementalBinding = Boolean.parseBoolean(_config.get("--incremental"));
		} 
	}


	
	public void fullRebinding() {
		try {
			log("Perform full rebinding...");
		
			initializeUnionConfigurations();

			CASTData<Proxy>[] proxies = getWorkingMemoryEntries(Proxy.class);
			for (int i = 0 ; i < proxies.length; i++) {
				incrementalBinding(proxies[i].getData());
			}

		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}

	
	
	private Vector<PerceivedEntity> getOtherProxies (Proxy[] proxies, Proxy proxyToExclude) {
		
		Vector<PerceivedEntity> otherProxies = new Vector<PerceivedEntity>();
		
		for (int i = 0; i < proxies.length ; i++) {
			Proxy prox = proxies[i];
			if (!prox.equals(proxyToExclude)) {
				otherProxies.add(prox);
			}
		}
		return otherProxies;
	}
	
	
	public void proxyUpdate (WorkingMemoryChange wmc) {
	
		log("--------START BINDING----------");
		log("binder working memory updated with an overwrite of an existing proxy!");
		
		try {
			Proxy updatedProxy= getMemoryEntry(wmc.address, Proxy.class);
			
			if (updatedProxy.distribution == null) {
				updatedProxy.features = ProbDistribUtils.addIndeterminateFeatureValues(updatedProxy.features);
				updatedProxy.distribution = ProbDistribUtils.generateProbabilityDistribution(updatedProxy);
			}
		
			for (Enumeration<UnionConfiguration> configs = 
				currentUnionConfigurations.elements() ; configs.hasMoreElements(); ) {
									
					UnionConfiguration existingUnionConfig = configs.nextElement();				
									
					for (int i = 0 ; i < existingUnionConfig.includedUnions.length; i++) {
						
						Union existingUnion = existingUnionConfig.includedUnions[i];
						
						for (int j = 0; j < existingUnion.includedProxies.length ; j++) {
							
							Proxy existingProxy = existingUnion.includedProxies[j];
							
							if (existingProxy.entityID.equals(updatedProxy.entityID)) {
								Vector<PerceivedEntity> proxies = 
									getOtherProxies(existingUnion.includedProxies, existingProxy);
								proxies.add(updatedProxy);
								Union updatedUnion = constructNewUnion(proxies);								
								updatedUnion.entityID = existingUnion.entityID ;
								existingUnionConfig.includedUnions[i] = updatedUnion;
							}
						}
					}
					
			}

			AlternativeUnionConfigurations alters = buildNewAlternativeUnionConfigurations();
			updateWM(alters); 
			
			}
			catch (Exception e) {
				e.printStackTrace();
			}

			log("--------STOP BINDING----------");
		}



	
	private UnionConfiguration removeUnionFromConfig
	(UnionConfiguration existingconfig, Union unionToDelete) {
		
		Vector<Union> unionsInConfig = new Vector<Union>();
		for (int i = 0; i < existingconfig.includedUnions.length ; i++) {
			Union curUnion = existingconfig.includedUnions[i];
			if (!curUnion.equals(unionToDelete)) {
				unionsInConfig.add(curUnion);
			}
		}
		existingconfig.includedUnions = new Union[unionsInConfig.size()];
		existingconfig.includedUnions = unionsInConfig.toArray(existingconfig.includedUnions);
		
		return existingconfig;
	}
	
	
	public void proxyDeletion (WorkingMemoryChange wmc) {
	
		log("--------START BINDING----------");
		log("binder working memory updated with a deletion of an existing proxy!");
		
		try {
			String deletedProxyID= wmc.address.id;
		
			for (Enumeration<UnionConfiguration> configs = 
				currentUnionConfigurations.elements() ; configs.hasMoreElements(); ) {
									
					UnionConfiguration existingUnionConfig = configs.nextElement();				
									
					for (int i = 0 ; i < existingUnionConfig.includedUnions.length; i++) {
						
						Union existingUnion = existingUnionConfig.includedUnions[i];
						
						for (int j = 0; j < existingUnion.includedProxies.length ; j++) {
							
							Proxy existingProxy = existingUnion.includedProxies[j];
							
							if (existingProxy.entityID.equals(deletedProxyID)) {
								Vector<PerceivedEntity> proxies = 
									getOtherProxies(existingUnion.includedProxies, existingProxy);
								if (proxies.size() > 0) { 
									Union updatedUnion = constructNewUnion(proxies);								
									updatedUnion.entityID = existingUnion.entityID ;
									existingUnionConfig.includedUnions[i] = updatedUnion;
								}
								else {
									existingUnionConfig = 
										removeUnionFromConfig(existingUnionConfig, existingUnion);
								}
							}
						}
					}
					
			}

			AlternativeUnionConfigurations alters = buildNewAlternativeUnionConfigurations();
			updateWM(alters); 
			
			}
			catch (Exception e) {
				e.printStackTrace();
			}

			log("--------STOP BINDING----------");
		}

	
	
	public void proxyInsertion(WorkingMemoryChange wmc) {
		log("--------START BINDING----------");
		log("binder working memory updated with a new proxy!");

		long initTime = System.currentTimeMillis();

		if (incrementalBinding) {
			try {
			Proxy newProxy = getMemoryEntry(wmc.address, Proxy.class);
			
			newProxy.features = ProbDistribUtils.addIndeterminateFeatureValues(newProxy.features);
			newProxy.distribution = ProbDistribUtils.generateProbabilityDistribution(newProxy);
			
			incrementalBinding(newProxy);
			}
			catch (Exception e) {
				e.printStackTrace();
			}
		}
		else  {
	//		fullRebinding();
		}

		long finalTime = System.currentTimeMillis();
		log("Total binding time: " + (finalTime - initTime)/1000.0 + " seconds");
		log("--------STOP BINDING----------");

	}
	
	
	
	public void incrementalBinding(Proxy newProxy) {
		try {
			log("Perform incremental binding...");

			log("Proxy ID: " + newProxy.entityID);

			Union newUnion = getInitialUnion(newProxy);
			
			log("Construction of initial unions finished, moving to unions of more than 1 proxy...");
						
			Vector<UnionConfiguration> newUnionConfigurations = new Vector<UnionConfiguration>();
			HashMap<String,Union> alreadyMergedUnions = new HashMap<String, Union>();
			
			for (Enumeration<UnionConfiguration> configs = 
				currentUnionConfigurations.elements() ; configs.hasMoreElements(); ) {
				
				UnionConfiguration existingUnionConfig = configs.nextElement();				
				
				UnionConfiguration newConfigWithSingleUnion = 
					createNewUnionConfiguration (existingUnionConfig, newUnion);
				newUnionConfigurations.add(newConfigWithSingleUnion);	
				
				for (int i = 0 ; i < existingUnionConfig.includedUnions.length; i++) {
					
					Union existingUnion = existingUnionConfig.includedUnions[i];
					
					Union newMergedUnion;
					if (!hasConflictingSubarch(existingUnion, newUnion)) {
						
						if (!alreadyMergedUnions.containsKey(existingUnion.entityID)) {
						Vector<PerceivedEntity> unionsToMerge = new Vector<PerceivedEntity>();
						unionsToMerge.add(existingUnion);
						unionsToMerge.add(newUnion);
						newMergedUnion = constructNewUnion(unionsToMerge);
						alreadyMergedUnions.put(existingUnion.entityID, newMergedUnion);
						}
						else {
							newMergedUnion = alreadyMergedUnions.get(existingUnion.entityID);
						}
						UnionConfiguration newConfigWithMergedUnion = 
							createNewUnionConfiguration (existingUnionConfig, newMergedUnion, existingUnion);
						newUnionConfigurations.add(newConfigWithMergedUnion);
						
					}
				}
			}
						
			Vector<UnionConfiguration> NBests = 
				GradientDescent.getNBestUnionConfigurations
				(newUnionConfigurations, NB_CONFIGURATIONS_TO_KEEP);
			
			currentUnionConfigurations = NBests;

			log("Total number of union configurations generated: " + currentUnionConfigurations.size());
			
	
			AlternativeUnionConfigurations alters = buildNewAlternativeUnionConfigurations();
			updateWM(alters); 

		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}

	
	private AlternativeUnionConfigurations buildNewAlternativeUnionConfigurations () {
		
		AlternativeUnionConfigurations alters = new AlternativeUnionConfigurations();
		alters.alterconfigs = new UnionConfiguration[currentUnionConfigurations.size()];
		for (int i = 0; i < alters.alterconfigs.length ; i++) {
			alters.alterconfigs[i] = currentUnionConfigurations.elementAt(i); 
		}
		return alters;
	}
	
	

	private UnionConfiguration createNewUnionConfiguration
	(UnionConfiguration existingUnionConfig, Union unionToAdd) {			
		return createNewUnionConfiguration(existingUnionConfig, unionToAdd, new Vector<Union>());
	}
	
	
	
	private UnionConfiguration createNewUnionConfiguration
		(UnionConfiguration existingUnionConfig, Union unionToAdd, Union unionToRemove) {
		
		Vector<Union> unionsToRemove = new Vector<Union>();
		unionsToRemove.add(unionToRemove);
		return createNewUnionConfiguration(existingUnionConfig, unionToAdd, unionsToRemove);
	}
	
	
	
	private UnionConfiguration createNewUnionConfiguration(UnionConfiguration existingUnionConfig, 
			Union unionToAdd, Vector<Union> unionsToRemove) {
		
		UnionConfiguration newConfig = new UnionConfiguration();
		newConfig.includedUnions = new Union[existingUnionConfig.includedUnions.length + 1 - unionsToRemove.size()];
		
		int count = 0;
		for (int i = 0 ; i < existingUnionConfig.includedUnions.length; i++) {
			if (!unionsToRemove.contains(existingUnionConfig.includedUnions[i])) {
				newConfig.includedUnions[i- count] = existingUnionConfig.includedUnions[i];
			}
			else {
				count ++;
			}
		}
		
		newConfig.includedUnions[existingUnionConfig.includedUnions.length - unionsToRemove.size()] = unionToAdd;
		
		return newConfig;
		
	}

	private boolean hasConflictingSubarch(Union union1, Union union2) {	
		for (int i = 0 ; i < union1.includedProxies.length ; i++) {
			Proxy proxyi = union1.includedProxies[i];
			for (int j = 0 ; j < union2.includedProxies.length ; j++) {
				Proxy proxyj = union2.includedProxies[j];
				if (proxyi.subarchId.equals(proxyj.subarchId)) {
					return true;
				}
			}
		}		
		return false;
	}

	private void updateWM(AlternativeUnionConfigurations configs) {

		try {
			
			CASTData<AlternativeUnionConfigurations>[] alterconfigs = 
				getWorkingMemoryEntries(AlternativeUnionConfigurations.class);
			
			if (alterconfigs.length == 0) {
				addToWorkingMemory(newDataID(), configs);
			}
			else {
				overwriteWorkingMemory(alterconfigs[0].getID(), configs);
			}
			
			

		}
		catch (Exception e) {
			e.printStackTrace();
		}
	}
	
}
