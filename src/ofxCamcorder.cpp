#include "ofxCamcorder.h"
#include <unistd.h>
#include <fcntl.h>

ofxCamcorder::ofxCamcorder(){
    bIsInitialized = false;
    pixelFormat = "rgb24";
    outputPixelFormat = "";
    ctl.setPtr( this );
}


string ofxCamcorder::getInfoString() {

    
    stringstream ss;
    ss
    << "video recorded: " << ofxCamcorderDefs::humanTimecode((videoFramesRecorded / frameRate)*1000) << endl
    << "audio recorded: " << ofxCamcorderDefs::humanTimecode(((audioSamplesRecorded / frameRate)*1000)/audioBufferSize) << endl
    << "video frames: " << frames.size() << "/" << videoFramesRecorded << endl
    << "audio samples: " << audioFrames.size() << "/" << audioSamplesRecorded << endl
    << "app FPS: " << ofGetFrameRate() << endl
    ;
    return ss.str();
}

//--------------------------------------------------------------
bool ofxCamcorder::setup(string fname, int w, int h, float fps, int sampleRate, int channels, bool sysClockSync, bool silent){
    if(bIsInitialized)
    {
        close();
    }

    fileName = fname;

    stringstream outputSettings;
    outputSettings
    << " -vcodec " << ctl.videoCodec
    << " -pix_fmt yuv420p "
    << " -b " << ctl.videoBitrate
    << " -acodec " << ctl.audioCodec
    << " -ab " << ctl.audioBitrate
    << " \"" << ofFilePath::getAbsolutePath(fileName) << "\"";
    
    ofLogNotice("ofxCamcorder") << "settings \"" << outputSettings.str() << "\" " << fname;
    bool b = setupCustomOutput(w, h, fps, sampleRate, channels, outputSettings.str(), sysClockSync, silent);
    
    
    return b;
}

//--------------------------------------------------------------
bool ofxCamcorder::setupCustomOutput(int w, int h, float fps, string outputString, bool sysClockSync, bool silent){
    return setupCustomOutput(w, h, fps, 0, 0, outputString, sysClockSync, silent);
}

//--------------------------------------------------------------
bool ofxCamcorder::setupCustomOutput(int w, int h, float fps, int sampleRate, int channels, string outputString, bool sysClockSync, bool silent){
    if(bIsInitialized)
    {
        close();
    }

    bIsSilent = silent;
    bSysClockSync = sysClockSync;
    audioBufferSize = 0;

    bRecordAudio = (sampleRate > 0 && channels > 0);
    bRecordVideo = (w > 0 && h > 0 && fps > 0);
    bFinishing = false;

    videoFramesRecorded = 0;
    audioSamplesRecorded = 0;
    
    repeatedFrames = 0;
    skippedFrames = 0;

    if(!bRecordVideo && !bRecordAudio) {
        ofLogWarning() << "ofxCamcorder::setupCustomOutput(): invalid parameters, could not setup video or audio stream.\n"
        << "video: " << w << "x" << h << "@" << fps << "fps\n"
        << "audio: " << "channels: " << channels << " @ " << sampleRate << "Hz\n";
        return false;
    }
    videoPipePath = "";
    audioPipePath = "";
    pipeNumber = requestPipeNumber();
    if(bRecordVideo) {
        width = w;
        height = h;
        frameRate = fps;

        // recording video, create a FIFO pipe
        videoPipePath = ofFilePath::getAbsolutePath("ofxvrpipe" + ofToString(pipeNumber));
        if(!ofFile::doesFileExist(videoPipePath)){
            string cmd = "bash --login -c 'mkfifo " + videoPipePath + "'";
            system(cmd.c_str());
            // TODO: add windows compatable pipe creation (does ffmpeg work with windows pipes?)
        }
    }

    if(bRecordAudio) {
        this->sampleRate = sampleRate;
        audioChannels = channels;

        // recording video, create a FIFO pipe
        audioPipePath = ofFilePath::getAbsolutePath("ofxarpipe" + ofToString(pipeNumber));
        if(!ofFile::doesFileExist(audioPipePath)){
            string cmd = "bash --login -c 'mkfifo " + audioPipePath + "'";
            system(cmd.c_str());

            // TODO: add windows compatable pipe creation (does ffmpeg work with windows pipes?)
        }
    }

    stringstream cmd;
    // basic ffmpeg invocation, -y option overwrites output file
    cmd << "bash --login -c '" << ctl.ffmpeg << (bIsSilent?" -loglevel quiet ":" ") << "-y";
    if(bRecordAudio){
        cmd << " -acodec pcm_s16le -f s16le -ar " << sampleRate << " -ac " << audioChannels << " -i \"" << audioPipePath << "\"";
    }
    else { // no audio stream
        cmd << " -an";
    }
    if(bRecordVideo){ // video input options and file
        cmd << " -r "<< fps << " -s " << w << "x" << h << " -f rawvideo -pix_fmt " << pixelFormat <<" -i \"" << videoPipePath << "\" -r " << fps;
        if (outputPixelFormat.length() > 0)
            cmd << " -pix_fmt " << outputPixelFormat;
    }
    else { // no video stream
        cmd << " -vn";
    }
    cmd << " "+ outputString +"' &";

    // start ffmpeg thread. Ffmpeg will wait for input pipes to be opened.
    ffmpegThread.setup(cmd.str());

    // wait until ffmpeg has started
    while (!ffmpegThread.isInitialized()) {
        usleep(10);
    }

    if(bRecordAudio){
        audioThread.setup(audioPipePath, &audioFrames);
    }
    if(bRecordVideo){
        videoThread.setup(videoPipePath, &frames);
    }

    bIsInitialized = true;
    bIsRecording = false;
    bIsPaused = false;

    startTime = 0;
    recordingDuration = 0;
    totalRecordingDuration = 0;

    return bIsInitialized;
}

//--------------------------------------------------------------
bool ofxCamcorder::addFrame(const ofPixels &pixels){
    if (!bIsRecording || bIsPaused) return false;

    if(bIsInitialized && bRecordVideo && ffmpegThread.isInitialized())
    {
        int framesToAdd = 1; // default add one frame per request

        if((bRecordAudio || bSysClockSync) && !bFinishing){

            double syncDelta;
            double videoRecordedTime = videoFramesRecorded / frameRate;

            if (bRecordAudio) {
                // if also recording audio, check the overall recorded time for audio and video to make sure audio is not going out of sync
                // this also handles incoming dynamic framerate while maintaining desired outgoing framerate
                double audioRecordedTime = (audioSamplesRecorded/audioChannels)  / (double)sampleRate;
                syncDelta = audioRecordedTime - videoRecordedTime;
            }
            else {
                // if just recording video, synchronize the video against the system clock
                // this also handles incoming dynamic framerate while maintaining desired outgoing framerate
                syncDelta = systemClock() - videoRecordedTime;
            }

            int actualFPS = 1.0/syncDelta; 

            if(syncDelta > 1.0/frameRate) {
                // not enought video frames, we need to send extra video frames.
                while(syncDelta > 1.0/frameRate) {
                    framesToAdd++;
                    repeatedFrames += 1;
                    syncDelta -= 1.0/frameRate;
                }
                ofLogVerbose() << "ofxCamcorder: recDelta = " << syncDelta << ". Not enough video frames for desired frame rate, copied this frame " << framesToAdd << " times.\n";
            }
            else if(syncDelta < -1.0/frameRate){
                // more than one video frame is waiting, skip this frame
                framesToAdd = 0;
                skippedFrames += 1;
                ofLogVerbose() << "ofxCamcorder: recDelta = " << syncDelta << ". Too many video frames, skipping.\n";
            }
        }

        for(int i=0;i<framesToAdd;i++){
            // add desired number of frames
            frames.Produce(new ofPixels(pixels));
            videoFramesRecorded++;
        }

        videoThread.signal();

        return true;
    }

    return false;
}

//--------------------------------------------------------------
void ofxCamcorder::addAudioSamples(ofSoundBuffer& buffer) {
    if (!bIsRecording || bIsPaused) return;

    if(bIsInitialized && bRecordAudio){
        int size = buffer.getNumFrames() * buffer.getNumChannels();
        ofxCamcorderDefs::audioFrameShort * shortSamples = new ofxCamcorderDefs::audioFrameShort;
        shortSamples->data = new short[size];
        shortSamples->size = size;

        audioBufferSize = buffer.getNumFrames();
        for(int i=0; i < audioBufferSize; i++){
            for(int j=0; j < buffer.getNumChannels(); j++){
                shortSamples->data[i * buffer.getNumChannels() + j] = (short) (buffer.getSample(i, j) * 32767.0f);
            }
        }
        audioFrames.Produce(shortSamples);
        audioThread.signal();
        audioSamplesRecorded += size;
    }
}

//--------------------------------------------------------------
void ofxCamcorder::start(){
    if(!bIsInitialized) return;

    if (bIsRecording) {
        // We are already recording. No need to go further.
       return;
    }

    // Start a recording.
    bIsRecording = true;
    bIsPaused = false;
    startTime = ofGetElapsedTimef();

    ofLogVerbose() << "Recording." << endl;
}

//--------------------------------------------------------------
void ofxCamcorder::setPaused(bool bPause){
    if(!bIsInitialized) return;

    if (!bIsRecording || bIsPaused == bPause) {
        //  We are not recording or we are already paused. No need to go further.
        return;
    }

    // Pause the recording
    bIsPaused = bPause;

    if (bIsPaused) {
        totalRecordingDuration += recordingDuration;
    } else {
        startTime = ofGetElapsedTimef();
    }
}

//--------------------------------------------------------------
void ofxCamcorder::close(){
    if(!bIsInitialized) return;

    bIsRecording = false;

    if(bRecordVideo && bRecordAudio) {
        // set pipes to non_blocking so we dont get stuck at the final writes
        // audioThread.setPipeNonBlocking();
        // videoThread.setPipeNonBlocking();

        if (frames.size() > 0 && audioFrames.size() > 0) {
            // if there are frames in the queue start a thread to finalize the output file without blocking the app.
            startThread();
            return;
        }
    }
    else if(bRecordVideo) {
        // set pipes to non_blocking so we dont get stuck at the final writes
        // videoThread.setPipeNonBlocking();

        if (frames.size() > 0) {
            // if there are frames in the queue start a thread to finalize the output file without blocking the app.
            startThread();
            return;
        }
        else {
            // cout << "ofxCamcorder :: we are good to go!" << endl;
        }

    }
    else if(bRecordAudio) {
        // set pipes to non_blocking so we dont get stuck at the final writes
        // audioThread.setPipeNonBlocking();

        if (audioFrames.size() > 0) {
            // if there are frames in the queue start a thread to finalize the output file without blocking the app.
            startThread();
            return;
        }
    }

    outputFileComplete();
}

//--------------------------------------------------------------
void ofxCamcorder::threadedFunction()
{
    if(bRecordVideo && bRecordAudio) {
        while(frames.size() > 0 && audioFrames.size() > 0) {
            // if there are frames in the queue or the thread is writing, signal them until the work is done.
            videoThread.signal();
            audioThread.signal();
        }
    }
    else if(bRecordVideo) {
        while(frames.size() > 0) {
            // if there are frames in the queue or the thread is writing, signal them until the work is done.
            videoThread.signal();
        }
    }
    else if(bRecordAudio) {
        while(audioFrames.size() > 0) {
            // if there are frames in the queue or the thread is writing, signal them until the work is done.
            audioThread.signal();
        }
    }

    waitForThread();

    outputFileComplete();
}

//--------------------------------------------------------------
void ofxCamcorder::outputFileComplete()
{
    // at this point all data that ffmpeg wants should have been consumed
    // one of the threads may still be trying to write a frame,
    // but once close() gets called they will exit the non_blocking write loop
    // and hopefully close successfully

    bIsInitialized = false;

    if (bRecordVideo) {
        videoThread.close();
    }
    if (bRecordAudio) {
        audioThread.close();
    }

    retirePipeNumber(pipeNumber);

    ffmpegThread.waitForThread();
    // TODO: kill ffmpeg process if its taking too long to close for whatever reason.

    ofNotifyEvent(completeEvent, fileName);
}

//--------------------------------------------------------------
bool ofxCamcorder::hasVideoError(){
    return videoThread.bNotifyError;
}

//--------------------------------------------------------------
bool ofxCamcorder::hasAudioError(){
    return audioThread.bNotifyError;
}

//--------------------------------------------------------------
float ofxCamcorder::systemClock(){
    recordingDuration = ofGetElapsedTimef() - startTime;
    return totalRecordingDuration + recordingDuration;
}

//--------------------------------------------------------------
set<int> ofxCamcorder::openPipes;

//--------------------------------------------------------------
int ofxCamcorder::requestPipeNumber(){
    int n = 0;
    while (openPipes.find(n) != openPipes.end()) {
        n++;
    }
    openPipes.insert(n);
    return n;
}

//--------------------------------------------------------------
void ofxCamcorder::retirePipeNumber(int num){
    if(!openPipes.erase(num)){
        ofLogNotice() << "ofxCamcorder::retirePipeNumber(): trying to retire a pipe number that is not being tracked: " << num << endl;
    }
}
