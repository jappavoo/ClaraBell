#include <festival/festival.h>

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
  int heap_size = 2100000;
  int load_init_files = 1;

  voice_volume_set();

  festival_initialize(load_init_files, heap_size);

  // put system into async mode
  festival_eval_command("(audio_mode 'async)");

  
  festival_say_text("Hello my name is ClaraBell");

  //  fprintf(stderr, "Voice subsystem initialized\n");

}

extern void
voice_wait()
{
  festival_wait_for_spooler();
}

extern void
voice_say(char *buf, int len)
{
  festival_say_text(buf);
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
