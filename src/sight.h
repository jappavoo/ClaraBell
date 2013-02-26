#ifndef __SIGHT_H__
#define __SIGHT_H__

#define SIGHT_MAX_DESTINATIONS 8
#define SIGHT_MAX_IMAGE_BYTES 200 * 1024


enum SightCmd { NONE=0, TAKE_PIC=1 };

struct Sight {

  volatile int  destinations[SIGHT_MAX_DESTINATIONS];
#if 0
  volatile chat image_bytes[SIGHT_MAX_IMAGE_BYTES];
  volatile int  image_bytes_cnt;
  volatile int  image_cnt;
#endif
  volatile int  dst_cnt;

  volatile int  take_pic;
  volatile int  send_fd;
  
  pthread_t        tid;
  pthread_cond_t   cond;
  pthread_mutex_t  mutex;
};

extern struct Sight sight;

int  sight_init(void);
void sight_take(void);
void sight_monitor(void);

static inline int  
sight_add_destination(int fd) 
{
  int rc=-1;
  // lock list so that we can safely find a slot for this
  // fd and record it.
  pthread_mutex_lock(&sight.mutex);
  for (int i=0; i<SIGHT_MAX_DESTINATIONS; i++) {
    if (sight.destinations[i]==-1) {
      sight.destinations[i]=fd;
      sight.dst_cnt++;
      rc=i;
      fprintf(stderr, "new sight fd=%d recorded at i=%d destcnt=%d\n", fd,i,sight.dst_cnt);
      break;
    }
  }
  pthread_mutex_unlock(&sight.mutex);
  return rc;
}

static inline void
sight_remove_destination(int i, int fd)
{
  pthread_mutex_lock(&sight.mutex);
  
  assert(sight.destinations[i]==fd && sight.dst_cnt>0);
    sight.destinations[i]=-1;
    sight.dst_cnt--;
  pthread_mutex_unlock(&sight.mutex);
  close(fd);

}
#endif
