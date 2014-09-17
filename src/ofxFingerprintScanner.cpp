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

bool ofxFingerprintScanner::getFingerprint(){
    if ( isThreadRunning() ){
        return false;
    }
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
    Image_Packet* packet = device.GetImage();
    static ofImage ret;
    ret.setUseTexture(false);
    if (!ret.isAllocated()){
        ret.allocate(240, 216, OF_IMAGE_GRAYSCALE);
    }
    for ( int i=0; i<240 * 216; i++){
        ret.getPixels()[i] = (unsigned char) packet->ImageData[i];
    }
    ofNotifyEvent(gotFingerprint, ret, this);
    stopThread();
    ofSetLogLevel(OF_LOG_ERROR);
}