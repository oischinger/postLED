#ifndef SCROLLINGTEXTGENERATOR_H
#define SCROLLINGTEXTGENERATOR_H

#include "led-matrix.h"
#include "graphics.h"
#include "MyThreadedCanvasManipulator.h"

class VolumeBarVisualization
{
public:
    VolumeBarVisualization(rgb_matrix::RGBMatrix *pMatrix);
    virtual ~VolumeBarVisualization();
    
    void Step();
    
private:
    void drawBarRow(int bar, int y, uint8_t r, uint8_t g, uint8_t b) {
      for (int x=bar*barWidth_; x<(bar+1)*barWidth_; ++x) {
        m_pMatrix->SetPixel(x, height_ + height_-1-y, r, g, b);
      }
    }

private:
    rgb_matrix::RGBMatrix *m_pMatrix;
    
    int delay_ms_;
    int numBars_;
    int* barHeights_;
    int barWidth_;
    int height_;
    int heightGreen_;
    int heightYellow_;
    int heightOrange_;
    int heightRed_;
    int* barFreqs_;
    int* barMeans_;
    int t_;
    int numMeans;
    int means[10] = {1,2,3,4,5,6,7,8,16,32};
};


class ScrollingTextGenerator : public MyThreadedCanvasManipulator
{
public:
    ScrollingTextGenerator(rgb_matrix::RGBMatrix *pMatrix);
    virtual ~ScrollingTextGenerator();
    
    void setText(std::string text) {m_text = text;}
    void setMusic(bool value) {m_bMusic = value;}
    
    /**
     * @brief setLoops set number of repetitions on text scroll
     * -1 for endless
     * @param bLoops
     */
    void setLoops(int loops);
    
    void Run() override;
    
private: 
    bool isFullSaturation(const rgb_matrix::Color &c);
    
    
private:
    rgb_matrix::RGBMatrix *m_pMatrix;
    rgb_matrix::Font m_font;
    rgb_matrix::Font *m_pOutlineFont;
    std::string m_text;
    bool m_bMusic;
    int m_loops = 3;
    VolumeBarVisualization m_volumeVis;
    
};

#endif // SCROLLINGTEXTGENERATOR_H
