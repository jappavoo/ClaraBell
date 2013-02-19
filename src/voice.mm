#import <AppKit/AppKit.h>
#include <stdio.h>

int voice_say(char *buf, int len)
{
    NSSpeechSynthesizer * synth = [[NSSpeechSynthesizer alloc] init];
    
    [synth startSpeakingString:
	    [[NSString alloc] initWithBytes:buf length:len encoding:NSUTF8StringEncoding]
    ];
 //       while ([synth isSpeaking]);
}
