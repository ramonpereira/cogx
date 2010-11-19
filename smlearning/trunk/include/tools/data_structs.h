/** @file data_structs.h
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
#ifndef SMLEARNING_DATASTRUCTS_H_
#define SMLEARNING_DATASTRUCTS_H_

#include <vector>

#include <Tools/Tools.h>

#include <netcdf.h>
#include <boost/regex.hpp>
#include <metalearning/RNN.h>
#include <tools/math_helpers.h>

#include <boost/tuple/tuple.hpp>

using namespace golem;
using namespace std;
using namespace boost;

namespace smlearning {

///
///this representation should also allow for labels, i.e., a vector of size 1
///properly discretized
///
typedef vector<double> FeatureVector;
typedef vector<FeatureVector> Sequence;
typedef vector<Sequence> DataSet;
/** LearningParams tuple <motorVectorSize, featureVectorSize, pfVectorSize, efVectorSize> */
enum { mVSize, fVSize, pfVSize, efVSize };
typedef tuple<int, int, int, int> LearningParams;
/** DataSetValueLimits tuple < minX, minY, minZ, maxX, maxY, maxZ> */
enum { minX, minY, minZ, maxX, maxY, maxZ };
typedef tuple<double, double, double, double, double, double> DataSetValueLimits;
/** DataSetParams tuple <LearningParams, storeLabels, DataSetValueLimits> */
enum {lParams, labels, limits };
typedef tuple<LearningParams, bool, DataSetValueLimits> DataSetParams;
typedef pair<DataSet, DataSetParams> DataSetStruct;

struct CanonicalData {
	struct FeatureVector {
		smlearning::FeatureVector rawVector;
		string motorCommand;
		string label;
	};
	typedef vector<FeatureVector> Sequence;
	typedef vector<Sequence> DataSet;
	typedef pair<DataSet, DataSetParams> DataSetStruct;
	
	DataSetStruct data;
};


/** Learning data format */
struct LearningData {

	/** Data chunk */
	struct Chunk {
		typedef std::vector<Chunk> Seq;
		
		/** Data chunk time stamp */
		golem::SecTmReal timeStamp;
		
		/** Arm state - (joint) dynamic configuration */
		golem::GenConfigspaceState armState;
		/** End-effector GLOBAL pose */
		golem::Mat34 effectorPose;
		/** Object GLOBAL pose */
		golem::Mat34 objectPose;
		/** End-effector orientation in Euler coordinates */
		golem::Real efRoll, efPitch, efYaw; 
		/** Object orientation in Euler coordinates */
		golem::Real obRoll, obPitch, obYaw;
		/** label */
		//TO DO!
		//T label;
		/** feature vector (here any vectorial representation is possible) */
		vector<Real> featureVector;
		
	};

	/** Motor Primitive */
	struct MotorCommand {
		/** initial effector pose before the movement */
		golem::Vec3 initEfPosition;
		/** horizontal angle */
		golem::Real horizontalAngle;
		/** speed ( 3 (fast), 4 (middle), 5 (low) */
		golem::Real pushDuration;
		/** feature vector (here any vectorial representation is possible) */
		vector<Real> featureVector;
	};
	typedef std::pair<MotorCommand, Chunk::Seq> Sequence;
	
	typedef std::vector<Sequence> DataSet;

	/** (Dynamic) Effector bounds in LOCAL coordinates; to obtain global pose multiply by Chunk::effectorPose */
	golem::Bounds::Seq effector;
	/** (Dynamic) Object bounds in LOCAL coordinates; to obtain global pose multiply by Chunk::objectPose */
	golem::Bounds::Seq object;
	/** (Static) Obstacles bounds in GLOBAL coordinates (usually ground plane) */
	golem::Bounds::Seq obstacles;
	
	/** Time-dependent data */
	Chunk::Seq currentChunkSeq;
	Sequence _currentSeq;
	//DataSet data;
	/** current predicted polyflap poses sequence */
	vector<Mat34> currentPredictedPfSeq;
	/** current predicted effector poses sequence */
	vector<Mat34> currentPredictedEfSeq;
	/** current polyflap poses and motor command sequence */
	smlearning::Sequence currentSeq;
	/** current motor command */
	FeatureVector currentMotorCommandVector;
	MotorCommand currentMotorCommand;
	/** size of motor vector for NN training */
	static const int motorVectorSize = 5;
	/** size of polyflap pose vector for NN training */
	static const int pfVectorSize = 6;
	/** size of effector pose vector for NN training */
	static const int efVectorSize = 6;
	struct CoordinateLimits {
		/** assumed minimum value for polyflap Z-coordinate location during experiment (in xml file should have value of -0.01... bug in PhysX?) */
		Real minZ;
		/** assumed minimum value for polyflap X-coordinate location during experiment */
		Real minX;
		/** assumed minimum value for polyflap Y-coordinate location during experiment */
		Real minY;	
		/** assumed maximum value for polyflap Z-coordinate location during experiment */
		Real maxZ;
		/** assumed maximum value for polyflap X-coordinate location during experiment */
		Real maxX;
		/** assumed maximum value for polyflap Y-coordinate location during experiment */
		Real maxY;
		/** assumed minimum value for duration of trajectory when pushing */
		Real minDuration;
		/** assumed maximum value for duration of trajectory when pushing */
		Real maxDuration;
		
	};
	CoordinateLimits coordLimits;
	/** Record validity */
	//bool bArmState;
	//bool bEffectorPose;
	//bool bObjectPose;
	//bool bFtsData;
	//bool bImageIndex;
	//bool bEffector;
	//bool bObject;
	//bool bObstacles;


	
	/** Reset to default (empty)*/
	void setToDefault(CoordinateLimits limits) {
		effector.clear();
		object.clear();
		obstacles.clear();
		coordLimits.minX = limits.minX;
		coordLimits.minY = limits.minY;
		coordLimits.minZ = limits.minZ;
		coordLimits.maxX = limits.maxX;
		coordLimits.maxY = limits.maxY;
		coordLimits.maxZ = limits.maxZ;
		coordLimits.minDuration = limits.minDuration;
		coordLimits.maxDuration = limits.maxDuration;
		//data.clear();
		//bArmState = false;
		//bEffectorPose = false;
		//bObjectPose = false;
		//bFtsData = false;
		//bImageIndex = false;
		//bEffector = false;
		//bObject = false;
		//bObstacles = false;
	}
	/** Check if the data is valid */
	bool isValid() const {
		// if (!data.empty()) // must not be empty
		// 	return false;
		//if (bEffector && effector.empty())
		//	return false;
		//if (bObject && object.empty())
		//	return false;
		//if (bObstacles && obstacles.empty())
		//	return false;

		return true;
	}

	///
	///Write DataSet vector to a file
	///
	static bool write_dataset (string fileName, const DataSet& data, const CoordinateLimits& limits);
	///
	///Read DataSet vector from a file
	///
	static bool read_dataset (string fileName, DataSet& data, CoordinateLimits& limits);

	///
	///print a DataSet vector
	///
	static void print_dataset (const DataSet &d);

	///
	///print coordinate limits for a dataset
	///
	static void print_dataset_limits (const CoordinateLimits limits);

	///
	///print a motor command struct
	///
	static void print_motorCommand (const MotorCommand& mC);

	///
	///print a chunk (features)
	///
	static void print_Chunk (const Chunk& c);

	///
	///write a netcdf nc file format for feature vectors using basis representation
	///
	template<class Normalization>
	static bool write_nc_file_NNbasis (string fileName, const DataSet& data, Normalization normalize, CoordinateLimits limits) {

		fileName += ".nc";

		FeatureVector inputVector;
		FeatureVector targetVector;
		vector<int> seqLengthsVector;
		size_t numTimesteps_len = 0;
		const int inputSize = motorVectorSize + pfVectorSize + efVectorSize;
		const int outputSize = pfVectorSize;

		DataSet::const_iterator d_iter;

		for (d_iter = data.begin(); d_iter != data.end(); d_iter++) {
			MotorCommand mC = d_iter->first;
			Chunk::Seq seq = d_iter->second;

			size_t seqSize = seq.size();
			seqLengthsVector.push_back ( seqSize );
			numTimesteps_len += seqSize;

			Chunk::Seq::const_iterator s_iter;
			for (s_iter=seq.begin(); s_iter != seq.end(); s_iter++) {
				if (s_iter+1 != seq.end()) {
					if (s_iter == seq.begin()) {
						//motor command
						write_motorCommand_to_featvector (inputVector, mC, normalize, limits);
						//completing with feature vector
						write_chunk_to_featvector (inputVector, *s_iter, normalize, limits);

						//zero padding
						for (int i=0; i<motorVectorSize; i++)
							inputVector.push_back (0.0);
						write_chunk_to_featvector (inputVector, *s_iter, normalize, limits);
					}
					
					else {
						//zero padding
						for (int i=0; i<motorVectorSize; i++)
							inputVector.push_back (0.0);
						write_chunk_to_featvector (inputVector, *s_iter, normalize, limits);
					}
				}
				//target vector corresponds to polyflap pose vector
				write_chunk_to_featvector (targetVector, *s_iter, normalize, limits, false);
			
			}
		}
		//netcdf file storing
		if (!write_nc_data (fileName, data.size(), inputSize, outputSize, inputVector, targetVector, seqLengthsVector, numTimesteps_len))
			return false;


		return true;
	}


	///
	///write a netcdf nc file format for feature vectors using basis representation
	///
	template<class Normalization>
	static bool write_nc_file_NNmarkov (string fileName, const DataSet& data, Normalization normalize, CoordinateLimits limits) {
		fileName += ".nc";

		FeatureVector inputVector;
		FeatureVector targetVector;
		vector<int> seqLengthsVector;
		size_t numTimesteps_len = 0;

		const int inputSize = /*motorVectorSize*/2 + efVectorSize + pfVectorSize;
		const int outputSize = pfVectorSize;

		DataSet::const_iterator d_iter;

		for (d_iter = data.begin(); d_iter != data.end(); d_iter++) {
			MotorCommand mC = d_iter->first;
			Chunk::Seq seq = d_iter->second;

			size_t seqSize = seq.size() - 1;
			seqLengthsVector.push_back ( seqSize );
			numTimesteps_len += seqSize;

			Chunk::Seq::const_iterator s_iter;

			for (s_iter=seq.begin(); s_iter != seq.end(); s_iter++) {
				if (s_iter+1 != seq.end()) {
					//motor command
					inputVector.push_back (normalize(mC.horizontalAngle/180.0*REAL_PI, -REAL_PI, REAL_PI));
					inputVector.push_back (normalize(mC.pushDuration, 3.0, 5.0));
					//completing with feature vector
					write_chunk_to_featvector (inputVector, *s_iter, normalize, limits);
				
				}
				if (s_iter != seq.begin())
					//target vector corresponds to polyflap pose vector
					write_chunk_to_featvector (targetVector, *s_iter, normalize, limits, false);
			
			}
		}
		//netcdf file storing
		if (!write_nc_data (fileName, data.size(), inputSize, outputSize, inputVector, targetVector, seqLengthsVector, numTimesteps_len))
			return false;
	
	
		return true;
	}

	
	///
	///write a chunk to feature vector
	///
	template<typename T, class Normalization>
	static void write_chunk_to_featvector (vector<T>& featVector, const Chunk& chunk, Normalization normalize, CoordinateLimits coordLimits, bool pfEfVector = true) {
		if ( pfEfVector ) {
			
			featVector.push_back (normalize(chunk.effectorPose.p.v1, coordLimits.minX, coordLimits.maxX));
			featVector.push_back (normalize(chunk.effectorPose.p.v2, coordLimits.minY, coordLimits.maxY));
			featVector.push_back (normalize(chunk.effectorPose.p.v3, coordLimits.minZ, coordLimits.maxZ));
			featVector.push_back (normalize(chunk.efRoll, -REAL_PI, REAL_PI));
			featVector.push_back (normalize(chunk.efPitch,-REAL_PI, REAL_PI));
			featVector.push_back (normalize(chunk.efYaw, -REAL_PI, REAL_PI));
		}
		featVector.push_back (normalize(chunk.objectPose.p.v1, coordLimits.minX, coordLimits.maxX));
		featVector.push_back (normalize(chunk.objectPose.p.v2, coordLimits.minY, coordLimits.maxY));
		featVector.push_back (normalize(chunk.objectPose.p.v3, coordLimits.minZ, coordLimits.maxZ));
		featVector.push_back (normalize(chunk.obRoll, -REAL_PI, REAL_PI));
		featVector.push_back (normalize(chunk.obPitch, -REAL_PI, REAL_PI));
		featVector.push_back (normalize(chunk.obYaw, -REAL_PI, REAL_PI));
	}

	///
	///write a chunk to feature vector with memory assigned
	///
	template<typename T, class Normalization>
	static void write_chunk_to_featvector (vector<T>& featVector, const Chunk& chunk, int& vectorIndex, Normalization normalize, CoordinateLimits coordLimits, bool pfEfVector = true) {
		if ( pfEfVector ) {

			featVector[vectorIndex++] = normalize(chunk.effectorPose.p.v1, coordLimits.minX, coordLimits.maxX);
			featVector[vectorIndex++] = normalize(chunk.effectorPose.p.v2, coordLimits.minY, coordLimits.maxY);
			featVector[vectorIndex++] = normalize(chunk.effectorPose.p.v3, coordLimits.minZ, coordLimits.maxZ);
			featVector[vectorIndex++] = normalize(chunk.efRoll, -REAL_PI, REAL_PI);
			featVector[vectorIndex++] = normalize(chunk.efPitch, -REAL_PI, REAL_PI);
			featVector[vectorIndex++] = normalize(chunk.efYaw, -REAL_PI, REAL_PI);
		}
		featVector[vectorIndex++] = normalize(chunk.objectPose.p.v1, coordLimits.minX, coordLimits.maxX);
		featVector[vectorIndex++] = normalize(chunk.objectPose.p.v2, coordLimits.minY, coordLimits.maxY);
		featVector[vectorIndex++] = normalize(chunk.objectPose.p.v3, coordLimits.minZ, coordLimits.maxZ);
		featVector[vectorIndex++] = normalize(chunk.obRoll, -REAL_PI, REAL_PI);
		featVector[vectorIndex++] = normalize(chunk.obPitch, -REAL_PI, REAL_PI);
		featVector[vectorIndex++] = normalize(chunk.obYaw, -REAL_PI, REAL_PI);

	}

	///
	///write motor command to feature vector
	///
	template <typename T, class Normalization>
	static void write_motorCommand_to_featvector (vector<T>& featVector, const MotorCommand& mC, Normalization normalize, CoordinateLimits coordLimits) {
		featVector.push_back (normalize(mC.initEfPosition.v1, coordLimits.minX, coordLimits.maxX));
		featVector.push_back (normalize(mC.initEfPosition.v2, coordLimits.minY, coordLimits.maxY));
		featVector.push_back (normalize(mC.initEfPosition.v3, coordLimits.minZ, coordLimits.maxZ));
		featVector.push_back (normalize(mC.pushDuration, /*coordLimits.minDuration*/3.0, /*coordLimits.maxDuration*/5.0));
		featVector.push_back (normalize(mC.horizontalAngle/180.0*REAL_PI, -REAL_PI, REAL_PI));
	}

	///
	///write motor command to feature vector with memory assigned
	///
	template <typename T, class Normalization>
	static void write_motorCommand_to_featvector (vector<T>& featVector, const MotorCommand& mC, int& vectorIndex, Normalization normalize, CoordinateLimits coordLimits) {
		featVector[vectorIndex++] = normalize(mC.initEfPosition.v1, coordLimits.minX, coordLimits.maxX);
		featVector[vectorIndex++] = normalize(mC.initEfPosition.v2, coordLimits.minY, coordLimits.maxY);
		featVector[vectorIndex++] = normalize(mC.initEfPosition.v3, coordLimits.minZ, coordLimits.maxZ);
		featVector[vectorIndex++] = normalize(mC.pushDuration, /*coordLimits.minDuration*/3.0, /*coordLimits.maxDuration*/5.0);
		featVector[vectorIndex++] = normalize(mC.horizontalAngle/180.0*REAL_PI, -REAL_PI, REAL_PI);
	}


	///
	///almost automatically generated netcdf function to store netcdf data files
	///
	static bool write_nc_data (string fileName, size_t numSeqs_len, int inputVectorSize, int targetVectorSize, FeatureVector& inputVector, FeatureVector& targetVector, vector<int>& seqLengthsVector, size_t& numTimesteps_len);


	///
	///generation of n-fold cross-validation sets from a particular sequence file
	///and using a certain write_netcdf_file function
	///
	template<class Function, class Normalization>
	static bool write_n_fold_cross_valid_sets (string seqFileName, int n, Function write_netcdf_file, Normalization normalize, CoordinateLimits limits, string target_dir, bool print_data = false) {
		
		DataSet data;
		if (n < 2) {
			cout << "You have to use at least 2 cross-validation sets" << endl;
			return false;
		}
		if (!read_dataset (seqFileName, data, limits)){
			cout << "file " + seqFileName + " could not be read" << endl;
			return false;
		}

		boost::regex seqfile_re (".*/(.*)$");
		boost::cmatch match;
		string seqBaseFileName;
		if (boost::regex_match(seqFileName.c_str(), match, seqfile_re)) {
			seqBaseFileName = string(match[1].first, match[1].second);
		}
		else
			seqBaseFileName = seqFileName;
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
				print_dataset (partitions_testing[i]);
			stringstream testingFileName;
			testingFileName << target_dir << "/" << seqBaseFileName << "_" << n << "_foldcv_set-" << i << "_testing";
			write_netcdf_file (testingFileName.str(), partitions_testing[i], normalize, limits);
			for (int j=0; j<n; j++)
				if (i != j) {
					DataSet::const_iterator s;
					for (s=partitions_testing[j].begin(); s!=partitions_testing[j].end(); s++)
						partitions_training[i].push_back (*s);
				}
			cout << "size of training partition " << i << ": " << partitions_training[i].size() << endl;
			if (print_data)
				print_dataset(partitions_training[i]);
			stringstream trainingFileName;
			trainingFileName << target_dir << "/" << seqBaseFileName << "_" << n << "_foldcv_set-" << i << "_training";
			write_netcdf_file (trainingFileName.str(), partitions_training[i], normalize, limits);
		
		}

		return true;
	
	}

	///
	///load training data in RNNLIB format
	///
	template<class LoadSequenceMethod, class Normalization >
	static rnnlib::DataSequence* load_NNtrainSeq ( Sequence& seq, LoadSequenceMethod loadSequenceMethod, Normalization normalize, CoordinateLimits limits) {
		rnnlib::DataSequence* trainSeq = new rnnlib::DataSequence (/*motorVectorSize*/2+pfVectorSize+efVectorSize, pfVectorSize);
		vector<int> inputShape, targetShape;
		cout << " loadSequenceMethod is: " << typeid(loadSequenceMethod).name() << endl;
		inputShape.push_back (seq.second.size() - 1);
		targetShape.push_back (seq.second.size() - 1);
		trainSeq->inputs.reshape(inputShape);
		trainSeq->targetPatterns.reshape(targetShape);
		// cout << "input size: " << trainSeq->inputs.data.size() << endl;
		// cout << "output size: " << trainSeq->targetPatterns.data.size() << endl;
		//load_NNsequence_basis (trainSeq->inputs.data, trainSeq->targetPatterns.data, seq, normalize, limits);
		//load_NNsequence_markov (trainSeq->inputs.data, trainSeq->targetPatterns.data, seq, normalize, limits);
		loadSequenceMethod (trainSeq->inputs.data, trainSeq->targetPatterns.data, seq, normalize, limits);
		return trainSeq;
	
	
	}


	///
	///load a sequence into inputs and target vectors (for NN training) (NN basis representation)
	///
	template<class Normalization>
	static void load_NNsequence_basis (vector<float>& inputVector, vector<float>& targetVector, Sequence s, Normalization normalize, CoordinateLimits limits) {
		int contInput = 0;
		int contTarget = 0;
		MotorCommand mC = s.first;
		Chunk::Seq seq = s.second;

		Chunk::Seq::const_iterator s_iter;
		for (s_iter=seq.begin(); s_iter!= seq.end(); s_iter++) {
			//put inputs and targetPatterns data
			if (s_iter+1 != seq.end()) {
				if (s_iter == seq.begin () ) {
					//motor command
					write_motorCommand_to_featvector (inputVector, mC, contInput, normalize, limits);
					//completing with next feature vector
					write_chunk_to_featvector (inputVector, *s_iter, contInput, normalize, limits);

					//zero padding
					for (int i=0; i<motorVectorSize; i++)
						inputVector[contInput++] = 0.0;
					write_chunk_to_featvector (inputVector, *s_iter, contInput, normalize, limits);
			
				}
				else {
					//zero padding
					for (int i=0; i<motorVectorSize; i++)
						inputVector[contInput++] = 0.0;
					write_chunk_to_featvector (inputVector, *s_iter, contInput, normalize, limits);
				
				}	
			}
			write_chunk_to_featvector (targetVector, *s_iter, contTarget, normalize, limits, false);
		}
	}

	///
	///load a sequence into inputs and target vectors (for NN training) (NN basis representation)
	///
	template<class Normalization>
	static void load_NNsequence_markov (vector<float>& inputVector, vector<float>& targetVector, Sequence s, Normalization normalize, CoordinateLimits limits) {
		int contInput = 0;
		int contTarget = 0;
		MotorCommand mC = s.first;
		Chunk::Seq seq = s.second;

		Chunk::Seq::const_iterator s_iter;
		for (s_iter=seq.begin(); s_iter!= seq.end(); s_iter++) {
			if (s_iter+1 != seq.end()) {
				inputVector[contInput++] = normalize(mC.horizontalAngle/180.0*REAL_PI, -REAL_PI, REAL_PI );
				inputVector[contInput++] = normalize(mC.pushDuration, 3.0, 5.0);
				//completing with feature vector
				write_chunk_to_featvector (inputVector, *s_iter, contInput, normalize, limits);
				
			}
			if (s_iter != seq.begin() )
				//target vector corresponds to polyflap pose vector
				write_chunk_to_featvector (targetVector, *s_iter, contTarget, normalize, limits, false);
			
		}
	}
	
};

}; /* smlearning namespace */

#endif /* SMLEARNING_DATASTRUCTS_H_*/
