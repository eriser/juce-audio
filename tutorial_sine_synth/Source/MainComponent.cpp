#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


class MainContentComponent   : public AudioAppComponent,
                               public Slider::Listener
{
public:
    MainContentComponent()
    :   currentSampleRate (0.0),
        currentAngle (0.0),
        angleDelta (0.0)
    {
        addAndMakeVisible (frequencySlider);
        frequencySlider.setRange (50.0, 5000.0);
        frequencySlider.setSkewFactorFromMidPoint (500.0); // [4]
        frequencySlider.addListener (this);
        
        addAndMakeVisible (levelSlider);
        levelSlider.setRange (0.0, 0.25);
        levelSlider.addListener (this);
        levelSlider.setValue (level, dontSendNotification);
        
        setSize (600, 100);
        setAudioChannels (0, 1); // no inputs, one output
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
    }
    
    void resized() override
    {
        frequencySlider.setBounds (10, 10, getWidth() - 20, 20);
        levelSlider.setBounds (10, 30, getWidth() - 20, 20);
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &frequencySlider)
        {
            if (currentSampleRate > 0.0)
                updateAngleDelta();
        }
        else
        {
            level = levelSlider.getValue ();
        }
    }
    
    void updateAngleDelta()
    {
        const double cyclesPerSample = frequencySlider.getValue() / currentSampleRate; // [2]
        angleDelta = cyclesPerSample * 2.0 * double_Pi;                                // [3]
    }
    
    
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        currentSampleRate = sampleRate;
        updateAngleDelta();
    }
    
    void releaseResources() override
    {
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        float* const buffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
        
        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            const float currentSample = (float) std::sin (currentAngle);
            currentAngle += angleDelta;
            buffer[sample] = currentSample * level;
        }
    }
    
    
private:
    Slider frequencySlider;
    Slider levelSlider;
    double currentSampleRate, currentAngle, angleDelta; // [1]
    float level = 0.125f;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
