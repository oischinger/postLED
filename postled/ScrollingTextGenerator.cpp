#include "ScrollingTextGenerator.h"


#include <string>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "math.h"

using namespace rgb_matrix;

VolumeBarVisualization::VolumeBarVisualization(rgb_matrix::RGBMatrix *pMatrix)
    : m_pMatrix(pMatrix)
{
    // Init Volume bars
    delay_ms_ = 50;
    numBars_ = 6;
    t_ = 0;
    
    const int width = pMatrix->width();
    height_ = pMatrix->height() / 2;
    barWidth_ = width/numBars_;
    barHeights_ = new int[numBars_];
    barMeans_ = new int[numBars_];
    barFreqs_ = new int[numBars_];
    heightGreen_  = height_*4/12;
    heightYellow_ = height_*8/12;
    heightOrange_ = height_*10/12;
    heightRed_    = height_*12/12;

    // Array of possible bar means
    numMeans = 10;
    for (int i=0; i<numMeans; ++i) {
      means[i] = height_ - means[i]*height_/8;
    }
    // Initialize bar means randomly
    srand(time(NULL));
    for (int i=0; i<numBars_; ++i) {
      barMeans_[i] = rand()%numMeans;
      barFreqs_[i] = 1<<(rand()%3);
    }
}

VolumeBarVisualization::~VolumeBarVisualization()
{
    delete [] barHeights_;
    delete [] barFreqs_;
    delete [] barMeans_;
}

void VolumeBarVisualization::Step()
{
    if (t_ % 8 == 0) {
      // Change the means
      for (int i=0; i<numBars_; ++i) {
        barMeans_[i] += rand()%3 - 1;
        if (barMeans_[i] >= numMeans)
          barMeans_[i] = numMeans-1;
        if (barMeans_[i] < 0)
          barMeans_[i] = 0;
      }
    }

    // Update bar heights
    t_++;
    for (int i=0; i<numBars_; ++i) {
      barHeights_[i] = (height_ - means[barMeans_[i]])
        * sin(0.1*t_*barFreqs_[i]) + means[barMeans_[i]];
      if (barHeights_[i] < height_/8)
        barHeights_[i] = rand() % (height_/8) + 1;
    }

    for (int i=0; i<numBars_; ++i) {
      int y;
      for (y=0; y<barHeights_[i]; ++y) {
        if (y<heightGreen_) {
          drawBarRow(i, y, 0, 200, 0);
        }
        else if (y<heightYellow_) {
          drawBarRow(i, y, 150, 140, 0);
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
}

ScrollingTextGenerator::ScrollingTextGenerator(rgb_matrix::RGBMatrix *pMatrix)
  : MyThreadedCanvasManipulator(pMatrix)
  , m_pMatrix(pMatrix)
  , m_pOutlineFont(nullptr)
  , m_volumeVis(pMatrix)
{
    const char *bdf_font_file = nullptr;
    
    // TODO
    m_text = "foo bar";
    bdf_font_file = "matrix/fonts/5x8.bdf";
    
    /*
     * Load font. This needs to be a filename with a bdf bitmap font.
     */
    if (!m_font.LoadFont(bdf_font_file)) {
      fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
      exit(1);
    }
    
    if (m_pMatrix == nullptr)
      exit(1);
}

ScrollingTextGenerator::~ScrollingTextGenerator() {
}


bool ScrollingTextGenerator::isFullSaturation(const Color &c)
{
  return (c.r == 0 || c.r == 255)
    && (c.g == 0 || c.g == 255)
    && (c.b == 0 || c.b == 255);
}

void ScrollingTextGenerator::setLoops(int loops)
{
    m_loops = loops;
}

void ScrollingTextGenerator::Run()
{
    Color color(255, 255, 200);
    Color bg_color(0, 0, 0);
    Color outline_color(0,0,0);
    bool with_outline = false;
    float speed = 7.0f;
    int x_orig = (m_pMatrix->width()) + 5;
    int y_orig = 0;
    uint8_t brightness = 100;
    int letter_spacing = 0;
    
    /*
     * If we want an outline around the font, we create a new font with
     * the original font as a template that is just an outline font.
     */
    if (with_outline) {
      m_pOutlineFont = m_font.CreateOutlineFont();
    }
    
    if (brightness < 1 || brightness > 100) {
      fprintf(stderr, "Brightness is outside usable range.\n");
      exit(1);
    }
    m_pMatrix->SetBrightness(brightness);
  
    const bool all_extreme_colors = (brightness == 100)
      && isFullSaturation(color)
      && isFullSaturation(bg_color)
      && isFullSaturation(outline_color);
    if (all_extreme_colors)
      m_pMatrix->SetPWMBits(1);
    
    int x = x_orig;
    int y = y_orig;
    int length = 0;

    
    // Create a new canvas to be used with led_matrix_swap_on_vsync
    FrameCanvas *offscreen_canvas = m_pMatrix->CreateFrameCanvas();
  
    int delay_speed_usec = 1000000 / speed / m_font.CharacterWidth('W');
    if (delay_speed_usec < 0) delay_speed_usec = 2000;
  
    while (running() && m_loops != 0) {
      offscreen_canvas->Clear(); // clear canvas

      if (m_bMusic)
        m_volumeVis.Step();

      if (m_pOutlineFont) {
        // The outline font, we need to write with a negative (-2) text-spacing,
        // as we want to have the same letter pitch as the regular text that
        // we then write on top.
        rgb_matrix::DrawText(offscreen_canvas, *m_pOutlineFont,
                             x - 1, y + m_font.baseline(),
                             outline_color, &bg_color,
                             m_text.c_str(), letter_spacing - 2);
      }
  
      // length = holds how many pixels our text takes up
      length = rgb_matrix::DrawText(offscreen_canvas, m_font,
                                    x, y + m_font.baseline(),
                                    color, m_pOutlineFont ? nullptr : &bg_color,
                                    m_text.c_str(), letter_spacing);
  
      if (--x + length < 0) {
        x = x_orig;
        if (m_loops > 0) --m_loops;
      }
  
      usleep(delay_speed_usec);
      // Swap the offscreen_canvas with canvas on vsync, avoids flickering
      offscreen_canvas = m_pMatrix->SwapOnVSync(offscreen_canvas);
    }
    
    if (all_extreme_colors)
      m_pMatrix->SetPWMBits(0);
    // Finished. Shut down the RGB matrix.
    m_pMatrix->Clear();
    setRunningFinished();
}
