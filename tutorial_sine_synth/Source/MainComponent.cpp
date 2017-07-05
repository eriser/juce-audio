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
        angleDelta (0.0),
        currentFrequency (500.0),
        targetFrequency (currentFrequency),
        currentLevel (0.125f),
        targetLevel(currentLevel)
    {
        addAndMakeVisible (frequencySlider);
        frequencySlider.setRange (50.0, 5000.0);
        frequencySlider.setSkewFactorFromMidPoint (500.0); // [4]
        frequencySlider.setValue (currentFrequency, dontSendNotification);
        frequencySlider.addListener (this);
        
        addAndMakeVisible (levelSlider);
        levelSlider.setRange (0.0, 0.25);
        levelSlider.addListener (this);
        levelSlider.setValue (currentLevel, dontSendNotification);
        
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
            targetFrequency = frequencySlider.getValue();
        else
            targetLevel = levelSlider.getValue ();
    }
    
    void updateAngleDelta()
    {
        const double cyclesPerSample = currentFrequency / currentSampleRate; // [2]
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
        
        const double localTargetFrequency = targetFrequency;
        const float localTargetLevel = targetLevel;

        float* const buffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
            
        if (localTargetFrequency != currentFrequency)
        {
            
            const double frequencyIncrement = (localTargetFrequency - currentFrequency) / bufferToFill.numSamples;
            
            for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                const float currentSample = (float) std::sin (currentAngle);
                currentFrequency += frequencyIncrement;
                updateAngleDelta();
                currentAngle += angleDelta;
                
                if(localTargetLevel != currentLevel)
                {
                    currentLevel += (localTargetLevel - currentLevel) / bufferToFill.numSamples;
                }
            
                buffer[sample] = currentSample * currentLevel;
            }
            
            currentFrequency = localTargetFrequency;
        }
        else
        {
            for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                const float currentSample = (float) std::sin (currentAngle);
                currentAngle += angleDelta;
                
                if(localTargetLevel != currentLevel)
                {
                    currentLevel += (localTargetLevel - currentLevel) / bufferToFill.numSamples;
                }
            
                buffer[sample] = currentSample * currentLevel;
            }
        }
    }
    
    
private:
    Slider frequencySlider;
    Slider levelSlider;
    double currentSampleRate, currentAngle, angleDelta, currentFrequency, targetFrequency; // [1]
    float currentLevel, targetLevel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
