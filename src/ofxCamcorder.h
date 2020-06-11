#pragma once

#include "ofMain.h"
#include "Poco/Condition.h"
#include <set>

#include "ofxCamcorderDefs.h"
#include "ofxCamcorderFrame.h"
#include "ofxCamcorderAudio.h"
#include "ofxCamcorderCtl.h"



class ofxCamcorder  : public ofThread
{
public:
    ofxCamcorder();
    ofxCamcorderCtl ctl;

    void threadedFunction();

    ofEvent<string> completeEvent;
    
    int _audioChannels, _audioSampleRate, _audioBufferSize;
    string humanCmd;

    bool setup(string fname, int w, int h, float fps, ofSoundStreamSettings & soundSettings, bool sysClockSync=false, bool silent=false);
    bool setupCustomOutput(int w, int h, float fps, string outputString, bool sysClockSync=false, bool silent=false);

    bool addFrame(const ofPixels &pixels);
    void addAudioSamples(ofSoundBuffer& buffer);

    void start();
    void close();
    void setPaused(bool bPause);

    bool hasVideoError();
    bool hasAudioError();

    void setPixelFormat( string pixelF){ //rgb24 || gray, default is rgb24
        pixelFormat = pixelF;
    };
    void setOutputPixelFormat(string pixelF) {
        outputPixelFormat = pixelF;
    }

    unsigned long long getNumVideoFramesRecorded() { return videoFramesRecorded; }
    unsigned long long getNumAudioSamplesRecorded() { return audioSamplesRecorded; }

    bool isInitialized(){ return bIsInitialized; }
    bool isRecording() { return bIsRecording; };
    bool isPaused() { return bIsPaused; };
    bool isSyncAgainstSysClock() { return bSysClockSync; };

    int getWidth(){return width;}
    int getHeight(){return height;}
    
    
    string getInfoString();

private:

    // internal variables...
    
    string fileName;
    string videoPipePath, audioPipePath;
    string pixelFormat, outputPixelFormat;


    // internal variables...

    int width, height;
    float frameRate;
    
    // info variables...
    
    int repeatedFrames, skippedFrames, actualFPS;


    // internal status...


    bool bIsInitialized;
    bool bRecordAudio;
    bool bRecordVideo;
    bool bIsRecording;
    bool bIsPaused;
    bool bFinishing;
    bool bIsSilent;

    bool bSysClockSync;
    float startTime;
    float recordingDuration;
    float totalRecordingDuration;
    float systemClock();

    ofxCamcorderDefs::lockFreeQueue<ofPixels *> frames;
    ofxCamcorderDefs::lockFreeQueue<ofxCamcorderDefs::audioFrameShort *> audioFrames;
    unsigned long long audioSamplesRecorded;
    unsigned long long videoFramesRecorded;
    ofxCamcorderFrame videoThread;
    ofxCamcorderAudio audioThread;
    
    ofxCamcorderDefs::execThread ffmpegThread;
    
    int videoPipeFd, audioPipeFd;
    int pipeNumber;

    static set<int> openPipes;
    static int requestPipeNumber();
    static void retirePipeNumber(int num);

    void outputFileComplete();
    
};
