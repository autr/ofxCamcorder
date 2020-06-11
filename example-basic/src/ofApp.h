#pragma once

#include "ofMain.h"
#include "ofxCamcorder.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

public:
    void setup();
    void update();
    void draw();
    void exit();

    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void audioIn(ofSoundBuffer& buffer);

    ofSoundStreamSettings soundSettings;
    ofSoundStream soundStream;
    
    ofVideoPlayer play;
    ofVideoGrabber grabber;
    ofxCamcorder rec;
    
    string fileName;
    bool bRecording;

    void onRecordingComplete(string & filename);

    ofFbo recordFbo;
    ofPixels recordPixels;
    
    ofxPanel gui;
};
