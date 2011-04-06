#ifndef MANIPULATION_ICE
#define MANIPULATION_ICE

#include <cast/slice/CDL.ice>
#include <VisionData.ice>

module manipulation {
    module slice {
		enum Completion {
			// failed to grasp the object
			FAILED,
			// grasped the object
			SUCCEEDED
		};
		
		/**
   		* @brief simulate the grasp command
   		* @param targetObject object to simulate to 
   		* @param translational error value of the inverse kinematic
   		* @author Torben Toeniges
   		**/
		class SimulateGraspCommand {
			VisionData::VisualObject targetObject;
			double errorValue;
		};

		/**
   		* @brief combination of the FarArmMovementCommand and the LinearGraspApproachCommand
   		* @param targetObject object to grasp 
   		* @param comp returns the completion of the whole grasp task
   		* @author Torben Toeniges
   		**/
		class GraspCommand {
			VisionData::VisualObject targetObject;
			Completion comp;
		};
		
		/**
   		* @brief perform a far arm movement to place the gripper in front of the given object
   		* @param targetObject object to grasp 
   		* @param comp returns the completion of the task
   		* @author Torben Toeniges
   		**/
		class FarArmMovmentCommand {
			VisionData::VisualObject targetObject;
			Completion comp;
		};
		
		/**
   		* @brief grasp a given visual object with a linear arm movement approach
   		* @param targetObject object to grasp 
   		* @param comp returns the completion of the task
   		* @author Torben Toeniges
   		**/
		class LinearGraspApproachCommand {
			VisionData::VisualObject targetObject;
			Completion comp;
		};
		
		/**
   		* @brief put down a given visual object
   		* @param basedOnObject object to put down the current object
   		* @param comp returns the completion of the put down task
   		* @author Torben Toeniges
   		**/
		class PutDownCommand {
			VisionData::VisualObject basedOnObject;
			Completion comp;			
		};
		
		/**
   		* @brief approach a given object with linear robot base movements
   		* @param targetObject the object to approach
   		* @param comp returns the completion of the approaching task
   		* @author Torben Toeniges
   		**/
		class LinearBaseMovementApproachCommand {
			VisionData::VisualObject targetObject;
			Completion comp;
		};
    };
};

#endif
