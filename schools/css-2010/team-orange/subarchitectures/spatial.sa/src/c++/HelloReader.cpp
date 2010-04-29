#include "HelloReader.hpp"

extern "C" {
  cast::CASTComponentPtr
  newComponent() {
    return new HelloReader();
  }


}
void HelloReader::start() {
addChangeFilter(cast::createLocalTypeFilter<SpatialData::Announcement>(cast::cdl::ADD),
                                                                      new cast::MemberFunctionChangeReceiver<HelloReader>(this,
                                                                                                                          &HelloReader::makeAnnouncement));

}

void HelloReader::makeAnnouncement(const cast::cdl::WorkingMemoryChange & _wmc) {
  SpatialData::AnnouncementPtr ann = getMemoryEntry<SpatialData::Announcement>(_wmc.address);


MarySpeech(ann->message) ;

if (ann->songId == "chaka")
	system("play ./wave/chakakhan.wav");
else if (ann->songId == "james")
	system("play ./wave/james.wav");
else if (ann->songId == "jesus")
	system("play ./wave/jones.wav");
else if (ann->songId == "heart")
	system("play ./wave/heartbreaker.wav");
else if (ann->songId == "all")
	system("play ./wave/all.wav");

}






void HelloReader::MarySpeech(std::string inputText) {
  int server_port = 59125;
  string server_host = "localhost";
  //string inputText = "Welcome to the world of speech synthesis!";
  string maryInFormat = "TEXT";
  string maryOutFormat = "AUDIO";
  //string maryOutFormat = "REALISED_DURATIONS";
  string locale = "en-US";
  string audioType = "WAV_FILE";
  string voice = "us1";
  string effects;
//  effects += "Volume(amount:5.0;)+";
//  effects += "TractScaler(amount:1.5;)+";
//  effects += "F0Scale(f0Scale:2.0;)+";
//  effects += "F0Add(f0Add:50.0;)+";
//  effects += "Rate(durScale:1.5;)+";
//  effects += "Robot(amount:100.0;)+";
//  effects += "Whisper(amount:100.0;)+";
//  effects += "Stadium(amount:100.0)+";
//  effects += "Chorus(delay1:466;amp1:0.54;delay2:600;amp2:-0.10;delay3:250;amp3:0.30)+";
//  effects += "FIRFilter(type:3;fc1:500.0;fc2:2000.0)+";
//  effects += "JetPilot";
  string result;

  MaryClient maryClient;
  maryClient.maryQuery( server_port, server_host, result, inputText, maryInFormat, maryOutFormat, locale, audioType, voice, effects);

  if (maryOutFormat == "AUDIO") {
    // write result into a file
    const char *filename = "./wave/output.wav";
    ofstream file( filename );
    file << result;
file.close();
    // play output
    system("play ./wave/output.wav");
  } else {
    cout << "RESULT: " << endl << result << endl;
  }

 
}


