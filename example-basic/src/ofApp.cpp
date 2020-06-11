#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetVerticalSync(true);
    ofEnableAlphaBlending();
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    grabber.setDesiredFrameRate(30);
    grabber.initGrabber(640, 480);

    fileName = ofFilePath::getUserHomeDir() + "/Desktop/ofxCamcoderTest.mp4";

    auto devices = soundStream.getDeviceList();
    soundSettings.setInDevice(devices[0]);
    soundSettings.setInListener(this);
    soundSettings.sampleRate = 44100;
    soundSettings.bufferSize = 256;
    soundSettings.numInputChannels = 2;
    soundSettings.numOutputChannels = 0;
    soundStream.setup(soundSettings);
    
    bRecording = false;
    ofAddListener(rec.completeEvent, this, &ofApp::onRecordingComplete);
}

//--------------------------------------------------------------
void ofApp::exit(){
    ofRemoveListener(rec.completeEvent, this, &ofApp::onRecordingComplete);
    rec.close();
}

//--------------------------------------------------------------
void ofApp::update(){
    grabber.update();
    if(grabber.isFrameNew() && bRecording){
        bool success = rec.addFrame(grabber.getPixels());
        if (!success) {
            ofLogWarning("This frame was not added!");
        }
    }

    // Check if the video recam encountered any error while writing video frame or audio smaples.
    if (rec.hasVideoError()) {
        ofLogWarning("The video recam failed to write some frames!");
    }

    if (rec.hasAudioError()) {
        ofLogWarning("The video recam failed to write some audio samples!");
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    grabber.draw(0,0);
    ofDrawBitmapStringHighlight( rec.getInfoString(), 10, 10);


    if(bRecording){
    ofSetColor(255, 0, 0);
    ofDrawCircle(ofGetWidth() - 20, 20, 5);
    }
}

//--------------------------------------------------------------
void ofApp::audioIn( ofSoundBuffer & buffer ){
    if(bRecording) rec.addAudioSamples( buffer );
}

//--------------------------------------------------------------
void ofApp::onRecordingComplete(string & filename) {
    ofLogNotice("example-basic") << "received completed recording event...";
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

    if(key=='r'){
        bRecording = !bRecording;
        if(bRecording && !rec.isInitialized()) {
            rec.setup(fileName, grabber.getWidth(), grabber.getHeight(), 30, soundSettings.sampleRate, soundSettings.numInputChannels);
            rec.start();
        }
        else if(!bRecording && rec.isInitialized()) {
            rec.setPaused(true);
        }
        else if(bRecording && rec.isInitialized()) {
            rec.setPaused(false);
        }
    }
    if(key=='c'){
        bRecording = false;
        ofLog() << "CLOSING";
        rec.close();
        
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){

}
