#import <AppKit/AppKit.h>
#include <stdio.h>

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
  if (synth==NULL) return;

  [synth setVolume:v];
}

void
voice_volume_inc(void)
{
  float v;

  if (synth==NULL) return;
 
  v = [synth volume];

  if (v<1.0) {
    v = v + 0.1;
    [synth setVolume:v];
  }
}

void
voice_volume_dec(void)
{
  float v;

  if (synth==NULL) return;
 
  v = [synth volume];

  if (v>=0.1) {
    v = v - 0.1;
    [synth setVolume:v];
  }
}
