#ifndef LISTUI_H
#define LISTUI_H

#include "led-matrix.h"
#include "graphics.h"
#include "MyThreadedCanvasManipulator.h"

class ListUI : public MyThreadedCanvasManipulator
{
public:
    ListUI(rgb_matrix::RGBMatrix *pMatrix);
    
    void setData(std::string text);
    void Run() override;
    
private:
    rgb_matrix::RGBMatrix *m_pMatrix;
    rgb_matrix::Font m_font1;
    rgb_matrix::Font m_font2;
    
    std::string m_str0;
    std::string m_str1;
    std::string m_str2;
    std::string m_str3;
    std::string m_str4;
    
    // Number of pixels to animate (positive or negative for next/prev)
    int m_pixelsToAnimate;
};

#endif // LISTUI_H
