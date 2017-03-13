//
//  midiParser.hpp
//  midiToHexagono
//
//  Created by Eduard Frigola on 06/03/2017.
//
//

#ifndef midiParser_hpp
#define midiParser_hpp

#include "ofMain.h"
#include "ofxMidi.h"
#include "ofxTweenzor.h"

const int RINGMODE_STEPS = 35;
const int SEQMODE_STEPS = 64;

typedef struct{
    int     pitch;
    float   velocity;
    int     ringId;
    int     midiChannel;
}noteInCanvas;

enum hexModes{
    HEX_RING = 0,
    HEX_CIRCULAR = 1,
    HEX_SPIRAL = 2,
    HEX_SPIRAL_ALL = 3
};

class midiParser : public ofxMidiListener{
public:
    midiParser();
    ~midiParser(){};
    
    void fillFbo(ofFbo* fbo);
    
    void newMidiMessage(ofxMidiMessage& eventArgs);
    void modeChange(int &m);
    void resetListener(bool &b);
    void symmetryChanged(int &s);
    
    ofParameterGroup getParameters(){return parameters;};
    
private:
    ofxMidiIn       midiIn;
    
    vector<int>     midiNotesCounter;
    deque<noteInCanvas> notes;
    
    ofParameterGroup    parameters;
    ofParameter<int>    numMidiNotes;
    ofParameter<int>    ringsPerRegister;
    ofParameter<int>    symmetry;
    ofParameter<int>    distance;
    ofParameter<bool>   noteOffEnable;
    ofParameter<float>  noteOffTime;
    ofParameter<int>    mode;
    ofParameter<bool>   dualTree;
    ofParameter<bool>   reset;
};

#endif /* midiParser_hpp */
