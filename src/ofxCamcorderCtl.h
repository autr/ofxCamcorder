#pragma once
#include "ofMain.h"

class ofxCamcorder;

class ofxCamcorderCtl {
public:
    ofParameterGroup ui;

    ofParameter<string> libraryPath; // ffmpeg location
    ofParameter<bool> record;
    ofParameter<bool> silent;
    ofParameter<string> outputDir;
    ofParameter<string> fileName;
    
    ofParameter<string> inputFormat;
    ofParameter<string> outputFormat;
    

    ofParameter<bool> useVideo;
    ofParameter<string> videoCodec;
    ofParameter<string> videoBitrate;
    ofParameter<int> width;
    ofParameter<int> height;
    ofParameter<int> fps;
    ofParameter<bool> sync; // system clock sync


    ofParameter<bool> useAudio;
    ofParameter<string> audioCodec;
    ofParameter<string> audioBitrate;
    ofParameter<int> audioChannels;

    ofxCamcorder * ptr = nullptr;

    ofxCamcorderCtl();
    void setPtr( ofxCamcorder * ptr_ );
};
