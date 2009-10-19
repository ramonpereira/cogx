// ==================================================================
// Place.SA - Place Classification Subarchitecture
// Copyright (C) 2008, 2009  Andrzej Pronobis
//
// This file is part of Place.SA.
//
// Place.SA is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Place.SA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Place.SA. If not, see <http://www.gnu.org/licenses/>.
// ==================================================================

/**
 * PlaceNodeLabeller class.
 * \file NodeLabeller.h
 * \author Andrzej Pronobis
 * \date 2008-09-03
 */

#ifndef __PLACE_NODE_LABELLER__
#define __PLACE_NODE_LABELLER__

#include "place/shared/LabelFile.h"
#include <cast/architecture/ManagedProcess.hpp>
#include <place/idl/PlaceData.hh>

namespace place
{
  class OutputsCache;
  class ConfidenceEstimator;
}


/**
 * Implements the NodeLabeller component
 */
class PlaceNodeLabeller: public cast::ManagedProcess,
                         public PullReceiver<PlaceData::NodePlaceInfo>
{
public: // Component management

  /** Constructor. */
  PlaceNodeLabeller(const std::string &_id);

  /** Destructor. */
  virtual ~PlaceNodeLabeller();

  /** Handles component configuration. */
  virtual void configure(std::map<std::string,std::string> &_config);

  /** Main thread of the component. */
  virtual void runComponent();

  /** Invoked on start. */
  virtual void start();

  /** Invoked on stop. */
  virtual void stop();


protected:

  /** Needed as pure virtual in the parent class. */
  virtual void taskAdopted(const std::string &_taskID)
    {}

  /** Needed as pure virtual in the parent class. */
  virtual void taskRejected(const std::string &_taskID)
    {}

  /** Receives a pull query for NodePlaceInfo. Several arguments must be 
    * provided with the query. The format is:
    * "node_id=\<id\> query_type=\<query_type\> gateway=\<0/1\>".
      node_id is the id of the node for which the placeinfo should be produced.
      query_type should be taken from PlaceData IDL.
      gateway is 1 if the node is a gateway.*/
  void receivePullQuery(const FrameworkQuery & query,
                        FrameworkLocalData<PlaceData::NodePlaceInfo>*& data);


private:

  /** New IntegratedResults added to wm. */
  void newIntegratedResults(const cast::cdl::WorkingMemoryChange & change);

  /** New Odometry added to wm. */
  void newOdometry(const cast::cdl::WorkingMemoryChange & change);

  bool parseQueryArguments(std::string argStr, int &nodeId, 
                           PlaceData::NodeLabellerQueryType &queryType,
                           bool &gateway);

  bool readyToAccumulate();

  void handleInfoQuery(int nodeId, PlaceData::NodePlaceInfo *nodePlaceInfo);
  void handleNewQuery(int nodeId, bool gateway, PlaceData::NodePlaceInfo *nodePlaceInfo);
  void handleUpdateQuery(int nodeId, bool gateway, PlaceData::NodePlaceInfo *nodePlaceInfo);

  void integrateWithPrior(PlaceData::ClassifierOutputs &priorOutputs, unsigned int &priorOutputsCount, const PlaceData::ClassifierOutputs &newOutputs, unsigned  int newOutputsCount);


private:

  std::string _nodeLabellerDataId;

  /** Queue for received results. */
  std::list<PlaceData::IntegratedResults *> _receivedResultsQueue;

  /** Queue for received odometries. */
  std::list<PlaceData::Odometry *> _receivedOdomQueue;

  /** Signal used when new input data received. */
  pthread_cond_t _dataSignalCond;

  /** Mutex used when new input data received. */
  pthread_mutex_t _dataSignalMutex;

  /** Mutex used for accessing caches and internal data. */
  pthread_mutex_t _intrDataMutex;


private:

  /** Name of the config file group. */
  const std::string _cfgGroup;

  double _confidenceThreshold;

  /** List of labels and names. */
  place::LabelFile _labels;

  double _posBinSize;
  double _headBinSize;

  PlaceData::SvmMulticlassAlg _multiclassAlg;
  int _hypFindAlg;

  /** Max. influence of prior. */
  unsigned int _maxPriorOutputsCount;

  /** Only confident classifier outputs will be accumulated. */
  bool _accumulateOnlyConfident;

  /** No. of traveled cache bins after which pose information is forgotten.
    * <0 means "never".*/
  int _cachePoseDecay;


private:

  PlaceData::NodeLabellerData _data;
  place::OutputsCache *_nodeCache;
  place::OutputsCache *_areaCache;

  place::ConfidenceEstimator *_confEstimator;

};


#endif

