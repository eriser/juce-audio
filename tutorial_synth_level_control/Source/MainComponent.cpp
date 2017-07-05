#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

class MainContentComponent   : public AudioAppComponent
{
public:
    MainContentComponent()
    {
        leftLevelSlider.setRange (0.0, 0.25);
        rightLevelSlider.setRange (0.0, 0.25);
        leftLevelSlider.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
        rightLevelSlider.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
        leftLevelLabel.setText ("Left Noise Level", dontSendNotification);
        rightLevelLabel.setText ("Right Noise Level", dontSendNotification);
        
        addAndMakeVisible (leftLevelSlider);
        addAndMakeVisible (leftLevelLabel);
        addAndMakeVisible (rightLevelSlider);
        addAndMakeVisible (rightLevelLabel);
        
        setSize (600, 100);
        setAudioChannels (0, 2);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double /*sampleRate*/) override
    {
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            const float level = channel == 0 ? (float) leftLevelSlider.getValue() : rightLevelSlider.getValue();
            const float levelScale = level * 2.0f;
            float* const buffer = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);
            
            for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
            {
                buffer[sample] = random.nextFloat() * levelScale - level;
            }
        }
    }
    
    void releaseResources() override
    {
    }

    void resized() override
    {
        leftLevelLabel.setBounds (10, 10, 90, 20);
        leftLevelSlider.setBounds (100, 10, getWidth() - 110, 20);
        rightLevelLabel.setBounds (10, 30, 90, 20);
        rightLevelSlider.setBounds (100, 30, getWidth() - 110, 20);
    }
    

private:
    Random random;
    Slider leftLevelSlider;
    Slider rightLevelSlider;
    Label leftLevelLabel;
    Label rightLevelLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
