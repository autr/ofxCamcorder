#include "ofxCamcorderFrame.h"

ofxCamcorderFrame::ofxCamcorderFrame(){
}

//--------------------------------------------------------------
void ofxCamcorderFrame::setup(string filePath, ofxCamcorderDefs::lockFreeQueue<ofPixels *> * q){
    this->filePath = filePath;
    fd = -1;
    queue = q;
    bIsWriting = false;
    bClose = false;
    bNotifyError = false;
    startThread(true);
}

//--------------------------------------------------------------
void ofxCamcorderFrame::threadedFunction(){
    if(fd == -1){
        ofLogVerbose("ofxCamcorderFrame") << "opening pipe: " <<  filePath;
        fd = ::open(filePath.c_str(), O_WRONLY);
        ofLogWarning("ofxCamcorderFrame") << "got file descriptor " << fd;
    }

    while(isThreadRunning())
    {
        ofPixels * frame = NULL;
        if(queue->Consume(frame) && frame){
            bIsWriting = true;
            int b_offset = 0;
            int b_remaining = frame->getWidth()*frame->getHeight()*frame->getBytesPerPixel();

            while(b_remaining > 0 && isThreadRunning())
            {
                errno = 0;

                int b_written = ::write(fd, ((char *)frame->getData())+b_offset, b_remaining);

                if(b_written > 0){
                    b_remaining -= b_written;
                    b_offset += b_written;
                    if (b_remaining != 0) {
                        ofLogWarning("ofxCamcorderFrame") << ofGetTimestampString("%H:%M:%S:%i") << " - b_remaining is not 0 -> " << b_written << " - " << b_remaining << " - " << b_offset << ".";
                        // break;
                    }
                }
                else if (b_written < 0) {
                    ofLogError("ofxCamcorderFrame") << ofGetTimestampString("%H:%M:%S:%i") << " - write to PIPE failed with error -> " << errno << " - " << strerror(errno) << ".";
                    bNotifyError = true;
                    break;
                }
                else {
                    if(bClose){
                        ofLogVerbose("ofxCamcorderFrame") << ofGetTimestampString("%H:%M:%S:%i") << " - Nothing was written and bClose is TRUE.";
                        break; // quit writing so we can close the file
                    }
                    ofLogWarning("ofxCamcorderFrame") << ofGetTimestampString("%H:%M:%S:%i") << " - Nothing was written. Is this normal?";
                }

                if (!isThreadRunning()) {
                    ofLogWarning("ofxCamcorderFrame") << ofGetTimestampString("%H:%M:%S:%i") << " - The thread is not running anymore let's get out of here!";
                }
            }
            bIsWriting = false;
            frame->clear();
            delete frame;
        }
        else{
            condition.wait(conditionMutex);
        }
    }

    ofLogVerbose("ofxCamcorderFrame") << "closing pipe: " <<  filePath;
    ::close(fd);
}

//--------------------------------------------------------------
void ofxCamcorderFrame::signal(){
    condition.signal();
}

//--------------------------------------------------------------
void ofxCamcorderFrame::setPipeNonBlocking(){
    ofxCamcorderDefs::setNonBlocking(fd);
}
