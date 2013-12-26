#ifndef __VOICE_H__
#define __VOICE_H__

void voice_init(void);
void voice_say(char *buf, int len);
void voice_volume(float v);
void voice_volume_inc(void);
void voice_volume_dec(void);
void voice_wait(void);

#endif
