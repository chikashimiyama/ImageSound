#pragma once
#include "ofMain.h"
#include "Player.h"
#include "const.h"

class Collision{
public:
    Collision(ofPoint &point, Player &player);
    void update();
    void draw();
    bool getDead();
protected:
    bool dead;
    float lifeTime;
    const ofPoint position;
    const ofColor color;
};

inline Collision::Collision(ofPoint &position, Player &player):
lifeTime(kLifeTime),
position(position),
color(player.getColor()),
dead(false){
}

inline void Collision::update(){
    lifeTime--;
    if(lifeTime < 0)
        dead = true;
}

inline void Collision::draw(){
    ofColor fadedColor = color;
    float alpha = lifeTime / kLifeTime;
    float squareAlpha = alpha * alpha * alpha;
    fadedColor.a =  squareAlpha * 40;
    ofSetColor(fadedColor);
    ofDrawCircle(position, alpha * 10);
}

inline bool Collision::getDead(){
    return dead;
}
