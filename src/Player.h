#pragma once
#include "ofMain.h"
#include "const.h"

class ofApp;

class Player {
public:
    Player(ofColor color, float height, ofApp &parent, ofPoint position = ofPoint());
    void draw();
    void setPosition(ofPoint );
    void setPositionFromIndex(int );
    void setActive(bool);
    bool getDirection();
    void setDirection(bool);
    const ofColor &getColor() const;
    void update();
protected:
    const ofColor color;
    const float length;
    ofPoint position;
    ofApp &parent;
    ofPixels croppedPixelsV;
    ofPixels croppedPixelsH;
    bool vertical; // true means vertical read
    bool active;
};

inline Player::Player(ofColor color, float length, ofApp &parent, ofPoint position):
color(color), length(length), parent(parent), position(position), active(false){
    croppedPixelsV.allocate(1,length, OF_PIXELS_GRAY);
    croppedPixelsH.allocate(length, 1, OF_PIXELS_GRAY);

}

inline void Player::setActive(bool a){
    active = a;
}

inline bool Player::getDirection(){
    return vertical;
}

inline void Player::setDirection(bool v){
    vertical = v;
}

inline void Player::setPosition(ofPoint p){
    position = p;
}

inline void Player::setPositionFromIndex(int index){
    setPosition(ofPoint( index % kWidth, index / kWidth));
}

inline const ofColor &Player::getColor() const{
    return color;
}

inline void Player::draw(){
    if(!active)return;
    ofFill();
    ofSetColor(color);
    if(vertical){
        ofDrawRectangle(position.x-1, position.y-1, 3+length, 3);
    }else{
        ofDrawRectangle(position.x-1, position.y-1, 3, 3+length);
    }
    
}
