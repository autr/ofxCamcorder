#pragma once
#include "ofMain.h"

class ofxCamcorder;

class ofxCamcorderCtl {
public:
    ofParameterGroup ui;

    ofParameter<bool> record;
    ofParameter<string> filePath;
    ofParameter<string> fileName;

    ofParameter<int> width;
    ofParameter<int> height;
    ofParameter<int> fps;

    ofParameter<bool> sync; // system clock sync
    ofParameter<bool> silent;

    ofParameter<string> ffmpeg; // ffmpeg location

    ofParameter<string> videoCodec;
    ofParameter<string> videoBitrate;

    ofParameter<bool> useAudio;
    ofParameter<string> audioCodec;
    ofParameter<string> audioBitrate;
    ofParameter<int> audioChannels;

    ofxCamcorder * ptr = nullptr;

    ofxCamcorderCtl();
    void setPtr( ofxCamcorder * ptr_ );
};
