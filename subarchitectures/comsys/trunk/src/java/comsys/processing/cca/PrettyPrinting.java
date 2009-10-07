package comsys.processing.cca;

import comsys.datastructs.comsysEssentials.*;
import Abducer.*;
import beliefmodels.adl.*;
import beliefmodels.domainmodel.cogx.*;
import binder.utils.BeliefModelUtils;

public class PrettyPrinting {

	public static String proofToString(MarkedQuery[] proof) {
		String s = "";
		for (int i = 0; i < proof.length; i++) {
			s += "  [" + proof[i].mark.toString() + "]\t";
			s += MercuryUtils.modalisedFormulaToString(proof[i].body);
			if (i < proof.length-1) { s += ",\n"; }
		}
		return s;
	}
		
	public static String proofBlockToString(ProofBlock block) {
		String s = "[ProofBlock ";
		s += "proofId = " + block.proofId + ",\n";
		s += "  assertions = {\n";
		for (int i = 0; i < block.assertions.length; i++) {
			s += "    " + beliefToString(block.assertions[i]);
			s += (i < block.assertions.length-1) ? ",\n" : "\n";
		}
		s += "  }\n";
		s += "  assumptions = {\n";
		for (int i = 0; i < block.assumptions.length; i++) {
			s += "    " + beliefToString(block.assumptions[i]);
			s += (i < block.assumptions.length-1) ? ",\n" : "\n";
		}
		s += "  }\n]";
		return s;
	}
	
	public static String agentStatusToString(AgentStatus as) {
		if (as instanceof PrivateAgentStatus) {
			PrivateAgentStatus priv = (PrivateAgentStatus) as;
			return "{" + priv.ag.id + "}";
		}
		else if (as instanceof AttributedAgentStatus) {
			AttributedAgentStatus attrib = (AttributedAgentStatus) as;
			return "{" + attrib.ag.id + "[" + attrib.ag2.id + "]}";
		}
		else if (as instanceof MutualAgentStatus) {
			MutualAgentStatus mutual = (MutualAgentStatus) as;
			String s = "";
			for (int i = 0; i < mutual.ags.length; i++) {
				s += mutual.ags[i].id;
				s += (i < mutual.ags.length-1) ? "," : "";
			}
			return "{" + s + "}";
		}
		else {
			return null;
		}
	}
	
	public static String beliefToString(Belief b) {
		String s = "[Belief ";
		s += "agents = " + agentStatusToString(b.ags);
		s += ": ";
		s += BeliefModelUtils.getFormulaPrettyPrint((UncertainSuperFormula)b.phi);
		s += "]";
		return s;
	}
	
}
