#import <AppKit/AppKit.h>
//#include <stdio.h>
#ifdef __ppc__

/****************/
// The ClaraBell's Laptop is an old ppc
// macbook running OSX 10.4 which does
// not support the new methods on the
// synthizer for controlling the volume
//
// The following code is used instead
// it is based on code from http://cocoadev.com/wiki/SoundVolume
/****************/
#import <CoreAudio/CoreAudio.h>

// getting system volume
float getVolume() 
{
	float			b_vol;
	OSStatus		err;
	AudioDeviceID		device;
	UInt32			size;
	UInt32			channels[2];
	float			volume[2];
	
	// get device
	size = sizeof device;
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &device);
	if(err!=noErr) {
		NSLog(@"audio-volume error get device");
		return 0.0;
	}
	
	// try set master volume (channel 0)
	size = sizeof b_vol;
	err = AudioDeviceGetProperty(device, 0, 0, kAudioDevicePropertyVolumeScalar, &size, &b_vol);	//kAudioDevicePropertyVolumeScalarToDecibels
	if(noErr==err) return b_vol;
	
	// otherwise, try seperate channels
	// get channel numbers
	size = sizeof(channels);
	err = AudioDeviceGetProperty(device, 0, 0,kAudioDevicePropertyPreferredChannelsForStereo, &size,&channels);
	if(err!=noErr) NSLog(@"error getting channel-numbers");
	
	size = sizeof(float);
	err = AudioDeviceGetProperty(device, channels[0], 0, kAudioDevicePropertyVolumeScalar, &size, &volume[0]);
	if(noErr!=err) NSLog(@"error getting volume of channel %d",channels[0]);
	err = AudioDeviceGetProperty(device, channels[1], 0, kAudioDevicePropertyVolumeScalar, &size, &volume[1]);
	if(noErr!=err) NSLog(@"error getting volume of channel %d",channels[1]);
	
	b_vol = (volume[0]+volume[1])/2.00;
	
	return  b_vol;
}


// setting system volume
void setVolume(float involume)
 {
	OSStatus		err;
	AudioDeviceID		device;
	UInt32			size;
	Boolean			canset	= false;
	UInt32			channels[2];
	//float			volume[2];
		
	// get default device
	size = sizeof device;
	err = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &size, &device);
	if(err!=noErr) {
		NSLog(@"audio-volume error get device");
		return;
	}
	
	
	// try set master-channel (0) volume
	size = sizeof canset;
	err = AudioDeviceGetPropertyInfo(device, 0, false, kAudioDevicePropertyVolumeScalar, &size, &canset);
	if(err==noErr && canset==true) {
		size = sizeof involume;
		err = AudioDeviceSetProperty(device, NULL, 0, false, kAudioDevicePropertyVolumeScalar, size, &involume);
		return;
	}

	// else, try seperate channes
	// get channels
	size = sizeof(channels);
	err = AudioDeviceGetProperty(device, 0, false, kAudioDevicePropertyPreferredChannelsForStereo, &size,&channels);
	if(err!=noErr) {
		NSLog(@"error getting channel-numbers");
		return;
	}
	
	// set volume
	size = sizeof(float);
	err = AudioDeviceSetProperty(device, 0, channels[0], false, kAudioDevicePropertyVolumeScalar, size, &involume);
	if(noErr!=err) NSLog(@"error setting volume of channel %d",channels[0]);
	err = AudioDeviceSetProperty(device, 0, channels[1], false, kAudioDevicePropertyVolumeScalar, size, &involume);
	if(noErr!=err) NSLog(@"error setting volume of channel %d",channels[1]);
	
}
#endif

NSSpeechSynthesizer * synth = NULL;

void
voice_init(void)
{
   if (synth==NULL) synth=[[NSSpeechSynthesizer alloc] init];
}

void
voice_say(char *buf, int len)
{

  if (synth==NULL) return; 
  
  
  [synth startSpeakingString:
   [[NSString alloc] initWithBytes:buf length:len encoding:NSUTF8StringEncoding]
   ];
 //       while ([synth isSpeaking]);
}

void
voice_volume(float v)
{
#ifndef __ppc__
  if (synth==NULL) return;

  [synth setVolume:v];
#else 
  setVolume(v);
#endif
}

void
voice_volume_inc(void)
{
  float v;
#ifndef __ppc__
  if (synth==NULL) return;
 
  v = [synth volume];

  if (v<1.0) {
    v = v + 0.1;
    [synth setVolume:v];
  }
#else
  v = getVolume();
  if (v<1.0) {
    v = v + 0.1;
    setVolume(v);
  }
#endif  
}

void
voice_volume_dec(void)
{

  float v;
#ifndef __ppc__
  if (synth==NULL) return;
 
  v = [synth volume];

  if (v>=0.1) {
    v = v - 0.1;
    [synth setVolume:v];
  }
#else
  v = getVolume();
  if (v>=0.1) {
    v = v - 0.1;
    setVolume(v);
  }
#endif
}
