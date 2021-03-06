#include "ofxCamcorder.h"
#include <unistd.h>
#include <fcntl.h>

ofxCamcorder::ofxCamcorder(){
    bIsInitialized = false;
    ctl.setPtr( this );
    
    ofAddListener( ctl.ui.parameterChangedE(), this, &ofxCamcorder::onParametersChanged);
}


string ofxCamcorder::getInfoString() {
    
    int videoMillis = videoFramesRecorded / (int)ctl.fps * 1000;
    int audioMillis = (_audioChannels > 0) ? (float)audioSamplesRecorded / (float)_audioSampleRate / (float)_audioChannels * 1000 : 0;
    
    stringstream ss;
    ss
    << "-------------------------------" << endl
    << "video recorded: " << ofxCamcorderDefs::humanTimecode( videoMillis ) << endl
    << "audio recorded: " << ofxCamcorderDefs::humanTimecode( audioMillis ) << endl
    << "-------------------------------" << endl
    << "video frames: "  << videoFramesRecorded << " (" << frames.size() << " buffered)" << endl
    << "skipped / repeated: " << skippedFrames << " / " << repeatedFrames << endl
    << "video settings: " << (string)ctl.videoCodec << "/" << (string)ctl.videoBitrate << " " << width << "/" << height << "/" << (int)ctl.fps << endl
    << "-------------------------------" << endl
    << "audio samples: " << audioSamplesRecorded << " (" << audioFrames.size() << " buffered)" << endl
    << "audio settings: "
        << _audioSampleRate << "/" << _audioBufferSize << "/" << _audioChannels << " "
        << (string)ctl.audioCodec << "/" << (string)ctl.audioBitrate << endl
    << "-------------------------------" << endl
    << "app FPS: " << (int)ofGetFrameRate() << endl
    << "target FPS: " << (int)ctl.fps << endl
    << "input FPS: " << (int)inputFPS << endl
    << "actual FPS: " << (int)actualFPS << endl
    << "-------------------------------" << endl
    << humanCmd << endl;
    return ss.str();
}



void ofxCamcorder::onParametersChanged( ofAbstractParameter & param ) {
    
}


string ofxCamcorder::getOutputCmdString() {

    _fps = (int)ctl.fps;
    
    _inputFormat = (string)ctl.inputFormat;
    _outputFormat = (string)ctl.outputFormat;

    _fileName = (string)ctl.outputDir + (string)ctl.fileName;
    _fullPath = ofFilePath::getAbsolutePath(_fileName);
    _videoCodec = ctl.videoCodec;
    
    stringstream cmd;
    cmd
    << " -vcodec " << ctl.videoCodec
    << " -pix_fmt yuv420p "
    << " -b " << ctl.videoBitrate
    << " -acodec " << ctl.audioCodec
    << " -ab " << ctl.audioBitrate;

        // overwrite!!!
    
    if ((int)ctl.preset > 0) {
        ofLogNotice("ofxCamcorder") << "using preset..." << endl;
        if ((int)ctl.preset == 1) {

                cmd.str(std::string()); // clear
                cmd << " -preset ultrafast -crf 0 ";
        }
    }
    cmd << " \"" << _fullPath << "\"";
        
        
    //    /* fast mpeg, crf = constant rate factor */
    //
    //    outputSettings.str(std::string()); // clear
    //    outputSettings << " -preset ultrafast -crf 27 ";
    //    outputSettings << " \"" << _fullPath << "\"";
//    //
//        /* test.avi */
//
//        outputSettings.str(std::string()); // clear
//        outputSettings << " \"" << _fullPath << ".avi" << "\"";
    
    return cmd.str();

}


//--------------------------------------------------------------
bool ofxCamcorder::setup(string fname, int w, int h, ofSoundStreamSettings & soundSettings){
    if(bIsInitialized) {
        close();
    }
    
    closeOnError = false;
    
    _audioBufferSize = soundSettings.bufferSize;
    _audioChannels = soundSettings.numInputChannels;
    _audioSampleRate = soundSettings.sampleRate;
    
    _bIsSilent = (bool)ctl.silent;
    _bSysClockSync = (bool)ctl.sync;
    _bRecordAudio = (bool)ctl.useAudio;
    _bRecordVideo = (bool)ctl.useVideo;
    
    
    _outputCmdStr = getOutputCmdString();
    
    ofLogNotice("ofxCamcorder") << "output command str... \"" << _outputCmdStr << "\" " << fname;
    
    bool b = setupCustomOutput(w, h);
    return b;
}

//--------------------------------------------------------------
bool ofxCamcorder::setupCustomOutput(int w, int h){
    if(bIsInitialized)
    {
        close();
    }


    bFinishing = false;

    videoFramesRecorded = 0;
    audioSamplesRecorded = 0;
    
    repeatedFrames = 0;
    skippedFrames = 0;

    _videoPipePath = "";
    _audioPipePath = "";
    pipeNumber = requestPipeNumber();
    
    string videoPipeStr = (string)ctl.pipeDir + "ofxvrpipe" + ofToString(pipeNumber);
    
    if(_bRecordVideo) {
        width = w;
        height = h;

        // recording video, create a FIFO pipe
        _videoPipePath = ofFilePath::getAbsolutePath(videoPipeStr);
        if(!ofFile::doesFileExist(_videoPipePath)){
            string cmd = "bash --login -c 'mkfifo " + _videoPipePath + "'";
            system(cmd.c_str());
            // TODO: add windows compatable pipe creation (does ffmpeg work with windows pipes?)
        }
    }
    
    string audioPipeStr = (string)ctl.pipeDir + "ofxarpipe" + ofToString(pipeNumber);
    
    if(_bRecordAudio) {

        // recording video, create a FIFO pipe
        _audioPipePath = ofFilePath::getAbsolutePath(audioPipeStr);
        if(!ofFile::doesFileExist(_audioPipePath)){
            string cmd = "bash --login -c 'mkfifo " + _audioPipePath + "'";
            system(cmd.c_str());

            // TODO: add windows compatable pipe creation (does ffmpeg work with windows pipes?)
        }
    }
    
    // basic ffmpeg invocation, -y option overwrites output file
    stringstream cmd;
    cmd << "bash --login -c '" << ctl.libraryPath << (_bIsSilent?" -loglevel quiet ":" ") << "-y";
    if(_bRecordAudio){
        cmd << " -acodec pcm_s16le -f s16le -ar " << _audioSampleRate << " -ac " << _audioChannels << " -i \"" << _audioPipePath << "\"";
    }
    else { // no audio stream
        cmd << " -an";
    }
    if(_bRecordVideo){ // video input options and file
        cmd << " -r "<< _fps << " -s " << w << "x" << h << " -f rawvideo -pix_fmt " << _inputFormat <<" -i \"" << _videoPipePath << "\" -r " << _fps;
        if (_outputFormat.length() > 0)
            cmd << " -pix_fmt " << _outputFormat;
    }
    else { // no video stream
        cmd << " -vn";
    }
    cmd << " "+ _outputCmdStr +"' &";
    
    // human readable...
    
    string human = cmd.str();
    human = human.substr(17, human.size()-3-17);
    ofStringReplace(human, "\""+_fullPath+"\"", "\n<outputPath>");
    ofStringReplace(human, "\""+_videoPipePath+"\"", "<"+videoPipeStr+">");
    ofStringReplace(human, "\""+_audioPipePath+"\"", "<"+audioPipeStr+">");
    
    
    int i = 0;
    humanCmd = "";
    for(auto & flag : ofSplitString(human, "-")) humanCmd += ((i++==0) ? "" : "-") + flag + "\n";
    
    ofLogNotice("ofxCamcorder")
    << "sending cmd string..." << endl
    << "-------------------------------" << endl
    << cmd.str() << endl
    << "-------------------------------" << endl
    << humanCmd << endl
    << "-------------------------------" << endl;

    // start ffmpeg thread. Ffmpeg will wait for input pipes to be opened.
    ffmpegThread.setup(cmd.str());

    // wait until ffmpeg has started
    while (!ffmpegThread.isInitialized()) {
        usleep(10);
    }

    if(_bRecordAudio){
        audioThread.setup(_audioPipePath, &audioFrames);
    }
    if(_bRecordVideo){
        videoThread.setup(_videoPipePath, &frames);
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
    
    float t = ofGetElapsedTimef();
    inputFPS =  1.0/((float)t-(float)inputTimestamp);
    inputTimestamp = t;
    
    if (hasVideoError()) {
        ofLogError("ofxCamcorder") << "closing after video error...";
        bIsRecording = false;
        closeOnError = true;
        videoThread.close();
        audioThread.close();
        close();
    }

    if(bIsInitialized && _bRecordVideo && ffmpegThread.isInitialized()) {
        int framesToAdd = 1; // default add one frame per request

        if((_bRecordAudio || _bSysClockSync) && !bFinishing){

            double syncDelta;
            double videoRecordedTime = videoFramesRecorded / _fps;

            if (_bRecordAudio) {
                // if also recording audio, check the overall recorded time for audio and video to make sure audio is not going out of sync
                // this also handles incoming dynamic _fps while maintaining desired outgoing _fps
                double audioRecordedTime = (audioSamplesRecorded/_audioChannels)  / (double)_audioSampleRate;
                syncDelta = audioRecordedTime - videoRecordedTime;
            } else {
                // if just recording video, synchronize the video against the system clock
                // this also handles incoming dynamic _fps while maintaining desired outgoing _fps
                syncDelta = systemClock() - videoRecordedTime;
            }

            actualFPS = 1.0/(float)syncDelta;

            if(syncDelta > 1.0/_fps) {
                // not enought video frames, we need to send extra video frames.
                while(syncDelta > 1.0/_fps) {
                    framesToAdd++;
                    repeatedFrames += 1;
                    syncDelta -= 1.0/_fps;
                }
//                ofLogVerbose() << "ofxCamcorder: recDelta = " << syncDelta << ". Not enough video frames for desired frame rate, copied this frame " << framesToAdd << " times.\n";
            }
            else if(syncDelta < -1.0/_fps){
                // more than one video frame is waiting, skip this frame
                framesToAdd = 0;
                skippedFrames += 1;
//                ofLogVerbose() << "ofxCamcorder: recDelta = " << syncDelta << ". Too many video frames, skipping.\n";
            }
        } else {
            actualFPS = inputFPS;
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
    if (hasAudioError()) {
        ofLogError("ofxCamcorder") << "closing after audio error...";
        bIsRecording = false;
        closeOnError = true;
        videoThread.close();
        audioThread.close();
        close();
    }

    if(bIsInitialized && _bRecordAudio){
        int size = buffer.getNumFrames() * buffer.getNumChannels();
        ofxCamcorderDefs::audioFrameShort * shortSamples = new ofxCamcorderDefs::audioFrameShort;
        shortSamples->data = new short[size];
        shortSamples->size = size;

        for(int i=0; i < buffer.getNumFrames(); i++){
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
    ofLogNotice("ofxCamcorder") << "closing...";
    bIsRecording = false;
    startThread();
    waitForThread();
}

//--------------------------------------------------------------
void ofxCamcorder::threadedFunction()
{
    
    // cleanup function...
    
    if(_bRecordVideo && _bRecordAudio) {
        while(frames.size() > 0 && audioFrames.size() > 0) {
            // if there are frames in the queue or the thread is writing, signal them until the work is done.
            videoThread.signal();
            audioThread.signal();
        }
    }
    else if(_bRecordVideo) {
        while(frames.size() > 0) {
            // if there are frames in the queue or the thread is writing, signal them until the work is done.
            videoThread.signal();
        }
    }
    else if(_bRecordAudio) {
        while(audioFrames.size() > 0) {
            // if there are frames in the queue or the thread is writing, signal them until the work is done.
            audioThread.signal();
        }
    }

    waitForThread();
    
    bIsInitialized = false;

    if (_bRecordVideo) {
        videoThread.close();
    }
    if (_bRecordAudio) {
        audioThread.close();
    }

    retirePipeNumber(pipeNumber);

    ffmpegThread.waitForThread();
    
    // wait before signalling (in case something tries to load the file)...
    
    sleep(1500);
    if (closeOnError) {
        string evt = "error recording...";
        ofNotifyEvent(errorEvent, evt);
    } else {
        ofNotifyEvent(completeEvent, _fileName);
    }
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
