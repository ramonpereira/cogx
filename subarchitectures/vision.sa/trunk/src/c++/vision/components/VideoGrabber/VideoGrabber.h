/**
 * @author Marko Mahnič
 * @date September 2010
 *
 */

#ifndef VIDEOGRABBER_V7JAPUYG
#define VIDEOGRABBER_V7JAPUYG

#include "ticker.h"

#include <cast/architecture/ManagedComponent.hpp>
#include <Video.hpp>
#include <VisionData.hpp>
#include <VideoClient2.h>

#ifdef FEAT_VISUALIZATION
#include <CDisplayClient.hpp>
#endif

namespace cogx
{

struct CRecordingInfo
{
   bool recording;
   std::string directory;
   long directoryStatus; // 0: don't; 1: check; 2: exists; -1: error
   std::string filenamePatt;
   std::vector<std::string> deviceNames;
   long counterDigits;
   long counterStart;
   long counterEnd;
   long counter;
   IceUtil::Time tmStart;
   IceUtil::Time tmEnd;
   CRecordingInfo() {
      recording = false;
      counterStart = 0;
      counterEnd = 0;
      counter = 0;
#if 1 // XXX testing
      directory = "/tmp/path/to/Model";
      filenamePatt = "image-%c-%d.png";
      deviceNames.push_back("L");
      deviceNames.push_back("R");
      counterDigits = 3;
      tmStart = IceUtil::Time::seconds(0);
      tmEnd = tmStart;
#endif
   }
};

class CVideoGrabber: public cast::ManagedComponent
{
private:
   /**
    * Which camera to get images from
    */
   std::vector<int> m_camIds;

   /**
    * component ID of the video server to connect to
    */
   std::string m_videoServerName;
   /**
    * our ICE proxy to the video server
    */
   //Video::VideoInterfacePrx m_pVideoServer;

   /**
    * wether we are currently receiving images from the server
    */
   bool m_bReceiving;

   int m_frameGrabCount;       // how many frames to save

   std::vector<Video::CVideoClient2*> m_video;
   CRecordingInfo m_RecordingInfo;
   bool m_fakeRecording;

#ifdef FEAT_VISUALIZATION
   // HACK: The image data in IplImage will point into char data of m_DisplayBuffer.
   // This way an IplImage can be prapared and sent to the display server without
   // copying the data to a temporary vector or Video::Image.
   IplImage* m_pDisplayCanvas; // Image for the DisplayServer
   std::vector<unsigned char> m_DisplayBuffer; // For transfering data to the server;
   void prepareCanvas(int width, int height);
   void releaseCanvas();
   void sendCachedImages();
   
   class CVvDisplayClient: public cogx::display::CDisplayClient
   {
      CVideoGrabber* pViewer;
   public:
      cogx::display::CFormValues m_frmSettings;
      // Access form variables
      std::string getModelName();
      std::string getDirectory();
      void setDirectory(const std::string& name);
      bool getCreateDirectory();
      std::string getDeviceNames();
      void setDeviceNames(const std::string& name);
      std::string getImageFilenamePatt();
      void setImageFilenamePatt(const std::string& pattern);
      long getCounterDigits();
      void setCounterDigits(long nDigits);
      long getCounterValue();
      void setCounterValue(long nValue);

   public:
      CVvDisplayClient() { pViewer = NULL; }
      void setClientData(CVideoGrabber* pVideoGrabber) { pViewer = pVideoGrabber; }

      void createForms();

      // Send current form data to the DisplayServer
      // (used when the counter must be updated)
      void updateDisplay();

      void handleEvent(const Visualization::TEvent &event); /*override*/
      std::string getControlState(const std::string& ctrlId); /*override*/
      void handleForm(const std::string& id, const std::string& partId,
            const std::map<std::string, std::string>& fields);
      bool getFormData(const std::string& id, const std::string& partId,
            std::map<std::string, std::string>& fields);

   };
   CVvDisplayClient m_display;
#endif

   IceUtil::Handle<IceUtil::Timer> m_pTimer;
   class CSaveQueThread: public IceUtil::Thread, public CTickSyncedTask
   {
      CVideoGrabber *m_pGrabber;

   public:
      struct CItem 
      {
         CRecordingInfo frameInfo;
         std::vector<Video::CCachedImagePtr> images;
      };

   private:
      std::vector<CItem> m_items;
      IceUtil::Monitor<IceUtil::Mutex> m_itemsLock;

   public:
      CSaveQueThread(CVideoGrabber *pGrabber);

      void getItems(std::vector<CItem>& items, unsigned int maxItems = 0);
      virtual void grab();
      virtual void run();
   };
   IceUtil::ThreadPtr m_pQueue;
   IceUtil::Handle<CTickerTask> m_pQueueTick;

   class CDrawingThread: public IceUtil::Thread, public CTickSyncedTask
   {
      CVideoGrabber *m_pGrabber;

   public:
      CDrawingThread(CVideoGrabber *pGrabber);
      virtual void run();
   };
   IceUtil::ThreadPtr m_pDrawer;
   IceUtil::Handle<CTickerTask> m_pDrawTick;

protected:
   virtual void configure(const std::map<std::string,std::string> & _config);
   virtual void start();
   virtual void destroy();
   virtual void runComponent();

public:
   CVideoGrabber()
   {
      m_frameGrabCount = 0;
#ifdef FEAT_VISUALIZATION
      m_pDisplayCanvas = NULL;
      m_display.setClientData(this);
#endif
   }
   virtual ~CVideoGrabber()
   {
      m_fakeRecording = false;
#ifdef FEAT_VISUALIZATION
      releaseCanvas();
#endif
   }
   /**
    * The callback function for images pushed by the image server.
    * To be overwritten by derived classes.
    */
   virtual void receiveImages(const std::string& serverName, const std::vector<Video::Image>& images);

   std::vector<std::string> getDeviceNames();
   void saveImages(const std::vector<Video::Image>& images);
   void saveQueuedImages(const std::vector<Video::CCachedImagePtr>& images, CRecordingInfo& frameInfo);
   void getClients(std::vector<Video::CVideoClient2*>& clients)
   {
      clients = m_video;
   }
   void fillRecordingInfo(CRecordingInfo &info);
   void startGrabbing(const std::string& command);
   void stopGrabbing();
   void checkStopGrabbing();
   bool isGrabbing()
   {
      return m_RecordingInfo.recording;
   }
};

} // namespace
#endif
/* vim: set sw=3 ts=8 et: */

