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
package binder.components;

import java.util.Enumeration;
import java.util.Map;
import java.util.Vector;

import binder.autogen.core.Proxy;
import binder.autogen.core.Union;
import binder.autogen.core.UnionConfiguration;
import binder.autogen.specialentities.RelationProxy;
import binder.gui.BinderMonitorGUI;
import cast.architecture.ManagedComponent;
import cast.architecture.ChangeFilterFactory;
import cast.architecture.WorkingMemoryChangeReceiver;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import cast.core.CASTData;


/**
 * Module monitoring the state of the binder working memory, and reporting
 * the evolution of its content (either textually or graphically)
 * 
 * @author Pierre Lison
 * @version 31/08/2009
 */

public class BinderMonitor extends ManagedComponent {

	// If a GUI is used, pointer to the GUI component
	BinderMonitorGUI gui;
	
	// Vector listing all the proxies currently in the WM
	Vector<Proxy> lastProxies;
	
	// Vector listing all the unions currently in the WM
	Vector<Union> lastUnions;
		
	@Override
	public void start() {

		// if the set of possible union configurations has been updated, update the
		// monitor accordingly
		addChangeFilter(ChangeFilterFactory.createGlobalTypeFilter(UnionConfiguration.class,
				WorkingMemoryOperation.WILDCARD), new WorkingMemoryChangeReceiver() {

			public void workingMemoryChanged(WorkingMemoryChange _wmc) {
				try {
					UnionConfiguration config = 
					getMemoryEntry(_wmc.address, UnionConfiguration.class);
					updateMonitorWithNewConfiguration(config);
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			} 
		});
		
		// If a proxy is updated, also update the monitor accordingly
		addChangeFilter(ChangeFilterFactory.createGlobalTypeFilter(Proxy.class,
				WorkingMemoryOperation.OVERWRITE), new WorkingMemoryChangeReceiver() {

			public void workingMemoryChanged(WorkingMemoryChange _wmc) {
				try {
					if (gui != null) {
						gui.updateGUI(getMemoryEntry(_wmc.address, Proxy.class));
					} 
					else {
						log ("Proxy " + getMemoryEntry
								(_wmc.address, Proxy.class).entityID + " has been updated");
					}
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			} 
		});
		

		lastProxies = new Vector<Proxy>();
		lastUnions = new Vector<Union>();
		
		log("Binding Monitor successfully started");
	}
	
	
	/**
	 * Configure the binding monitor with various parameters
	 * (use --gui to activate the graphical interface)
	 * 
	 */
	@Override
	public void configure(Map<String, String> _config) {
		if (_config.containsKey("--gui")) {
			 gui = new BinderMonitorGUI(this);
	//		 gui.LOGGING = false;
		} 
	}
	
	/**
	 * Update the monitor with the new union configurations
	 * 
	 * @param alterconfigs (the new union configuration)
	 */
	public void updateMonitorWithNewConfiguration
			(UnionConfiguration bestConfig) {
		
		log("Change in the binding working memory, update necessary");
		Vector<Proxy> proxiesV = new Vector<Proxy>();
		Vector<Union> UnionsV = new Vector<Union>();
		
		Vector<Proxy> proxiesToDelete = new Vector<Proxy>();
		Vector<Union> UnionsToDelete= new Vector<Union>();
		
		try {
			
			// Extract the proxies in the working memory
			CASTData<Proxy>[] proxies = getWorkingMemoryEntries(Proxy.class);
			log("Current number of (normal) proxies: " + proxies.length);
			for (int i = (proxies.length - 1) ; i >= 0 ; i--) {
				proxiesV.add(proxies[i].getData());
			}
			
			CASTData<RelationProxy>[] rproxies = getWorkingMemoryEntries(RelationProxy.class);
			log("Current number of relation proxies: " + rproxies.length);
			for (int i = (rproxies.length - 1) ; i >= 0 ; i--) {
				proxiesV.add(rproxies[i].getData());
			}			
			
			// Extract the unions in the union configuration
			for (int i = 0 ; i < bestConfig.includedUnions.length ; i++) {
				UnionsV.add(bestConfig.includedUnions[i]);
			}
			
			
			// Check if proxies need to be deleted from the monitor
			for (Enumeration<Proxy> e = lastProxies.elements(); e.hasMoreElements(); ) {
				Proxy proxy = e.nextElement();
				if (!proxiesV.contains(proxy)) {
					proxiesToDelete.add(proxy);
				}
			}
			
			// Check if unions need to be deleted from the monitor
			for (Enumeration<Union> e = lastUnions.elements(); e.hasMoreElements(); ) {
				Union union = e.nextElement();
				if (!UnionsV.contains(union)) {
					UnionsToDelete.add(union);
				}
			}
		}
		catch (Exception e) {
			e.printStackTrace();
		}
		
		// Update the proxy and union sets
		lastProxies = proxiesV;
		lastUnions = UnionsV;
		log("Current number of Unions: " + UnionsV.size());
		
		// And finally, trigger the GUI update
		if (gui != null) {
			gui.updateGUI(proxiesV, UnionsV, proxiesToDelete, UnionsToDelete);
		}
	}
	
	
}
