#include "Alarm.h"
#include "math.h"
#include <unistd.h>

using namespace rgb_matrix;

Alarm::Alarm(rgb_matrix::RGBMatrix *pMatrix)
  : MyThreadedCanvasManipulator(pMatrix),
    t_(0), m_pMatrix(pMatrix) {
}

Alarm::~Alarm() {
}


void Alarm::Run() {
  uint32_t continuum = 0;
  int loops = 0;
  int maxloops = 6000;
    while (running() && loops < maxloops) {
      loops++;
      usleep(5 * 1000);
      continuum += 1;
      continuum %= 3 * 255;
      int r = 0, g = 0, b = 0;
      if (continuum <= 255) {
        int c = continuum;
        b = 255 - c;
        r = c;
      } else if (continuum > 255 && continuum <= 511) {
        int c = continuum - 256;
        r = 255 - c;
        g = c;
      } else {
        int c = continuum - 512;
        g = 255 - c;
        b = c;
      }
      r = 0;
      g = 0;
      canvas()->Fill(r, g, b);
   }  
  // Finished. Shut down the RGB matrix.
  canvas()->Clear();
  setRunningFinished();
}

