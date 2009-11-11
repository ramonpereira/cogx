from standalone import mapl_new as mapl
from standalone import state_new as state
from standalone import plans
from standalone import statistics

from standalone.task import PlanningStatusEnum, Task
from standalone.planner import Planner as StandalonePlanner

from standalone import config
log = config.logger("mapsim")

statistics_defaults = dict(
    failed_execution_attempts=0,
    physical_actions_executed=0,
    sensor_actions_executed=0,
    speech_acts_executed=0,
    )


task_id = 0
def next_id():
    global task_id
    task_id += 1
    return task_id-1

def loggingScope(f):
    def new_f(self, *args, **kwargs):
        oldscope = config.logfile_scope
        config.logfile_scope = self.name
        rval = f(self, *args, **kwargs)
        config.logfile_scope = oldscope
        return rval
        
    new_f.__name__ = f.__name__
    return new_f

class BaseAgent(object):
    def __init__(self, simulator):
        self.simulator = simulator
        self.running = False

    def run(self):
        self.running = True
        
    def execute(self, action, args):
        self.simulator.schedule(action, args)

    def done(self):
        self.running = False
        self.simulator.signal_done(self)

    def is_running(self):
        return self.running


class Agent(BaseAgent):
    def __init__(self, name, mapltask, planner, simulator):
        BaseAgent.__init__(self, simulator)
        
        self.name = name
        self.mapltask = mapltask
        self.planner = planner
        self.statistics = statistics.Statistics(statistics_defaults)
        
        self.task = Task(next_id(), mapltask)
        
        self.planner.register_task(self.task)

    def getState(self):
        return self.task.get_state()

    @loggingScope
    def run(self):
        BaseAgent.run(self)
        self.task.replan()
        self.execute_plan(self.task)
        
    @loggingScope
    def execute_plan(self, task):
        if task.planning_status == PlanningStatusEnum.PLANNING_FAILURE:
            return
            
        plan = task.get_plan()

        ordered_plan = plan.topological_sort()
        outplan = []
        first_action = -1
        for i,pnode in enumerate(ordered_plan):
            if isinstance(pnode, plans.DummyNode) or not pnode.is_executable():
                continue
            if first_action == -1:
                first_action = i
      
            outplan.append(pnode)

        plan.execution_position = first_action
        if outplan:
            log.debug("trying to execute (%s %s)", outplan[0].action.name, " ".join(map(str, outplan[0].args)))
            self.execute(outplan[0].action.name, outplan[0].full_args)
        else:
            log.debug("nothing more to do.")
            self.done()

  
    @loggingScope
    def updateTask(self, new_facts, action_status=None):
        plan = self.task.get_plan()

        if plan is not None:
            executable_plan = plan.topological_sort()[plan.execution_position:-1]
            if action_status:
                executable_plan[0].status = action_status
            
        for f in new_facts:
            self.task.get_state().set(f)
            
        self.task.mark_changed()
        self.task.replan()
        self.execute_plan(self.task)

    def collect_statistics(self):
        """ return all stats collected by this agent (usually to the simulation) """
        return self.statistics.merge(self.task.statistics)
