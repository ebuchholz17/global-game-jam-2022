#include "quickmake_synth.h"

void initSynthSounds (synth_manager *synthManager, memory_arena *memory, float sampleRate) {
    *synthManager = {};
    synthManager->playingSounds = synth_sound_NEWListInit(memory, MAX_SYNTH_SOUNDS);
    synthManager->synthNodes = synth_nodeListInit(memory, 100);
    synthManager->sampleRate = sampleRate;

    // sine instrument
    {
        synth_node osc = {};
        osc.type = SYNTH_NODE_TYPE_OSCILLATOR;
        osc.oscillator.type = OSCILLATOR_TYPE_NEW_SINE;
        listPush(&synthManager->synthNodes, osc);

        synth_node *oscPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node contour = {};
        contour.type = SYNTH_NODE_TYPE_CONTOUR_GENERATOR;

        contour.contourGenerator.attackTime = 0.05f;
        contour.contourGenerator.attackValue = 1.0f;
        contour.contourGenerator.decayTime = 0.05f;
        contour.contourGenerator.shouldSustain = true;
        contour.contourGenerator.sustainValue = 0.7f;
        contour.contourGenerator.releaseTime = 0.3f;
        contour.contourGenerator.endsNote = true;

        listPush(&synthManager->synthNodes, contour);

        synth_node *contourPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];


        synth_node amplitudeControl = {};
        amplitudeControl.type = SYNTH_NODE_TYPE_AMPLITUDE_CONTROL;

        amplitudeControl.inputs[0] = oscPointer;
        amplitudeControl.inputs[1] = contourPointer;

        listPush(&synthManager->synthNodes, amplitudeControl);
        synth_node *amplitudeControlPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synthManager->instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_SINE_WAVE] = amplitudeControl;

        // temp
        synthManager->instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_PIANO] = amplitudeControl;
        //synthManager->instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_TRUMPET] = amplitudeControl;
        synthManager->instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_GUITAR] = amplitudeControl;
        synthManager->instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_VIOLIN] = amplitudeControl;
        synthManager->instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_FLUTE] = amplitudeControl;
        synthManager->instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_SNARE_DRUM] = amplitudeControl;
    }
    {
        synth_node osc = {};
        osc.type = SYNTH_NODE_TYPE_OSCILLATOR;
        osc.oscillator.type = OSCILLATOR_TYPE_NEW_SAWTOOTH;
        listPush(&synthManager->synthNodes, osc);

        synth_node *oscPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node contour = {};
        contour.type = SYNTH_NODE_TYPE_CONTOUR_GENERATOR;

        contour.contourGenerator.attackTime = 0.05f;
        contour.contourGenerator.attackValue = 1.0f;
        contour.contourGenerator.decayTime = 0.00f;
        contour.contourGenerator.shouldSustain = true;
        contour.contourGenerator.sustainValue = 1.0f;
        contour.contourGenerator.releaseTime = 0.05f;
        contour.contourGenerator.endsNote = true;

        listPush(&synthManager->synthNodes, contour);

        synth_node *contourPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node lfo = {};
        lfo.type = SYNTH_NODE_TYPE_LOW_FREQ_OSCILLATOR;
        lfo.lowFreqOscillator.type = OSCILLATOR_TYPE_NEW_TRIANGLE;
        lfo.lowFreqOscillator.frequency = 5.0f;
        listPush(&synthManager->synthNodes, lfo);

        synth_node *lfoPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node mixerNode = {};
        mixerNode.type = SYNTH_NODE_TYPE_MIXER;
        mixerNode.inputs[0] = contourPointer;
        mixerNode.mixer.coefficient0 = 0.85f;
        mixerNode.inputs[1] = lfoPointer;
        mixerNode.mixer.coefficient1 = 0.15f;
        listPush(&synthManager->synthNodes, mixerNode);

        synth_node *mixerPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node amplitudeControl = {};
        amplitudeControl.type = SYNTH_NODE_TYPE_AMPLITUDE_CONTROL;

        amplitudeControl.inputs[0] = oscPointer;
        amplitudeControl.inputs[1] = mixerPointer;

        listPush(&synthManager->synthNodes, amplitudeControl);
        synth_node *amplitudeControlPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node brightnessContour = {};
        brightnessContour.type = SYNTH_NODE_TYPE_CONTOUR_GENERATOR;

        brightnessContour.contourGenerator.attackTime = 0.3f;
        brightnessContour.contourGenerator.attackValue = 1.0f;
        brightnessContour.contourGenerator.decayTime = 0.0f;
        brightnessContour.contourGenerator.shouldSustain = true;
        brightnessContour.contourGenerator.sustainValue = 1.0f;
        brightnessContour.contourGenerator.releaseTime = 0.5f;

        listPush(&synthManager->synthNodes, brightnessContour);

        synth_node *brightnessContourPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node pitchScaler = {};
        pitchScaler.type = SYNTH_NODE_TYPE_PITCH_SCALER;

        pitchScaler.pitchScaler.coefficient = 1000.00f;

        listPush(&synthManager->synthNodes, pitchScaler);

        synth_node *pitchScalerPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];


        synth_node growlLFO = {};
        growlLFO.type = SYNTH_NODE_TYPE_LOW_FREQ_OSCILLATOR;
        growlLFO.lowFreqOscillator.type = OSCILLATOR_TYPE_NEW_TRIANGLE;
        growlLFO.lowFreqOscillator.frequency = 80.0f;
        listPush(&synthManager->synthNodes, growlLFO);

        synth_node *growlLFOPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node growlContour = {};
        growlContour.type = SYNTH_NODE_TYPE_CONTOUR_GENERATOR;

        growlContour.contourGenerator.attackTime = 0.01f;
        growlContour.contourGenerator.attackValue = 0.5f;
        growlContour.contourGenerator.decayTime = 0.4f;
        growlContour.contourGenerator.shouldSustain = false;
        growlContour.contourGenerator.sustainValue = 0.0f;
        growlContour.contourGenerator.releaseTime = 0.0f;

        listPush(&synthManager->synthNodes, growlContour);

        synth_node *growlContourPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node growlAmplitude = {};
        growlAmplitude.type = SYNTH_NODE_TYPE_AMPLITUDE_CONTROL;

        growlAmplitude.inputs[0] = growlLFOPointer;
        growlAmplitude.inputs[1] = growlContourPointer;

        listPush(&synthManager->synthNodes, growlAmplitude);
        synth_node *growlAmplitudePointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node cutoffMixerNode = {};
        cutoffMixerNode.type = SYNTH_NODE_TYPE_MIXER;
        cutoffMixerNode.inputs[0] = brightnessContourPointer;
        cutoffMixerNode.mixer.coefficient0 = 0.85f;
        cutoffMixerNode.inputs[1] = growlAmplitudePointer;
        cutoffMixerNode.mixer.coefficient1 = 0.15f;
        listPush(&synthManager->synthNodes, cutoffMixerNode);

        synth_node *cutoffMixerPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synth_node lowPassFilter = {};
        lowPassFilter.type = SYNTH_NODE_TYPE_FILTER;

        lowPassFilter.filter.type = SYNTH_FILTER_TYPE_LOW_PASS;
        lowPassFilter.filter.sampleRate = synthManager->sampleRate;
        lowPassFilter.inputs[0] = amplitudeControlPointer;
        lowPassFilter.inputs[1] = cutoffMixerPointer;
        lowPassFilter.inputs[2] = pitchScalerPointer;
        lowPassFilter.filter.cutoffFrequency = 3000.0f; // modified by pitch scaler

        listPush(&synthManager->synthNodes, lowPassFilter);
        synth_node *lowPassFilterPointer = 
            &synthManager->synthNodes.values[synthManager->synthNodes.numValues-1];

        synthManager->instrumentOutputNodes[SYNTH_INSTRUMENT_TYPE_TRUMPET] = lowPassFilter;
    }
}

void restartSound (synth_sound_NEW *sound, synth_instrument_type instrument, 
                 float pitch, float velocity) 
{
    sound->pitch = pitch;
    sound->velocity = velocity;
    sound->triggerActivated = true;
    sound->gateOpen = true;

    sound->gateT = 0.0f;
    sound->gateCloseTime = 0.0f;

    sound->instrument = instrument;

    // leave filter info, oscillator t, etc. unmodified
}

void startSound (synth_sound_NEW *sound, synth_instrument_type instrument, 
                 float pitch, float velocity) 
{
    sound->pitch = pitch;
    sound->velocity = velocity;
    sound->triggerActivated = true;
    sound->gateOpen = true;
    sound->instrument = instrument;
}

void startSoundByID (synth_manager *synthManager, synth_instrument_type instrument, 
                 float pitch, float velocity, int id) 
{
    for (int soundIndex = 0; soundIndex < synthManager->playingSounds.numValues; ++soundIndex) {
        synth_sound_NEW *sound = &synthManager->playingSounds.values[soundIndex];
        if (sound->id == id) {
            restartSound(sound, instrument, pitch, velocity);
            return;
        }
    }
    
    // no sound found, add a new one
    if (synthManager->playingSounds.numValues < synthManager->playingSounds.capacity) {
        synth_sound_NEW sound = {};
        startSound(&sound, instrument, pitch, velocity);
        sound.id = id;
        listPush(&synthManager->playingSounds, sound);
    }
}

void stopSynthSound (synth_sound_NEW *sound) {
    sound->gateOpen = false;
    sound->gateCloseTime = sound->gateT;
}

void stopSoundByID (synth_manager *synthManager, int id) {
    for (int soundIndex = 0; soundIndex < synthManager->playingSounds.numValues; ++soundIndex) {
        synth_sound_NEW *sound = &synthManager->playingSounds.values[soundIndex];
        if (sound->id == id) {
            stopSynthSound(sound);
            return;
        }
    }
}

float getSynthNodeOutput(synth_node *node, synth_sound_NEW *sound);

// TODO(ebuchholz): rework
float getPitchScalerOutput (synth_node *node, synth_sound_NEW *sound) {
    return node->pitchScaler.coefficient / sound->pitch;
}

float getFilterOutput (synth_node *node, synth_sound_NEW *sound) {
    assert(node->inputs[0] != 0);
    float input = getSynthNodeOutput(node->inputs[0], sound);

    float cutoffFrequency = node->filter.cutoffFrequency;
    assert(sound->currentFilterIndex < 4);
    delayed_filter_values *filterData = &sound->filterData[sound->currentFilterIndex];
    ++sound->currentFilterIndex;

    // modify cutoff frequency, (for example, with contour generator)
    for (int inputIndex = 1; inputIndex < 4; ++inputIndex) {
        if (node->inputs[inputIndex] != 0) {
            float cutoffCoefficient = getSynthNodeOutput(node->inputs[inputIndex], sound);
            cutoffFrequency *= cutoffCoefficient;
        }
    }

    // TODO(ebuchholz): avoid this case??
    if (cutoffFrequency <= 0) {
        cutoffFrequency = 1;
    }

    // NOTE(ebuchholz): butterworth filter implementation from http://basicsynth.com/index.php?page=filters
    // TODO(ebuchholz): learn about signal processing

    float squareRootTwo = 1.414213562f;
    float amplitudeIn0 = 0.0f;
    float amplitudeIn1 = 0.0f;
    float amplitudeIn2 = 0.0f;
    float amplitudeOut0 = 0.0f;
    float amplitudeOut1 = 0.0f;

    switch (node->filter.type) {
        case SYNTH_FILTER_TYPE_LOW_PASS: {
            float c = 1.0f / tanf((PI / node->filter.sampleRate) * cutoffFrequency);
            float cSquared = c * c;
            float cTimesSquareRootTwo = squareRootTwo * c;
            float d = cSquared + cTimesSquareRootTwo + 1.0f;
            amplitudeIn0 = 1.0f / d;
            amplitudeIn1 = amplitudeIn0 + amplitudeIn0;
            amplitudeIn2 = amplitudeIn0;
            amplitudeOut0 = (2.0f * (1.0f - cSquared)) / d;
            amplitudeOut1 = (cSquared - cTimesSquareRootTwo + 1.0f) / d;
        } break;
        case SYNTH_FILTER_TYPE_HIGH_PASS: {
            float c = tanf((PI / node->filter.sampleRate) * cutoffFrequency);
            float cSquared = c * c;
            float cTimesSquareRootTwo = squareRootTwo * c;
            float d = cSquared + cTimesSquareRootTwo + 1.0f;
            amplitudeIn0 = 1.0f / d;
            amplitudeIn1 = -(amplitudeIn0 + amplitudeIn0);
            amplitudeIn2 = amplitudeIn0;
            amplitudeOut0 = (2.0f * (cSquared - 1.0f)) / d;
            amplitudeOut1 = (1.0f - cTimesSquareRootTwo + cSquared) / d;
        } break;
        case SYNTH_FILTER_TYPE_BAND_PASS: {
            float c = 1.0f / tanf((PI / node->filter.sampleRate) * cutoffFrequency);
            float d = 1.0f + c;
            amplitudeIn0 = 1.0f / d;
            amplitudeIn1 = 0.0f;
            amplitudeIn2 = -amplitudeIn0;
            amplitudeOut0 = (-c * 2.0f * cosf(2.0f * PI * cutoffFrequency / node->filter.sampleRate)) / d;
            amplitudeOut1 = (c - 1.0f) / d;
        } break;
    }

    float result = input * amplitudeIn0 + 
                   filterData->delayedInput0 * amplitudeIn1 + 
                   filterData->delayedInput1 * amplitudeIn2 - 
                   filterData->delayedOutput0 * amplitudeOut0 - 
                   filterData->delayedOutput1 * amplitudeOut1;

    filterData->delayedInput1 = filterData->delayedInput0;
    filterData->delayedInput0 = input;
    filterData->delayedOutput1 = filterData->delayedOutput0;
    filterData->delayedOutput0 = result;

    return result;
}

float getContourGeneratorOutput (synth_node *node, synth_sound_NEW *sound) {
    contour_generator_node *contour = &node->contourGenerator;
    float decayStart = contour->attackTime;
    float sustainStart = contour->attackTime + contour->decayTime;

    float result = 0.0f;

    if (sound->gateT < contour->attackTime) {
        result = (sound->gateT / contour->attackTime) * contour->attackValue;
    }
    else if (sound->gateT >= decayStart && sound->gateT < sustainStart) {
        float t = (sound->gateT - decayStart) / contour->decayTime;
        result = contour->attackValue + t * (contour->sustainValue - contour->attackValue);
    }
    else if (sound->gateT >= sustainStart) {
        if (contour->shouldSustain) {
            if (sound->gateOpen) {
                result = contour->sustainValue;
            }
            else {
                float releaseStart = sound->gateCloseTime > sustainStart ? sound->gateCloseTime : sustainStart;
                float t = (sound->gateT - releaseStart) / contour->releaseTime;
                result = contour->sustainValue + t * -contour->sustainValue;
            }
        }
        else {
            result = 0.0f;
        }
    }

    if (result < 0.0f) {
        result = 0.0f;
    }

    if (contour->endsNote && result != 0.0f) {
        sound->ended = false;
    }
    return result;
}

float getAmplitudeOutput (synth_node *node, synth_sound_NEW *sound) {
    assert(node->inputs[0] != 0);
    float result = getSynthNodeOutput(node->inputs[0], sound);
    int inputIndex = 1;
    while (node->inputs[inputIndex] != 0 && inputIndex < MAX_INPUT_NODES) {
        float amplitude = getSynthNodeOutput(node->inputs[inputIndex], sound);
        result *= amplitude;
        ++inputIndex;
    }
    return result;
}

float getWaveformValueForFrequency (oscillator_type_NEW type, float t, float frequency) {
    switch (type) {
        default:
        case OSCILLATOR_TYPE_NEW_SINE: {
            return sinf(t * frequency * 2.0f * PI);
        } break;
        case OSCILLATOR_TYPE_NEW_SQUARE: {
            float sine = sinf(t * frequency * 2.0f * PI);
            return sine > 0.0f ? 1.0f : -1.0f;
        } break;
        case OSCILLATOR_TYPE_NEW_TRIANGLE: {
            float sine = sinf(t * frequency * 2.0f * PI);
            return (2.0f / PI) * asinf(sine);
        } break;
        case OSCILLATOR_TYPE_NEW_SAWTOOTH: {
            return 2.0f * (t * frequency - floorf(0.5f + (t * frequency)));
        } break;
        case OSCILLATOR_TYPE_NEW_NOISE: {
            return randomFloat() * 2.0f - 1.0f;
        } break;
    }
}

float getOscillatorOutput (synth_node *node, synth_sound_NEW *sound) {
    // TODO(ebuchholz); pulse width modulation 
    return getWaveformValueForFrequency(node->oscillator.type, sound->oscillatorT, sound->pitch);
}

float getLowFreqOscillatorOutput (synth_node *node, synth_sound_NEW *sound) {
    // TODO(ebuchholz): figure out t to use for this
    return getWaveformValueForFrequency(node->lowFreqOscillator.type, sound->gateT, node->lowFreqOscillator.frequency);
}

float getMixerOutput (synth_node *node, synth_sound_NEW *sound) {
    assert(node->inputs[0] != 0);
    assert(node->inputs[1] != 0);
    float input0 = getSynthNodeOutput(node->inputs[0], sound);
    float input1 = getSynthNodeOutput(node->inputs[1], sound);
    return node->mixer.coefficient0 * input0 + node->mixer.coefficient1 * input1;
}

float getSynthNodeOutput(synth_node *node, synth_sound_NEW *sound) {
    switch (node->type) {
        default: {
            assert(false); // invalid node type
            return 0.0f;
        } break;
        case SYNTH_NODE_TYPE_CONTOUR_GENERATOR: {
            return getContourGeneratorOutput(node, sound);
        } break;
        case SYNTH_NODE_TYPE_AMPLITUDE_CONTROL: {
            return getAmplitudeOutput(node, sound);
        } break;
        case SYNTH_NODE_TYPE_OSCILLATOR: {
            return getOscillatorOutput(node, sound);
        } break;
        case SYNTH_NODE_TYPE_FILTER: {
            return getFilterOutput(node, sound);
        } break;
        case SYNTH_NODE_TYPE_PITCH_SCALER: {
            return getPitchScalerOutput(node, sound);
        } break;
        case SYNTH_NODE_TYPE_LOW_FREQ_OSCILLATOR: {
            return getLowFreqOscillatorOutput(node, sound);
        } break;
        case SYNTH_NODE_TYPE_MIXER: {
            return getMixerOutput(node, sound);
        } break;
    }
}

float generateSynthOutput (synth_node *instrumentOutputNode, synth_sound_NEW *sound) {
    switch (sound->instrument) {
        default:
        case SYNTH_INSTRUMENT_TYPE_SINE_WAVE: {
            return getSynthNodeOutput(instrumentOutputNode, sound);
        } break;
    }
}

float updateSynthSound (synth_node *instrumentOutputNode, synth_sound_NEW *sound, float dt) {
    sound->oscillatorT += dt;
    float period = 1.0f / sound->pitch;
    while (sound->oscillatorT >= period) {
        sound->oscillatorT -= period;
    }

    if (sound->triggerActivated) {
        sound->triggerActivated = false;
        sound->gateT = 0.0f;
    }
    sound->gateT += dt;

    sound->ended = true;
    sound->currentFilterIndex = 0;
    return generateSynthOutput(instrumentOutputNode, sound);
}

float updateSynthSounds (synth_manager *synthManager, float dt) {
    float sampleValue = 0.0f;

    for (int soundIndex = synthManager->playingSounds.numValues - 1; soundIndex >= 0; --soundIndex) {
        synth_sound_NEW *sound = &synthManager->playingSounds.values[soundIndex];

        float soundValue = updateSynthSound(&synthManager->instrumentOutputNodes[sound->instrument], sound, dt);
        // TODO(ebuchholz): add velocity based nodes
        soundValue *= sound->velocity;
        sampleValue += soundValue;

        if (sound->ended) {
            // remove
            listSplice(&synthManager->playingSounds, soundIndex);
        }
    }

    return sampleValue;
}
