/**
 * @author Alen Vrecko
 * @date October 2009
 */

#include <cast/architecture/ChangeFilterFactory.hpp>
#include "VisualMediator.h"

/**
 * The function called to create a new instance of our component.
 */
extern "C"
{
  cast::CASTComponentPtr newComponent()
  {
    return new cast::VisualMediator();
  }
}

namespace cast
{

using namespace std;
using namespace cdl;
using namespace VisionData;

using namespace boost::interprocess;
using namespace boost::posix_time;

using namespace binder::autogen::core;
using namespace binder::autogen::featvalues;

using namespace beliefmodels::adl;
using namespace beliefmodels::domainmodel::cogx;

void VisualMediator::configure(const map<string,string> & _config)
{
  BindingWorkingMemoryWriter::configure(_config);
  
  map<string,string>::const_iterator it;

  updateThr = UPD_THR_DEFAULT;
  
  if (_config.find("-bsa") != _config.end()) {
	m_bindingSA=_config.find("-bsa")->second;
  } else if (_config.find("--bsa") != _config.end()) {
	m_bindingSA=_config.find("--bsa")->second;
  } else {
   m_bindingSA="binder";
  }

  if((it = _config.find("--upd")) != _config.end())
  {
	istringstream str(it->second);
	str >> updateThr;
  }

  if((it = _config.find("--display")) != _config.end())
  {
	doDisplay = true;
  }
}

void VisualMediator::start()
{
  //must call super start to ensure that the reader sets up change
  //filters
  BindingWorkingMemoryReader::start();

  const char *name = "mediatorSemaphore";
  named_semaphore(open_or_create, name, 0);
  queuesNotEmpty = new named_semaphore(open_only, name);
log("HELLO, Mediator active");
  if (doDisplay)
  {
  }

  // we want to receive detected VisualObjects
  addChangeFilter(createLocalTypeFilter<VisionData::VisualObject>(cdl::ADD),
	  new MemberFunctionChangeReceiver<VisualMediator>(this,
		&VisualMediator::newVisualObject));
  // .., when they are updated
  addChangeFilter(createLocalTypeFilter<VisionData::VisualObject>(cdl::OVERWRITE),
	  new MemberFunctionChangeReceiver<VisualMediator>(this,
		&VisualMediator::updatedVisualObject));
  // .. and when they are deleted
  addChangeFilter(createLocalTypeFilter<VisionData::VisualObject>(cdl::DELETE),
	  new MemberFunctionChangeReceiver<VisualMediator>(this,
		&VisualMediator::deletedVisualObject));

 // filters for belief updates
  addChangeFilter(createGlobalTypeFilter<Belief>(cdl::ADD),
	  new MemberFunctionChangeReceiver<VisualMediator>(this,
		&VisualMediator::updatedBelief));

  
  addChangeFilter(createGlobalTypeFilter<Belief>(cdl::OVERWRITE),
	  new MemberFunctionChangeReceiver<VisualMediator>(this,
		&VisualMediator::updatedBelief));
}

void VisualMediator::runComponent()
{
  // ADD a test beliefs
 /* 
  sleep(5);
  
  log("sumbitting a fake belief 1");
  
  BeliefPtr tb = new Belief();  
  tb->ags = new AttributedAgentStatus();
  
  ComplexFormulaPtr cf = new ComplexFormula();
  
  UnionRefPropertyPtr un =  new UnionRefProperty();
  un->unionRef = "whatever";
  
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
  un->unionRef = "whatever";
  
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
		log("An add object instruction");
		VisualObjectData &data = VisualObjectMap[proxyToAdd.front()];

		if(data.status == STABLE)
		{
		  try
		  {
			VisualObjectPtr objPtr = getMemoryEntry<VisionData::VisualObject>(data.addr);

			WorkingMemoryPointerPtr origin = createWorkingMemoryPointer(getSubarchitectureID(), data.addr.id, "VisualObject");

			FeatureValuePtr value = createStringValue (objPtr->label.c_str(), objPtr->labelConfidence);
			FeaturePtr label = createFeatureWithUniqueFeatureValue ("label", value);

			ProxyPtr proxy = createNewProxy (origin, 1.0f);

			addFeatureToProxy (proxy, label);
			
			addFeatureListToProxy(proxy, objPtr->labels, objPtr->distribution);

			addProxyToWM(proxy);

			data.proxyId = proxy->entityID;

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
		log("An update object instruction");
		VisualObjectData &data = VisualObjectMap[proxyToAdd.front()];
		
		if(data.status == STABLE)
		{
		  try
		  {
			VisualObjectPtr objPtr = getMemoryEntry<VisionData::VisualObject>(data.addr);

			WorkingMemoryPointerPtr origin = createWorkingMemoryPointer(getSubarchitectureID(), data.addr.id, "VisualObject");

			FeatureValuePtr value = createStringValue (objPtr->label.c_str(), objPtr->labelConfidence);
			FeaturePtr label = createFeatureWithUniqueFeatureValue ("label", value);

			ProxyPtr proxy = createNewProxy (origin, 1.0f);

			addFeatureToProxy (proxy, label);
			addFeatureListToProxy(proxy, objPtr->labels, objPtr->distribution);
			
			proxy->entityID = data.proxyId;
			
			overwriteProxyInWM(proxy);
			
			log("A visual proxy ID %s was updated following the visual object ID %s",
				proxy->entityID.c_str(), data.addr.id.c_str());

		  }
		  catch (DoesNotExistOnWMException e)
		  {
			log("VisualObject ID: %s was removed before it could be processed", data.addr.id.c_str());
		  }
		}
		
		proxyToUpdate.pop();
	  }
	  else if(!proxyToDelete.empty())
	  {
		log("A delete proto-object instruction");
		/* VisualObjectData &obj = VisualObjectMap[proxyToDelete.front()];

		  if(obj.status == DELETED)
		  {
			try
			{
			  deleteFromWorkingMemory(obj.objId);

			  VisualObjectMap.erase(proxyToDelete.front());

			  log("A proto-object deleted ID: %s TIME: %u",
			  obj.objId, obj.stableTime.s, obj.stableTime.us);
			}
			catch (DoesNotExistOnWMException e)
			{
			  log("WARNING: Proto-object ID %s already removed", obj.objId);
			}
		  }

		  proxyToDelete.pop(); */
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

void VisualMediator::newVisualObject(const cdl::WorkingMemoryChange & _wmc)
{
  VisualObjectPtr obj =
	getMemoryEntry<VisionData::VisualObject>(_wmc.address);

  VisualObjectData data;

  data.addr = _wmc.address;
  data.addedTime = obj->time;
  data.status = STABLE;

  VisualObjectMap.insert(make_pair(data.addr.id, data));
  proxyToAdd.push(data.addr.id);
  debug("A new VisualObject ID %s ", data.addr.id.c_str());  

  queuesNotEmpty->post();
}

void VisualMediator::updatedVisualObject(const cdl::WorkingMemoryChange & _wmc)
{
  VisionData::VisualObjectPtr obj =
	getMemoryEntry<VisionData::VisualObject>(_wmc.address);

  VisualObjectData &data = VisualObjectMap[_wmc.address.id];

  CASTTime time=getCASTTime();

  data.status= STABLE;
  data.lastUpdateTime = time;
  proxyToUpdate.push(data.addr.id);
  debug("A VisualObject ID %s ",data.addr.id.c_str());
  
  queuesNotEmpty->post();
}

void VisualMediator::deletedVisualObject(const cdl::WorkingMemoryChange & _wmc)
{

  VisualObjectData &obj = VisualObjectMap[_wmc.address.id];

  CASTTime time=getCASTTime();
  obj.status= DELETED;
  obj.deleteTime = time;
  proxyToDelete.push(obj.addr.id);

  log("Deleted Visual object ID %s ",
	  obj.addr.id.c_str());

  queuesNotEmpty->post();		 
}


void VisualMediator::updatedBelief(const cdl::WorkingMemoryChange & _wmc)
{
  log("A belief was updated. ID: %s SA: %s", _wmc.address.id.c_str(), _wmc.address.subarchitecture.c_str());
  
  BeliefPtr obj =
	getMemoryEntry<Belief>(_wmc.address);
	
  debug("Got a belief from WM. ID: %s", _wmc.address.id.c_str());
  
  string unionID;
  string visObjID;
  
  if(! AttrAgent(obj->ags))
  {
	log("The agent status is not an attributed one - will not learn what I already know");
//	return;
  }
  
  
  if(unionRef(obj->phi, unionID))
  {
	debug("Found reference to union ID %s SA %s in belief", unionID.c_str(), m_bindingSA.c_str());

	UnionPtr uni;

	if(m_currentUnions.find(unionID) != m_currentUnions.end())
	  uni = m_currentUnions[unionID];
	else
	{
	  if(m_haveAddr)
	  {
		addrFetchThenExtract(m_currentUnionsAddr);

		if(m_currentUnions.find(unionID) != m_currentUnions.end())
		  uni = m_currentUnions[unionID];
		else
		{
		  log("Union ID %s not in the current configuration. There are %i unions in the configuration.", unionID.c_str(), m_currentUnions.size());
		  return;
		}    
	  }
	  else
	  {
		log("Union ID %s not in the current configuration. There are %i unions in the configuration.", unionID.c_str(), m_currentUnions.size());
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
	log("No reference to a binding union");
	return;
  }
	
  vector<Shape> shapes;
  vector<Color> colors;
  vector<float> colorDist, shapeDist;
	
  if(findAsserted(obj->phi, colors, shapes, colorDist, shapeDist))
  {
	debug("Found asserted colors or shapes");
	
	compileAndSendLearnTask(visObjID, colors, shapes, colorDist, shapeDist);
	
	log("Added a learning task for visual object ID %s", visObjID.c_str());
  }
  else
  {
	debug("No asserted colors or shapes - no learning");
	return;
  }
}


bool VisualMediator::unionRef(FormulaPtr fp, string &unionID)
{
  Formula *f = &(*fp);

  if(typeid(*f) == typeid(UnionRefProperty))
  {
	UnionRefPropertyPtr unif = dynamic_cast<UnionRefProperty*>(f);
	unionID =  unif->unionRef;	

	return true;
  }
  else if(typeid(*f) == typeid(ComplexFormula))
  {
	ComplexFormulaPtr unif = dynamic_cast<ComplexFormula*>(f);
	vector<SuperFormulaPtr>::iterator it; 

	for(it = unif->formulae.begin(); it != unif->formulae.end(); it++)
	{
	  SuperFormulaPtr sf = *it;
  
	  if(unionRef(sf, unionID))
		return true;
	}
  
	unionID = "";
	return false;
	
  }
  else
  {
	unionID = "";
	return false;
  }

}


bool VisualMediator::findAsserted(FormulaPtr fp, vector<Color> &colors, vector<Shape> &shapes,
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
  else if(typeid(*f) == typeid(ComplexFormula))
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


bool VisualMediator::AttrAgent(AgentStatusPtr ags)
{
  AgentStatus *as = &(*ags);
  
  debug("The agent status class is %s", typeid(*as).name());

  if(typeid(*as) == typeid(AttributedAgentStatus))
  {
	return true;
  }
  else
  {
	return false;
  }

}

void VisualMediator::addFeatureListToProxy(ProxyPtr proxy, IntSeq labels, DoubleSeq distribution)
{
  vector<int>::iterator labi;
  vector<double>::iterator disti = distribution.begin();
  
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
			value = createStringValue ("black", *disti);
			break;

		case 6:
			value = createStringValue ("white", *disti);	
			break;
			
		case 7:
			value = createStringValue ("orange", *disti);
			break;

		case 8:
			value = createStringValue ("pink", *disti);	
			break;
			
		case 11:
			value = createStringValue ("compact", *disti);
			break;

		case 12:
			value = createStringValue ("elongated", *disti);	
			break;

		default:
			value = createStringValue ("unknown", *disti);
			break;
	}
	
	FeaturePtr label;
	if(*labi < 10)
	{
//	  value = createStringValue (colorStrEnums[*labi], *disti);
	  label = createFeatureWithUniqueFeatureValue ("Color", value);
	  
	  addFeatureToProxy (proxy, label);
	}
	else if(*labi < 13)
	{
//	  value = createStringValue (shapeStrEnums[*labi], *disti);
	  label = createFeatureWithUniqueFeatureValue ("Shape", value);
	  
	  addFeatureToProxy (proxy, label);
	} 	
	
	disti++;
  }
	  
}


void VisualMediator::compileAndSendLearnTask(const string visualObjID,
				vector<Color> &colors, vector<Shape> &shapes,
				vector<float> &colorDist, vector<float> &shapeDist)
{
  VisualObjectPtr objPtr = getMemoryEntry<VisualObject>(visualObjID);
  
  VisualLearnerLearningTaskPtr ltask = new VisualLearnerLearningTask();
  
  ltask->visualObjectId = visualObjID;
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
/*			
		case black:
			ltask->labels.push_back(5);	
			break;

		case white:
			ltask->labels.push_back(6);		
			break;
			
		case orange:
			ltask->labels.push_back(7);	
			break;

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
		case spherical:
			debug("Adding spherical shape to learning task");
			ltask->labels.push_back(11);	
			break;

		case cylindrical:
			debug("Adding cylindrical shape to learning task");
			ltask->labels.push_back(12);		
			break;
			
		default:
			ltask->labels.push_back(10);
			break;
	}

  }
  debug("Leaning task compiled");
  
  addToWorkingMemory(newDataID(), subarchitectureID(), ltask);
  
  debug("Learning task sent");
}


}
/* vim:set fileencoding=utf-8 sw=2 ts=4 noet:vim*/

