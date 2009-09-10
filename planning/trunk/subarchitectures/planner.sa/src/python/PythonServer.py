import os, sys, traceback, Ice
from os.path import abspath, dirname, join, isdir

import autogen.Planner as Planner
import binder.autogen.core
import cast.core

this_path = abspath(dirname(__file__))

def extend_pythonpath():
  """add standalone planner to PYTHONPATH"""
  standalone_path = join(this_path, "standalone")
  sys.path.insert(0, standalone_path)
extend_pythonpath()  

from standalone.task import Task
from standalone.planner import Planner as StandalonePlanner


TEST_DOMAIN_FN = join(dirname(__file__), "../../test_data/minidora.domain.mapl")

class PythonServer(Planner.PythonServer, cast.core.CASTComponent):
  def __init__(self):
    cast.core.CASTComponent.__init__(self)
    self.client = None
    self.planner = StandalonePlanner()
    print "created new PythonServer"

  def configureComponent(self, config):
    print "registering Python planner server"
    self.registerIceServer(Planner.PythonServer,self)
    print "and trying to get it back again immediately"
    server = self.getIceServer("PlannerPythonServer", Planner.PythonServer, Planner.PythonServerPrx)
    print "It worked. We got a server:", server
    self.client_name = config.get("--wm", "Planner")

  def getClient(self):
    if not self.client:
      self.client = self.getIceServer(self.client_name, Planner.CppServer, Planner.CppServerPrx)
      print "Connected to CppServer %s" % self.client_name
    return self.client

  def startComponent(self):
    pass

  def stopComponent(self,config):
    pass

  def runComponent(self):
    pass

  def registerTask(self, task_desc, current=None):
    # MB: id?
    print "Planner PythonServer: New PlanningTask received:"
    print "GOAL: " + task_desc.goal;
    #print "OBJECTS: " + task_desc.objects;
    #print "INIT: " + task_desc.state;

    import task_preprocessor
    task = task_preprocessor.generate_mapl_task(task_desc, TEST_DOMAIN_FN)

    self.planner.register_task(task)

    task.mark_changed()
    task.activate_change_dectection()
    plan = task.get_plan()
    
    make_dot = True
    if make_dot:
      dot_str = plan.to_dot()
      dot_fn = abspath(join(this_path, "plan.dot"))
      print "Generating and showing plan in .dot format next.  If this doesn't work for you, edit show_dot.sh"
      print "Dot file is stored in", dot_fn
      show_dot_script = abspath(join(this_path, "../..", "show_dot.sh"))
      open(dot_fn, "w").write(dot_str)
      os.system("%s %s" % (show_dot_script, dot_fn)) 

#     if(self.client is None):
#       print "ERROR!!"

#     task_desc.plan = str(plan)
#     self.getClient().deliverPlan(task_desc);
    # add task to some queue or start planning right away. when done call self.client.deliverPlan(string plan)
    
  def registerClient(self, Client, current=None):
    pass
