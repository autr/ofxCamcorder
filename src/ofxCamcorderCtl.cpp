#include "ofxCamcorderCtl.h"

ofxCamcorderCtl::ofxCamcorderCtl() {

    ui.add( libraryPath.set( "libraryPath", "ffmpeg" )); // ffmpeg location
    

#ifdef TARGET_RASPBERRY_PI
    ui.add( pipeDir.set("pipeDir", "/usr/ramDisk/") );
#else
    ui.add( pipeDir.set("pipeDir", "") );
#endif
    ui.add( record.set("record", false) );
    ui.add( silent.set("silent", false) );

    ui.add( outputDir.set("outputDir", ofFilePath::getUserHomeDir() + "/Desktop/") );
    ui.add( fileName.set("fileName", "Video.mp4") );
    
    ui.add( preset.set("preset", 0, 1, 2) ); // or gray
    ui.add( inputFormat.set("inputFormat", "rgb24") ); // or gray
    
    // output format is important for osx...
    
    #if defined(TARGET_OSX)
        ui.add( outputFormat.set("outputFormat", "yuv420p") );
    #else
        ui.add( outputFormat.set("outputFormat", "") );
    #endif

    ui.add( useVideo.set("useVideo", true) );
#ifdef TARGET_RASPBERRY_PI
    ui.add( videoCodec.set( "videoCodec", "h264_omx" )); // libx264, mpeg
#else
    ui.add( videoCodec.set( "videoCodec", "libx264" )); // libx264, mpeg
    
#endif
    ui.add( videoBitrate.set( "videoBitrate", "2000k" ));
    ui.add( width.set( "width", 1280, 0, 1920 ));
    ui.add( height.set( "height", 720, 0, 1080 ));
    ui.add( fps.set( "fps", 30, 10, 120 ));
    ui.add( sync.set( "syncFPS", false )); // system clock sync

    ui.add( useAudio.set( "useAudio", false ));
    ui.add( audioCodec.set( "audioCodec", "mp3" ));
    ui.add( audioBitrate.set( "audioBitrate", "192k" ));
    ui.add( audioChannels.set( "audioChannels", 2, 1, 4 ));
}


void ofxCamcorderCtl::setPtr( ofxCamcorder * ptr_ ) {
    ptr = ptr_;
}
