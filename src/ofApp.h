#pragma once

#include <forward_list>

#include "const.h"
#include "Player.h"
#include "ofxOpenCv.h"
#include "ofMain.h"
#include "ofxPd.h"
#include "Collision.h"


class ofApp : public ofBaseApp, public pd::PdReceiver{

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
    void audioIn(float * input, int bufferSize, int nChannels);
    void audioOut(float * input, int bufferSize, int nChannels);
    
    void print(const std::string& message);
    void receiveMessage(const std::string &dest, const std::string &msg, const pd::List &list);
    ofPixels &getScore();
    void addCollision(ofPoint position, Player &player);
    
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
    void drawCollisions();
    void scoreAnalysis();
    ofVideoGrabber videoCamera;
    ofxCvColorImage colorImage;
    ofxCvGrayscaleImage grayImage;
    ofPixels score;
    ofSoundStream soundStream;
    ofTexture scoreTexture;
    ofTrueTypeFont font;
    ofxPd pd;
    
    int waitTime;
    
    ofTexture texture;
    Mode mode;
    float alpha;
    
    std::vector<float> imageData;
    std::vector<Player> players;
    std::forward_list<Collision> collisions;
private:
    ofPoint pointFromIndex(int index);

};
