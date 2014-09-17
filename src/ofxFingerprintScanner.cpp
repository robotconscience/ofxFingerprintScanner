//
//  ofxFingerprintScanner.cpp
//
//  Created by Brett Renfer on 9/11/14.
//

#include "ofxFingerprintScanner.h"

ofxFingerprintScanner::~ofxFingerprintScanner()
{
    if ( bConnected ){
        if ( isThreadRunning() ) waitForThread();
        device.SetLED(false);
        device.ChangeBaudRate(9600); // this happens when it resets anyways
        device.Close();
    }
}

bool ofxFingerprintScanner::setup( string portname, int baudRate ){
    bConnected = device.Open(portname,baudRate);
    return bConnected;
}

void ofxFingerprintScanner::setLED( bool on ){
    if ( !bConnected ) return;
    device.SetLED(on);
}

bool ofxFingerprintScanner::isFingerDown(){
    if ( !bConnected || isThreadRunning() ) return false;
    return device.IsPressFinger();
}

bool ofxFingerprintScanner::getFingerprint( bool raw ){
    if ( isThreadRunning() ){
        return false;
    }
    bGetRaw = raw;
    startThread();
    return true;
}

bool ofxFingerprintScanner::canCaptureFingerprint(){
    return !isThreadRunning();
}


ofxFPS_GT511C3 & ofxFingerprintScanner::getDevice(){
    return device;
}

void ofxFingerprintScanner::threadedFunction(){
    ofSetLogLevel(OF_LOG_VERBOSE);
    Image_Packet* packet = bGetRaw ? device.GetRawImage() : device.GetImage();
    if ( packet == NULL){
    } else {
        static ofImage ret;
        ret.setUseTexture(false);
        int w = 240;
        int h = 216;
        ret.allocate(w, h, OF_IMAGE_GRAYSCALE);
        for ( int i=0; i<w * h; i++){
            ret.getPixels()[i] = (unsigned char) packet->ImageData[i];
        }
        ofNotifyEvent(gotFingerprint, ret, this);
    }
    stopThread();
    ofSetLogLevel(OF_LOG_ERROR);
}