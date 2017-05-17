#include <stdio.h>
#include <stdlib.h>
#include <swift.h>
#define VOICE "Robin"

swift_engine *engine;
swift_port *port = NULL;
swift_voice *voice;
swift_background_t tts_stream;
swift_result_t res;

static int volume = 100;

void
voice_volume_set()
{
  char cmd[160];
  snprintf(cmd, 160, "/usr/bin/amixer sset PCM,0 %d%%", volume);
  system(cmd);
}

extern void
voice_init()
{
    /* Open the Swift TTS Engine */
    if ( (engine = swift_engine_open(NULL)) == NULL) {
        fprintf(stderr, "Failed to open Swift Engine.");
        goto all_done;
    }
    /* Open a Swift Port through which to make TTS calls */
    if ( (port = swift_port_open(engine, NULL)) == NULL ) {
        fprintf(stderr, "Failed to open Swift Port.");
        goto all_done;
    }
    
    /* Set the voice found in the last step as the port's current voice */
    voice = swift_port_set_voice_by_name(port, VOICE); 
    if ( voice == NULL ) {
        fprintf(stderr, "ERROR: swift_port_set_voice_by_name: %s", VOICE);
        goto all_done;
    }
    return;
all_done:
  exit(-1);
}

extern void
voice_wait()
{
 if ( (SWIFT_STATUS_RUNNING == swift_port_status(port, tts_stream)) ||
      (SWIFT_STATUS_PAUSED == swift_port_status(port, tts_stream)) ) { 
   swift_port_wait(port, tts_stream);
 }
}

extern void
voice_say(char *buf, int len)
{
  swift_port_speak_text(port, buf, len, NULL, &tts_stream, NULL);
}

extern void 
voice_volume(float v)
{
  volume = (int) (100.0 * v);
  voice_volume_set();
}

extern void 
voice_volume_inc(void)
{
  volume += 10;
  if (volume > 100) volume = 100;
}

extern void 
voice_volume_dec(void)
{
  volume -= 10;
  if (volume<0) volume = 0;
}
