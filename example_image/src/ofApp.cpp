#include "ofApp.h"

ofImage drawMe;
string currentState;

//--------------------------------------------------------------
void ofApp::setup(){
    ofSerial dummy;
    scanner.setup(dummy.getDeviceList()[0].getDevicePath());
    scanner.getDevice().ChangeBaudRate(115200);
    scanner.setLED(true);
    ofAddListener(scanner.gotFingerprint, this, &ofApp::onNewImage);
    
    ofBackground(0);
    currentState = "Place your finger on the device";
}

//--------------------------------------------------------------
void ofApp::update(){
    if ( scanner.canCaptureFingerprint() && scanner.isFingerDown() ){
        scanner.getFingerprint();
        currentState = "Scanning";
    }
    if ( drawMe.isAllocated() ){
        currentState = "";
        drawMe.setUseTexture(true);
        drawMe.update(); // since we don't have any texture when we first get the image.
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if ( drawMe.isAllocated()){
        drawMe.draw(0, 0);
    }
    if ( currentState != "") ofDrawBitmapString(currentState, ofGetWidth()/2.0, ofGetHeight()/2.0);
}

//--------------------------------------------------------------
void ofApp::onNewImage( ofImage & img ){
    cout <<"got it!"<<endl;
    drawMe.setUseTexture(false);
    drawMe.setFromPixels(img.getPixelsRef());
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
