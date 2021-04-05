#ifndef STATICTEXTGENERATOR_H
#define STATICTEXTGENERATOR_H

#include "led-matrix.h"
#include "graphics.h"
#include "MyThreadedCanvasManipulator.h"

class StaticTextGenerator : public MyThreadedCanvasManipulator
{
public:
    StaticTextGenerator(rgb_matrix::RGBMatrix *pMatrix);
    virtual ~StaticTextGenerator();
    
    void setText(std::string text) {m_text = text;}
    
    /**
     * @brief setDelay how long is the text shown in milliseconds
     * -1 for endless
     * @param delay
     */
    void setDelay(int delay);
    
    void Run() override;
    
private: 
    bool isFullSaturation(const rgb_matrix::Color &c);
    
    
private:
    rgb_matrix::RGBMatrix *m_pMatrix;
    rgb_matrix::Font m_font;
    rgb_matrix::Font *m_pOutlineFont;
    std::string m_text;
    int m_delayMs = 60000;
    
};

#endif // STATICTEXTGENERATOR_H
