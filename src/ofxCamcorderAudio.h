#pragma once
#include "ofMain.h"
#include "ofxCamcorderDefs.h"
#include "Poco/Condition.h"
#include <set>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>



class ofxCamcorderAudio : public ofThread {
public:
    ofxCamcorderAudio();
    void setup(string filePath, ofxCamcorderDefs::lockFreeQueue<ofxCamcorderDefs::audioFrameShort *> * q);
    void threadedFunction();
    void signal();
    void setPipeNonBlocking();
    bool isWriting() { return bIsWriting; }
    void close() { bClose = true; stopThread(); signal();  }
    bool bNotifyError;
private:
    ofMutex conditionMutex;
    Poco::Condition condition;
    string filePath;
    int fd;
    ofxCamcorderDefs::lockFreeQueue<ofxCamcorderDefs::audioFrameShort *> * queue;
    bool bIsWriting;
    bool bClose;
};

