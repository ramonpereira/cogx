// =================================================================                                                        
// Copyright (C) 2009-2011 Pierre Lison (pierre.lison@dfki.de)                                                                
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


package binder.fakeproxygenerators;

import binder.autogen.core.*;
import binder.autogen.featvalues.StringValue;

public class FakeHapticProxyGenerator extends AbstractProxyGenerator {

	String proxyOneId;
	String proxyTwoId;
	
	public void start () {
		log("Fake haptic proxy generator successfully started");
	}
	
	
	public void run() {
		randomInsertion();
	}
	
	
	public Proxy createProxy(int nb) {
		if (nb == 1) {
			return createProxyOne();
		}
		if (nb == 2) {
			return createProxyTwo();
		}
		
		if (nb == 3) {
			return createRelationProxy();
		}
		return null;
	}
	

	private Proxy createProxyOne() {
		
		Proxy proxy = createNewProxy("fakehaptic", 0.35f);
		
		FeatureValue cylindrical = createStringValue ("cylindrical", 0.73f);
		Feature feat = createFeatureWithUniqueFeatureValue ("shape", cylindrical);
		addFeatureToProxy (proxy, feat);
		
		proxyOneId = proxy.entityID;
		
		return proxy;
	}
	
	

	private Proxy createProxyTwo() {
		Proxy proxy = createNewProxy ("fakehaptic", 0.75f);
		
		FeatureValue spherical = createStringValue ("spherical", 0.67f);
		Feature feat1 = createFeatureWithUniqueFeatureValue("shape", spherical);
		addFeatureToProxy (proxy, feat1);
	
		FeatureValue trueval = createStringValue ("true", 0.8f);
		FeatureValue falseval = createStringValue ("false", 0.15f);
		FeatureValue[] vals = {trueval, falseval};
		Feature feat2 = createFeatureWithAlternativeFeatureValues("graspable", vals);
		addFeatureToProxy (proxy, feat2);
		
		proxyTwoId = proxy.entityID;
		
		return proxy;
	}
	
	
	private Proxy createRelationProxy() {
		
		StringValue[] sources = new StringValue[1];
		sources[0] = createStringValue(proxyOneId, 0.9f);
		
		StringValue[] targets = new StringValue[1];
		targets[0] = createStringValue(proxyTwoId, 0.91f);
		
		Proxy proxy = createNewRelationProxy("fakehaptic", 0.81f, sources, targets);
		
		return proxy;
	}
	
}
