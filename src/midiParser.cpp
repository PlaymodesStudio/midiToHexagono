//
//  midiParser.cpp
//  midiToHexagono
//
//  Created by Eduard Frigola on 06/03/2017.
//
//

#include "midiParser.hpp"
#include "parametersControl.h"

midiParser::midiParser(){

    
    
    mode.addListener(this, &midiParser::modeChange);
    mode = 1;
    
    parameters.setName("Gui");
    parameters.add(ringsPerRegister.set("Rings x Register", 12, 1, 64));
    parameters.add(symmetry.set("Symmetry", 1, 1, 64));
    parameters.add(distance.set("Sym Distance", 64, 1, 64));
    parameters.add(noteOffEnable.set("noteOff Enable", false));
    parameters.add(noteOffTime.set("noteOff Time", 0.5, 0, 10));
    parametersControl::addDropdownToParameterGroupFromParameters(parameters, "Ease",
    {"EASE_LINEAR",
        "EASE_IN_QUAD",
        "EASE_OUT_QUAD",
        "EASE_IN_OUT_QUAD",
        "EASE_IN_CUBIC",
        "EASE_OUT_CUBIC",
        "EASE_IN_OUT_CUBIC",
        "EASE_IN_QUART",
        "EASE_OUT_QUART",
        "EASE_IN_OUT_QUART",
        "EASE_IN_QUINT",
        "EASE_OUT_QUINT",
        "EASE_IN_OUT_QUINT",
        "EASE_IN_SINE",
        "EASE_OUT_SINE",
        "EASE_IN_OUT_SINE",
        "EASE_IN_EXPO",
        "EASE_OUT_EXPO",
        "EASE_IN_OUT_EXPO",
        "EASE_IN_CIRC",
        "EASE_OUT_CIRC",
        "EASE_IN_OUT_CIRC",
        "EASE_IN_ELASTIC",
        "EASE_OUT_ELASTIC",
        "EASE_IN_OUT_ELASTIC",
        "EASE_OUT_IN_ELASTIC",
        "EASE_IN_BACK",
        "EASE_OUT_BACK",
        "EASE_IN_OUT_BACK",
        "EASE_OUT_IN_BACK",
        "EASE_IN_BOUNCE",
        "EASE_OUT_BOUNCE",
        "EASE_IN_OUT_BOUNCE",
        "EASE_OUT_IN_BOUNCE"
    }, easeFunction);
    parametersControl::addDropdownToParameterGroupFromParameters(parameters, "Mode", {"Ring", "Circular", "Spiral", "Spirall All", "Size Mode"}, mode);
    parameters.add(scaleX.set("Scale X", 1, 1, 20));
    parameters.add(scaleY.set("Scale Y", 1, 1, 20));
    parameters.add(dualTree.set("Dual Tree", false));
    parameters.add(reset.set("Reset", false));
    
    parametersControl::getInstance().createGuiFromParams(parameters);
    
    noteOffEnable.addListener(this, &midiParser::resetListener);
    reset.addListener(this, &midiParser::resetListener);
    symmetry.addListener(this, &midiParser::symmetryChanged);
    
    
    Tweenzor::init();
    
    ofxMidiIn::listPorts();
    midiIn.openPort(0);
    midiIn.addListener(this);
}

void midiParser::fillFbo(ofFbo *fbo){
    processMidiMessages();
    vector<vector<bool>> drawnPitchChecker;
    vector<bool> tempAuxVec;
    tempAuxVec.resize(64,0);
    drawnPitchChecker.resize(3,tempAuxVec);

    fbo->begin();
    ofPushMatrix();
    if(mode == HEX_CIRCULAR)
        ofScale(scaleX, scaleY);
    ofDisableBlendMode();
    ofSetColor(0);
    ofDrawRectangle(0, 0, fbo->getWidth(), fbo->getHeight());
    ofEnableBlendMode(OF_BLENDMODE_SCREEN);
    for(int i = notes.size()-1 ; i >= 0 ; i--){
        auto note = notes[i];
        if(!drawnPitchChecker[note.midiChannel-1][note.pitch]){
            if((mode == HEX_RING || mode == HEX_SPIRAL_ALL))
                drawnPitchChecker[note.midiChannel-1][note.pitch] = true;
        ofSetColor(note.velocity*255);
        switch(mode){
            case HEX_RING:
                ofDrawRectangle(21*(note.midiChannel-1), note.pitch, 21, 1);
                break;
            case HEX_CIRCULAR:{
                for(int j = 0; j < symmetry ; j++){
                    int drawPos = (note.pitch % (64/scaleX)+(j*distance))%SEQMODE_STEPS;
                    ofDrawRectangle(drawPos, note.ringId % (int)ceil((float)ringsPerRegister/(float)scaleY), 1, 1);
                }
                break;
            }
            case HEX_SPIRAL:{
                for(int j = 0; j < symmetry ; j++){
                    int drawPos = (note.ringId+(j*distance)+(int)ceil((float)note.pitch/2.0))%SEQMODE_STEPS;
                    ofDrawRectangle(drawPos, note.pitch, 1, 1);
                    if(dualTree){
                        drawPos = (-note.ringId-(j*distance)-(int)ceil((float)note.pitch/2.0));//%SEQMODE_STEPS;
                        while(drawPos < 0) drawPos += 64;
                        ofDrawRectangle(drawPos, note.pitch, 1, 1);
                    }
                }
                break;
            }
            case HEX_SPIRAL_ALL:{
                if(note.midiChannel == 3)
                    ofDrawRectangle(0, note.pitch, 64, 1);
                else{
                    for(int j = 0; j < symmetry ; j++){
                        for(int k = 0 ; k < 35 ; k++){
                            int drawPos;
                            if(note.midiChannel == 1)
                                drawPos = (note.pitch+(j*distance)+(int)ceil((float)k/2.0))%SEQMODE_STEPS;
                            else
                                drawPos = ((-note.pitch-(j*distance)-(int)ceil((float)(k+1)/2.0))+128)%SEQMODE_STEPS;
                            
                            ofDrawRectangle(drawPos, k, 1, 1);
                        }
                    }
                }
                break;
            }
            default:
                break;
        }
        }
        if(note.velocity < 0)
            notes.erase(notes.begin() + i);
    }
    ofPopMatrix();
    fbo->end();
    Tweenzor::update(ofGetElapsedTimeMillis());
}

void midiParser::draw(){
    for (int i = 0; i < notes.size() ; i++){
        ofDrawBitmapString("ID: " + ofToString(i) + "Pitch: " + ofToString(notes[i].pitch), 20, 20+(20*i));
    }
}

void midiParser::newMidiMessage(ofxMidiMessage &eventArgs){
    mutex.lock();
    midiMessages.push_back(eventArgs);
    mutex.unlock();
}

void midiParser::processMidiMessages(){
    mutex.lock();
    vector<ofxMidiMessage> midiMessagesCopy = midiMessages;
    midiMessages.clear();
    mutex.unlock();
    for(auto eventArgs : midiMessagesCopy){
    int mappedPitch;
    if(mode == HEX_RING || mode == HEX_SPIRAL)
        mappedPitch = (eventArgs.pitch)%RINGMODE_STEPS;
    else
        mappedPitch = (eventArgs.pitch)%SEQMODE_STEPS;
    
    if(eventArgs.velocity != 0 && eventArgs.status == MIDI_NOTE_ON){
        
        noteInCanvas note;
        note.pitch = mappedPitch;
        note.velocity = 1.;
        note.ringId = midiNotesCounter[mappedPitch] + ((eventArgs.channel-1) * ringsPerRegister);
        note.midiChannel = eventArgs.channel;
        notes.push_back(note);
        
        if(!noteOffEnable){
            Tweenzor::add((float*)&(notes[notes.size()-1].velocity), notes[notes.size()-1].velocity, -1.0f, 0.0f, noteOffTime*2, easeFunction);
        }
        
        if(mode == HEX_RING || mode == HEX_SPIRAL_ALL){
            for(int i = 0; i < notes.size()-1 ;  i++){
                if(notes[i].pitch == mappedPitch && notes[i].midiChannel == eventArgs.channel){
                    //notes[i].velocity = 0;
                    break;
                }
            }
        }
        
        midiNotesCounter[mappedPitch]++;
        midiNotesCounter[mappedPitch] %= ringsPerRegister;
        
    }else if(noteOffEnable && (eventArgs.velocity == 0 || eventArgs.status == MIDI_NOTE_OFF)){
        for(int i = notes.size()-1; i >= 0 ;  i--){
            auto &note = notes[i];
            if(note.pitch == mappedPitch && note.midiChannel == eventArgs.channel){
                Tweenzor::add((float*)&(note.velocity), note.velocity, -1.0f, 0.0f, noteOffTime*2, easeFunction);
                cout<<note.pitch<<endl;
                break;
            }
        }
    }
    }
}

void midiParser::modeChange(int &m){
    if(mode == HEX_RING || mode == HEX_SPIRAL)
        midiNotesCounter.resize(RINGMODE_STEPS, 0);
    else
        midiNotesCounter.resize(SEQMODE_STEPS, 0);
    reset = true;
}

void midiParser::resetListener(bool &b){
    if(reset){
        notes.clear();
        midiNotesCounter.assign(midiNotesCounter.size(), 0);
        parameters.getBool("Reset") = false;
    }
}

void midiParser::symmetryChanged(int &s){
    distance = floor((float)SEQMODE_STEPS/(float)s);
}
