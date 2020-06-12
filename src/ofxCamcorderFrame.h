#pragma once
#include "ofMain.h"
#include "ofxCamcorderDefs.h"
#include "Poco/Condition.h"
#include <set>

// new
//#include <unistd.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>



class ofxCamcorderFrame : public ofThread {
public:
    ofxCamcorderFrame();
    void setup(string filePath, ofxCamcorderDefs::lockFreeQueue<ofPixels *> * q);
    void threadedFunction();
    void signal();
    void setPipeNonBlocking();
    bool isWriting() { return bIsWriting; }
    void close() {
        bClose = true;
        stopThread();
        signal();
    }
    bool bNotifyError;
private:
    ofMutex conditionMutex;
    Poco::Condition condition;
    string filePath;
    int fd;
    ofxCamcorderDefs::lockFreeQueue<ofPixels *> * queue;
    bool bIsWriting;
    bool bClose;
};
