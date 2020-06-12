#include "ofxCamcorderAudio.h"
//
//#include <unistd.h>
//#include <fcntl.h>
//#include <set>
//#include <sys/ioctl.h>


ofxCamcorderAudio::ofxCamcorderAudio(){
}

//--------------------------------------------------------------
void ofxCamcorderAudio::setup(string filePath, ofxCamcorderDefs::lockFreeQueue<ofxCamcorderDefs::audioFrameShort *> *q) {
    
    this->filePath = filePath;
    fd = -1;
    queue = q;
    bIsWriting = false;
    bNotifyError = false;
    startThread(true);
    
}

//--------------------------------------------------------------
void ofxCamcorderAudio::threadedFunction(){
    if(fd == -1){
        ofLogVerbose("ofxCamcorderAudio") << "opening pipe: " <<  filePath;
        fd = ::open(filePath.c_str(), O_WRONLY);
        ofLogWarning("ofxVideoDataWriterThread") << "got file descriptor " << fd;
    }

    while(isThreadRunning())
    {
        ofxCamcorderDefs::audioFrameShort * frame = NULL;
        if(queue->Consume(frame) && frame){
            bIsWriting = true;
            int b_offset = 0;
            int b_remaining = frame->size*sizeof(short);
            while(b_remaining > 0 && isThreadRunning()){
                int b_written = ::write(fd, ((char *)frame->data)+b_offset, b_remaining);

                if(b_written > 0){
                    b_remaining -= b_written;
                    b_offset += b_written;
                }
                else if (b_written < 0) {
                    ofLogError("ofxCamcorderAudio") << ofGetTimestampString("%H:%M:%S:%i") << " - write to PIPE failed with error -> " << errno << " - " << strerror(errno) << ".";
                    bNotifyError = true;
                    break;
                }
                else {
                    if(bClose){
                        // quit writing so we can close the file
                        break;
                    }
                }

                if (!isThreadRunning()) {
                    ofLogWarning("ofxCamcorderAudio") << ofGetTimestampString("%H:%M:%S:%i") << " - The thread is not running anymore let's get out of here!";
                }
            }
            bIsWriting = false;
            delete [] frame->data;
            delete frame;
        }
        else{
            condition.wait(conditionMutex);
        }
    }

    ofLogVerbose("ofxCamcorderAudio") << "closing pipe: " <<  filePath;
    ::close(fd);
}

//--------------------------------------------------------------
void ofxCamcorderAudio::signal(){
    condition.signal();
}

//--------------------------------------------------------------
void ofxCamcorderAudio::setPipeNonBlocking(){
    ofxCamcorderDefs::setNonBlocking(fd);
}
