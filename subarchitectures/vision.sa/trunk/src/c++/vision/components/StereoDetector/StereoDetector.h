/**
 * @file StereoDetector.h
 * @author Andreas Richtsfeld
 * @date October 2009
 * @version 0.1
 * @brief Detecting objects with stereo rig for cogx project.
 */

#ifndef STEREO_DETECTOR_H
#define STEREO_DETECTOR_H

#include <cast/architecture/ManagedComponent.hpp>
#include <VideoClient.h>
#include <VisionData.hpp>
#include <VideoUtils.h>
#include <vector>

#include "StereoCore.hh"
#include "Pose3.h"
#include "StereoBase.h"

namespace cast
{

/**
	* @brief Class Stereo Detector.
	*/
class StereoDetector : public VideoClient, public ManagedComponent
{
private:

	Z::StereoCore *score;										///< Stereo core
	std::vector<int> camIds;								///< Which cameras to get images from
	std::string videoServerName;						///< Component ID of the video server to connect to
	Video::VideoInterfacePrx videoServer;		///< ICE proxy to the video server
	Video::Image image_l, image_r;					///< Left and right stereo image from video server. Original images.
	IplImage *iplImage_l, *iplImage_r;			///< Converted left and right stereo images (openCV ipl-images)
//	IplImage *iplImage_l_s, *iplImage_r_s;	///< Copys of iplImage_x to draw overlays and display at openCV windows.
	bool cmd_detect;												///< detection command
	bool cmd_single;												///< single detection commmand
	bool showImages;												///< show openCV images
	bool debug;															///< debug the stereo detector
	std::vector<std::string> objectIDs;			///< IDs of the currently stored visual objects
	int overlays;														///< Number of overlay
																					///<     1 = all
																					///<     2 = flaps
																					///<     3 = rectangles
																					///<     4 = closures
																					///<     5 = ellipses

  /**
   * @brief Show both stereo images in the openCV windows.
   * @param convertNewIpl Convert image again into iplImage to delete overlays.
   */
	void ShowImages(bool convertNewIpl);

  /**
   * @brief Receive a changed detection command, written to the working memory
   * @param wmc Working memory address. 
   */
  void receiveDetectionCommand(const cdl::WorkingMemoryChange & _wmc);

  /**
   * @brief Call debug mode.
   */
	void Debug();

  /**
   * @brief Read the SOIs from the working memory and display it.
   */
	void ReadSOIs();

	/**
	 * @brief Delete all visual objects from the working memory. 
	 * The IDs are stored in the vector "objectIDs".
	 */
	void DeleteVisualObjects();


protected:
  /**
   * @brief Called by the framework to configure our component
   * @param config Config TODO
   */
  virtual void configure(const std::map<std::string,std::string> & _config);
  /**
   * @brief Called by the framework after configuration, before run loop
   */
  virtual void start();
  /**
   * @brief Called by the framework to start compnent run loop
   */
  virtual void runComponent();

  /**
   * @brief Called to start processing of one image
   * @param image_l Left stereo image from video server.
   * @param image_r Right stereo image from video server.
   */
	virtual void processImage(/*const Video::Image &image_l, const Video::Image &image_r*/);


public:
  StereoDetector() {}
  virtual ~StereoDetector();

  /**
   * @brief The callback function for images pushed by the image server.
   * To be overwritten by derived classes.
   * @param images Vector of images from video server.
   */
  virtual void receiveImages(const std::vector<Video::Image>& images);
};

}

#endif



