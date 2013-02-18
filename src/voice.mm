#import <AppKit/AppKit.h>
#include <stdio.h>

int voice_say(int argc, char *argv[])
{
    NSSpeechSynthesizer * synth = [[NSSpeechSynthesizer alloc] init];

    for (int word = 0; word < argc; ++word)
    {
        printf("saying: %s", argv[word]);

        [synth startSpeakingString:
            [NSString stringWithUTF8String:argv[word]]
        ];
        while ([synth isSpeaking]);
    }
}
