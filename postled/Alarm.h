#ifndef ALARM_H
#define ALARM_H

#include "led-matrix.h"
#include "graphics.h"
#include "MyThreadedCanvasManipulator.h"

class Alarm : public MyThreadedCanvasManipulator {
public:
  Alarm(rgb_matrix::RGBMatrix *pMatrix);
  ~Alarm();

  void Run();

private:
  int t_;
  
  rgb_matrix::RGBMatrix *m_pMatrix;
};

#endif // ALARM_H
