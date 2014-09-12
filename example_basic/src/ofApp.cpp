#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    ofSetVerticalSync(true);
    ofSerial s;
    s.listDevices();
    
//    ofSetLogLevel(OF_LOG_VERBOSE);
    bool bConnected = scanner.setup("/dev/tty.usbmodem1411");
    cout << "Connected? "<<bConnected<<endl;
    scanner.setLED(true);
}

//--------------------------------------------------------------
void ofApp::update(){
    if ( scanner.isFingerDown() ){
        ofBackground(ofColor::white);
    } else {
        ofBackground(ofColor::black);
    }
}

//--------------------------------------------------------------
void ofApp::exit(){
    scanner.setLED(false);
}


//--------------------------------------------------------------
void ofApp::draw(){

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
