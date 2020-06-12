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
    ~ofxCamcorder() { close(); }
    ofxCamcorderCtl ctl;

    void threadedFunction();
    
    ofEvent<string> completeEvent;
    ofEvent<string> errorEvent;
    

    bool setup(string fname, int w, int h, ofSoundStreamSettings & soundSettings);
    bool setupCustomOutput(int w, int h, string outputString);

    bool addFrame(const ofPixels &pixels);
    void addAudioSamples(ofSoundBuffer& buffer);

    void start();
    void close();
    void setPaused(bool bPause);

    bool hasVideoError();
    bool hasAudioError();

    unsigned long long getNumVideoFramesRecorded() { return videoFramesRecorded; }
    unsigned long long getNumAudioSamplesRecorded() { return audioSamplesRecorded; }

    bool isInitialized(){ return bIsInitialized; }
    bool isRecording() { return bIsRecording; };
    bool isPaused() { return bIsPaused; };

    int getWidth(){return width;}
    int getHeight(){return height;}
    
    
    string getInfoString();
    
    ofParameterGroup & getGui() { return ctl.ui; }
    void setOutputDirectory( string dir ) { ctl.outputDir = dir; }
    void setOutputFilename( string name ) { ctl.fileName = name; }

private:

    // GUI / external variables (locked during setup)...
    
    string _fileName;
    string _fullPath;
    bool _bIsSilent;
    bool _bSysClockSync;
    int _audioChannels, _audioSampleRate, _audioBufferSize;
    float _fps;
    string _inputFormat, _outputFormat;
    string _videoPipePath, _audioPipePath;
    
    
    string humanCmd;
    bool closeOnError;
    


    // internal variables...

    int width, height;
    
    // info variables...
    
    int repeatedFrames, skippedFrames, actualFPS;


    // internal status...


    bool bIsInitialized;
    bool bRecordAudio;
    bool bRecordVideo;
    bool bIsRecording;
    bool bIsPaused;
    bool bFinishing;
    

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
    
};
