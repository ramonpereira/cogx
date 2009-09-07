/** @file data_handling.h
 * 
 * 
 * @author	Sergio Roa (DFKI)
 *
 * @version 1.0
 *
 * Copyright 2007,2008 Alex Graves
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

#ifndef TOOLS_DATAHANDLING_H_
#define TOOLS_DATAHANDLING_H_

#include <cstdlib>
#include <ctime>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <MultilayerNet.hpp>

#include <netcdf.h>

#include <boost/filesystem/operations.hpp>
#include <boost/regex.hpp>

using namespace std;
using namespace boost::filesystem;

namespace smlearning {

typedef vector<double> FeatureVector;
typedef vector<FeatureVector> Sequence;
typedef vector<Sequence> DataSet;

// function that prints the passed argument
template <typename T>
void print_item (const T& elem) {
    cout << elem << " ";
}

template <typename T>
void print_featvector (const FeatureVector& v) {

	for_each (v.begin(), v.end(), print_item<T>);

}

template <typename T>
void print_sequence (const Sequence& s) {
	typename Sequence::const_iterator i;
	cout << "{";
	for (i=s.begin(); i != s.end(); i++) {
		cout << "[";
		print_featvector<T> (*i);
		if (i+1 == s.end())
			cout << "]";
		else
			cout << "], " << endl;
	}
	cout << "}\n" << endl;


}

template <typename T>
void print_dataset (const DataSet& d) {
	for_each (d.begin(), d.end(), print_sequence<T>);
}


void generate_rand_sequences (DataSet& data, long numSeq, long seqSize);

bool write_dataset (string fileName, const DataSet& data);

bool read_dataset (string fileName, DataSet& data);

bool write_cdl_file_padding (string fileName, const DataSet& data);

bool write_cdl_file_basis (string fileName, const DataSet& data);

bool write_nc_file_basis (string fileName, const DataSet& data);

bool concatenate_datasets (string dir, string writeFileName);

template<class Function>
bool write_n_fold_cross_valid_sets (string seqFileName, int n, Function write_netcdf_file, string target_dir, bool print_data = false) {

	DataSet data;
	if (!read_dataset (seqFileName, data)){
		cout << "file " + seqFileName + " could not be read" << endl;
		return false;
	}

	boost::regex seqfile_re (".*/(.*)$");
	boost::cmatch match;
	string seqBaseFileName;
	if (boost::regex_match(seqFileName.c_str(), match, seqfile_re)) {
		seqBaseFileName = string(match[1].first, match[1].second);
	}
	cout << "seqFileBaseName: " << seqBaseFileName << endl;
	cout << "seqFile: " << seqFileName << endl;

	
	vector<DataSet> partitions_testing;
	vector<DataSet> partitions_training;
	
	// initialize random seed:
	srand ((unsigned)time(NULL) );
	
	// generate random number:
	int randNr;

	long int partitionSize = data.size() / n;
	vector<bool> availablePartitions;
	
	for (int i=0; i<n; i++) {
		DataSet partition_testing;
		DataSet partition_training;
		partitions_testing.push_back (partition_testing);
		partitions_training.push_back (partition_training);
		availablePartitions.push_back (true);		
	}

	DataSet::const_iterator s;
	for (s=data.begin(); s!=data.end(); s++) {
		bool available_partitions = false;
		//check available partitions
		for (int i=0; i<n; i++)
			if (availablePartitions[i] == true) {
				available_partitions = true;
				break;
			}

		if (available_partitions) {
			bool available_partition_found = false;
			do {
				randNr = rand() % n;
				if (availablePartitions[randNr]) {
					partitions_testing[randNr].push_back(*s);
					available_partition_found = true;
					if (partitions_testing[randNr].size() == partitionSize)
						availablePartitions[randNr] = false;
				}				
			} while (!available_partition_found);
		}
		else
			break;
	}

	//store partitions in cdl files
	for (int i=0; i< n; i++) {
		cout << "size of testing partition " << i << ": " << partitions_testing[i].size() << endl;
		if (print_data)
			print_dataset<double>(partitions_testing[i]);
		stringstream testingFileName;
		testingFileName << target_dir << "/" << seqBaseFileName << "_" << n << "_foldcv_set-" << i << "_testing";
		write_netcdf_file (testingFileName.str(), partitions_testing[i]);
		for (int j=0; j<n; j++)
			if (i != j) {
				DataSet::const_iterator s;
				for (s=partitions_testing[j].begin(); s!=partitions_testing[j].end(); s++)
					partitions_training[i].push_back (*s);
			}
		cout << "size of training partition " << i << ": " << partitions_training[i].size() << endl;
		if (print_data)
			print_dataset<double>(partitions_training[i]);
		stringstream trainingFileName;
		trainingFileName << target_dir << "/" << seqBaseFileName << "_" << n << "_foldcv_set-" << i << "_training";
		write_netcdf_file (trainingFileName.str(), partitions_training[i]);
		
	}

	
}

bool generate_network_files_nfoldcv_set (const string defaultnetFileName, const string baseDataFileName, int n, string targetDir );

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

	void print_net_data (ostream& out = cout);
	
	ostream& save_config_file (ostream& out = cout);

	void load_net (ostream& out = cout);
	
	void set_config_file (rnnlib::ConfigFile &configFile);

	void set_testdatafile (string fileName);

	void set_traindatafile (string fileName);
};

}; /* smlearning namespace */

#endif /* TOOLS_DATAHANDLING_H_*/
