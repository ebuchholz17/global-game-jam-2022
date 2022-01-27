#ifndef QUICKMAKE_SYNTH_H
#define QUICKMAKE_SYNTH_H

#define MAX_SYNTH_SOUNDS 64
#define MAX_INPUT_NODES 4

enum synth_instrument_type {
    SYNTH_INSTRUMENT_TYPE_SINE_WAVE,
    SYNTH_INSTRUMENT_TYPE_PIANO,
    SYNTH_INSTRUMENT_TYPE_TRUMPET,
    SYNTH_INSTRUMENT_TYPE_GUITAR,
    SYNTH_INSTRUMENT_TYPE_VIOLIN,
    SYNTH_INSTRUMENT_TYPE_FLUTE,
    SYNTH_INSTRUMENT_TYPE_SNARE_DRUM,
    SYNTH_INSTRUMENT_TYPE_COUNT
};

enum synth_node_type {
    SYNTH_NODE_TYPE_OSCILLATOR,
    SYNTH_NODE_TYPE_CONTOUR_GENERATOR,
    SYNTH_NODE_TYPE_AMPLITUDE_CONTROL,
    SYNTH_NODE_TYPE_FILTER,
    SYNTH_NODE_TYPE_PITCH_SCALER,
    SYNTH_NODE_TYPE_LOW_FREQ_OSCILLATOR,
    SYNTH_NODE_TYPE_MIXER
};

// NOTE(ebuchholz): only mixes two inputs for now
// TODO(ebuchholz): option to vary coefficient with another input node
struct mixer_node {
    float coefficient0;
    float coefficient1;
};

struct pitch_scaler_node {
    float coefficient;
};

enum synth_filter_type {
    SYNTH_FILTER_TYPE_LOW_PASS,
    SYNTH_FILTER_TYPE_HIGH_PASS,
    SYNTH_FILTER_TYPE_BAND_PASS
};

struct filter_node {
    synth_filter_type type;
    float sampleRate;
    float cutoffFrequency;
};

struct delayed_filter_values {
    float delayedInput0;
    float delayedInput1;
    float delayedOutput0;
    float delayedOutput1;
};

enum oscillator_type_NEW {
    OSCILLATOR_TYPE_NEW_SINE,
    OSCILLATOR_TYPE_NEW_SQUARE,
    OSCILLATOR_TYPE_NEW_TRIANGLE,
    OSCILLATOR_TYPE_NEW_SAWTOOTH,
    OSCILLATOR_TYPE_NEW_NOISE
};

struct oscillator_node {
    oscillator_type_NEW type;
    // TODO(ebuchholz): pulse width
};

struct low_freq_oscillator_node {
    oscillator_type_NEW type;
    float frequency;
};


struct contour_generator_node {
    float attackTime;
    float attackValue;

    float decayTime;

    bool shouldSustain;
    float sustainValue;

    float releaseTime;
    bool endsNote;

    // TODO(ebuchholz): way to represent 5-part contour? for trumpets, e.g.
};

struct synth_node {
    synth_node *inputs[MAX_INPUT_NODES];
    synth_node_type type;

    union {
        oscillator_node oscillator;
        contour_generator_node contourGenerator;
        filter_node filter;
        pitch_scaler_node pitchScaler;
        low_freq_oscillator_node lowFreqOscillator;
        mixer_node mixer;
    }; 
};
#define LIST_TYPE synth_node
#include "list.h"

struct synth_sound_NEW {
    int id;

    float pitch;
    float velocity;

    bool triggerActivated;

    bool gateOpen;
    float gateT;
    float gateCloseTime;

    float oscillatorT;

    synth_instrument_type instrument;

    // store delayed samples for a set number of filters, to achieve butterworth filter effects
    delayed_filter_values filterData[4];
    int currentFilterIndex;

    bool ended; // set to true if all contour generators have ended, so it can be removed from the 
                // playing sounds list
};
#define LIST_TYPE synth_sound_NEW
#include "list.h"

struct synth_manager {
    synth_sound_NEW_list playingSounds;
    synth_node instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_COUNT];

    float sampleRate;

    // TODO(ebuchholz): define in txt file?
    synth_node_list synthNodes;
};

#endif
