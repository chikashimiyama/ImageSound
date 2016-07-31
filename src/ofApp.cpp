#include "ofApp.h"
#include "ofxPd.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofBackground(ofColor::white);
    vector<ofVideoDevice> devices = videoCamera.listDevices();
    
    for(int i = 0; i < devices.size(); i++){
        if(devices[i].bAvailable){
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName;
        }else{
            ofLogNotice() << devices[i].id << ": " << devices[i].deviceName << " - unavailable ";
        }
    }
    
    videoCamera.setDeviceID(0);
    videoCamera.initGrabber(ofGetWidth(), ofGetHeight());
    
    score.allocate(ofGetWidth(), ofGetHeight(), OF_PIXELS_RGB);
    scoreTexture.allocate(score);
    processedScore.allocate(ofGetWidth(), ofGetHeight(), OF_PIXELS_RGB);
    processedTexture.allocate(processedScore);

    ofSetVerticalSync(true);
    font.load("Arial.ttf", 20);
    waitTime = 10;
    
    
    int ticksPerBuffer = 8; // 8 * 64 = buffer len of 512
    
    ofSoundStreamSetup(2, 0, this, 44100, ofxPd::blockSize()*ticksPerBuffer, 3);
    if(!pd.init(2, 0, 44100, ticksPerBuffer, false)) {
        OF_EXIT_APP(1);
    }

    
    pd.addReceiver(*this);
    pd.subscribe("toOF");
    pd.start();
    pd.openPatch("sonify.pd");
    mode = Mode::wait;
    
    players.emplace_back(Cell(0,0, ofColor::red));
    players.emplace_back(Cell(0,0, ofColor::blue));
    players.emplace_back(Cell(0,0, ofColor::orange));

    
}



//--------------------------------------------------------------
void ofApp::update(){
    
    switch(mode){
        case Mode::wait:{
            videoCamera.update();
            break;
        }
        case Mode::capture:{
            score = videoCamera.getPixels();
            scoreAnalysis();
            
            
            scoreTexture.loadData(score);
            mode = Mode::transition;
            alpha = 0.0;
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
            processedScore = score;
            processedTexture.loadData(processedScore);
            break;
        }
    }
}

void ofApp::scoreAnalysis(){
    std::vector<float> hue;
    std::vector<float> saturation;
    std::vector<float> brightness;
    std::vector<float> detail;
    
    ofColor prevColor;
    for(int i = 0 ; i < kNumVerticalCells; i++){
        for(int j = 0; j < kNumHorizontalCells; j++){

            int vOffset = kCellSize * i;
            int hOffset = kCellSize * j;
            float avgHue =0.0, avgSat =0.0, avgBrit =0.0, avgDetail = 0.0;
            for(int k = 0; k < kCellSize; k++){
            
                for (int l = 0; l < kCellSize; l++){
                    float hue, saturation, brightness;
//                    ofLog() << ofToString(hOffset+l) + " " << ofToString(vOffset+k);
                    ofColor color = score.getColor(hOffset+l, vOffset+k);
                    color.getHsb(hue, saturation, brightness);
                    avgHue += hue;
                    avgSat += saturation;
                    avgBrit += brightness;
                
                    if(l != 0){
                        int colorDef = abs(color.r - prevColor.r);
                        colorDef += abs(color.g - prevColor.g);
                        colorDef += abs(color.b - prevColor.b);
                        colorDef /= 3;
                        avgDetail += static_cast<float>(colorDef);
                    }
                    prevColor = color;
                }
            }
            avgHue /= 4096;
            avgSat /= 4096;
            avgBrit /= 4096;
            avgDetail /= 4096;
            
            ofLog() << "cell " << ofToString(j) << " " << ofToString(i) << ": " <<
            ofToString(avgHue) << " " << ofToString(avgSat)<< " " << ofToString(avgBrit) << " " << ofToString(avgDetail);
            
            hue.emplace_back(avgHue);
            saturation.emplace_back(avgHue);
            brightness.emplace_back(avgBrit);
            detail.emplace_back(avgDetail);
        }
    }
    
    pd.writeArray("hue", hue);
    pd.writeArray("saturation", saturation);
    pd.writeArray("brightness", brightness);
    pd.writeArray("detail", detail);
}

//--------------------------------------------------------------
void ofApp::draw(){
    switch (mode) {
        case Mode::wait:{
            ofSetColor(ofColor::white);
            videoCamera.draw(0, 0);
            drawTextRegion();
            drawMessage();
            break;
        }
        case Mode::capture:{
            break;
        }
        case Mode::transition:{
            ofSetColor(ofFloatColor(1,1,1,alpha));
            scoreTexture.draw(0, 0);
            break;
        }
        case Mode::sonification:{
            ofSetColor(ofFloatColor(1,1,1,alpha));
            processedTexture.draw(0,0);
            drawMatrix();
            drawPlayer();
            break;
        }
        default:
            break;
    }
}

void ofApp::drawTextRegion(){
    ofSetColor(ofColor(0,0,200,100));
    ofFill();
    ofDrawRectangle(0, ofGetHeight()/2 - 50, ofGetWidth(), 100);
}

void ofApp::drawMatrix(){
    ofSetLineWidth(0.5);
    ofSetColor(ofColor::black);
    int offset = 64;
    for(int i = 0; i < 16; i++){
        ofDrawLine(offset, 0, offset, ofGetHeight());
        offset += 64;
    }
    offset = 64;
    for(int i = 0; i < 12; i++){
        ofDrawLine(0, offset, ofGetWidth(), offset);
        offset += 64;
    }
}

void ofApp::drawPlayer(){
    
    ofNoFill();
    ofSetLineWidth(2);

    for( auto &&player : players){
        ofSetColor(player.color);
        ofDrawRectangle(player.x *kCellSize, player.y*kCellSize, kCellSize, kCellSize);
    }
}

void ofApp::drawMessage(){
    ofSetColor(ofColor::white);

    std::string message = ofToString(waitTime);
    
    float width = font.stringWidth(message);
    font.drawString(message, (ofGetWidth() - width) / 2, ofGetHeight()/2);
}

void ofApp::receiveMessage(const std::string &dest, const std::string &msg, const pd::List &list){
    
    if(msg == "waitTime"){
        waitTime = static_cast<int>(list.getFloat(0));
        if(waitTime == 0){
            mode = Mode::capture;
        }
    }else if(msg == "player"){
        int id = static_cast<int>(list.getFloat(0));
        int x = static_cast<int>(list.getFloat(1));
        int y =static_cast<int>(list.getFloat(2));
        players[id].x = x;
        players[id].y = y;

    }
}

void ofApp::keyPressed(int key){
    
}
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::audioIn(float * input, int bufferSize, int nChannels) {
    pd.audioIn(input, bufferSize, nChannels);
}

//--------------------------------------------------------------
void ofApp::audioOut(float * output, int bufferSize, int nChannels) {
    pd.audioOut(output, bufferSize, nChannels);
}
