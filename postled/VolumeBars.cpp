#include "VolumeBars.h"
#include "math.h"
#include <unistd.h>

using namespace rgb_matrix;

VolumeBars::VolumeBars(rgb_matrix::RGBMatrix *pMatrix, int delay_ms, int numBars)
  : MyThreadedCanvasManipulator(pMatrix), delay_ms_(delay_ms),
    numBars_(numBars), t_(0), m_volume(0), m_pMatrix(pMatrix) {
}

VolumeBars::~VolumeBars() {
  delete [] barHeights_;
}


void VolumeBars::setVolume(int volume) {
    m_volume = volume;
}

void VolumeBars::Run() {
  int loops = 0;
  int maxloops = 20;
  const int width = canvas()->width();
  height_ = canvas()->height();
  barWidth_ = width/numBars_;
  barHeights_ = new int[numBars_];
  heightGreen_  = height_*4/12;
  heightYellow_ = height_*8/12;
  heightOrange_ = height_*10/12;
  heightRed_    = height_*12/12;

  // Start the loop
  while (running() && loops < maxloops) {
    loops++;

    // Update bar heights
    for (int i=0; i<numBars_; ++i) {
      barHeights_[i] = i;
    }
    
    int brightness = (maxloops-loops) * 10;
    if (brightness < 1) {
      fprintf(stderr, "Brightness is outside usable range.\n");
      m_pMatrix->SetBrightness(10);
    } else if (brightness > 100) {
      fprintf(stderr, "Brightness is outside usable range.\n");
      m_pMatrix->SetBrightness(100);
    } else {
      m_pMatrix->SetBrightness(brightness);
    }
    
    for (int i=0; i<numBars_; ++i) {
      int y;
      for (y=0; y<barHeights_[i]; ++y) {
        if ((float)m_volume/2.0f < (float)i) {
          drawBarRow(i, y, 30, 30, 30);
        }
        else if (y<heightGreen_) {
          drawBarRow(i, y, 0, 200, 0);
        }
        else if (y<heightYellow_) {
          drawBarRow(i, y, 150, 150, 0);
        }
        else if (y<heightOrange_) {
          drawBarRow(i, y, 250, 100, 0);
        }
        else {
          drawBarRow(i, y, 200, 0, 0);
        }
      }
      // Anything above the bar should be black
      for (; y<height_; ++y) {
        drawBarRow(i, y, 0, 0, 0);
      }
    }
    usleep(delay_ms_ * 1000);
  }
  
  // Finished. Shut down the RGB matrix.
  canvas()->Clear();
  setRunningFinished();
}


void VolumeBars::drawBarRow(int bar, int y, uint8_t r, uint8_t g, uint8_t b) {
  for (int x=bar*barWidth_; x<(bar+1)*barWidth_; ++x) {
    canvas()->SetPixel(x, height_-1-y, r, g, b);
  }
}

