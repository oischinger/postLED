#ifndef VOLUMEBARS_H
#define VOLUMEBARS_H

#include "led-matrix.h"
#include "graphics.h"
#include "MyThreadedCanvasManipulator.h"

class VolumeBars : public MyThreadedCanvasManipulator {
public:
  VolumeBars(rgb_matrix::RGBMatrix *pMatrix, int delay_ms=32, int numBars=16);
  ~VolumeBars();

  void Run();
  void setVolume(int volume);

private:
  void drawBarRow(int bar, int y, uint8_t r, uint8_t g, uint8_t b);

  int delay_ms_;
  int numBars_;
  int* barHeights_;
  int barWidth_;
  int height_;
  int heightGreen_;
  int heightYellow_;
  int heightOrange_;
  int heightRed_;
  int t_;
  
  int m_volume;
  rgb_matrix::RGBMatrix *m_pMatrix;
};

#endif // VOLUMEBARS_H
