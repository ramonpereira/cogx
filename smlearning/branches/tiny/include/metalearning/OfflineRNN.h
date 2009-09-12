/** @file OfflineRNN.h
 * 
 * 
 * @author	Sergio Roa (DFKI)
 *
 * @version 1.0
 *
 *           2009      Sergio Roa
 
   This is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This package is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License.
   If not, see <http://www.gnu.org/licenses/>.

 
 */

#ifndef SMLEARNING_OFFLINERNN_H_
#define SMLEARNING_OFFLINERNN_H_

#include <MultilayerNet.hpp>

namespace smlearning {

///
///encapsulation of structs to generate RNNs config files in offline experiments
///
class OfflineRNN {

	rnnlib::Mdrnn *net;
	rnnlib::ConfigFile conf;
	string task;
	
 public:

	OfflineRNN () : conf ("/usr/local/bin/SMLearning/defaultnet.config") {
		//data loaded in from config file (default values below)
		rnnlib::GlobalVariables::instance().setVerbose (false);
		task = conf.get<string>("task");
	}

	///
	///print network topology and learning algorithm information
	///
	void print_net_data (ostream& out = cout);

	///
	///save RNN config file to be used for offline experiments
	///
	ostream& save_config_file (ostream& out = cout);

	///
	///open RNN for verification
	///
	void load_net (ostream& out = cout);

	///
	///read config file data from a given file
	///
	void set_config_file (rnnlib::ConfigFile &configFile);

	///
	///set test data file to be used with the RNN
	///
	void set_testdatafile (string fileName);

	///
	///set train data file to be used with the RNN
	///
	void set_traindatafile (string fileName);
};

///
///generate config files for RNNs for offline experiments
///
bool generate_network_files_nfoldcv_set (const string defaultnetFileName, const string baseDataFileName, int n, string targetDir );


}; /* namespace smlearning */


#endif /* SMLEARNING_OFFLINERNN_H_ */
