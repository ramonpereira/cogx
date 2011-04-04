// =================================================================
// Copyright (C) 2010 DFKI GmbH Talking Robots 
// Geert-Jan M. Kruijff (gj@dfki.de), Pierre Lison (pierre.lison@dfki.de) 
//                                                                                                                          
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License 
// as published by the Free Software Foundation; either version 2.1 of
// the License, or (at your option) any later version.
//                                                                                                                          
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//                                                                                                                          
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
// =================================================================
  
 

#ifndef BINDER_ICE
#define BINDER_ICE 


module org {
module cognitivesystems {
module binder {

	const string POINTERLABEL = "about";

	const string thisAgent = "self";
	const string humanAgent = "human";
	
module mln {

	sequence<string> PredStrSeq;
	sequence<double> WeightSeq;
	sequence<float> ProbSeq;
	
	class Evidence {
		string mrfId;
		PredStrSeq trueEvidence;
		PredStrSeq falseEvidence;
		PredStrSeq oldEvidence;
		PredStrSeq probEvidence;
		WeightSeq weights;
	};
	
	class Query {
		string mrfId;
		PredStrSeq atoms;
	};
	
	class Result {
		string mrfId;
		PredStrSeq atoms;
		ProbSeq probs;
	};
};

};
}; 
}; 

#endif
