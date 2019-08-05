#include "ListUI.h"
#include <unistd.h>
#include <iostream>
#include <iterator>
#include <string>
#include <regex>

using namespace rgb_matrix;

ListUI::ListUI(rgb_matrix::RGBMatrix *pMatrix)
    : MyThreadedCanvasManipulator(pMatrix)
    , m_pMatrix(pMatrix)
    , m_str0("")
    , m_str1("")
    , m_str2("")
    , m_str3("")
    , m_str4("")
    , m_pixelsToAnimate(0)
{
    const char *bdf_font_file1 = "matrix/fonts/4x6.bdf";
    const char *bdf_font_file2 = "matrix/fonts/5x8.bdf";
    
    /*
     * Load font. This needs to be a filename with a bdf bitmap font.
     */
    if (!m_font1.LoadFont(bdf_font_file1)) {
      fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file1);
      exit(1);
    }
    if (!m_font2.LoadFont(bdf_font_file2)) {
      fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file2);
      exit(1);
    }
    
    if (m_pMatrix == nullptr)
      exit(1);
}

void ListUI::setData(std::string strPayload)
{
    std::stringstream ss(strPayload);
    std::string item;
    const std::regex re{"((?:[^\\\\,]|\\\\.)*?)(?:,|$)"};
    
    std::getline(ss, item);
    std::vector<std::string> vecFields;
    vecFields.insert(vecFields.end(), std::sregex_token_iterator(item.begin(), item.end(), re, 1), std::sregex_token_iterator());

    size_t listSize = vecFields.size() - 1;
    std::string currentItem = vecFields[0];
    std::vector<std::string> vecList;
    vecList.resize(listSize);
    
    const std::regex base_regex("[ |\\[]*\\'(.*)\\'[\\]]*");
    for (uint64_t i = 1; i < vecFields.size(); i++)
    {
        std::smatch base_match;
        if (std::regex_match(vecFields[i], base_match, base_regex))
        {
            if (base_match.size() == 2)
            {
                std::ssub_match base_sub_match = base_match[1];
                std::string base = base_sub_match.str();
                vecList[i] = base;
            }
        }
    }
    
    for (uint64_t i = 0; i < vecList.size(); i++)
    {
        if (strcmp(currentItem.c_str(), vecList[i].c_str()) == 0)
        {
            if (i > 1)
            {
                m_str0 = vecList[i-2];
            }
            else
            {
                m_str0 = vecList[vecList.size()-1];
            }
            
            if (i > 0)
            {
                m_str1 = vecList[i-1];
            }
            else
            {
                m_str1 = vecList[vecList.size()-1];
            }
            
            if (i+1 >= vecList.size())
            {
                m_str3 = vecList[0];
            }
            else
            {
                m_str3 = vecList[i+1];
            }
            
            if (i+2 >= vecList.size())
            {
                m_str4 = vecList[0];
            }
            else
            {
                m_str4 = vecList[i+2];
            }
            
            if (strcmp(m_str2.c_str(), m_str1.c_str()) == 0)
            {
                m_pixelsToAnimate = 8;
            }
            else if (strcmp(m_str2.c_str(), m_str3.c_str()) == 0)
            {
                m_pixelsToAnimate = -8;
            } else 
                m_pixelsToAnimate = 0;
            
            m_str2 = vecList[i];
            break;
        }
    }

}

void ListUI::Run()
{
    int loop = 0;
    Color color(255, 255, 255);
    Color colorActive(255, 255, 0);
    Color bg_color(0, 0, 0);
    int x_orig = 1;
    int x_orig_anim = 1;
    int length_anim = 0;
    uint8_t brightness = 100;
    int letter_spacing = 0;
    
    if (brightness < 1 || brightness > 100) {
      fprintf(stderr, "Brightness is outside usable range.\n");
      exit(1);
    }
    m_pMatrix->SetBrightness(brightness);
    
    // Create a new canvas to be used with led_matrix_swap_on_vsync
    FrameCanvas *offscreen_canvas = m_pMatrix->CreateFrameCanvas();
  
    while (running() && loop < 100) {
      loop++;
      offscreen_canvas->Clear(); // clear canvas
      int y_orig = -5;
      Color color1(0,0,0);
      Color color2(0,0,0);
      Color color3(0,0,0);
      if (m_pixelsToAnimate < 0) {
        m_pixelsToAnimate += 2;
        y_orig += m_pixelsToAnimate;
        color1 = color;
        color2 = color;
        color3 = colorActive;
      } else if (m_pixelsToAnimate > 0) {
        m_pixelsToAnimate -= 2;
        y_orig += m_pixelsToAnimate;
        color1 = colorActive;
        color2 = color;
        color3 = color;
      } else  {
          color1 = color;
          color2 = colorActive;
          color3 = color;
      }
      
      //if (loop > 5)
      //    x_orig_anim--;
      
      if (y_orig + m_font2.baseline() > 0)
          rgb_matrix::DrawText(offscreen_canvas, m_font2,
                               x_orig, y_orig - 1,
                               color, &bg_color,
                               m_str0.c_str(), letter_spacing);

      rgb_matrix::DrawText(offscreen_canvas, m_font2,
                           x_orig, y_orig + m_font2.baseline(),
                           color1, &bg_color,
                           m_str1.c_str(), letter_spacing);
      
      length_anim = rgb_matrix::DrawText(offscreen_canvas, m_font2,
                           x_orig_anim, y_orig + 2*m_font2.baseline() + 1,
                           color2, &bg_color,
                           m_str2.c_str(), letter_spacing);
      
      rgb_matrix::DrawText(offscreen_canvas, m_font2,
                           x_orig, y_orig + 3*m_font2.baseline() + 2,
                           color3, &bg_color,
                           m_str3.c_str(), letter_spacing);
      
      if (y_orig + 4*m_font2.baseline() + 3 < 32)
          rgb_matrix::DrawText(offscreen_canvas, m_font2,
                               x_orig, y_orig + 4*m_font2.baseline() + 3,
                               color, &bg_color,
                               m_str4.c_str(), letter_spacing);

      if (x_orig_anim <= x_orig - length_anim)
          x_orig_anim = x_orig;
      usleep(100000);
      // Swap the offscreen_canvas with canvas on vsync, avoids flickering
      offscreen_canvas = m_pMatrix->SwapOnVSync(offscreen_canvas);
    }
    
    // Finished. Shut down the RGB matrix.
    m_pMatrix->Clear();
    setRunningFinished();
}
