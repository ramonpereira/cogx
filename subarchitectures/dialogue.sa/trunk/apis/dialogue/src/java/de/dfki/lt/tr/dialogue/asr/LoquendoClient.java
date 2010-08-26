// =================================================================
// Copyright (C) 2010 DFKI GmbH Talking Robots
// Miroslav Janicek (miroslav.janicek@dfki.de)
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

package de.dfki.lt.tr.dialogue.asr;

import Ice.Current;
import Ice.Identity;
import de.dfki.lt.tr.dialogue.slice.asr._ResultListenerDisp;
import de.dfki.lt.tr.dialogue.slice.asr.RecognitionResult;
import de.dfki.lt.tr.dialogue.slice.asr.loquendo.LoquendoException;
import de.dfki.lt.tr.dialogue.slice.asr.loquendo.RecogniserPrx;
import de.dfki.lt.tr.dialogue.slice.asr.loquendo.RecogniserPrxHelper;
import de.dfki.lt.tr.meta.TRResultListener;
import java.util.LinkedList;
import java.util.List;

/**
 * A client to the Loquendo ICE server.
 *
 * @author Miroslav Janicek
 */
public class LoquendoClient
extends _ResultListenerDisp {

    protected Ice.Communicator ic;
    protected RecogniserPrx prx = null;

	public boolean logging = true;

	private List<TRResultListener> listeners = new LinkedList<TRResultListener>();

	/**
	 * Connect to the Loquendo server and register with it as a client.
	 *
	 * @param name server name
	 * @param endpoint server endpoint
	 */
	public LoquendoClient(String name, String endpoint) {
		log("connecting to the Loquendo server \"" + name + "\" at \"" + endpoint + "\"");
        ic = Ice.Util.initialize();
		ic = Ice.Util.initialize();
		Ice.ObjectPrx base = ic.stringToProxy(name + ":" + endpoint);
		prx = RecogniserPrxHelper.checkedCast(base);
		if (prx == null) {
			throw new Error("Unable to create proxy");
		}

		Ice.ObjectAdapter adapter = ic.createObjectAdapter("");
		Ice.Identity ident = new Identity();
		ident.name = Ice.Util.generateUUID();
		ident.category = "";
		adapter.add(this, ident);
		adapter.activate();
		prx.ice_getConnection().setAdapter(adapter);
		prx.addListener(ident);
		log("connected");
	}

	/**
	 * Receive a recognition result from the server. This method is called
	 * by the Loquendo server whenever a new result is produced.
	 *
	 * @param res the recognition result
	 * @param __current
	 */
	@Override
	public void receiveRecognitionResult(RecognitionResult res, Current __current) {
		log("received a recognition result");
		notify(res);
	}

	/**
	 * Get notified on being unregistered by the server.
	 *
	 * @param reason reason given by the server
	 * @param __current
	 */
	@Override
	public void unregisteredFromServer(String reason, Current __current)
	{
		log("unregistered from the server, reason: " + reason);
	}


	/**
	 * Registers a process with the client, to be notified whenever the engine
	 * produces a new result.
	 *
	 * @param listener The process to be informed
	 * @throws UnsupportedOperationException Thrown if the process is null
	 */
	public void registerNotification(TRResultListener listener) {
		if (listener == null) {
			throw new UnsupportedOperationException("Cannot register process to be notified: "
					+"Provided process is null");
		}
		else {
			log("registering a process: " + listener.getClass().getCanonicalName());
			this.listeners.add(listener);
		}
	}

	/**
	 * Notify all the registered listeners of the obtained result.
	 *
	 * @param result the recognition result
	 */
	private void notify (Object result) {
		log("notifying listeners");
		if (listeners.isEmpty()) {
			// TODO: convert the result to string
			log("no listeners registered");
		}
		else {
			// Notify all the registered listeners
			for (TRResultListener listener : listeners) {
				listener.notify(result);
			}
		}
	}

	/**
	 * Start the recognition on the server.
	 */
	public void start() {
		log("starting");
		try {
			prx.start();
		}
		catch (LoquendoException ex) {
			log("loquendo exception: " + ex.errorMessage);
		}
	}

	/**
	 * Stop recognition on the server.
	 */
	public void stop() {
		log("starting");
		prx.stop();
//		throw new UnsupportedOperationException("Not yet implemented");
	}

	private void log(String str) {
		if (logging)
			System.out.println("\033[31m[LoqClient]\t" + str + "\033[0m");
	}

}
