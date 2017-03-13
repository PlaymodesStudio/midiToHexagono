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
    parametersControl::addDropdownToParameterGroupFromParameters(parameters, "Mode", {"Ring", "Circular", "Spiral", "Spirall All"}, mode);
    parameters.add(dualTree.set("Dual Tree", false));
    parameters.add(reset.set("Reset", false));
    
    parametersControl::getInstance().createGuiFromParams(parameters);
    
    
    reset.addListener(this, &midiParser::resetListener);
    symmetry.addListener(this, &midiParser::symmetryChanged);
    
    
    Tweenzor::init();
    
    ofxMidiIn::listPorts();
    midiIn.openPort(0);
    midiIn.addListener(this);
}

void midiParser::fillFbo(ofFbo *fbo){
    fbo->begin();
    if(reset){
        ofSetColor(0);
        ofDrawRectangle(0, 0, fbo->getWidth(), fbo->getHeight());
        parameters.getBool("Reset") = false;
    }
    for(int i = 0 ; i < notes.size() ; i++){
        auto note = notes[i];
        ofSetColor(note.velocity*255);
        switch(mode){
            case HEX_RING:
                ofDrawRectangle(21*(note.midiChannel-1), note.pitch, 21, 1);
                break;
            case HEX_CIRCULAR:{
                for(int j = 0; j < symmetry ; j++){
                    int drawPos = (note.pitch+(j*distance))%SEQMODE_STEPS;
                    ofDrawRectangle(drawPos, note.ringId, 1, 1);
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
                for(int j = 0; j < symmetry ; j++){
                    for(int k = 0 ; k < 35 ; k++){
                        int drawPos;
                        if(note.midiChannel == 1)
                            drawPos = (note.pitch+(j*distance)+(int)ceil((float)k/2.0));
                        else
                            drawPos = (-note.pitch-(j*distance)-(int)ceil((float)(k+1)/2.0))+64;

                        ofDrawRectangle(drawPos, k, 1, 1);
                    }
//                    for(int k = 0 ; k < 35 ; k++){
//                        int drawPos = -k-(j*distance)-(int)ceil((float)note.pitch/2.0);
//                        while(drawPos < 0) drawPos += 64;
//                        ofDrawRectangle(drawPos, note.pitch, 1, 1);
//                    }
                }
                break;
            }
            default:
                break;
        }
        if(note.velocity == 0)
            notes.erase(notes.begin() + i);
    }
//    cout<<notes.size()<<endl;
    fbo->end();
    Tweenzor::update(ofGetElapsedTimeMillis());
}

void midiParser::newMidiMessage(ofxMidiMessage &eventArgs){
    
//    cout<<eventArgs.toString()<<endl;
    
    int mappedPitch;
    if(mode == HEX_RING || mode == HEX_SPIRAL)
        mappedPitch = (eventArgs.pitch-35)%RINGMODE_STEPS;
    else
        mappedPitch = (eventArgs.pitch-35)%SEQMODE_STEPS;
    
    if(eventArgs.velocity != 0 && eventArgs.status == MIDI_NOTE_ON){
        noteInCanvas note;
        note.pitch = mappedPitch;
        note.velocity = 1.;
        note.ringId = midiNotesCounter[mappedPitch] + ((eventArgs.channel-1) * ringsPerRegister);
        note.midiChannel = eventArgs.channel;
        notes.push_back(note);
        
        if(!noteOffEnable){
            if(mode == HEX_RING || mode == HEX_SPIRAL_ALL){
                for(int i = 0; i < notes.size()-1 ;  i++){
                    if(notes[i].pitch == mappedPitch && notes[i].midiChannel == eventArgs.channel)
                        notes.erase(notes.begin() + i);
                }
            }
            
            Tweenzor::add((float*)&(notes.back().velocity), notes.back().velocity, 0.0f, 0.0f, noteOffTime);
        }
        
        midiNotesCounter[mappedPitch]++;
        midiNotesCounter[mappedPitch] %= ringsPerRegister;
        
    }else if(noteOffEnable && (eventArgs.velocity == 0 || eventArgs.status == MIDI_NOTE_OFF)){
        for(auto &note : notes){
            if(note.pitch == mappedPitch && note.midiChannel == eventArgs.channel)
                note.velocity = 0;
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
    }
}

void midiParser::symmetryChanged(int &s){
    distance = floor((float)SEQMODE_STEPS/(float)s);
}
