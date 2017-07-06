#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


class MainContentComponent   : public AudioAppComponent,
                               private ButtonListener,
                               private SliderListener
{
public:
    MainContentComponent()
    :   currentLevel (0.125f),
        targetLevel (currentLevel)
    {
        addAndMakeVisible (openButton);
        openButton.setButtonText ("Open...");
        openButton.addListener (this);

        addAndMakeVisible (clearButton);
        clearButton.setButtonText ("Clear");
        clearButton.addListener (this);
        
        levelSlider.setRange (0.0, 0.25);
        levelSlider.setTextBoxStyle (Slider::TextBoxRight, false, 100, 20);
        levelLabel.setText ("Level", dontSendNotification);
        levelSlider.setValue (currentLevel);
        levelSlider.addListener(this);

        addAndMakeVisible (levelSlider);
        addAndMakeVisible (levelLabel);
        
        setSize (300, 200);

        formatManager.registerBasicFormats();
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
        const int numInputChannels = fileBuffer.getNumChannels();
        const int numOutputChannels = bufferToFill.buffer->getNumChannels();

        int outputSamplesRemaining = bufferToFill.numSamples;                                   // [8]
        int outputSamplesOffset = bufferToFill.startSample;                                     // [9]

        while (outputSamplesRemaining > 0)
        {
            int bufferSamplesRemaining = fileBuffer.getNumSamples() - position;                 // [10]
            int samplesThisTime = jmin (outputSamplesRemaining, bufferSamplesRemaining);        // [11]

            for (int channel = 0; channel < numOutputChannels; ++channel)
            {
                bufferToFill.buffer->copyFrom (channel,                                         // [12]
                                               outputSamplesOffset,                             //  [12.1]
                                               fileBuffer,                                      //  [12.2]
                                               channel % numInputChannels,                      //  [12.3]
                                               position,                                        //  [12.4]
                                               samplesThisTime);                                //  [12.5]
                
                const float localTargetLevel = currentLevel + ((targetLevel - currentLevel) * (samplesThisTime/bufferToFill.numSamples));
                bufferToFill.buffer->applyGainRamp (bufferToFill.startSample, samplesThisTime, currentLevel, localTargetLevel);
            }

            outputSamplesRemaining -= samplesThisTime;                                          // [13]
            outputSamplesOffset += samplesThisTime;                                             // [14]
            position += samplesThisTime;                                                        // [15]

            if (position == fileBuffer.getNumSamples())
                position = 0;                                                                   // [16]
            
            currentLevel = targetLevel;
        }
    }

    void releaseResources() override
    {
        fileBuffer.setSize (0, 0);
    }

    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() - 20, 20);
        clearButton.setBounds (10, 40, getWidth() - 20, 20);
        levelLabel.setBounds (10, 70, 90, 20);
        levelSlider.setBounds (100, 70, getWidth() - 110, 20);
    }

    void buttonClicked (Button* button) override
    {
        if (button == &openButton)      openButtonClicked();
        if (button == &clearButton)     clearButtonClicked();
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &levelSlider)
            targetLevel = (float) levelSlider.getValue();
    }

private:
    void openButtonClicked()
    {
        shutdownAudio();                                                                    // [1]

        FileChooser chooser ("Select a Wave file shorter than 2 seconds to play...",
                             File::nonexistent,
                             "*.wav");

        if (chooser.browseForFileToOpen())
        {
            const File file (chooser.getResult());
            ScopedPointer<AudioFormatReader> reader (formatManager.createReaderFor (file)); // [2]

            if (reader != nullptr)
            {
                const double duration = reader->lengthInSamples / reader->sampleRate;       // [3]

                if (duration < 2)
                {
                    fileBuffer.setSize (reader->numChannels, reader->lengthInSamples);      // [4]
                    reader->read (&fileBuffer,                                              // [5]
                                  0,                                                        //  [5.1]
                                  reader->lengthInSamples,                                  //  [5.2]
                                  0,                                                        //  [5.3]
                                  true,                                                     //  [5.4]
                                  true);                                                    //  [5.5]
                    position = 0;                                                           // [6]
                    setAudioChannels (0, reader->numChannels);                              // [7]
                }
                else
                {
                    // handle the error that the file is 2 seconds or longer..
                    String title, message;
                    title << "File Error" ;
                    message << "File too long, select a file smaller than 2s";
                    AlertWindow::showOkCancelBox(AlertWindow::WarningIcon, title, message);
                }
            }
        }
    }

    void clearButtonClicked()
    {
        shutdownAudio();
    }

    //==========================================================================
    TextButton openButton;
    TextButton clearButton;
    Slider levelSlider;
    Label levelLabel;

    AudioFormatManager formatManager;
    AudioSampleBuffer fileBuffer;
    int position;
    float currentLevel, targetLevel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
