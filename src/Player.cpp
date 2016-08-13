//
//  Player.cpp
//  ImageSound
//
//  Created by Chikashi Miyama on 05/08/16.
//
//

#include "Player.h"
#include "ofApp.h"

void Player::update(){
    if(!active)return;
    if(vertical){
        parent.getScore().cropTo(croppedPixelsH, position.x, position.y, length, 1);
        for(int i = 0; i < croppedPixelsH.size(); i++){
            unsigned char color = croppedPixelsH[i];
            if(color < 255){
                parent.addCollision(ofPoint(position.x+i, position.y), *this);
            }
        }
    }else{
        parent.getScore().cropTo(croppedPixelsV, position.x, position.y, 1, length);
        for(int i = 0; i < croppedPixelsV.size(); i++){
            unsigned char color = croppedPixelsV[i];
            if(color < 255){
                parent.addCollision(ofPoint(position.x, position.y+i), *this);
            }
        }
    }

}

