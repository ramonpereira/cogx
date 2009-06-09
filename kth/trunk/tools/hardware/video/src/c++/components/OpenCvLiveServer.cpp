/**
 * @author Michael Zillich
 * @date February 2009
 */

#include <cmath>
#include <cast/core/CASTUtils.hpp>
#include <VideoUtils.h>
#include "OpenCvLiveServer.h"

/**
 * The function called to create a new instance of our component.
 */
extern "C"
{
  cast::CASTComponentPtr newComponent()
  {
    return new cast::OpenCvLiveServer();
  }
}

namespace cast
{

using namespace std;

OpenCvLiveServer::Timer::Timer()
: count(0),
  rate(0.),
  lastRate(0.),
  changeThresh(0.),
  sigChange(false)
{
}

void OpenCvLiveServer::Timer::increment()
{
  // increment
  count++;

  // if this is the first time
  if(count == 1)
  {
    // start timer
    gettimeofday(&startTime, 0);
  }
  else
  {
    // current time
    timeval now;
    gettimeofday(&now, 0);
    timeval start(startTime);

    // from http://www.delorie.com/gnu/docs/glibc/libc_428.html
    // Perform the carry for the later subtraction by updating y.
    if(now.tv_usec < start.tv_usec)
    {
      int nsec = (start.tv_usec - now.tv_usec) / 1000000 + 1;
      start.tv_usec -= 1000000 * nsec;
      start.tv_sec += nsec;
    }
    if(now.tv_usec - start.tv_usec > 1000000)
    {
      int nsec = (now.tv_usec - start.tv_usec) / 1000000;
      start.tv_usec += 1000000 * nsec;
      start.tv_sec -= nsec;
    }

    // tv_usec is certainly positive.
    double diffSeconds = (double)(now.tv_sec - start.tv_sec);
    double diffMicros = (double)(now.tv_usec - start.tv_usec);

    double totalDiffSeconds = diffSeconds  + (diffMicros / 1000000.);

    // get new rate
    rate = (double)count/totalDiffSeconds;

    // diff the old and new rates
    double diffRate = abs(lastRate - rate);

    // compare diff to a percentage of the old rate
    if(diffRate > changeThresh)
    {
      sigChange = true;
      lastRate = rate;
      // get 3% of the old rate... HACK WARNING CONSTANT
      changeThresh = lastRate * 0.03;
    }
    else
    {
      sigChange = false;
    }
  }
}

OpenCvLiveServer::OpenCvLiveServer()
{
  bayerCvt = CV_COLORCVT_MAX;
  framerateMillis = 0;
  width = height = 0;
}

OpenCvLiveServer::~OpenCvLiveServer()
{
  for(size_t i = 0; i < captures.size(); i++)
    cvReleaseCapture(&captures[i]);
}

/**
 * @param ids  list of camera IDs
 * @param dev_nums  list of device numbers (typically 0, 1) corresponding to
 *                  camera IDs
 * @param bayer  Some cameras (e.g. Point Grey Flea) return the raw Bayer
 *               pattern rather than YUV or RGB. For these we have to perform
 *               Bayer ro RGB conversion. This parameter indicates the
 *               order of R,G and B pixels in the Bayer pattern: one of
 *               "BGGR" "GBBR" "RGGB" "GRRB" or "" (for no conversion).
 */
void OpenCvLiveServer::init(const vector<int> &dev_nums, const string &bayer)
  throw(runtime_error)
{
  if(dev_nums.size() == 0)
    throw runtime_error(exceptionMessage(__HERE__,
          "must specify at least one camera"));
  if(dev_nums.size() != camIds.size())
    throw runtime_error(exceptionMessage(__HERE__,
          "number of devices %d does not match number of camera IDs %d",
          (int)dev_nums.size(), (int)camIds.size()));

  captures.resize(dev_nums.size());
  grabTimes.resize(dev_nums.size());
  retrievedImages.resize(dev_nums.size());
  for(size_t i = 0; i < dev_nums.size(); i++)
    retrievedImages[i] = 0;
  for(size_t i = 0; i < dev_nums.size(); i++)
  {
    captures[i] = cvCreateCameraCapture(dev_nums[i]);
    if(captures[i] == 0)
      throw runtime_error(exceptionMessage(__HERE__,
        "failed to create capture for video device %d", dev_nums[i]));
    if(bayer.empty())
    {
      bayerCvt = CV_COLORCVT_MAX;
    }
    else
    {
      if(bayer == "BGGR")
        bayerCvt = CV_BayerBG2RGB;
      else if(bayer == "GBBR")
        bayerCvt = CV_BayerGB2RGB;
      else if(bayer == "RGGB")
        bayerCvt = CV_BayerRG2RGB;
      else if(bayer == "GRRB")
        bayerCvt = CV_BayerGR2RGB;
      else
        throw runtime_error(exceptionMessage(__HERE__,
            "invalid bayer order '%s', must be one of 'BGGR' 'GBBR' 'RGGB' 'GRRB'",
            bayer.c_str()));
      // the default in opencv is CV_CAP_PROP_CONVERT_RGB=1 which causes
      // cameras with bayer encoding to be converted from mono to rgb
      // without using the bayer functions. CV_CAP_PROP_CONVERT_RGB=0
      // keeps the original format.
      cvSetCaptureProperty(captures[i], CV_CAP_PROP_CONVERT_RGB, 0.0);
    }
  }
  // HACK
  cvSetCaptureProperty(captures[0], CV_CAP_PROP_FRAME_WIDTH, 640);
  cvSetCaptureProperty(captures[0], CV_CAP_PROP_FRAME_HEIGHT, 480);
  // HACK END
  width = (int)cvGetCaptureProperty(captures[0], CV_CAP_PROP_FRAME_WIDTH);
  height = (int)cvGetCaptureProperty(captures[0], CV_CAP_PROP_FRAME_HEIGHT);
  // frames per second
  double fps = cvGetCaptureProperty(captures[0], CV_CAP_PROP_FPS);
  if(fps > 0.)
    // milliseconds per frame
    framerateMillis = (int)(1000./fps);
  else
    // just some huge value (better than 0. as that might result in divides by
    // zero somewhere)
    framerateMillis = 1000000;
  // to make sure we have images in the capture's buffer
  grabFramesInternal();
}

void OpenCvLiveServer::configure(const map<string,string> & _config)
  throw(runtime_error)
{
  vector<int> dev_nums;
  string bayer;
  map<string,string>::const_iterator it;

  // first let the base class configure itself
  VideoServer::configure(_config);

  if((it = _config.find("--devnums")) != _config.end())
  {
    istringstream str(it->second);
    int dev;
    while(str >> dev)
      dev_nums.push_back(dev);
  }
  else
  {
    // assume 0 as default device
    dev_nums.push_back(0);
  }

  // if cameras return raw Bayer patterns
  if((it = _config.find("--bayer")) != _config.end())
  {
    bayer = it->second;
  }

  // do some initialisation based on configured items
  init(dev_nums, bayer);
}

void OpenCvLiveServer::grabFramesInternal()
{
  for(size_t i = 0; i < captures.size(); i++)
  {
    // grab image into internal storage of the capture device
    cvGrabFrame(captures[i]);
    // and invalidate the corresponding retrieved image
    //retrievedImages[i] = 0;
  }
  for(size_t i = 0; i < captures.size(); i++)
  {
    retrievedImages[i] = cvRetrieveFrame(captures[i]);
  }

  // needed to prevent retrieving while grabbing
  lockComponent();

  cdl::CASTTime time = getCASTTime();
  for(size_t i = 0; i < grabTimes.size(); i++)
    grabTimes[i] = time;
  timer.increment();
  // frames per second
  if(timer.getRate() > 0.)
    // milliseconds per frame
    framerateMillis = (int)(1000./timer.getRate());

  unlockComponent();
}

void OpenCvLiveServer::grabFrames()
{
  grabFramesInternal();
}

/**
 */
void OpenCvLiveServer::retrieveFrames(std::vector<Video::Image> &frames)
  throw(std::runtime_error)
{
  // needed to prevent retrieving while grabbing
  lockComponent();

  frames.resize(retrievedImages.size());
  for(size_t i = 0; i < captures.size(); i++)
  {
    // note: calling cvRetrieveFrame() only when really needed reduces system
    // load (on Core 2 Duo 2.4GHz) from 18% to virtually 0%
    //if(retrievedImages[i] == 0)
    //  retrievedImages[i] = cvRetrieveFrame(captures[i]);
    copyImage(retrievedImages[i], frames[i]);
    frames[i].time = grabTimes[i];
    frames[i].camId = camIds[i];
    frames[i].camPars = camPars[i];
  }

  unlockComponent();
}

void OpenCvLiveServer::retrieveFrame(int camId, Video::Image &frame)
  throw(std::runtime_error)
{
  // needed to prevent retrieving while grabbing
  lockComponent();

  bool haveCam = false;
  for(size_t i = 0; i < captures.size(); i++)
  {
    if(camId == camIds[i])
    {
      // note: calling cvRetrieveFrame() only when really needed reduces system
      // load (on Core 2 Duo 2.4GHz) from 18% to virtually 0%
      //if(retrievedImages[i] == 0)
      //  retrievedImages[i] = cvRetrieveFrame(captures[i]);
      copyImage(retrievedImages[i], frame);
      frame.time = grabTimes[i];
      frame.camId = camIds[i];
      frame.camPars = camPars[i];
      haveCam = true;
    }
  }
  if(!haveCam)
    throw runtime_error(exceptionMessage(__HERE__, "video has no camera %d", camId));

  unlockComponent();
}

int OpenCvLiveServer::getNumCameras()
{
  return captures.size();
}

void OpenCvLiveServer::getImageSize(int &width, int &height)
{
  width = this->width;
  height = this->height;
}

int OpenCvLiveServer::getFramerateMilliSeconds()
{
  return framerateMillis;
}

/**
 * note: If img is of appropriate size already, no memory allocation takes
 * place.
 */
void OpenCvLiveServer::copyImage(const IplImage *iplImg, Video::Image &img)
  throw(runtime_error)
{
  int channels = 3;  // Image frame always has RGR24
  IplImage *tmp = 0;

  assert(iplImg != 0);
  if(!haveBayer())
  {
    if(iplImg->nChannels != channels)
      throw runtime_error(exceptionMessage(__HERE__,
        "can only handle colour images - the video seems to be grey scale"));
  }
  else
  {
    // HACK: how do we know the correct colour code?
    tmp = cvCreateImage(cvSize(iplImg->width, iplImg->height), IPL_DEPTH_8U, 3);
    if(tmp == 0)
      throw runtime_error(exceptionMessage(__HERE__,
            "failed to allocate image buffer"));
    cvCvtColor(iplImg, tmp, bayerCvt);
    iplImg = tmp;
  }

  img.width = iplImg->width;
  img.height = iplImg->height;
  img.data.resize(iplImg->width*iplImg->height*iplImg->nChannels);
  // note: this neat triple loop might be somewhat slower than a memcpy, but
  // makes sure images are copied correctly irrespective of memory layout and
  // line padding.
  if(iplImg->depth == (int)IPL_DEPTH_8U || iplImg->depth == (int)IPL_DEPTH_8S)
  {
    int x, y;  // c;
    for(y = 0; y < iplImg->height; y++)
      for(x = 0; x < iplImg->width; x++)
      {
        //for(c = 0; c < channels; c++)
        //  img.data[channels*(y*img.width + x) + c] =
        //    iplImg->imageData[y*iplImg->widthStep + channels*x + c];
        img.data[channels*(y*img.width + x) + 0] =
           iplImg->imageData[y*iplImg->widthStep + channels*x + 2];
        img.data[channels*(y*img.width + x) + 1] =
           iplImg->imageData[y*iplImg->widthStep + channels*x + 1];
        img.data[channels*(y*img.width + x) + 2] =
           iplImg->imageData[y*iplImg->widthStep + channels*x + 0];
      }
  }
  else
    throw runtime_error(exceptionMessage(__HERE__,
      "can only handle 8 bit colour values"));

  if(haveBayer())
    cvReleaseImage(&tmp);
}

}

