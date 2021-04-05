#include "StaticTextGenerator.h"


#include <string>

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "math.h"

using namespace rgb_matrix;


StaticTextGenerator::StaticTextGenerator(rgb_matrix::RGBMatrix *pMatrix)
  : MyThreadedCanvasManipulator(pMatrix)
  , m_pMatrix(pMatrix)
  , m_pOutlineFont(nullptr)
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

StaticTextGenerator::~StaticTextGenerator() {
}


bool StaticTextGenerator::isFullSaturation(const Color &c)
{
  return (c.r == 0 || c.r == 255)
    && (c.g == 0 || c.g == 255)
    && (c.b == 0 || c.b == 255);
}

void StaticTextGenerator::setDelay(int delay)
{
    m_delayMs = delay;
}

void StaticTextGenerator::Run()
{
    Color color(255, 255, 200);
    Color bg_color(0, 0, 0);
    Color outline_color(0,0,0);
    bool with_outline = false;
    int x_orig = 5;
    int y_orig = 5;
    uint8_t brightness = 50;
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

    
    // Create a new canvas to be used with led_matrix_swap_on_vsync
    FrameCanvas *offscreen_canvas = m_pMatrix->CreateFrameCanvas();

    int delay = m_delayMs * 1000;

    while (running() && delay > 0) {
      offscreen_canvas->Clear(); // clear canvas

      if (m_pOutlineFont) {
        // The outline font, we need to write with a negative (-2) text-spacing,
        // as we want to have the same letter pitch as the regular text that
        // we then write on top.
        rgb_matrix::DrawText(offscreen_canvas, *m_pOutlineFont,
                             x - 1, y + m_font.baseline(),
                             outline_color, &bg_color,
                             m_text.c_str(), letter_spacing - 2);
      }
  
      rgb_matrix::DrawText(offscreen_canvas, m_font,
                                    x, y + m_font.baseline(),
                                    color, m_pOutlineFont ? nullptr : &bg_color,
                                    m_text.c_str(), letter_spacing);
  
      delay = delay - 1000000;
      usleep(1000000);

      // Swap the offscreen_canvas with canvas on vsync, avoids flickering
      offscreen_canvas = m_pMatrix->SwapOnVSync(offscreen_canvas);
    }
    
    if (all_extreme_colors)
      m_pMatrix->SetPWMBits(0);
    // Finished. Shut down the RGB matrix.
    m_pMatrix->Clear();
    setRunningFinished();
}
