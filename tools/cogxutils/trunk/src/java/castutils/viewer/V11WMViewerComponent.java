/**
 * @author Marc Hanheide (marc@hanheide.de)
 */
package castutils.viewer;

import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.StringTokenizer;
import java.util.Vector;

import si.unilj.fri.cogx.v11n.core.DisplayClient;
import Ice.ObjectImpl;
import cast.CASTException;
import cast.architecture.ManagedComponent;
import cast.cdl.WorkingMemoryAddress;
import cast.cdl.WorkingMemoryChange;
import cast.cdl.WorkingMemoryOperation;
import castutils.castextensions.WMEntrySet;
import castutils.castextensions.WMEntrySet.ChangeHandler;
import castutils.viewer.plugins.Plugin;

/**
 * @author Marc Hanheide (marc@hanheide.de)
 * 
 */
public class V11WMViewerComponent extends ManagedComponent {

	final WMEntrySet entrySet;
	final private MyDisplayClient displayHandler = new MyDisplayClient();
	final Map<Class<?>, Plugin> objectDispatcherMap = new HashMap<Class<?>, Plugin>();

	private class MyDisplayClient extends DisplayClient implements
			ChangeHandler {
		final Map<WorkingMemoryAddress, String> rows = Collections
				.synchronizedMap(new LinkedHashMap<WorkingMemoryAddress, String>());

		@Override
		public void entryChanged(Map<WorkingMemoryAddress, ObjectImpl> map,
				WorkingMemoryChange wmc, ObjectImpl newEntry,
				ObjectImpl oldEntry) throws CASTException {
			getLogger().info("entryChanged");
			
			switch (wmc.operation) {
			case ADD:
			case OVERWRITE:
				Plugin pluginToCall = findPlugin(newEntry.getClass());

				Vector<Object> row = new Vector<Object>();
				// mark additions
				if (wmc.operation == WorkingMemoryOperation.ADD)
					row.add("*");
				else
					row.add("");
				row.add(addrToString(wmc.address));
				row.add(newEntry.getClass().getSimpleName());
				if (pluginToCall != null) { // if we have a plugin for this
					// object
					Vector<Object> extraInfo = pluginToCall.toVector(newEntry);
					row.addAll(extraInfo);
				}
				String logString = "";
				for (Object o : row) {
					logString += "<td>" + o.toString() + "</td>";
				}
				rows.put(wmc.address, "<tr>" + logString + "</tr>");
				break;
			case DELETE:
				rows.remove(wmc.address);
				break;
			}

			updateView();
		}

		public void updateView() {
			String tableHtml = "<table frame=\"border\" border=\"1\" rules=\"all\">" +
					"<tr><th>NEW?</th><th>address</th><th>type</th><th>info1</th><th>info2</th><th>info3</th><th>info4</th></th>";
			for (String r : rows.values()) {
				tableHtml += r;
			}
			tableHtml += "</table>";
			getLogger().info("update View: " + tableHtml);

			displayHandler.setHtml(getComponentID()+".view", "1", tableHtml);
		}

		private String addrToString(WorkingMemoryAddress wma) {
			return wma.id + "@" + wma.subarchitecture;
		}

	}

	/**
	 * 
	 */
	public V11WMViewerComponent() {
		entrySet = WMEntrySet.create(this);
		entrySet.setHandler(displayHandler);
	}

	public Plugin findPlugin(Class<? extends ObjectImpl> origType) {
		Class<?> oType = origType;
		if (objectDispatcherMap.containsKey(oType))
			return objectDispatcherMap.get(oType);
		// if not yet found, look for supertype plugins
		Plugin pluginToCall = null;
		while (pluginToCall == null) {
			String SimpleName = oType.getSimpleName();
			String fullName = this.getClass().getPackage().getName()
					+ ".plugins." + SimpleName + "Info";
			try {
				ClassLoader.getSystemClassLoader().loadClass(fullName);
				pluginToCall = (Plugin) Class.forName(fullName).newInstance();
				break;
			} catch (ClassNotFoundException e) {
			} catch (InstantiationException e) {
			} catch (IllegalAccessException e) {
			}
			oType = oType.getSuperclass();
			if (oType == null) // if no superclass exists, we have to give up
				break;
			if (oType == Ice.Object.class) // we don't need to look up
				// further
				break;

		}
		objectDispatcherMap.put(origType, pluginToCall);
		return pluginToCall;
	}

	@SuppressWarnings("unchecked")
	@Override
	protected void configure(Map<String, String> arg0) {
		// TODO Auto-generated method stub
		super.configure(arg0);
		String subscrStr = arg0.get("--subscribe");
		if (subscrStr != null) {
			StringTokenizer st = new StringTokenizer(subscrStr, ",");
			while (st.hasMoreTokens()) {
				String className = st.nextToken();
				className = className.trim();
				try {
					System.out.println("add type '" + className + "'");
					ClassLoader.getSystemClassLoader().loadClass(className);
					entrySet.addType((Class<? extends ObjectImpl>) Class
							.forName(className));
				} catch (ClassNotFoundException e) {
					println("trying to register for a class that doesn't exist: "
							+ className);
				}
			}
		}
		displayHandler.configureDisplayClient(arg0);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see castutils.viewer.ViewerCastComponent#start()
	 */
	@Override
	protected void start() {
		entrySet.start();
		displayHandler.connectIceClient(this);
		displayHandler.installEventReceiver();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see castutils.viewer.ViewerCastComponent#stop()
	 */
	@Override
	protected void stop() {
		super.stop();
	}

}
