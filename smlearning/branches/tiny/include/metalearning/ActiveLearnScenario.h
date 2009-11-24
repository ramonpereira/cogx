/** @file ActiveLearnScenario.h
 * 
 * @author	Marek Kopicki (The University Of Birmingham)
 *
 * @version 1.0
 *
 */

// #pragma once
#ifndef _SMLEARNING_ACTIVELEARNSCENARIO_H_
#define _SMLEARNING_ACTIVELEARNSCENARIO_H_

//------------------------------------------------------------------------------

#include <Golem/Application.h>
#include <Golem/PhysReacPlanner.h>
#include <Golem/Katana.h>
#include <Golem/Simulator.h>
#include <Golem/Rand.h>

#include <metalearning/Scenario.h>
#include <metalearning/ActiveRNN.h>
#include <Ice/Ice.h>
#include <PlotApp.hh>


//------------------------------------------------------------------------------

namespace smlearning {

/** Learning data format */
class LearningData {
public:
	/** Data chunk */
	class Chunk {
	public:
		typedef std::vector<Chunk> Seq;
		
		/** Do nothing */
		Chunk() {
		}
		
		/** Data chunk time stamp */
		golem::SecTmReal timeStamp;
		
		/** Arm state - (joint) dynamic configuration */
		golem::GenConfigspaceState armState;
		/** End-effector GLOBAL pose */
		golem::Mat34 effectorPose;
		/** Object GLOBAL pose */
		golem::Mat34 objectPose;
		
	};

	/** (Dynamic) Effector bounds in LOCAL coordinates; to obtain global pose multiply by Chunk::effectorPose */
	golem::Bounds::Seq effector;
	/** (Dynamic) Object bounds in LOCAL coordinates; to obtain global pose multiply by Chunk::objectPose */
	golem::Bounds::Seq object;
	/** (Static) Obstacles bounds in GLOBAL coordinates (usually ground plane) */
	golem::Bounds::Seq obstacles;
	
	/** Time-dependent data */
// 	Chunk::Seq data;
	vector<DataSet> data;
	/** current predicted polyflap poses sequence */
	vector<Mat34> currentPredictedObjSeq;
	/** current polyflap poses and motor command sequence */
	smlearning::Sequence currentSeq;
	/** current motor command */
	FeatureVector currentMotorCommandVector;
	/** training sequence for the LSTM */
	rnnlib::DataSequence* trainSeq;
	/** Record validity */
	//bool bArmState;
	//bool bEffectorPose;
	//bool bObjectPose;
	//bool bFtsData;
	//bool bImageIndex;
	//bool bEffector;
	//bool bObject;
	//bool bObstacles;

	/** load training data in RNNLIB format */
	void loadCurrentTrainSeq (int inputSize, int outputSize);
	
	/** Reset to default (empty)*/
	void setToDefault() {
		effector.clear();
		object.clear();
		obstacles.clear();
		data.clear();
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
		if (!data.empty()) // must not be empty
			return false;
		//if (bEffector && effector.empty())
		//	return false;
		//if (bObject && object.empty())
		//	return false;
		//if (bObstacles && obstacles.empty())
		//	return false;

		return true;
	}
};



//------------------------------------------------------------------------------

/** ActiveLearnScenario class */
class ActiveLearnScenario : public golem::Object, public smlearning::Scenario {
public:	
	/** Just Interrupted */
	class Interrupted {};
	
	/** Object description */
	class Desc : public golem::Object::Desc {
	protected:
		/** Creates the object from the description. */
		CREATE_FROM_OBJECT_DESC(ActiveLearnScenario, golem::Object::Ptr, golem::Scene)

	public:
		/** Arm */
		golem::PhysReacPlanner::Desc armDesc;
		/** Effector bounds group */
		golem::U32 effectorGroup;
		/** Finger */
		golem::Bounds::Desc::Seq fingerDesc;
		/** Local end-effector reference pose */
		golem::WorkspaceCoord referencePose;
		/** Global end-effector home pose */
		golem::WorkspaceCoord homePose;
		/** Motion effects stabilization time period */
		golem::SecTmReal speriod;
		/** 2D motion constraint */
		bool motion2D;
		
		/** Constructs description object */
		Desc() {
			Desc::setToDefault();
		}
		/** Sets the parameters to the default values */
		void setToDefault() {
			golem::Object::Desc::setToDefault();
			
			// default arm description
			armDesc.setToDefault();
			armDesc.pArmDesc.reset(new golem::KatSimArm::Desc);
			armDesc.pPlannerDesc->pHeuristicDesc->distJointcoordMax[4] = golem::Real(1.0)*golem::REAL_PI; // last joint of Katana
			// Effector group
			effectorGroup = 0x4;
			// finger setup
			fingerDesc.clear();
			// end-effector reference pose
			referencePose.setId();
			// end-effector home pose
			homePose.R.rotX(-0.5*golem::REAL_PI); // end-effector pointing downwards
			homePose.p.set(golem::Real(0.0), golem::Real(0.1), golem::Real(0.1));
			// Other stuff
			motion2D = false;
			speriod = 1.0;
		}
		/** Checks if the description is valid. */
		virtual bool isValid() const {
			if (!golem::Object::Desc::isValid())
				return false;
			if (!armDesc.isValid() || !referencePose.isFinite() || !homePose.isFinite())
				return false;
			for (golem::Bounds::Desc::Seq::const_iterator i = fingerDesc.begin(); i != fingerDesc.end(); i++)
				if (!(*i)->isValid())
					return false;
			
			return true;
		}
	};

	/** Run experiment */
	void run(int argc, char* argv[]);

protected:
	/** Description */
	Desc desc;
	/** Arm */
	golem::PhysReacPlanner* arm;
	/** End-effector */
	golem::JointActor* effector;
	/** End-effector bounds */
	golem::Bounds::Seq effectorBounds;
	/** Object */
	golem::Actor* object;
	/** Obstacles */
	golem::Actor* obstacles;
	/** default polyflap bounds */
	golem::Bounds::SeqPtr objectLocalBounds;
	/** Trial data */
	LearningData learningData;
	/** Time */
	golem::SecTmReal trialTime;
	/** Random number generator */
	golem::RealRand randomG;

	/** LSTM active learner */
	ActiveRNN learner;
	/** to know if net has been already built (initialized in constructor) */
	bool netBuilt;
	/** assumed maximum range of polyflap X-coordinate location during the experiment */
	Real maxRange;
	/** iteration counter */
	int iteration;
	/** const number of SM regions */
	static const int smregionsCount = 18;

	/** Creator */
	golem::Creator creator;
	/** Synchronization objects */
	golem::CriticalSection cs;
	golem::Event ev;
	volatile bool bStart, bStop, bRec;

	/** (Post)processing function called AFTER every physics simulation step and before randering. */
	virtual void postprocess(golem::SecTmReal elapsedTime);
	/** Creates ActiveLearnScenario from description. */
	bool create(const ActiveLearnScenario::Desc& desc);
	/** Releases resources */
	virtual void release();
	/** Objects can be constructed only in the Scene context. */
	ActiveLearnScenario(golem::Scene &scene);

	/** restore predicted last polyflap pose from neural activations */
	golem::Mat34 getPfPoseFromOutputActivations (rnnlib::SeqBuffer<double> outputActivations, int startIndex, Real maxRange);
	/** restore sequence of predicted polyflap poses from neural activations */
	void getPfSeqFromOutputActivations (rnnlib::SeqBuffer<double> outputActivations, int startIndex, Real maxRange);
	/** Renders the object. */
        virtual void render();
 

};

/** Reads/writes ActiveLearnScenario description from/to a given context */
bool XMLData(ActiveLearnScenario::Desc &val, golem::XMLContext* context, bool create = false);

//------------------------------------------------------------------------------

/** MyApplication */
class MyApplication : public golem::Application {
	
protected:
	/** Runs MyApplication */
	virtual void run(int argc, char *argv[]);
};

//------------------------------------------------------------------------------

};	// namespace

#endif /* _SMLEARNING_ACTIVELEARNSCENARIO_H_ */
