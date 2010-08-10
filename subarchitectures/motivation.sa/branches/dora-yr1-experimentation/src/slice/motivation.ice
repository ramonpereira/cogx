#ifndef MOTIVATION_ICE
#define MOTIVATION_ICE

#include <cast/slice/CDL.ice>

module motivation {
    module slice {
    
      interface RemoteFilterServer {
    	void setPriority(string motiveType, string priority);    	
      };
    
    
		class WMMutex {
			string name;
			string holderName;
			cast::cdl::WorkingMemoryAddress addr;
		};

    	class TestSource {
    		string name;
    		cast::cdl::CASTTime time;
    	};
    
    
    	enum MotiveStatus {
    		UNSURFACED,
    		SURFACED,
    		POSSIBLE,
    		IMPOSSIBLE,
    		ACTIVE,
    		COMPLETED,
    		WILDCARD
    	};
    	
    	enum MotivePriority {
    		UNSURFACE,
    		LOW,
    		NORMAL,
    		HIGH
    	};
    		
    	class Motive {
			cast::cdl::CASTTime created;
			cast::cdl::CASTTime updated;
			cast::cdl::WorkingMemoryAddress referenceEntry;
			string correspondingUnion;
			cast::cdl::WorkingMemoryAddress thisEntry;
    		MotiveStatus status;
    		/** a counter for the number of tries this motive has been planned */
    		long tries; 	
    		/** [0-1] encoding for a priority */
    		MotivePriority priority;
    		/** [0-inf] encoding for costs */
    		float costs;
    		/** [0-1] encoding for information gain */
    		double informationGain;
    		/** rank of activated motives */
    		int rank;
    		/** the derived single goal for the planner */
    		string goal;
    		/** planCosts */
    		double plannedCosts;
    		/** maxplanningTime in sec **/
    		int maxPlanningTime;
    		/** max execution time in sec **/
    		int maxExecutionTime;
    		
    	};
    	
    	class TestMotive extends Motive {
    		string value;
    		
    	};

    	class HomingMotive extends Motive {
    		long homePlaceID;
    	};

    	class GeneralGoalMotive extends Motive {
    		string internalGoal;
    	};

    	class ExploreMotive extends Motive {
    		/**
	 		* The ID of the place.
	 		*/
			long placeID;
    		
    	};

    	class PatrolMotive extends Motive {
    		/**
	 		* The ID of the place.
	 		*/
			long placeID;
			
			/**
			* last time we have been here
			*/
			cast::cdl::CASTTime lastVisisted;
    		
    	};
    	
    	class CategorizePlaceMotive extends Motive {
    		/**
	 		* The ID of the place.
	 		*/
			long placeID;
    		
    	};

    	class CategorizeRoomMotive extends Motive {
      		/**
	 		* The ID of the place.
	 		*/
			long roomId;
    	};
    	
    	class PlanProxy {
    	    cast::cdl::WorkingMemoryAddress planAddress;
    	};
    };
};

#endif
