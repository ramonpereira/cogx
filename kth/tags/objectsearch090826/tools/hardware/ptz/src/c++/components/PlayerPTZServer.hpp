#include <PTZServer.hpp>

#include <libplayerc++/playerc++.h>
#include <boost/shared_ptr.hpp>

namespace ptz {

  /**
   * An implementation of the ptz server using the player ptz device.
   *
  */
  class PlayerPTZServer : public PTZServer {
  private:
    std::string m_playerHost;
    int m_playerPort;
    int m_playerPTZDeviceID;

    boost::shared_ptr<PlayerCc::PlayerClient> m_playerClient;
    boost::shared_ptr<PlayerCc::PtzProxy> m_ptzProxy;

  protected:

    /**
     * Get the current ptz pose.
     */  
    virtual PTZReading getPose() const;
    
    /**
     * Set the desired ptz pose.
     */
    virtual void 
    setPose(const PTZPose & _pose);

    virtual void 
    configure(const std::map<std::string,std::string> & _config);

    virtual void start();


  public:
    PlayerPTZServer();
    
  };

}
