#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetVerticalSync(true);
    ofEnableAlphaBlending();
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    grabber.setDesiredFrameRate(30);
    grabber.initGrabber(1080, 1080);


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
    ofAddListener(rec.errorEvent, this, &ofApp::onRecordingError);
    
    rec.ctl.outputDir = ofFilePath::getUserHomeDir() + "/Desktop/";
    rec.ctl.fileName = "ofxCamcoderTest.mp4";
    play.load(ofFilePath::getUserHomeDir() + "/Desktop/" + "ofxCamcoderTest.mp4");
    play.play();
    gui.setup( rec.getGui() );
    gui.setPosition( 700, 120 );
}

//--------------------------------------------------------------
void ofApp::exit(){
    ofRemoveListener(rec.completeEvent, this, &ofApp::onRecordingComplete);
    rec.close();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (play.isLoaded() && !bRecording ) play.update();
    if (bRecording) {
        grabber.update();
        if(grabber.isFrameNew() && bRecording){
            bool success = rec.addFrame(grabber.getPixels());
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofSetColor(255);
    if ( bRecording ) grabber.draw(120,40);
    ofDrawBitmapStringHighlight( rec.getInfoString(), 10, 10);


    if(bRecording){
    ofSetColor(255, 0, 0);
    ofDrawCircle(ofGetWidth() - 20, 20, 5);
    }
    
    if (play.isLoaded() && !bRecording ) play.draw(10, ofGetHeight()-250,320, 240);
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::audioIn( ofSoundBuffer & buffer ){
    if( bRecording ) rec.addAudioSamples( buffer );
}

//--------------------------------------------------------------
void ofApp::onRecordingComplete(string & filename) {
    ofLogNotice("example-basic") << "received completed recording event..." << filename;
    play.load(filename);
    play.play();
}
//--------------------------------------------------------------
void ofApp::onRecordingError(string & error) {
    ofLogError("example-basic") << "received recording error..." << error;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

    if(key=='r'){
        bRecording = !bRecording;
        if(bRecording && !rec.isInitialized()) {
            play.close();
            rec.setup(fileName, grabber.getWidth(), grabber.getHeight(), soundSettings);
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
