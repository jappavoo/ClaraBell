#ifndef __SIGHT_H__
#define __SIGHT_H__

enum SightCmd { NONE=0, TAKE_PIC=1 };

struct Sight {
  char *img;
  int   img_len;
  int   img_num;
  volatile int  take_pic;
  volatile int  send_fd;

  pthread_t        tid;
  pthread_cond_t   cond;
  pthread_mutex_t  mutex;
};

extern struct Sight sight;

int  sight_init(void);
void sight_take(int sendfd);
void sight_monitor(int sendfd);

#endif
