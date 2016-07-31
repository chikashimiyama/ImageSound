#pragma once
#include "const.hpp"


#include "ofMain.h"
#include "ofxPd.h"


class ofApp : public ofBaseApp, public pd::PdReceiver{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
    void audioIn(float * input, int bufferSize, int nChannels);
    void audioOut(float * input, int bufferSize, int nChannels);
    void receiveMessage(const std::string &dest, const std::string &msg, const pd::List &list);

protected:
    enum class Mode{
        wait,
        capture,
        transition,
        sonification
    };
    
    struct Cell{
        int x;
        int y;
        ofColor color;
        Cell(int x, int y, ofColor color):x(x),y(y), color(color){}
    };
    
    void drawTextRegion();
    void drawMessage();
    void drawMatrix();
    void drawPlayer();
    void scoreAnalysis();
    ofVideoGrabber videoCamera;
    ofPixels score, processedScore;
    ofTexture scoreTexture, processedTexture;

    ofTrueTypeFont font;
    ofxPd pd;
    
    int waitTime;
    
    ofTexture texture;
    Mode mode;
    float alpha;
    
    std::vector<Cell> players;
};
