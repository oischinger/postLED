#include "MyThreadedCanvasManipulator.h"


void MyThreadedCanvasManipulator::Start(int realtime_priority, uint32_t affinity_mask)
{
    {
      rgb_matrix::MutexLock l(&mutex_);
      running_ = true;
    }
    rgb_matrix::Thread::Start(realtime_priority, affinity_mask);
}

void MyThreadedCanvasManipulator::Stop()
{
    rgb_matrix::MutexLock l(&mutex_);
    running_ = false;
}
