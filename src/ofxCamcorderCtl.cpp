#include "ofxCamcorderCtl.h"

ofxCamcorderCtl::ofxCamcorderCtl() {

	ui.add( record.set("record", false) );

    ui.add( width.set( "width", 1280, 0, 1920 ));
    ui.add( height.set( "height", 720, 0, 1080 ));
    ui.add( fps.set( "fps", 30, 10, 120 ));

    ui.add( sync.set( "sync", false )); // system clock sync
    ui.add( silent.set( "silent", false ));

    ui.add( ffmpeg.set( "ffmpeg", "ffmpeg" )); // ffmpeg location

    ui.add( videoCodec.set( "videoCodec", "libx264" ));
    ui.add( videoBitrate.set( "videoBitrate", "2000k" ));

    ui.add( useAudio.set( "useAudio", false ));
    ui.add( audioCodec.set( "audioCodec", "mp3" ));
    ui.add( audioBitrate.set( "audioBitrate", "192k" ));
    ui.add( audioChannels.set( "audioChannels", 2, 1, 4 ));
}


void ofxCamcorderCtl::setPtr( ofxCamcorder * ptr_ ) {
    ptr = ptr_;
}
