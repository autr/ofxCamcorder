#pragma once
#include "ofMain.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

namespace ofxCamcorderDefs {

    static string humanTimecode(int millis) {
        
        if (millis <= 0) return "00:00:00.0";
        int milliseconds = (int)((millis%1000)/100)
            , seconds = (int)((millis/1000)%60)
            , minutes = (int)((millis/(1000*60))%60)
            , hours = (int)((millis/(1000*60*60))%24);

        string hoursStr = (hours < 10) ? "0" + ofToString(hours) : ofToString(hours);
        string minutesStr = (minutes < 10) ? "0" + ofToString(minutes) : ofToString(minutes);
        string secondsStr = (seconds < 10) ? "0" + ofToString(seconds) : ofToString(seconds);

        return hoursStr + ":" + minutesStr + ":" + secondsStr + "." + ofToString(milliseconds);
    };

    static int setNonBlocking(int fd){
        int flags;

        /* If they have O_NONBLOCK, use the Posix way to do it */
    #if defined(O_NONBLOCK)
        /* Fixme: O_NONBLOCK is defined but broken on SunOS 4.1.x and AIX 3.2.5. */
        if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
            flags = 0;
        return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    #else
        /* Otherwise, use the old way of doing it */
        flags = 1;
        return ioctl(fd, FIOBIO, &flags);
    #endif
    }

    class execThread : public ofThread{
    public:
        execThread();
        void setup(string command);
        void threadedFunction();
        bool isInitialized() { return initialized; }
    private:
        string execCommand;
        bool initialized;
    };

    struct audioFrameShort {
        short * data;
        int size;
    };

    template <typename T>
    struct lockFreeQueue {
        lockFreeQueue(){
            list.push_back(T());
            iHead = list.begin();
            iTail = list.end();
        }
        void Produce(const T& t){
            list.push_back(t);
            iTail = list.end();
            list.erase(list.begin(), iHead);
        }
        bool Consume(T& t){
            typename TList::iterator iNext = iHead;
            ++iNext;
            if (iNext != iTail)
            {
                iHead = iNext;
                t = *iHead;
                return true;
            }
            return false;
        }
        int size() { return distance(iHead,iTail)-1; }
        typename std::list<T>::iterator getHead() { return iHead; }
        typename std::list<T>::iterator getTail() { return iTail; }


    private:
        typedef std::list<T> TList;
        TList list;
        typename TList::iterator iHead, iTail;
    };


};


