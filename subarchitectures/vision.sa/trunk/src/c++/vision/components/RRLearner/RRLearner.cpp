/**
 * @author Alen Vrecko
 * @date October 2009
 */

#include <cast/architecture/ChangeFilterFactory.hpp>
#include "RRLearner.h"

/**
 * The function called to create a new instance of our component.
 */
extern "C"
{
  cast::CASTComponentPtr newComponent()
  {
    return new cast::RRLearner();
  }
}

namespace cast
{

using namespace std;
using namespace cdl;
using namespace VisionData;

using namespace boost::interprocess;
using namespace boost::posix_time;

using namespace de::dfki::lt::tr::beliefs::slice;
using namespace de::dfki::lt::tr::beliefs::slice::distribs;
using namespace de::dfki::lt::tr::beliefs::slice::logicalcontent;

void RRLearner::configure(const map<string,string> & _config)
{
  BindingWorkingMemoryWriter::configure(_config);
  
  map<string,string>::const_iterator it;
  
  if (_config.find("-bsa") != _config.end()) {
	m_bindingSA=_config.find("-bsa")->second;
  } else if (_config.find("--bsa") != _config.end()) {
	m_bindingSA=_config.find("--bsa")->second;
  } else {
   m_bindingSA="binder";
  }

  if((it = _config.find("--display")) != _config.end())
  {
	doDisplay = true;
  }
}

void RRLearner::start()
{
  //must call super start to ensure that the reader sets up change
  //filters
  BindingWorkingMemoryReader::start();
  first = true;
  const char *name = "mediatorSemaphore";
  named_semaphore(open_or_create, name, 0);
  queuesNotEmpty = new named_semaphore(open_only, name);
  log("RRLearner active");
  if (doDisplay)
  {
  }

 // filters for belief updates
  addChangeFilter(createGlobalTypeFilter<PresupposedBelief>(cdl::ADD),
	  new MemberFunctionChangeReceiver<RRLearner>(this,
		&RRLearner::newdBelief));
  
//  addChangeFilter(createGlobalTypeFilter<Belief>(cdl::OVERWRITE),
//	  new MemberFunctionChangeReceiver<RRLearner>(this,
//		&RRLearner::updatedBelief));
}

void RRLearner::runComponent()
{
  // ADD a test beliefs
 /* 
  sleep(5);
  
  log("sumbitting a fake belief 1");
  
  BeliefPtr tb = new Belief();  
  tb->ags = new AttributedAgentStatus();
  
  ComplexFormulaPtr cf = new ComplexFormula();
  
  UnionRefPropertyPtr un =  new UnionRefProperty();
  un->beliefRef = "whatever";
  
  cf->formulae.push_back(un);
  
  ShapePropertyPtr sh = new ShapeProperty();
  sh->shapeValue = cubic;
  sh->polarity = true;
  sh->prob = 1.0f;
  sh->cstatus = assertion;
  
  cf->formulae.push_back(sh);
  
  tb->phi = cf;
  tb->id = newDataID();
  
  addToWorkingMemory(tb->id, m_bindingSA, tb);
  
  
  sleep(5);
  
  
  log("sumbitting a fake belief 2");
  
  tb = new Belief();
  tb->ags = new AttributedAgentStatus();

  cf = new ComplexFormula();
  
  un =  new UnionRefProperty();
  un->beliefRef = "whatever";
  
  cf->formulae.push_back(un);
  
  ColorPropertyPtr co = new ColorProperty();
  co->colorValue = red;
  co->polarity = true;
  co->prob = 1.0f;
  co->cstatus = proposition;
  
  cf->formulae.push_back(co);
  
  tb->phi = cf;
  tb->id = newDataID();
  
  addToWorkingMemory(tb->id, m_bindingSA, tb);
  */
  
  while(isRunning())
  {
	ptime t(second_clock::universal_time() + seconds(2));

	if (queuesNotEmpty->timed_wait(t))
	{
	  log("Got something in my queues");

	  if(!proxyToAdd.empty())
	  { 
		log("An add proxy instruction");
		VisualObjectData &data = VisualObjectMap[proxyToAdd.front()];

		if(data.status == OBJECT)
		{
		  data.status = PROXY;
		  try
		  {
			VisualObjectPtr objPtr = getMemoryEntry<VisionData::VisualObject>(datattributedEpiStatusa.addr);

			WorkingMemoryPointerPtr origin = createWorkingMemoryPointer(getSubarchitectureID(), data.addr.id, "VisualObject");

			FeatureValuePtr value = createUnknownValue(1.00f); //createStringValue (objPtr->label.c_str(), objPtr->labelConfidence);
			//FeatureValuePtr value = createStringValue ("thing", 1.00f);
			FeaturePtr label = createFeatureWithUniqueFeatureValue ("obj_label", value);

			FeatureValuePtr salvalue = createStringValue ("high", 1.00f);
			FeaturePtr saliency = createFeatureWithUniqueFeatureValue ("saliency", salvalue);

			ProxyPtr proxy = createNewProxy (origin, 1.0f);

			addFeatureToProxy (proxy, label);
			addFeatureToProxy (proxy, saliency);
			addFeatureListToProxy(proxy, objPtr->labels, objPtr->distribution);

			addProxyToWM(proxy);
			
			if(!first && existsOnWorkingMemory(m_salientObjID, m_bindingSA))
			{
			  
			  ProxyPtr salProxy = getMemoryEntry<Proxy>(m_salientObjID, m_bindingSA);
			  
			  WorkingMemoryPointerPtr ovrOrigin = createWorkingMemoryPointer(getSubarchitectureID(), salProxy->origin->address.id, "VisualObject");
			  ProxyPtr ovrProxy = createNewProxy (ovrOrigin, 1.0f);
			  
			  vector<FeaturePtr>::iterator it;
			  
			  for(it = salProxy->features.begin(); it != salProxy->features.end(); it++)
			   if(string((*it)->featlabel) == "saliency")
			   {
				  addFeatureToProxy(ovrProxy, createFeatureWithUniqueFeatureValue ("saliency", createStringValue ("low", 1.00f)));
			   }
			   else
			   {
				  addFeatureToProxy(ovrProxy, *it);
			   }
			  
			  ovrProxy->entityID = salProxy->entityID;
			  overwriteProxyInWM(ovrProxy);		 
			}
					
			m_salientObjID = data.proxyId = proxy->entityID;
			first = false;

			log("A visual proxy ID %s added for visual object ID %s",
				proxy->entityID.c_str(), data.addr.id.c_str());

		  }
		  catch (DoesNotExistOnWMException e)
		  {
			log("VisualObject ID: %s was removed before it could be processed", data.addr.id.c_str());
		  }
		}

		proxyToAdd.pop();
	  }
	  else if(!proxyToUpdate.empty())
	  {
		log("An update proxy instruction"); 
		VisualObjectData &data = VisualObjectMap[proxyToUpdate.front()];
	
		if(data.status == PROXY)
		{
		  try
		  {
			VisualObjectPtr objPtr = getMemoryEntry<VisionData::VisualObject>(data.addr);

			WorkingMemoryPointerPtr origin = createWorkingMemoryPointer(getSubarchitectureID(), data.addr.id, "VisualObject");
			
			// check if we can reliable recognise the color
			bool known = false;
			for(int i=0; i<objPtr->distribution.size(); i++)
				if(objPtr->labels[i] <= 7 && objPtr->distribution[i] > 0.7)
				  known = true;
			
			FeatureValuePtr value;	  
			if(known)
			   value = createStringValue ("thing", 1.00f);
			else
			  value = createUnknownValue(1.00f);
			
			FeaturePtr label = createFeatureWithUniqueFeatureValue ("obj_label", value);

			ProxyPtr proxy = createNewProxy (origin, 1.0f);

			addFeatureToProxy (proxy, label);
			addFeatureListToProxy(proxy, objPtr->labels, objPtr->distribution);
			
			proxy->entityID = data.proxyId;
			
			string salval;
			if(m_salientObjID == proxy->entityID)
			  salval = "high";
			 else
			  salval = "low";
			
			FeaturePtr saliency = createFeatureWithUniqueFeatureValue ("saliency", createStringValue (salval, 1.0f));
			addFeatureToProxy (proxy, saliency);
			
			overwriteProxyInWM(proxy);
			
			log("A visual proxy ID %s was updated following the visual object ID %s",
				proxy->entityID.c_str(), data.addr.id.c_str());
				
			checkDistribution4Clarification(proxy->entityID, objPtr->labels, objPtr->distribution);

		  }
		  catch (DoesNotExistOnWMException e)
		  {
			log("VisualObject ID: %s was removed before it could be processed", data.addr.id.c_str());
		  }
		}
		else if(data.status == OBJECT)
		{
		  proxyToUpdate.push(data.addr.id);
		  queuesNotEmpty->post();
		  log("No updating, waiting for the proxy to be created");
		}
		proxyToUpdate.pop();
	  }
	  else if(!proxyToDelete.empty())
	  {
		log("A delete proxy instruction");
		VisualObjectData &obj = VisualObjectMap[proxyToDelete.front()];

		  if(obj.status == PROXY)
		  {
			obj.status == DELETED;
			try
			{  
			  deleteEntityInWM(obj.proxyId);
			  
			  log("A proxy deleted ID: %s", obj.proxyId.c_str());
//			  VisualObjectMap.erase(proxyToDelete.front());
			}
			catch (DoesNotExistOnWMException e)
			{
			  log("WARNING: Proto-object ID %s already removed", obj.proxyId);
			}
		  }

		  proxyToDelete.pop();
	  }
	}
	//    else
	//		log("Timeout");   
  }

  log("Removing semaphore ...");
  queuesNotEmpty->remove("mediatorSemaphore");
  delete queuesNotEmpty;

  if (doDisplay)
  {
  }
}


void RRLearner::newBelief(const cdl::WorkingMemoryChange & _wmc)
{
  log("A presupposed belief was updated. ID: %s SA: %s", _wmc.address.id.c_str(), _wmc.address.subarchitecture.c_str());
  
  dBeliefPtr belief; 
  
  try {
	belief = getMemoryEntry<PresupposedBelief>(_wmc.address);
  }
  catch (DoesNotExistOnWMException e) {
	log("WARNING: belief ID %s was removed before it could be processed.", _wmc.address.id.c_str());
	return;
  }
  		
  debug("Got a belief from WM. ID: %s", _wmc.address.id.c_str());  

  if(!epistemicStatus<SharedEpistemicStatus>(belief))
  {
	log("WARNING: the epistemic status is not a shared one. No learning case.");
	return;
  }
  
  cdl::WorkingMemoryAddress refWmaAddr;
  
  if(!pointedCASTAddr("about", belief, refWmaAddr))
  {
	log("WARNING: no reference property in the presupposed belief. No learning case.");
	return;
  }
  
  debug("Found reference to WM addr %s in the presupposed belief.", refWmaAddr.c_str());
  
  dBeliefPtr refBelief; 
  
  try {
	  refBelief = getMemoryEntry<dBelief>(refWmaAddr);
  }	
  catch (DoesNotExistOnWMException e) {
	  log("WARNING: referenced belief ID %s not in working memory.", refWmaAddr.id.c_str());
	  return;
  }
	
  if(!epistemicStatus<SharedEpistemicStatus>(refBelief))
  {
	log("WARNING: the epistemic status of the referenced is not a shared one. No learning case.");
	return;
  }
  
  

	if(m_currentUnions.find(refBeliefId) != m_currentUnions.end())
	  uni = m_currentUnions[refBeliefId];
	else
	{
	  if(m_haveAddr)
	  {
		addrFetchThenExtract(m_currentUnionsAddr);

		if(m_currentUnions.find(refBeliefId) != m_currentUnions.end())
		  uni = m_currentUnions[refBeliefId];
		else
		{
		  log("Union ID %s not in the current configuration. There are %i unions in the configuration.",
				refBeliefId.c_str(), m_currentUnions.size());
		  return;
		}    
	  }
	  else
	  {
		log("Union ID %s not in the current configuration. There are %i unions in the configuration.",
			  refBeliefId.c_str(), m_currentUnions.size());
		return;
	  }
	}

	debug("Union ID %s exists", uni->entityID.c_str());
	
	vector<ProxyPtr>::iterator it;
	
	bool found = false;
	string ourSA = getSubarchitectureID();

	for(it = uni->includedProxies.begin(); it != uni->includedProxies.end() && !found; it++)
	  if((*it)->origin->address.subarchitecture == ourSA && (*it)->origin->type == "VisualObject")
	  {
		  found = true;
		  visObjID = (*it)->origin->address.id;
	  }
	
	 if(!found)
	 {
		log("The belief concerns no proxy from our SA");  
		return;
	 }
 
	log("Found the object of the belief: visualObject ID %s", visObjID.c_str());
  }
  else
  {
	log("No reference to a belief");
	return;
  }
	
  vector<Shape> shapes;
  vector<Color> colors;
  vector<float> colorDist, shapeDist;
	
  if(findAsserted(obj->phi, colors, shapes, colorDist, shapeDist))
  {
	debug("Found asserted colors or shapes");
	
	compileAndSendLearnTask(visObjID,  _wmc.address.id, colors, shapes, colorDist, shapeDist);
	
	log("Added a learning task for visual object ID %s", visObjID.c_str());
  }
  else
  {
	debug("No asserted colors or shapes - no learning");
	return;
  }
}

/*
void RRLearner::updatedLearningTask(const cdl::WorkingMemoryChange & _wmc)
{
  log("A learning Task was updated. ID: %s SA: %s", _wmc.address.id.c_str(), _wmc.address.subarchitecture.c_str());
  
  VisualLearnerLearningTaskPtr task =
	getMemoryEntry<VisualLearnerLearningTask>(_wmc.address);
	
  debug("Got an overwritten learning task ID: %s", _wmc.address.id.c_str());
  
  BeliefPtr belief = getMemoryEntry<Belief>(task->beliefId, m_bindingSA);
  GroundedBeliefPtr grobelief = new GroundedBelief();
  grobelief->sigma = belief->sigma;
  grobelief->phi = belief->phi;
	removeLearnedAssertions(grobelief->phi, task);
	
	MutualAgentStatusPtr agstatus = new MutualAgentStatus();
	agstatus->ags.push_back(new Agent("robot"));
	agstatus->ags.push_back(new Agent("human"));
  grobelief->ags = agstatus;
  grobelief->grounding = new Ground();
  grobelief->grounding->gstatus = assertionVerified;
  grobelief->grounding->modality = getSubarchitectureID();
//  grobelief->grounding->indexSet
  grobelief->grounding->reason = new SuperFormula();
  grobelief->timeStamp = getCASTTime();
  grobelief->id = belief->id;
  
  overwriteWorkingMemory(task->beliefId, m_bindingSA, grobelief);
  log("Updated the belief ID %s with grounding", belief->id.c_str());

  deleteFromWorkingMemory(_wmc.address);
  log("Removed the learning task ID %s", _wmc.address.id.c_str());
  
  removeChangeFilter(TaskFilterMap[_wmc.address.id]);
  TaskFilterMap.erase(_wmc.address.id);
}
*/


bool RRLearner::pointedCASTAddr(std::string key, map<string,ProbDistribution> distibutions, cast::cdl::WorkingMemoryAddress &refWmaAddr)
{
  // BasicProbDistribution
  map<string,ProbDistribution>::iterator bpd = distributions.find(key);
  
  if(bpd==distributions.end())
	return false;
  else if(bpd->key==key)
	{
	  refWmaAddr=bpd->values->values.begin().val->pointer;
	  return true;
	}
	else
	{
	  log("Basic distribution key did not match the Distributions key");
	  return false;
	}	  
}


bool RRLearner::pointedCASTAddr(std::string key, ProbDistribution distribution, cast::cdl::WorkingMemoryAddress &refWmaAddr)
{
  ProbDistribution *pd= &(*distribution);
  
  if(typeid(*pd) == typeid(CondIndependentDistribs))
	return pointedCASTAddr(key, pd->distributions, refWmaAddr)
  else
  {
	log("UNEXPECTED STRUCTURE in belief content");
	return false;
  }
}

bool RRLearner::pointedCASTAddr(std::string key, dBelief belief, cast::cdl::WorkingMemoryAddress &refWmaAddr)
{
  return pointedCASTAddr(key, belief->content, refWmaAddr);
}


bool RRLearner::findAsserted(FormulaPtr fp, vector<Color> &colors, vector<Shape> &shapes,
								  vector<float> &colorDist, vector<float> &shapeDist)
{
  Formula *f = &(*fp);

  if(typeid(*f) == typeid(ColorProperty))
  {
	ColorPropertyPtr color = dynamic_cast<ColorProperty*>(f);
	
	if(color->cstatus == assertion)
	{ 
	  colors.push_back(color->colorValue);
	
	  if(color->polarity)
		colorDist.push_back(color->prob);
	  else
		colorDist.push_back(-color->prob);

	  return true;
	}
	else
	  return false;
  }
  else if(typeid(*f) == typeid(ShapeProperty))
  {
	ShapePropertyPtr shape = dynamic_cast<ShapeProperty*>(f);
	
	if(shape->cstatus == assertion)
	{
	  shapes.push_back(shape->shapeValue);
	
	  if(shape->polarity)
		shapeDist.push_back(shape->prob);
	  else
		shapeDist.push_back(-shape->prob);

	  return true;
	}
	else
	  return false;
  }
  else if(typeid(*f) == typeid(ComplexFormula))epistemicStatus
  {
	ComplexFormulaPtr unif = dynamic_cast<ComplexFormula*>(f);
	vector<SuperFormulaPtr>::iterator it;
	
	bool found = false;

	for(it = unif->formulae.begin(); it != unif->formulae.end(); it++)
	{
	  SuperFormulaPtr sf = *it;
  
	  if(findAsserted(sf, colors, shapes, colorDist, shapeDist))
		found = true;
	}
  
	return found;
	
  }
  else
	return false;

}


void RRLearner::removeLearnedAssertions(FormulaPtr fp, VisualLearnerLearningTaskPtr task)
{
  Formula *f = &(*fp);

  if(typeid(*f) == typeid(ColorProperty))
  {
	ColorPropertyPtr color = dynamic_cast<ColorProperty*>(f);
	
	if(color->cstatus == assertion)
	  color->cstatus = proposition;
  }
  else if(typeid(*f) == typeid(ShapeProperty))
  {
	ShapePropertyPtr shape = dynamic_cast<ShapeProperty*>(f);
	
	if(shape->cstatus == assertion)
	  shape->cstatus = proposition;
  }
  else if(typeid(*f) == typeid(ComplexFormula))
  {
	ComplexFormulaPtr unif = dynamic_cast<ComplexFormula*>(f);
	vector<SuperFormulaPtr>::iterator it;

	for(it = unif->formulae.begin(); it != unif->formulae.end(); it++)
	{
	  SuperFormulaPtr sf = *it;
  
	  removeLearnedAssertions(sf, task);
	}	
  }
}


void RRLearner::addFeatureListToProxy(ProxyPtr proxy, IntSeq labels, DoubleSeq distribution)
{
  vector<int>::iterator labi;
  vector<double>::iterator disti = distribution.begin();
  
  vector<FeatureValuePtr> colorValues, shapeValues;
  
  for(labi = labels.begin(); labi != labels.end(); labi++)
  {
	FeatureValuePtr value;

	switch(*labi)
	{
		case 1:
			value = createStringValue ("red", *disti);
			break;

		case 2:
			value = createStringValue ("green", *disti);	
			break;
			
		case 3:
			value = createStringValue ("blue", *disti);
			break;

		case 4:
			value = createStringValue ("yellow", *disti);	
			break;
			
		case 5:
			value = createStringValue ("black", *disti); //black
			break;

		case 6:
			value = createStringValue ("white", *disti);	 //white
			break;
			
		case 7:
			value = createStringValue ("orange", *disti);
			break;

		case 8:
			value = createStringValue ("pink", *disti);	
			break;
			
		case 9:
			value = createStringValue ("compact", *disti);
			break;

		case 10:
			value = createStringValue ("elongated", *disti);	
			break;

		default:
			value = createStringValue ("unknown", *disti);
			break;
	}
	
	FeaturePtr label;
	if(*labi <= 8)
	{
//	  value = createStringValue (colorStrEnums[*labi], *disti);
	  colorValues.push_back(value);
	}
	else if(*labi <= 10 )
	{
//	  value = createStringValue (shapeStrEnums[*labi], *disti);
	  shapeValues.push_back(value);  
	} 	
	
	disti++;
  }
  
  if(colorValues.size() > 0)
	addFeatureToProxy(proxy, createFeatureWithAlternativeFeatureValues ("colour", colorValues)); //!!!!!!!
	
  if(shapeValues.size() > 0)   
	addFeatureToProxy(proxy, createFeatureWithAlternativeFeatureValues ("shape", shapeValues));	
}


void RRLearner::compileAndSendLearnTask(const string visualObjID, const string beliefID,
				vector<Color> &colors, vector<Shape> &shapes,
				vector<float> &colorDist, vector<float> &shapeDist)
{
  VisualObjectPtr objPtr = getMemoryEntry<VisualObject>(visualObjID);
  
  VisualLearnerLearningTaskPtr ltask = new VisualLearnerLearningTask();
  
  ltask->visualObjectId = visualObjID;
  ltask->beliefId = beliefID;
  ltask->protoObjectId = objPtr->protoObjectID;
  
  debug("Size of color list %i, distribution list %i", colors.size(), colorDist.size());  
  
  vector<float>::iterator disti = colorDist.begin();
  
  for(vector<Color>::iterator labi = colors.begin(); labi != colors.end(); labi++)
  {
	ltask->distribution.push_back((double) (*disti));
	disti++;
//	ltask->labels.push_back((int) *labi);

	switch(*labi)
	{
		case red:
			debug("Adding red color to learning task");
			ltask->labels.push_back(1);		
			break;

		case green:
			debug("Adding green color to learning task");
			ltask->labels.push_back(2);		
			break;
			
		case blue:
			debug("Adding blue color to learning task");
			ltask->labels.push_back(3);	
			break;

		case yellow:
			debug("Adding yellow color to learning task");
			ltask->labels.push_back(4);		
			break;
			
		case black:
			ltask->labels.push_back(5);	
			break;

		case white:
			ltask->labels.push_back(6);		
			break;
			
		case orange:
			ltask->labels.push_back(7);	
			break;
/*
		case pink:
			ltask->labels.push_back(8);		
			break;
*/		
		default:debug("unknown");
			ltask->labels.push_back(0);
			break;
			
	}
  }
  debug("Size of shape list %i, distribution list %i", shapes.size(), shapeDist.size());
  
  disti = shapeDist.begin();
  
  for(vector<Shape>::iterator labi = shapes.begin(); labi != shapes.end(); labi++)
  {
	ltask->distribution.push_back(*disti);
	disti++;
//	ltask->labels.push_back((int) *labi + 10);
	
	switch(*labi)
	{ 
		case compact:
			debug("Adding compact shape to learning task");
			ltask->labels.push_back(9); //11	
			break;

		case elongated:
			debug("Adding elongated shape to learning task");
			ltask->labels.push_back(10);	//12	
			break;
			
		default:
			ltask->labels.push_back(0);
			break;
	}

  }
  debug("Leaning task compiled");
  
  string newID = newDataID();
  
  WorkingMemoryChangeReceiver *receiver = new MemberFunctionChangeReceiver<RRLearner>(this, &RRLearner::updatedLearningTask);
   
  TaskFilterMap.insert(make_pair(newID, receiver));
  
  addChangeFilter(createIDFilter(newID, cdl::OVERWRITE), receiver);
	  
  addToWorkingMemory(newID, subarchitectureID(), ltask);
  
  debug("Learning task sent");
}


void RRLearner::checkDistribution4Clarification(string proxyID, IntSeq labels, DoubleSeq distribution)
{
  // check if we have a clarification case for colour
  DoubleSeq::iterator i;
  
  int nc=0;
  int ns=0;
  int col, sha;
  
  for(int i=0; i<distribution.size(); i++)
  {
	if(labels[i] <= 7 && distribution[i] > 0.35 && distribution[i] <= 0.7)
	{
	  nc++; 
	  col = labels[i]; 
	}
	
	if(labels[i] >= 9 && labels[i] <= 10 && distribution[i] > 0.35 && distribution[i] <= 0.6)
	{
	  ns++; 
	  sha = labels[i]; 
	}
  }
  
  string refBeliefId;
  
  if(nc > 0 || ns > 0 ) 
  { 
	// retrieve union ID	
	map<string, UnionPtr>::iterator itu;
	bool found = false;
	
	for(itu = m_currentUnions.begin(); itu != m_currentUnions.end(); itu++)
	{
	  ProxySeq proxySeq = (*itu).second->includedProxies;
	  ProxySeq::iterator itp;
	  
	  for(itp = proxySeq.begin(); itp != proxySeq.end(); itp++)
		if((*itp)->entityID == proxyID)
		{
		  refBeliefId = (*itu).first;
		  found = true;
		  break;
		}
		
	  if(found)
		break;
	}
  }
	 
  if(nc > 0) 
  { 
	// Start a new complex formula
	ComplexFormulaPtr formula = new ComplexFormula();
	formula->id = newDataID();
	formula->prob = 1.0f;
	
	ColorPropertyPtr colorProp = new ColorProperty();
	colorProp->cstatus = proposition;
	colorProp->prob = 1.0f;
	
	if(nc==1)
	{   	  
	  switch(col)
	  {
		case 1:
			colorProp->colorValue = red;	
			break;

		case 2:
			colorProp->colorValue = green;	
			break;
			
		case 3:
			colorProp->colorValue = blue;
			break;

		case 4:
			colorProp->colorValue = yellow;		
			break;
		
		case 5:
			colorProp->colorValue = black;	
			break;

		case 6:
			colorProp->colorValue = white;		
			break;
			
		case 7:
			colorProp->colorValue = orange;	
			break;

		default:
			debug("unknown color");
			colorProp->colorValue = unknownColor;
			break;			
	  }
	}
	else
	  colorProp->colorValue = unknownColor;
	
	SuperFormulaSeq frms;
	frms.push_back(colorProp);
	formula->op = none;
	formula->formulae = frms;
	
	ClarificationRequestPtr cr = new ClarificationRequest();
	cr->id = newDataID();
	cr->about = formula;
	cr->clarificationNeed = formula;
	cr->sourceModality = getSubarchitectureID();
	cr->sourceEntityID = refBeliefId;
	
	addToWorkingMemory(newDataID(), "comsys", cr);
	log("Submitted clarification request for colour");
  }
  
  if(ns > 0) 
  { 
	// Start a new complex formula
	ComplexFormulaPtr formula = new ComplexFormula();
	formula->id = newDataID();
	formula->prob = 1.0f;
	
	ShapePropertyPtr shapProp = new ShapeProperty();
	shapProp->cstatus = proposition;
	shapProp->prob = 1.0f;
	
	if(ns==1)
	{   	  
	  switch(sha)
	  {
		case 9:
			shapProp->shapeValue = compact;	
			break;

		case 10:
			shapProp->shapeValue = elongated;	
			break;
			
		default:
			debug("unknown shape");
			shapProp->shapeValue = unknownShape;
			break;			
	  }
	}
	else
	  shapProp->shapeValue = unknownShape;
	
	SuperFormulaSeq frms;
	frms.push_back(shapProp);
	formula->op = none;
	formula->formulae = frms;
	
	ClarificationRequestPtr cr = new ClarificationRequest();
	cr->id = newDataID();
	cr->about = formula;
	cr->clarificationNeed = formula;
	cr->sourceModality = getSubarchitectureID();
	cr->sourceEntityID = refBeliefId;
	
	addToWorkingMemory(newDataID(), "comsys", cr);
	log("Submitted clarification request for shape");
  }

}

}
/* vim:set fileencoding=utf-8 sw=2 ts=4 noet:vim*/

