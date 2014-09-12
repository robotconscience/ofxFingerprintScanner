//
//  ofxFingerprintScanner.h
//
//  Created by Brett Renfer on 9/11/14.
//

#pragma once

#include "ofMain.h"

#define DEVICE_FPS_GT511C3
#ifdef DEVICE_FPS_GT511C3
#include "ofxFPS_GT511C3.h"
#endif

class ofxFingerprintScanner : protected ofThread
{
public:
    
    ~ofxFingerprintScanner();
    
    bool setup( string portname, int baudRate = 9600 );
    
    // device methods
    void setLED( bool on );
    bool isFingerDown();
    bool getFingerprint();
    
    // utils
    bool canCaptureFingerprint();
    ofxFPS_GT511C3 & getDevice();
    
    ofEvent<ofImage> gotFingerprint;
    
protected:
    ofxFPS_GT511C3 device;
    bool bConnected;
    
    void threadedFunction();
};

typedef ofxFingerprintScanner ofxFS;

