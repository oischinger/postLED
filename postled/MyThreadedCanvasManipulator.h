#ifndef MYTHREADEDCANVASMANIPULATOR_H
#define MYTHREADEDCANVASMANIPULATOR_H

#include "thread.h"
#include "canvas.h"

class MyThreadedCanvasManipulator : public rgb_matrix::Thread
{
public:
  MyThreadedCanvasManipulator(rgb_matrix::Canvas* m) : running_(false), canvas_(m) {}
  virtual ~MyThreadedCanvasManipulator() {  Stop(); }

  virtual void Start(int realtime_priority=50, uint32_t affinity_mask=1<<3);

  // Stop the thread at the next possible time Run() checks the running_ flag.
  void Stop();
  
  bool isRunning() { return running_; }

  // Implement this and run while running() returns true.
  virtual void Run() = 0;
  
protected:
  inline rgb_matrix::Canvas *canvas() { return canvas_; }
  inline bool running() {
    rgb_matrix::MutexLock l(&mutex_);
    return running_;
  }
  
  // Signal that Run() has finished 
  void setRunningFinished() { running_ = false; }

private:
  rgb_matrix::Mutex mutex_;
  bool running_;
  rgb_matrix::Canvas *const canvas_;
};

#endif // MYTHREADEDCANVASMANIPULATOR_H
