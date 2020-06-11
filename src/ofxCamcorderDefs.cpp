#include "ofxCamcorderDefs.h"





ofxCamcorderDefs::execThread::execThread(){
    execCommand = "";
    initialized = false;
}

//--------------------------------------------------------------
void ofxCamcorderDefs::execThread::setup(string command){
    execCommand = command;
    initialized = false;
    startThread(true);
}

//--------------------------------------------------------------
void ofxCamcorderDefs::execThread::threadedFunction(){
    if(isThreadRunning()){
        ofLogVerbose("execThread") << "starting command: " <<  execCommand;
        int result = system(execCommand.c_str());
        if (result == 0) {
            ofLogVerbose("execThread") << "command completed successfully.";
            initialized = true;
        } else {
            ofLogError("execThread") << "command failed with result: " << result;
        }
    }
}

