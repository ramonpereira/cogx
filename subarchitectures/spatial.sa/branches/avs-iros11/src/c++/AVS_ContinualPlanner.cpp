/*
 * AVS_ContinualPlanner.cpp
 *
 *  Created on: Mar 1, 2011
 *      Author: alper
 */

#include <CureHWUtils.hpp>
#include <AddressBank/ConfigFileReader.hh>

#include "AVS_ContinualPlanner.h"
#include <cast/architecture/ChangeFilterFactory.hpp>
#include <cast/core/CASTUtils.hpp>
#include "ComaData.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include "GridDataFunctors.hh"
#include "GridDataFunctors.hh"

#include "PBVisualization.hh"

using namespace cast;
using namespace std;
using namespace boost;
using namespace SpatialGridMap;
using namespace cogx;
using namespace Math;
using namespace de::dfki::lt::tr::beliefs::slice;
using namespace de::dfki::lt::tr::beliefs::slice::sitbeliefs;
using namespace de::dfki::lt::tr::beliefs::slice::distribs;
using namespace de::dfki::lt::tr::beliefs::slice::logicalcontent;
using namespace eu::cogx::beliefs::slice;

namespace spatial {


extern "C" {
cast::CASTComponentPtr newComponent() {
	return new AVS_ContinualPlanner();
}
}

AVS_ContinualPlanner::AVS_ContinualPlanner() :
	m_sampler(&m_relationEvaluator) {
	// TODO Auto-generated constructor stub
	m_defaultBloxelCell.pdf = 0;
	m_defaultBloxelCell.occupancy = SpatialGridMap::UNKNOWN;
	gotPC = false;

}

AVS_ContinualPlanner::~AVS_ContinualPlanner() {
	// TODO Auto-generated destructor stub
}

void AVS_ContinualPlanner::runComponent() {
	log("I am running");
}

void AVS_ContinualPlanner::owtWeightedPointCloud(
		const cast::cdl::WorkingMemoryChange &objID) {
	log("got weighted PC");
	FrontierInterface::ObjectPriorRequestPtr req = getMemoryEntry<
			FrontierInterface::ObjectPriorRequest> (objID.address);
	m_cloud = req->outCloud;
}

void AVS_ContinualPlanner::receivePointCloud(
		FrontierInterface::WeightedPointCloudPtr cloud, double totalMass) {

}

void AVS_ContinualPlanner::start() {
	//Todo subscribe to View Cone Generation

	addChangeFilter(createGlobalTypeFilter<
			SpatialData::RelationalViewPointGenerationCommand> (cdl::ADD),
			new MemberFunctionChangeReceiver<AVS_ContinualPlanner> (this,
					&AVS_ContinualPlanner::newViewPointGenerationCommand));

	addChangeFilter(createGlobalTypeFilter<
			FrontierInterface::ObjectPriorRequest> (cdl::OVERWRITE),
			new MemberFunctionChangeReceiver<AVS_ContinualPlanner> (this,
					&AVS_ContinualPlanner::owtWeightedPointCloud));

	addChangeFilter(createLocalTypeFilter<NavData::RobotPose2d> (cdl::ADD),
			new MemberFunctionChangeReceiver<AVS_ContinualPlanner> (this,
					&AVS_ContinualPlanner::newRobotPose));

	// TODO: add code to take in spatial objects
	/*	addChangeFilter(
	 createLocalTypeFilter<SpatialData::SpatialObject> (cdl::ADD),
	 new MemberFunctionChangeReceiver<AVS_ContinualPlanner> (this,
	 &AVS_ContinualPlanner::newSpatialObject));*/

	addChangeFilter(
			createLocalTypeFilter<NavData::RobotPose2d> (cdl::OVERWRITE),
			new MemberFunctionChangeReceiver<AVS_ContinualPlanner> (this,
					&AVS_ContinualPlanner::newRobotPose));
	log("getting ice queryHandlerServer");
	m_queryHandlerServerInterfacePrx = getIceServer<
			ConceptualData::QueryHandlerServerInterface> (m_queryHandlerName);
}

void AVS_ContinualPlanner::newViewPointGenerationCommand(
		const cast::cdl::WorkingMemoryChange &objID) {
	log("got new GenerateViewPointCommand");
	SpatialData::RelationalViewPointGenerationCommandPtr newVPCommand = getMemoryEntry<SpatialData::RelationalViewPointGenerationCommand> (
					objID.address);
	SpatialData::RelationalViewPointGenerationCommandPtr cmd = new SpatialData::RelationalViewPointGenerationCommand(*newVPCommand);

	generateViewCones(cmd, objID.address.id);

}


/* Generate view cones for <object,relation , object/room, room> */
void AVS_ContinualPlanner::generateViewCones(
		SpatialData::RelationalViewPointGenerationCommandPtr newVPCommand, std::string WMAddress) {
	log("Generating View Cones!");
	// if we already don't have a room map for this then get the combined map
	if (m_templateRoomBloxelMaps.count(newVPCommand->roomId) == 0) {
		log("Creating a new BloxelMap for room: %d", newVPCommand->roomId);
		log("lalala");
		FrontierInterface::LocalGridMap combined_lgm;

		vector<comadata::ComaRoomPtr> comarooms;
		getMemoryEntries<comadata::ComaRoom> (comarooms, "coma");

		log("Got %d rooms", comarooms.size());

		if (comarooms.size() == 0){
			log("No such ComaRoom with id %d! Returning", newVPCommand->roomId);
			return;
		}
		unsigned int i = 0;
		for (; i < comarooms.size(); i++) {
			log("Got coma room with room id: %d", comarooms[i]->roomId);
			if (comarooms[i]->roomId == newVPCommand->roomId) {
				break;
			}
		}

		FrontierInterface::LocalMapInterfacePrx agg2(getIceServer<
				FrontierInterface::LocalMapInterface> ("map.manager"));
		combined_lgm = agg2->getCombinedGridMap(comarooms[i]->containedPlaceIds);

		for (unsigned int j =0; j < comarooms[i]->containedPlaceIds.size(); j++){
			log("getting room which contains, placeid: %d", comarooms[i]->containedPlaceIds[j]);
		}

		m_templateRoomBloxelMaps[newVPCommand->roomId]
				= new SpatialGridMap::GridMap<GridMapData>(combined_lgm.size*2 + 1,
						combined_lgm.size*2 + 1, m_cellsize, m_minbloxel, 0, 2.0,
						combined_lgm.xCenter, combined_lgm.yCenter, 0,
						m_defaultBloxelCell);

		//convert 2D map to 3D
		CureObstMap* lgm = new CureObstMap(combined_lgm.size, m_cellsize, '2',
				CureObstMap::MAP1, combined_lgm.xCenter, combined_lgm.yCenter);
		IcetoCureLGM(combined_lgm, lgm);
		m_templateRoomGridMaps[newVPCommand->roomId] = lgm;
		GDMakeObstacle makeobstacle;
		for (int x = -combined_lgm.size; x < combined_lgm.size; x++) {
			for (int y = -combined_lgm.size; y < combined_lgm.size; y++) {
				if ((*lgm)(x, y) == '1') {
					m_templateRoomBloxelMaps[newVPCommand->roomId]->boxSubColumnModifier(
							x + combined_lgm.size, y + combined_lgm.size,
							m_LaserPoseR.getZ(), m_minbloxel * 2, makeobstacle);
				}
			}
		}
	}

	bool alreadyGenerated = false;

	//string id = convertLocation2Id(newVPCommand);
	string plus = "p(+";
		string closebracket = ")";
	string id = plus + m_namegenerator.getUnexploredObjectVarName(newVPCommand->roomId, newVPCommand->searchedObjectCategory, newVPCommand->relation,
			newVPCommand->supportObjectCategory, newVPCommand->supportObject) + closebracket;

	if (m_objectBloxelMaps.count(id) > 0) {
		alreadyGenerated = true;
	}
	// if we already have a bloxel map for configuration
	// else create a new one
	if (!alreadyGenerated) {
		//Since we don't have this location currently, first initialize a bloxel map for it from the template room map
		m_objectBloxelMaps[id] = new SpatialGridMap::GridMap<GridMapData>(
				*m_templateRoomBloxelMaps[newVPCommand->roomId]);
	} else {
		// Todo: We already have a map for this configuration
	}

	if(m_usePeekabot){
		pbVis->DisplayMap(*m_objectBloxelMaps[id]);
	}

	// now we have our room map let's fill it
	//Query Conceptual to learn the initial pdf values

	ConceptualData::ProbabilityDistributions conceptualProbdist = m_queryHandlerServerInterfacePrx->query(id);
	SpatialProbabilities::ProbabilityDistribution probdist = conceptualProbdist[0];

	//Todo: Somehow get the probability from probDist
	double pdfmass = 0.5;

	//Todo: Generate viewpoints on the selected room's bloxel map and for a given pdf id
	if (newVPCommand->supportObject != "") {
		log("Searching with a support object, making a distribution query");
		// indirect search this must be known object
		// Ask for the cloud

		// Construct Request object
		std::vector<std::string> labels;
		vector<Vector3> centers;
		std::vector<FrontierInterface::ObjectRelation> relation;
		labels.push_back(newVPCommand->supportObject);
		labels.push_back(newVPCommand->searchedObjectCategory);
		relation.push_back(
				(newVPCommand->relation == SpatialData::INOBJECT ? FrontierInterface::IN
						: FrontierInterface::ON));

		FrontierInterface::WeightedPointCloudPtr queryCloud =
				new FrontierInterface::WeightedPointCloud;
		FrontierInterface::ObjectPriorRequestPtr objreq =
				new FrontierInterface::ObjectPriorRequest;
		objreq->relationTypes = relation; // ON or IN or whatnot
		objreq->objects = labels; // Names of objects, starting with the query object
		objreq->cellSize = m_cellsize; // Cell size of map (affects spacing of samples)
		objreq->outCloud = queryCloud; // Data struct to receive output
		objreq->totalMass = 1.0;
		//wait until we get the cloud back
		{
			unlockComponent();
			addToWorkingMemory(newDataID(), objreq);
			gotPC = false;
			while (!gotPC)
				usleep(2500);
			log("got PC for direct search");
		}
		lockComponent();
		if (m_cloud->isBaseObjectKnown) {
			log("Got distribution around known object pose");
			m_sampler.kernelDensityEstimation3D(*m_objectBloxelMaps[id],
					centers, m_cloud->interval, m_cloud->xExtent,
					m_cloud->yExtent, m_cloud->zExtent, m_cloud->values, 1.0,
					pdfmass, m_templateRoomGridMaps[newVPCommand->roomId]);
			normalizePDF(*m_objectBloxelMaps[id], pdfmass);
		}

	} else {
		log("Searching in the room, assuming uniformn probability");
		// uniform over the room
		GDProbSet resetter(0.0);
		m_objectBloxelMaps[id]->universalQuery(resetter, true);
		double fixedpdfvalue = pdfmass / (m_objectBloxelMaps[id]->getZBounds().second - m_objectBloxelMaps[id]->getZBounds().first);

		GDProbInit initfunctor(fixedpdfvalue);
		log("Setting each bloxel to a fixed value of %f, in total: %f", fixedpdfvalue, pdfmass);

		CureObstMap* lgm = m_templateRoomGridMaps[newVPCommand->roomId];
		for (int x = -lgm->getSize(); x <= lgm->getSize(); x++) {
			for (int y = -lgm->getSize(); y <= lgm->getSize(); y++) {
				int bloxelX = x + lgm->getSize();
				int bloxelY = y + lgm->getSize();
				if ((*lgm)(x, y) == '3' || (*lgm)(x, y) == '1') {
					// For each "high" obstacle cell, assign a uniform probability density to its immediate neighbors
					// (Only neighbors which are not unknown in the Cure map. Unknown 3D
					// space is still assigned, though)

					for (int i = -1; i <= 1; i++) {
						for (int j = -1; j <= 1; j++) {
							if ((*lgm)(x + i, y + j) == '0'
									&& (bloxelX + i
											<= m_objectBloxelMaps[id]->getMapSize().first
											&& bloxelX + i > 0)
									&& (bloxelY + i
											<= m_objectBloxelMaps[id]->getMapSize().second
											&& bloxelY + i > 0)) {
//								/log("modifying bloxelmap pdf");
								m_objectBloxelMaps[id]->boxSubColumnModifier(
										bloxelX + i, bloxelY + j, m_mapceiling
												/ 2, m_mapceiling, initfunctor);
							}
						}
					}
				}
			}
		}

		double massAfterInit = initfunctor.getTotal();
			//    double normalizeTo = (initfunctor.getTotal()*m_pout)/(1 - m_pout);
		normalizePDF(*m_objectBloxelMaps[id], pdfmass, massAfterInit);

	}


	//Now that we've got our map generate cones for this
	//Todo: and generateViewCones based on this
    if(m_usePeekabot){
    	pbVis->DisplayMap(*m_objectBloxelMaps[id]);
      pbVis->AddPDF(*m_objectBloxelMaps[id]);

    }
	log("getting cones..");
	ViewPointGenerator coneGenerator(this,
			m_templateRoomGridMaps[newVPCommand->roomId],
			m_objectBloxelMaps[id], m_samplesize, m_sampleawayfromobs,
			m_conedepth, m_horizangle, m_vertangle, m_minDistance, pdfmass,
			0.7, 0, 0);
	vector<ViewPointGenerator::SensingAction> viewcones =
			coneGenerator.getBest3DViewCones();

	log("got %d cones..", viewcones.size());


	// 1. Create conegroups out of viewcones
	// 2. Fill in relevant fields
	// 3. Create beliefs about them

	for (unsigned int i=0; i < viewcones.size(); i++){
		if(m_usePeekabot){
			PostViewCone(viewcones[i]);
		}
		// FIXME ASSUMPTION: One cone per conegroup
		ConeGroup c;	c.relation = SpatialData::INROOM;
		c.searchedObjectCategory = newVPCommand->searchedObjectCategory;
		c.bloxelMapId = id;
		c.viewcones.push_back(viewcones[i]);
		if (newVPCommand->supportObject == ""){
			c.relation = SpatialData::INROOM;
			c.supportObjectId = "";
			c.supportObjectCategory = "";
			c.searchedObjectCategory = newVPCommand->searchedObjectCategory;
		}
		else {
			c.relation = (newVPCommand->relation == SpatialData::INOBJECT ? SpatialData::INOBJECT
					: SpatialData::ON);
					c.supportObjectId = newVPCommand->supportObject;
					//FIXME: Get category from object ID
					c.supportObjectCategory = "";
					c.searchedObjectCategory = newVPCommand->searchedObjectCategory;
		}


		log("Looking for Coma room beliefs...");
		 cast::cdl::WorkingMemoryAddress WMaddress;
		if (newVPCommand->relation == SpatialData::INROOM){
			//get the roomid belief WMaddress
			vector< boost::shared_ptr< cast::CASTData<GroundedBelief> > > comaRoomBeliefs;
			getWorkingMemoryEntries<GroundedBelief> ("coma", 0, comaRoomBeliefs);

			if (comaRoomBeliefs.size() ==0){
				log("Could not get any room beliefs returning without doing anything...");
				return;
			}
			else{
				log("Go %d ComaRoom beliefs", comaRoomBeliefs.size());
			}

			for(unsigned int i=0; i < comaRoomBeliefs.size(); i++){
			CondIndependentDistribsPtr dist(CondIndependentDistribsPtr::dynamicCast(comaRoomBeliefs[i]->getData()->content));
			BasicProbDistributionPtr  basicdist(BasicProbDistributionPtr::dynamicCast(dist->distribs["RoomId"]));
			FormulaValuesPtr formulaValues(FormulaValuesPtr::dynamicCast(basicdist->values));

			IntegerFormulaPtr intformula(IntegerFormulaPtr::dynamicCast(formulaValues->values[0].val));
			int roomid = intformula->val;
			log("ComaRoom Id for this belief: %d", roomid);
			if (newVPCommand->roomId == roomid){
				log("Got room belief from roomid: %d", newVPCommand->roomId);
				WMaddress.id = comaRoomBeliefs[i]->getID();
				WMaddress.subarchitecture = "binder";
			}
		}
		}

		log("Creating ConeGroup belief");
		m_beliefConeGroups[m_coneGroupId++] = c;
		eu::cogx::beliefs::slice::GroundedBeliefPtr b = new eu::cogx::beliefs::slice::GroundedBelief;
		epstatus::PrivateEpistemicStatusPtr beliefEpStatus= new epstatus::PrivateEpistemicStatus;
		beliefEpStatus->agent = "self";

		b->estatus = beliefEpStatus;
		b->type = "conegroup";
		b->id = id;
		CondIndependentDistribsPtr CondIndProbDist = new  CondIndependentDistribs;

		BasicProbDistributionPtr coneGroupIDProbDist = new BasicProbDistribution;
		BasicProbDistributionPtr searchedObjectLabelProbDist = new BasicProbDistribution;
		BasicProbDistributionPtr relationLabelProbDist = new BasicProbDistribution;
		BasicProbDistributionPtr supportObjectLabelProbDist = new BasicProbDistribution;
		BasicProbDistributionPtr coneProbabilityProbDist = new BasicProbDistribution;

		FormulaValuesPtr formulaValues = new FormulaValues;
		FormulaProbPairs pairs;
		FormulaProbPair searchedObjectFormulaPair, coneGroupIDLabelFormulaPair,
		relationLabelFormulaPair,supportObjectLabelFormulaPair, coneProbabilityFormulaPair ;

		IntegerFormulaPtr coneGroupIDLabelFormula = new IntegerFormula;
		ElementaryFormulaPtr searchedObjectLabelFormula = new ElementaryFormula;
		ElementaryFormulaPtr relationLabelFormula = new ElementaryFormula;
		PointerFormulaPtr supportObjectLabelFormula = new PointerFormula;
		FloatFormulaPtr coneProbabilityFormula = new FloatFormula;

		coneGroupIDLabelFormula->val = m_coneGroupId;
		searchedObjectLabelFormula->prop =c.searchedObjectCategory;
		relationLabelFormula->prop = (newVPCommand->relation == SpatialData::INOBJECT ? "in" : "on");
		supportObjectLabelFormula->pointer =  WMaddress; //c.supportObjectId; // this should be a pointer ideally
		coneProbabilityFormula->val = c.getTotalProb();

		searchedObjectFormulaPair.val = coneGroupIDLabelFormula;
		searchedObjectFormulaPair.prob = 1;

		coneGroupIDLabelFormulaPair.val = searchedObjectLabelFormula;
		coneGroupIDLabelFormulaPair.prob = 1;

		relationLabelFormulaPair.val = relationLabelFormula;
		relationLabelFormulaPair.prob = 1;

		supportObjectLabelFormulaPair.val = supportObjectLabelFormula;
		supportObjectLabelFormulaPair.prob = 1;

		coneProbabilityFormulaPair.val = coneProbabilityFormula;
		coneProbabilityFormulaPair.prob = 1;


		pairs.push_back(coneGroupIDLabelFormulaPair);
		formulaValues->values = pairs;
		coneGroupIDProbDist->values = formulaValues;
		pairs.clear();

		pairs.push_back(searchedObjectFormulaPair);
		formulaValues->values = pairs;
		searchedObjectLabelProbDist->values = formulaValues;
		pairs.clear();

		pairs.push_back(coneProbabilityFormulaPair);
		formulaValues->values = pairs;
		relationLabelProbDist->values = formulaValues;
		pairs.clear();


		pairs.push_back(relationLabelFormulaPair);
		formulaValues->values = pairs;
		relationLabelProbDist->values = formulaValues;
		pairs.clear();

		pairs.push_back(coneProbabilityFormulaPair);
		formulaValues->values = pairs;
		supportObjectLabelProbDist->values = formulaValues;
		pairs.clear();

		CondIndProbDist->distribs["id"] = coneGroupIDProbDist;
		CondIndProbDist->distribs["searchedobject"] = searchedObjectLabelProbDist;
		CondIndProbDist->distribs["relation"] = relationLabelProbDist;
		CondIndProbDist->distribs["supportobject"] = supportObjectLabelProbDist;
		CondIndProbDist->distribs["prob"] = coneProbabilityProbDist;

		b->content = CondIndProbDist;
		log("writing belief to WM..");
		addToWorkingMemory(newDataID(), "binder", b);
		log("wrote belief to WM..");
		newVPCommand->status = SpatialData::SUCCESS;
		 overwriteWorkingMemory<SpatialData::RelationalViewPointGenerationCommand>(WMAddress , newVPCommand);
	}

}

void AVS_ContinualPlanner::IcetoCureLGM(FrontierInterface::LocalGridMap icemap,
		CureObstMap* lgm) {
	log(
			"icemap.size: %d, icemap.data.size %d, icemap.cellSize: %f, centerx,centery: %f,%f",
			icemap.size, icemap.data.size(), icemap.cellSize, icemap.xCenter,
			icemap.yCenter);
	int lp = 0;
	for (int x = -icemap.size; x <= icemap.size; x++) {
		for (int y = -icemap.size; y <= icemap.size; y++) {
			(*lgm)(x, y) = (icemap.data[lp]);
			lp++;
		}
	}
	log("converted icemap to Cure::LocalGridMap");
}

/* Process ConeGroup with id */
void AVS_ContinualPlanner::processConeGroup(string id) {
	// Todo: Loop over cones in conegroup
	// FIXME ASSUMPTION ConeGroup contains one cone
	// FIXME ASSUMPTION Always observe one object at a time

	// Todo: Once we get a response from vision, calculate the remaining prob. value

	// Get ConeGroup

	if (m_beliefConeGroups.count(atoi(id.c_str())) == 0 ){
		log("No bloxelmap with id: %s, this is an indication of id mismatch!", id.c_str());
		return;
	}

	m_currentConeGroup = m_beliefConeGroups[atoi(id.c_str())];
	m_currentViewCone = m_currentConeGroup.viewcones[0];
	ViewConeUpdate(m_currentViewCone, m_objectBloxelMaps[m_currentConeGroup.bloxelMapId]);

	Cure::Pose3D pos;
	pos.setX(m_currentViewCone.pos[0]);
	pos.setY(m_currentViewCone.pos[1]);
	pos.setTheta(m_currentViewCone.pan);
	PostNavCommand(pos, SpatialData::GOTOPOSITION);
}


void AVS_ContinualPlanner::ViewConeUpdate(ViewPointGenerator::SensingAction viewcone, BloxelMap* map){

double sensingProb = 0.8;
GDProbSum sumcells;
GDIsObstacle isobstacle;
/* DEBUG */
GDProbSum conesum;
map->coneQuery(viewcone.pos[0],viewcone.pos[1],
    viewcone.pos[2], viewcone.pan, viewcone.tilt, m_horizangle, m_vertangle, m_conedepth, 10, 10, isobstacle, conesum, conesum, m_minDistance);
	log("ViewConeUpdate: Cone sums to %f",conesum.getResult());

/* DEBUG */
map->universalQuery(sumcells);

double mapsum = 0;
mapsum = sumcells.getResult();
log("ViewConeUpdate: Whole map PDF sums to: %f", mapsum);
// then deal with those bloxels that belongs to this cone

GDProbScale scalefunctor(1-sensingProb);
map->coneModifier(viewcone.pos[0],viewcone.pos[1],
    viewcone.pos[2], viewcone.pan, viewcone.tilt, m_horizangle, m_vertangle, m_conedepth, 10, 10, isobstacle, scalefunctor,scalefunctor, m_minDistance);

sumcells.reset();
map->universalQuery(sumcells);

log("ViewConeUpdate: Map sums to: %f", sumcells.getResult());
	if(m_usePeekabot)
	  pbVis->AddPDF(*map);
 }
void AVS_ContinualPlanner::modifyPDFAfterRecognition(){

}


void AVS_ContinualPlanner::Recognize(){
	// we always ask for all the objects to be recognized
	for (unsigned int i=0; i< m_siftObjects.size(); i++){
		//todo: ask for visual object recognition
	}
	for (unsigned int i=0; i<m_ARtaggedObjects.size(); i++){
		//todo: ask for tagged objects
	}
}


std::string AVS_ContinualPlanner::convertLocation2Id(
		SpatialData::RelationalViewPointGenerationCommandPtr newVPCommand) {
	string id = "";
	string relationstring =
			(newVPCommand->relation == SpatialData::INOBJECT) ? "INOBJECT"
					: "INROOM";
	if (newVPCommand->supportObject != "")
		id = newVPCommand->searchedObjectCategory + relationstring
				+ newVPCommand->supportObject + boost::lexical_cast<string>(
				newVPCommand->roomId);
	else
		id = newVPCommand->searchedObjectCategory + relationstring
				+ newVPCommand->supportObject + boost::lexical_cast<string>(
				newVPCommand->roomId);
	return id;
}

void AVS_ContinualPlanner::configure(
		const std::map<std::string, std::string>& _config) {
	map<string, string>::const_iterator it = _config.find("-c");

	if (it == _config.end()) {
		println("configure(...) Need config file (use -c option)\n");
		std::abort();
	}
	std::string configfile = it->second;

	Cure::ConfigFileReader *cfg;
	if (it != _config.end()) {
		cfg = new Cure::ConfigFileReader;
		log("About to try to open the config file");
		if (cfg->init(it->second) != 0) {
			delete cfg;
			cfg = 0;
			log("Could not init Cure::ConfigFileReader with -c argument");
		} else {
			log("Managed to open the Cure config file");
		}
	}

	if (cfg->getSensorPose(1, m_LaserPoseR)) {
		println("configure(...) Failed to get sensor pose for laser");
		std::abort();
	}

    cfg->getRoboLookHost(m_PbHost);
    std::string usedCfgFile, tmp;
    if (cfg && cfg->getString("PEEKABOT_HOST", true, tmp, usedCfgFile) == 0) {
      m_PbHost = tmp;
    }

	m_gridsize = 200;
	m_cellsize = 0.05;
	it = _config.find("--gridsize");
	if (it != _config.end()) {

		m_gridsize = (atoi(it->second.c_str()));
		log("Gridsize set to: %d", m_gridsize);
	}
	it = _config.find("--cellsize");
	if (it != _config.end()) {
		m_cellsize = (atof(it->second.c_str()));
		log("Cellsize set to: %f", m_cellsize);
	}

	m_samplesize = 100;

	 it = _config.find("--samplesize");
	    if (it != _config.end()) {
	      m_samplesize = (atof(it->second.c_str()));
	      log("Samplesize set to: %d", m_samplesize);
	    }

	m_minbloxel = 0.05;
	it = _config.find("--minbloxel");
	if (it != _config.end()) {
		m_minbloxel = (atof(it->second.c_str()));
		log("Min bloxel height set to: %f", m_minbloxel);
	}

	m_mapceiling = 2.0;
	it = _config.find("--mapceiling");
	if (it != _config.end()) {
		m_mapceiling = (atof(it->second.c_str()));
		log("Map ceiling set to: %d", m_mapceiling);
	}

	it = _config.find("--kernel-width-factor");
	if (it != _config.end()) {
		m_sampler.setKernelWidthFactor(atoi(it->second.c_str()));
	}

	m_horizangle = M_PI / 4;
	it = _config.find("--cam-horizangle");
	if (it != _config.end()) {
		m_horizangle = (atof(it->second.c_str())) * M_PI / 180.0;
		log("Camera FoV horizontal angle set to: %f", m_horizangle);
	}

	m_vertangle = M_PI / 4;
	it = _config.find("--cam-vertangle");
	if (it != _config.end()) {
		m_vertangle = (atof(it->second.c_str())) * M_PI / 180.0;
		log("Camera FoV vertical angle set to: %f", m_vertangle);
	}

	m_conedepth = 2.0;
	it = _config.find("--cam-conedepth");
	if (it != _config.end()) {
		m_conedepth = (atof(it->second.c_str()));
		log("Camera view cone depth set to: %f", m_conedepth);
	}

	m_sampleawayfromobs= 0.2;
	    it = _config.find("--sampleawayfromobs");
	    if (it != _config.end()) {
	      m_sampleawayfromobs= (atof(it->second.c_str()));
	      log("Distance to nearest obs for samples set to: %f", m_sampleawayfromobs);
	    }

	m_minDistance = m_conedepth / 4.0;

	  m_usePeekabot = false;
	    if (_config.find("--usepeekabot") != _config.end())
	      m_usePeekabot= true;

	    if(m_usePeekabot){
	      pbVis = new VisualPB_Bloxel(m_PbHost,5050,m_gridsize,m_gridsize,m_cellsize,1,true);//host,port,xsize,ysize,cellsize,scale, redraw whole map every time
	      pbVis->connectPeekabot();
	    }

	m_usePTZ = false;
	if (_config.find("--ctrl-ptu") != _config.end()) {
		m_usePTZ = true;
		log("will use ptu");
	}

	m_ignoreTilt = false;
		if (_config.find("--ignore-tilt") != _config.end())
			m_ignoreTilt = true;

	// QueryHandler name
	if ((it = _config.find("--queryhandler")) != _config.end()) {
		m_queryHandlerName = it->second;
	}

	if (m_usePTZ) {
			log("connecting to PTU");
			Ice::CommunicatorPtr ic = getCommunicator();

			Ice::Identity id;
			id.name = "PTZServer";
			id.category = "PTZServer";

			std::ostringstream str;
			str << ic->identityToString(id) << ":default" << " -h localhost"
					<< " -p " << cast::cdl::CPPSERVERPORT;

			Ice::ObjectPrx base = ic->stringToProxy(str.str());
			m_ptzInterface = ptz::PTZInterfacePrx::uncheckedCast(base);
		}

	if ((it = _config.find("--siftobjects")) != _config.end()) {
		istringstream istr(it->second);
		string label;
		while (istr >> label) {
			m_siftObjects.push_back(label);
		}
	}
	log("Loaded sift objects.");
	for (unsigned int i = 0; i < m_siftObjects.size(); i++)
		log("%s , ", m_siftObjects[i].c_str());


	if ((it = _config.find("--ARtagobjects")) != _config.end()) {
			istringstream istr(it->second);
			string label;
			while (istr >> label) {
				m_ARtaggedObjects.push_back(label);
			}
		}
		log("Loaded tagged objects.");
		for (unsigned int i = 0; i < m_ARtaggedObjects.size(); i++)
			log("%s , ", m_ARtaggedObjects[i].c_str());

		m_coneGroupId = -1;
}

void AVS_ContinualPlanner::newRobotPose(const cdl::WorkingMemoryChange &objID) {
	try {
		lastRobotPose = getMemoryEntry<NavData::RobotPose2d> (objID.address);

	} catch (DoesNotExistOnWMException e) {
		log("Error! robotPose missing on WM!");
		return;
	}

}

AVS_ContinualPlanner::NavCommandReceiver::NavCommandReceiver(
		AVS_ContinualPlanner & _component, SpatialData::NavCommandPtr _cmd) :
	m_component(_component), m_cmd(_cmd) {
	m_component.log("received NavCommandReceiver notification");
	string id(m_component.newDataID());
	m_component.log("ID post: %s", id.c_str());

	m_component.addChangeFilter(createIDFilter(id, cdl::OVERWRITE), this);
	m_component.addToWorkingMemory<SpatialData::NavCommand> (id, m_cmd);

}

void AVS_ContinualPlanner::NavCommandReceiver::workingMemoryChanged(
		const cast::cdl::WorkingMemoryChange &objID) {
	m_component.log("received inner notification");
	try {
		m_component.owtNavCommand(objID);
	} catch (const CASTException &e) {
		//      log("failed to delete SpatialDataCommand: %s", e.message.c_str());
	}

}

void AVS_ContinualPlanner::owtNavCommand(
		const cast::cdl::WorkingMemoryChange &objID) {
	SpatialData::NavCommandPtr cmd(getMemoryEntry<SpatialData::NavCommand> (objID.address));
	try {
	if (cmd->comp == SpatialData::COMMANDSUCCEEDED) {
		// it means we've reached viewcone position
		MovePanTilt(0.0, m_currentViewCone.tilt, 0.08);
		Recognize();

	}
	if (cmd->comp == SpatialData::COMMANDFAILED) {
				// it means we've failed to reach the viewcone position
			}
	} catch (const CASTException &e) {
			//      log("failed to delete SpatialDataCommand: %s", e.message.c_str());
		}
	}

void AVS_ContinualPlanner::PostNavCommand(Cure::Pose3D position,
		SpatialData::CommandType cmdtype) {
	SpatialData::NavCommandPtr cmd = new SpatialData::NavCommand();
	cmd->prio = SpatialData::URGENT;
	cmd->cmd = cmdtype;
	cmd->pose.resize(3);
	cmd->pose[0] = position.getX();
	cmd->pose[1] = position.getY();
	cmd->pose[2] = position.getTheta();
	cmd->tolerance.resize(1);
	cmd->tolerance[0] = 0.1;
	cmd->status = SpatialData::NONE;
	cmd->comp = SpatialData::COMMANDPENDING;
	new NavCommandReceiver(*this, cmd);
	log("posted nav command");
}

void AVS_ContinualPlanner::addRecognizer3DCommand(
		VisionData::Recognizer3DCommandType cmd, std::string label,
		std::string visualObjectID) {
	log("posting recognizer command: %s.", label.c_str());
	VisionData::Recognizer3DCommandPtr rec_cmd =
			new VisionData::Recognizer3DCommand;
	rec_cmd->cmd = cmd;
	rec_cmd->label = label;
	rec_cmd->visualObjectID = visualObjectID;
	log("constructed visual command, adding to WM");
	addToWorkingMemory(newDataID(), "vision.sa", rec_cmd);
	log("added to WM");
}


void AVS_ContinualPlanner::MovePanTilt(double pan, double tilt, double tolerance) {

	if (m_usePTZ) {
		log(" Moving pantilt to: %f %f with %f tolerance", pan, tilt, tolerance);
		ptz::PTZPose p;
		ptz::PTZReading ptuPose;
		p.pan = pan;
		p.tilt = tilt;

		p.zoom = 0;
		m_ptzInterface->setPose(p);
		bool run = true;
		ptuPose = m_ptzInterface->getPose();
		double actualpan = ptuPose.pose.pan;
		double actualtilt = ptuPose.pose.tilt;

		while (run) {
			m_ptzInterface->setPose(p);
			ptuPose = m_ptzInterface->getPose();
			actualpan = ptuPose.pose.pan;
			actualtilt = ptuPose.pose.tilt;

			log("desired pan tilt is: %f %f", pan, tilt);
			log("actual pan tilt is: %f %f", actualpan, actualtilt);
			log("tolerance is: %f", tolerance);

			//check that pan falls in tolerance range
			if (actualpan < (pan + tolerance) && actualpan > (pan - tolerance)) {
				run = false;
			}

			//only check tilt if pan is ok

			if (!run) {
				if (m_ignoreTilt) {
					run = false;
				} else {
					if (actualtilt < (tilt + tolerance) && actualtilt > (tilt
							- tolerance)) {
						run = false;
					} else {
						//if the previous check fails, loop again
						run = true;
					}
				}
			}

			usleep(100000);
		}
		log("Moved.");
		sleep(1);
	}
}

void AVS_ContinualPlanner::addARTagCommand(){
	//todo: add new AR tag command
}

void AVS_ContinualPlanner::PostViewCone(const ViewPointGenerator::SensingAction &nbv)
{
/* Add plan to PB BEGIN */
NavData::ObjectSearchPlanPtr obs = new NavData::ObjectSearchPlan;
NavData::ViewPoint viewpoint;
viewpoint.pos.x = nbv.pos[0];
viewpoint.pos.y = nbv.pos[1];
viewpoint.pos.z = nbv.pos[2];
viewpoint.length = m_conedepth;
viewpoint.pan = nbv.pan;
viewpoint.tilt = nbv.tilt;
obs->planlist.push_back(viewpoint);
addToWorkingMemory(newDataID(), obs);
/* Add plan to PB END */

//cout << "selected cone " << viewpoint.pos.x << " " << viewpoint.pos.y << " " <<viewpoint.pos.z << " " << viewpoint.pan << " " << viewpoint.tilt << endl;
}

} //namespace
