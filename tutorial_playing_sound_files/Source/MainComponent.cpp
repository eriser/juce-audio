#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"


class MainContentComponent   : public AudioAppComponent,
                               public ChangeListener,
                               public Button::Listener,
                               public Timer
{
public:
    MainContentComponent()
    :   state (Stopped)
    {
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Open...");
        openButton.addListener (this);
        
        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play");
        playButton.addListener (this);
        playButton.setColour (TextButton::buttonColourId, Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.addListener (this);
        stopButton.setColour (TextButton::buttonColourId, Colours::red);
        stopButton.setEnabled (false);
        
        addAndMakeVisible (&timeLabel);
        timeLabel.setText ("0", dontSendNotification);
        
        setSize (300, 200);
        
        formatManager.registerBasicFormats();       // [1]
        transportSource.addChangeListener (this);   // [2]

        setAudioChannels (0, 2);
        startTimer (20);
    }
    
    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
        
        transportSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() - 20, 20);
        playButton.setBounds (10, 40, getWidth() - 20, 20);
        stopButton.setBounds (10, 70, getWidth() - 20, 20);
        timeLabel.setBounds (10, 130, getWidth() - 20, 20);
    }
    
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else if ((state == Stopping) || (state == Playing))
                changeState (Stopped);
            else if (Pausing == state)
                changeState (Paused);
        }
    }
    
    void timerCallback() override
    {
        double currentTime = transportSource.getCurrentPosition();
        timeLabel.setText (String (currentTime), dontSendNotification);
    }

    void buttonClicked (Button* button) override
    {
        if (button == &openButton)  openButtonClicked();
        if (button == &playButton)  playButtonClicked();
        if (button == &stopButton)  stopButtonClicked();
    }

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };
    
    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;
            
            switch (state)
            {
                case Stopped:                           // [3]
                    playButton.setButtonText ("Play");
                    stopButton.setButtonText ("Stop");
                    stopButton.setEnabled (false);
                    transportSource.setPosition (0.0);
                    break;
                    
                case Starting:                          // [4]
                    transportSource.start();
                    break;
                    
                case Playing:                           // [5]
                    playButton.setButtonText ("Pause");
                    stopButton.setButtonText ("Stop");
                    stopButton.setEnabled (true);
                    break;
                    
                case Pausing:
                    transportSource.stop();
                    break;
                
                case Paused:
                    playButton.setButtonText ("Resume");
                    stopButton.setButtonText ("Return to Zero");
                    break;
                    
                case Stopping:                          // [6]
                    transportSource.stop();
                    break;
            }
        }
    }
    
    void openButtonClicked()
    {
        FileChooser chooser ("Select a Wave file to play...",
                             File::nonexistent,
                             "*.wav;*.aif;*.aiff");                           // [7]
        
        if (chooser.browseForFileToOpen())                                    // [8]
        {
            File file (chooser.getResult());                                  // [9]
            AudioFormatReader* reader = formatManager.createReaderFor (file); // [10]
            
            if (reader != nullptr)
            {
                ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true); // [11]
                transportSource.setSource (newSource, 0, nullptr, reader->sampleRate);                         // [12]
                playButton.setEnabled (true);                                                                  // [13]
                readerSource = newSource.release();                                                            // [14]
            }
        }
    }
    
    void playButtonClicked()
    {
        if ((state == Stopped) || (state == Paused))
            changeState (Starting);
        else if (state == Playing)
            changeState (Pausing);
    }
    
    void stopButtonClicked()
    {
        if (state == Paused)
            changeState (Stopped);
        else
            changeState (Stopping);
    }
    
    //==========================================================================
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    
    Label timeLabel;
    
    AudioFormatManager formatManager;
    ScopedPointer<AudioFormatReaderSource> readerSource;
    AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


Component* createMainContentComponent()     { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
