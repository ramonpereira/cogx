package binder.abstr;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import java.util.Map.Entry;

import binder.utils.FileUtils;

import beliefmodels.arch.BeliefException;
import beliefmodels.autogen.beliefs.Belief;
import beliefmodels.autogen.beliefs.MultiModalBelief;
import beliefmodels.autogen.beliefs.TemporalUnionBelief;
import beliefmodels.autogen.distribs.FeatureValueProbPair;
import beliefmodels.autogen.featurecontent.PointerValue;
import beliefmodels.autogen.history.CASTBeliefHistory;
import beliefmodels.builders.TemporalUnionBuilder;
import beliefmodels.utils.DistributionUtils;
import beliefmodels.utils.FeatureContentUtils;
import binder.arch.BindingWorkingMemory;
import binder.ml.MLException;
import binder.utils.MLNGenerator;
import binder.utils.MLNPreferences;
import cast.AlreadyExistsOnWMException;
import cast.ConsistencyException;
import cast.DoesNotExistOnWMException;
import cast.PermissionException;
import cast.SubarchitectureComponentException;
import cast.UnknownSubarchitectureException;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryAddress;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTData;

public abstract class MarkovLogicComponent<T extends Belief> extends FakeComponent {

	protected static String markovlogicDir = "subarchitectures/binder/markovlogic/";

	protected String inferCmd = "tools/alchemy/bin/infer";

	protected String emptyFile = markovlogicDir + "empty.db";

	private String query = "Outcome";

	protected String resultsFile = markovlogicDir + "unions.results";

	public float lowestProbThreshold = 0.20f;
	public int maxAlternatives = 1;
	public float minProbDifferenceForUpdate = 0.1f;

	
	public MarkovLogicComponent(Class<T> cls) {
	}

	/**
	 * 
	 * @param mlnFile
	 * @param resultsFile
	 * @return
	 * @throws BeliefException
	 */
	public HashMap<String,Float> runAlchemyInference(String mlnFile, String resultsFile) throws BeliefException {

		Runtime run = Runtime.getRuntime(); 
		log("Now running Alchemy...");
		try {
			String[] args = {inferCmd, "-i", mlnFile, "-e", emptyFile, "-r", resultsFile, "-q", query, "-maxSteps", "50"};
			Process p = run.exec(args);

			BufferedReader stdInput = new BufferedReader(new InputStreamReader(p.getInputStream()));

			boolean inferenceSuccessful = true;

			String s;
			String output = "";
			while ((s = stdInput.readLine()) != null) {
				output += s + "\n";
				if (s.contains("ERROR")) {
					inferenceSuccessful = false;
				}
			}

			if (inferenceSuccessful) {
				log("Alchemy inference was successful, now retrieving the results");

				return readResultsFile (resultsFile);
			}

			else {
				log("ERROR: Alchemy inference failed");
				System.out.println(output);
				throw new BeliefException("ERROR: Alchemy inference failed");
			}
		}
		catch (IOException e) {
			e.printStackTrace();
		}
		return new HashMap<String,Float>();
	}
	
	protected abstract HashMap<String, Belief> extractExistingUnions();
	
	
	/**
	 * Perform the generic inference operation for the given belief, and subsequently update
	 * the working memory with the outcome (and possibly also with updates on existing ones)
	 * 
	 * @param belief the new belief which was inserted
	 */
	public List<Belief> performInference(T belief, WorkingMemoryAddress beliefWMAddress, MLNPreferences prefs)
		throws BeliefException {
		// extract the unions already existing in the binder WM
		Map<String, Belief> existingUnions = extractExistingUnions();
		
		Map<String, Belief> relevantUnions = selectRelevantUnions(existingUnions, belief);
		
		log("nb existing unions: " + existingUnions.size());
		log("nb relevant unions: " + relevantUnions.size());
		log("tracking activated for belief: " + prefs.isTrackingActivated());
		
		if (relevantUnions.size() > 0 && 
				prefs.isTrackingActivated()) {
			
			log("doing markov logic inference on belief "  + belief.id);
			log("Markov Logic Network used: " + prefs.getFile_correlations());
			return performMarkovLogicInference(belief, beliefWMAddress, relevantUnions, prefs);
		}
		
		else { 
			return performDirectInference(belief, beliefWMAddress);
		}
	}

	private List<Belief> performMarkovLogicInference (T belief,
			WorkingMemoryAddress beliefWMAddress,
			Map<String, Belief> relevantUnions, MLNPreferences prefs) {
		
		List<Belief> resultingBeliefs = new LinkedList<Belief>();
		
		// Create identifiers for each possible new union		
		Map<String, String> unionsMapping = createIdentifiersForNewUnions(relevantUnions);
		
		String newSingleUnionId = newDataID();
		
		// Write the Markov logic network to a file
		writeMarkovLogic(belief, relevantUnions, unionsMapping, newSingleUnionId, prefs);

		// run the alchemy inference
		try { 
			HashMap<String,Float> inferenceResults = runAlchemyInference(prefs.getGeneratedMLNFile(), resultsFile);

			log("filtering inference results to keep only the " + maxAlternatives + " best alternatives");
			HashMap<String,Float> filteredInferenceResults = filterInferenceResults(inferenceResults);

			for (String id : filteredInferenceResults.keySet()) {
				if (existsOnWorkingMemory(id)) {
					
					if (filteredInferenceResults.get(id) > lowestProbThreshold) {
						resultingBeliefs.add(getMemoryEntry(new WorkingMemoryAddress(id, BindingWorkingMemory.BINDER_SA), 
							TemporalUnionBelief.class));
					}
				}
				else {		
					
					TemporalUnionBelief newBelief = TemporalUnionBuilder.createNewSingleUnionBelief((MultiModalBelief) belief, beliefWMAddress, id);
					float existProb = DistributionUtils.getExistenceProbability(newBelief) * filteredInferenceResults.get(id);
					DistributionUtils.setExistenceProbability(newBelief, existProb);
					
					if (DistributionUtils.getExistenceProbability(newBelief) > lowestProbThreshold)  {

						resultingBeliefs.add(newBelief);
					}
				}
			}
						
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		return resultingBeliefs;
	}


	private List<Belief> performDirectInference (T belief,
			WorkingMemoryAddress beliefWMAddress) {

		List<Belief> resultingBeliefs = new LinkedList<Belief>();
		try {
			log("no markov logic tracking for mmbelief " + belief.id);
			Belief union = createNewSingleUnionBelief(belief, beliefWMAddress);
			resultingBeliefs.add(union);
		
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		return resultingBeliefs;
	}
	
	
	protected abstract Belief createNewSingleUnionBelief(T belief,
			WorkingMemoryAddress beliefWMAddress) throws BeliefException;

	
	private Map<String, String> createIdentifiersForNewUnions(
			Map<String, Belief> relevantUnions) {
		Map<String,String> unionsMapping = new HashMap<String,String>();
		
		for (String existingUnionId : relevantUnions.keySet()) {
			String newUnionId = newDataID();
			unionsMapping.put(existingUnionId, existingUnionId);
		}
		
		log("newly created union ids: " + unionsMapping.keySet().toString());
		return unionsMapping;
	}
	

	private void writeMarkovLogic(T mmbelief,
			Map<String, Belief> relevantUnions,
			Map<String, String> unionsMapping, String newSingleUnionId, MLNPreferences prefs) {
		try {
			MLNGenerator gen = new MLNGenerator(prefs);
			gen.writeMLNFile(mmbelief, relevantUnions.values(), unionsMapping, newSingleUnionId, prefs.getGeneratedMLNFile());
		} catch (MLException e1) {
			e1.printStackTrace();
		}
	}

	/**
	 * 
	 * @param resultsFile
	 * @return
	 */
	public HashMap<String,Float> readResultsFile (String resultsFile) {
		HashMap<String,Float> inferenceResults = new HashMap<String,Float>();

		try {
			FileInputStream fstream = new FileInputStream(resultsFile);
			DataInputStream in = new DataInputStream(fstream);
			BufferedReader br = new BufferedReader(new InputStreamReader(in));
			String strLine;
			while ((strLine = br.readLine()) != null)   {
				log (strLine);
				String line2 = strLine.replace(query+"(", "");
				String markovlogicunion = line2.substring(0, line2.indexOf(")"));
				float prob = Float.parseFloat(line2.substring(line2.indexOf(" ")));

				String union = MLNGenerator.getIDFromMarkovLogicConstant(markovlogicunion);

				inferenceResults.put(union, prob);
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}

		return inferenceResults;
	}

	
	/**
	 * Filter the inference results to retain only the maxSize-best results
	 * 
	 * @param results the results of Alchemy inference
	 * @return the filtered results
	 */
	protected HashMap<String, Float> filterInferenceResults(HashMap<String,Float> results) {

		List<Entry<String,Float>> list = new LinkedList<Entry<String,Float>>(results.entrySet());
		Collections.sort(list, new Comparator<Entry<String,Float>>() {
			public int compare(Entry<String,Float> o1, Entry<String,Float> o2) {
				return -(Float.compare(o1.getValue().floatValue(), o2.getValue().floatValue()));
			}
		});

		int increment = 0;
		HashMap<String,Float> newResults = new HashMap<String,Float>();
		for (Iterator<Entry<String,Float>> it = list.iterator(); it.hasNext();) {
			Map.Entry<String,Float> entry = (Map.Entry<String,Float>)it.next();
			if (increment < maxAlternatives) {
				newResults.put(entry.getKey(), entry.getValue());
				increment++;
			}
			else {
				newResults.put(entry.getKey(), new Float(0.0f)); 
			}
		}
		return newResults;
	}
	

	/**
	 * Only selecting the unions which are originating from the same subarchitecture as the current 
	 * mmbelief
	 * 
	 * @param existingUnions
	 * @param belief
	 * @return
	 * @throws BeliefException
	 */
	abstract protected Map<String,Belief> selectRelevantUnions(Map<String, Belief> existingUnions, T belief) throws BeliefException  ;


	protected MultiModalBelief duplicateBelief(MultiModalBelief b) throws BeliefException {
		return new MultiModalBelief(b.frame, b.estatus, 	b.id,b.type, 
				FeatureContentUtils.duplicateContent(b.content), b.hist);
	}
}
