/**
 * @author Michael Zillich
 * @date April 2011
*/

#include <iostream>
#include <cast/architecture/ChangeFilterFactory.hpp>
#include "PlayerArmBridge.h"

/**
 * The function called to create a new instance of our component.
 */
extern "C"
{
  cast::CASTComponentPtr newComponent()
  {
    return new cogx::PlayerArmBridge();
  }
}

namespace cogx
{

using namespace std;
using namespace cast;
using namespace manipulation::slice;

PlayerArmBridge::PlayerArmBridge()
{
#ifdef FEAT_VISUALIZATION
  display.setClientData(this);
#endif
  robot = 0;
  arm = 0;
  playerHost = PlayerCc::PLAYER_HOSTNAME;
  playerPort = PlayerCc::PLAYER_PORTNUM;
}

void PlayerArmBridge::configure(const map<string, string> &_config)
{
  map<string,string>::const_iterator it;

  if((it = _config.find("--playerhost")) != _config.end())
  {
    playerHost = it->second;
  }
  if((it = _config.find("--playerport")) != _config.end())
  {
    istringstream str(it->second);
    str >> playerPort;
  }
}

void PlayerArmBridge::start()
{
  robot = new PlayerCc::PlayerClient(playerHost, playerPort);
  arm = new PlayerCc::ActArrayProxy(robot, 0);
  ostringstream s;
  s << "connected to player robot '" << robot << "'\n";
  log(s.str());

  addChangeFilter(createLocalTypeFilter<PlayerBridgeSendTrajectoryCommand>(cdl::ADD),
    new MemberFunctionChangeReceiver<PlayerArmBridge>(this, &PlayerArmBridge::receiveSendTrajectory));
  addChangeFilter(createLocalTypeFilter<PlayerBridgeOpenGripperCommand>(cdl::ADD),
    new MemberFunctionChangeReceiver<PlayerArmBridge>(this, &PlayerArmBridge::receiveOpenGripper));
  addChangeFilter(createLocalTypeFilter<PlayerBridgeCloseGripperCommand>(cdl::ADD),
    new MemberFunctionChangeReceiver<PlayerArmBridge>(this, &PlayerArmBridge::receiveCloseGripper));
}

void PlayerArmBridge::destroy()
{
  delete arm;
  delete robot;
}

void PlayerArmBridge::receiveSendTrajectory(const cdl::WorkingMemoryChange &_wmc)
{
  PlayerBridgeSendTrajectoryCommandPtr cmd = getMemoryEntry<PlayerBridgeSendTrajectoryCommand>(_wmc.address);
  // let the caller know we are executing the command
  cmd->comp = ONTHEWAY;
  overwriteWorkingMemory(_wmc.address, cmd);
  // NOTE: sendTrajectory() blocks until trajectory is finished
  if(sendTrajectory(cmd->trajectory))
    cmd->comp = SUCCEEDED;
  else
    cmd->comp = FAILED;
  overwriteWorkingMemory(_wmc.address, cmd);
}

void PlayerArmBridge::receiveOpenGripper(const cdl::WorkingMemoryChange &_wmc)
{
  PlayerBridgeOpenGripperCommandPtr cmd = getMemoryEntry<PlayerBridgeOpenGripperCommand>(_wmc.address);
  // let the caller know we are executing the command
  cmd->comp = ONTHEWAY;
  overwriteWorkingMemory(_wmc.address, cmd);
  // NOTE: openGripper() blocks until fingers no longer moving
  openGripper();
  // let the caller know we are done
  cmd->comp = SUCCEEDED;
  overwriteWorkingMemory(_wmc.address, cmd);
}

void PlayerArmBridge::receiveCloseGripper(const cdl::WorkingMemoryChange &_wmc)
{
  PlayerBridgeCloseGripperCommandPtr cmd = getMemoryEntry<PlayerBridgeCloseGripperCommand>(_wmc.address);
  // let the caller know we are executing the command
  cmd->comp = ONTHEWAY;
  overwriteWorkingMemory(_wmc.address, cmd);
  // NOTE: closeGripper() blocks until fingers no longer moving
  if(closeGripper())
    cmd->graspStatus = NOTGRASPING;
  else
    cmd->graspStatus = GRASPING;
  cmd->comp = SUCCEEDED;
  overwriteWorkingMemory(_wmc.address, cmd);
}

bool PlayerArmBridge::equals(GenConfigspaceCoord &p1, GenConfigspaceCoord &p2)
{
  const double eps = 0.034907;  // 2 degrees
  for(size_t i = 0; i < (size_t)NUM_JOINTS; i++)
    if(fabs(p1.pos[i] - p2.pos[i]) > eps)
      return false;
  return true;
}

void PlayerArmBridge::waitGripperMoving()
{
  // Emprically determined values for gripper of simulated katana arm.
  // If finger joint speeds are below 0.002, fingers can be regarded still
  const double eps_vel = 0.002;  // m/s

  // Don't wait longer than that for the gripper to close.
  double timeout = 5.;
  bool moving = true;
  cdl::CASTTime time = getCASTTime();
  double t = (double)time.s + (double)time.us*1e-6;
  timeout += t;
  player_actarray_actuator_t l, r;
  while(moving && t < timeout)
  {
    robot->Read();
    l = arm->GetActuatorData(LEFT_FINGER_JOINT);
    r = arm->GetActuatorData(RIGHT_FINGER_JOINT);
    moving = fabs(l.speed) < eps_vel && fabs(r.speed) < eps_vel;
    // gripper closes slowly, so can wait quite some time
    usleep(250000);
  }
}

bool PlayerArmBridge::sendTrajectory(GenConfigspaceStateSeq &trajectory)
{
  assert(trajectory.size() >= 1);
  GenConfigspaceCoord reached;
  for(size_t i = 1; i < trajectory.size(); i++)
  {
    // NOTE: MoveToMulti() did not work, so setting joints individually.
    for(size_t j = 0; j < (size_t)NUM_JOINTS; j++)
      arm->MoveTo(j, trajectory[i].coord.pos[j]);
    long t0 = (long)(1e6*trajectory[i-1].t);
    long t1 = (long)(1e6*trajectory[i].t);
    usleep(t1 - t0);
  }
  // now read which position we actually reached
  robot->Read();
  for(size_t j = 0; j < (size_t)NUM_JOINTS; j++)
  {
    player_actarray_actuator_t a = arm->GetActuatorData(j);
    reached.pos[j] = a.position;
    reached.vel[j] = 0.;
  }
  return equals(reached, trajectory[trajectory.size() - 1].coord);
}

void PlayerArmBridge::openGripper()
{
  arm->MoveTo(LEFT_FINGER_JOINT, GRIPPER_OPEN_POS);
  arm->MoveTo(RIGHT_FINGER_JOINT, GRIPPER_OPEN_POS);
  waitGripperMoving();
}

bool PlayerArmBridge::closeGripper()
{
  // Emprically determined values for gripper of simulated katana arm.
  // NOTE: for some reason the fingers fail to close to position 0, so we have
  // to give a very large epsilon of over a cm here
  const double eps_pos = 0.012;  // m

  arm->MoveTo(LEFT_FINGER_JOINT, GRIPPER_CLOSE_POS);
  arm->MoveTo(RIGHT_FINGER_JOINT, GRIPPER_CLOSE_POS);
  waitGripperMoving();

  robot->Read();
  player_actarray_actuator_t l = arm->GetActuatorData(LEFT_FINGER_JOINT);
  player_actarray_actuator_t r = arm->GetActuatorData(RIGHT_FINGER_JOINT);

  return fabs(l.position - GRIPPER_CLOSE_POS) < eps_pos &&
         fabs(r.position - GRIPPER_CLOSE_POS) < eps_pos;
}

}
