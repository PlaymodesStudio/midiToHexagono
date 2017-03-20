#include "ofApp.h"
#include "parametersControl.h"

//--------------------------------------------------------------
void ofApp::setup(){
    midi = new midiParser();
    
    texture.allocate(64, 35, GL_RGB);
    texture.getTexture().setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
    texture.begin();
    ofSetColor(ofColor::black);
    ofDrawRectangle(0, 0, texture.getWidth(), texture.getHeight());
    texture.end();
    ofSetColor(255);
    
    parametersControl::getInstance().setup();
    
    syphon.setName("MIDIFICATOR");
}

//--------------------------------------------------------------
void ofApp::update(){
    midi->fillFbo(&texture);
    syphon.publishTexture(&texture.getTexture());
}

//--------------------------------------------------------------
void ofApp::draw(){
    texture.draw(0,0, ofGetWidth(), ofGetHeight());
    midi->draw();
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
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

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
