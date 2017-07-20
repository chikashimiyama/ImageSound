#include "ofApp.h"
#include "ofxPd.h"
#include "Externals.h"

void ofApp::setup(){
    ofSetDataPathRoot("../Resources/data/");

    ofBackground(ofColor::white);
    playerSetup();
    videoSetup();
    pixelSetup();
    soundSetup();
    ofSetVerticalSync(true);
    font.load("Arial.ttf", 20);
    waitTime = 5;
    mode = Mode::wait;
    shader.load("shaders/shader");

    //ofSetFullscreen(true);
}

void ofApp::playerSetup(){
    players.emplace_back(ofColor::green, 16, *this);
    players.emplace_back(ofColor::orange, 32, *this );
    players.emplace_back(ofColor::red, 64, *this);
    players.emplace_back(ofColor::blue, 128, *this);
    players.emplace_back(ofColor::purple, 8, *this);
    players.emplace_back(ofColor::lightGray, 768, *this);
}

void ofApp::videoSetup(){
    vector<ofVideoDevice> devices = videoCamera.listDevices();
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    videoCamera.setDeviceID(0);
    videoCamera.initGrabber(kWidth, kHeight);
    captureScaling.x = kWidth / videoCamera.getWidth();
    captureScaling.y = kHeight/ videoCamera.getHeight();
}

void ofApp::pixelSetup(){
    scalerFbo.allocate(kWidth, kHeight);
    score.allocate(kWidth, kHeight, OF_IMAGE_GRAYSCALE); // 1 channel
}

void ofApp::soundSetup(){
    int ticksPerBuffer = 8; // 8 * 64 = buffer len of 512
    ofSoundStreamSetup(2, 0, this, 44100, ofxPd::blockSize()*ticksPerBuffer, 3);
    if(!pd.init(2, 0, 44100, ticksPerBuffer, false)) {
        OF_EXIT_APP(1);
    }
    slide_tilde_setup();
    edge_tilde_setup();
    pd.openPatch("sonify.pd");
    pd.addReceiver(*this);
    pd.subscribe("toOF");
    pd.start();
}

ofPixels &ofApp::getScore(){
    return score;
}

void ofApp::addCollision(ofPoint position, Player &player){
    collisions.emplace_front(position,player);
}

#pragma mark update
void ofApp::scoreAnalysis(){
    ofImage screenImage;
    screenImage.grabScreen(0, 0, kWidth, kHeight); // always RGB unsigned char
    const ofPixels &pixels = screenImage.getPixels(); // RGB unsigned char
    score = pixels.getChannel(0); // get r channel
    
    
    imageData.clear(); // Mono unsigned char
    for(int i = 0 ; i < score.size(); i++){ // check only red channel
        imageData.push_back(255.0 - static_cast<float>(score[i]));
    }
    pd.writeArray("imageData", imageData);
}

void ofApp::update(){
    
    switch(mode){
        case Mode::wait:{
            grabImage();
            break;
        }
        case Mode::capture:{

            break;
        }
        case Mode::transition:{
            alpha += 0.01;
            if(alpha > 1){//fade in from white (shutter effect)
                mode = Mode::sonification;
                pd.sendMessage("fromOF", "start");
            }
            break;
        }
        case Mode::sonification:{
            for(auto &player:players){player.update();}
            for(auto &collision: collisions){collision.update();}
            collisions.remove_if([](Collision &collision){ return collision.getDead();});
            break;
        }
    }
}

void ofApp::grabImage(){
    videoCamera.update();
    const ofTexture &texture = videoCamera.getTexture();

    scalerFbo.begin();
    ofClear(0);
    ofScale(captureScaling.x, captureScaling.y);
    shader.begin();
    texture.draw(0,0);
    shader.end();
    scalerFbo.end();
}

#pragma mark draw

void ofApp::drawTextRegion(){
    ofSetColor(ofColor(0,0,200,100));
    ofFill();
    ofDrawRectangle(0, kHeight/2 - 50, kWidth, 100);
}

void ofApp::drawMatrix(){
    ofSetLineWidth(0.5);
    ofSetColor(ofColor::black);
    int offset = 64;
    for(int i = 0; i < 16; i++){
        ofDrawLine(offset, 0, offset, kHeight);
        offset += 64;
    }
    offset = 64;
    for(int i = 0; i < 12; i++){
        ofDrawLine(0, offset, kWidth, offset);
        offset += 64;
    }
}

void ofApp::drawPlayer(){
    for(auto &player:players){player.draw();}
}

void ofApp::drawCollisions(){
    for(auto &collision:collisions){collision.draw();}
}

void ofApp::drawMessage(){
    ofSetColor(ofColor::white);
    std::string message = ofToString(waitTime);
    float width = font.stringWidth(message);
    font.drawString(message, (kWidth - width) / 2, kHeight/2);
}

void ofApp::draw(){
    switch (mode) {
        case Mode::wait:{
            ofSetColor(ofColor::white);
            scalerFbo.draw(0,0);
            drawTextRegion();
            drawMessage();
            break;
        }
        case Mode::capture:{
            ofSetColor(ofColor::white);
            scalerFbo.draw(0,0);
            scoreAnalysis();
            mode = Mode::transition;
            alpha = 0.0;
            break;
        }
        case Mode::transition:{
            ofSetColor(ofFloatColor(1,1,1,alpha));
            scalerFbo.draw(0,0);

            break;
        }
        case Mode::sonification:{
            ofSetColor(ofFloatColor(1,1,1,alpha));
            scalerFbo.draw(0,0);
            drawPlayer();
            drawCollisions();
            break;
        }
        default:
            break;
    }
}

#pragma mark Pd
void ofApp::receiveMessage(const std::string &dest, const std::string &msg, const pd::List &list){
    
    if(msg == "waitTime"){
        waitTime = static_cast<int>(list.getFloat(0));
        if(waitTime == 0){ mode = Mode::capture; }
    }else if(msg == "player"){
        int id = static_cast<int>(list.getFloat(0));
        std::string descriptor = list.getSymbol(1);
        int value = static_cast<int>(list.getFloat(2));
        if(descriptor == "position"){
            players[id].setPositionFromIndex(value);
        }else if(descriptor == "direction"){
            players[id].setDirection(static_cast<bool>(value));
        }else if(descriptor == "active"){
            players[id].setActive(static_cast<bool>(value));
        }
    }else if(msg == "reset"){
        mode = Mode::wait;
    }
}

void ofApp::print(const std::string &msg){
    ofLog() << msg;
}

void ofApp::audioIn(float * input, int bufferSize, int nChannels) {
    pd.audioIn(input, bufferSize, nChannels);
}

void ofApp::audioOut(float * output, int bufferSize, int nChannels) {
    pd.audioOut(output, bufferSize, nChannels);
}
