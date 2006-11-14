import COPASI
import unittests
from types import *


class Test_CTrajectoryProblem(unittests.TestCase):
  def setUp(self):
    self.problem=COPASI.CTrajectoryProblem()

  def test_getStepNumber(self):
    v=self.problem.getStepNumber()
    self._assert(type(v)==IntType)

  def test_setStepNumber(self):
    t=42
    self.problem.setStepNumber(t)
    self._assert(math.abs((self.problem.getStepNumber()-t)/t))

  def test_getStepSize(self):
    v=self.problem.getStepSize()
    self._assert(type(v)==FloatType)


  def test_setStepSize(self):
    t=13.7
    self.problem.setStepSize(t)
    self._assert(math.abs((self.problem.getStepSize()-t)/t))

  def test_getDuration(self):
    v=self.problem.getDuration()
    self._assert(type(v)==FloatType)

  def test_setDuration(self):
    t=13.7
    self.problem.setDuration(t)
    self._assert(math.abs((self.problem.getDuration()-t)/t))

  def test_getOutputStartTime(self):
    v=self.problem.getOutputStartTime()
    self._assert(type(v)==FloatType)

  def test_setOutputStartTime(self):
    t=13.7
    self.problem.setOutputStartTime(t)
    self._assert(math.abs((self.problem.getOutputStartTime()-t)/t))

  def test_setTimeSeriesRequested(self):
    v=false
    self.problem.setTimeSeriesRequested(v)
    self._assert(self.problem.getTimeSeriesRequested()==v)
    v=true
    self.problem.setTimeSeriesRequested(v)
    self._assert(self.problem.getTimeSeriesRequested()==v)

  def test_timeSeriesRequested(self):
    v=self.problem.getTimeSeriesRequested()
    self._assert(type(v)==BooleanType)



