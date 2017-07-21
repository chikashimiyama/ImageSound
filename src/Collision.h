#pragma once
#include <math.h>
#include "ofMain.h"
#include "Player.h"
#include "const.h"

class CircleVbo{
    
public:
    static CircleVbo &getInstance(){
        static CircleVbo instance;
        return instance;
    };
    void draw(){
        vbo.draw(GL_POLYGON, 0, 20);
    }
    
private:
    CircleVbo(){
        if(vbo.getNumVertices() > 0 )return;
        std::vector<ofVec2f> points;
        for(int i = 0; i < 20; i++){
            float radian = M_PI * (static_cast<float>(i) / 10.0);
            points.emplace_back(sin(radian), cos(radian));
        }
        vbo.setVertexData(&points[0], points.size(), GL_STATIC_DRAW);
    };
    ofVbo vbo;
    
public:
    CircleVbo(CircleVbo const&) = delete;
    void operator=(CircleVbo const&) = delete;
};

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
    float scale = alpha * 10;
    ofPushMatrix();
    ofTranslate(position);
    ofScale(scale,scale,1.0);
    CircleVbo::getInstance().draw();
    ofPopMatrix();
//    ofDrawCircle(position, alpha * 10);
}

inline bool Collision::getDead(){
    return dead;
}
