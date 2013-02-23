#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <string.h>
#include <assert.h>
#include "arduino-serial.h"
#include "net.h"
#include "voice.h"
#include "sight.h"

//#define __TRACE__
//#define SUPPRESS_MOTORCMDS

#ifndef FD_COPY
#define FD_COPY(src,dest) memcpy((dest),(src),sizeof(dest))
#endif

#define BAUD 115200
#define PORT 8000
#define BUFLEN 4096

struct Message {
  char *str;
  int len;
} messages[] = { 
  {(char *)"Hello.", 6},
  {(char *)"What's going on?", 16 },
  {(char *)"Please?", 7 },
  {(char *)"Yes.", 4 },
  {(char *)"No.", 3 },
  {(char *)"Thank you.", 10 },
  {(char *)"Cool!",  5},
  {(char *)"Shanti", 6},
  {(char *)"Raja", 5},

}; 
#define HELLOMSG 0
#define WAZUPMSG 1
#define PLSMSG   2
#define YESMSG   3
#define NOMSG    4
#define TNKUMSG  5
#define COOLMSG  6
#define SHANTMSG 7
#define RAJAMSG  8

char sbuf[BUFLEN], mbuf[BUFLEN], nbuf[BUFLEN];
int sn, mn, nn;
#define LINELEN 4096


int netfd;

struct Connection {
  char line[LINELEN];
  int  len;
  int  fd;
} *cons[FD_SETSIZE];

struct DeviceDesc {
  int fd;
  struct Connection *owner;
} motorBoard, sensorBoard;

enum Motion_States { STOPPED=0, STRAIGHT_FORWARD, STRAIGHT_BACKWARD, ROTATE_LEFT, ROTATE_RIGHT, FWD_TURN_LEFT, FWD_TURN_RIGHT, BWD_TURN_LEFT, BWD_TURN_RIGHT };
enum Motor_Speed  { S0=0, S1=1, S2=2, S3=3, S4=4, S5=5, S6=6, S7=7, S8=8, S9=9, S10=10 };
enum Motion_Direction { FORWARD='F', BACKWARD='B', LEFT='L', RIGHT='R' };


struct MotorState {
  enum Motion_States mstate;
  enum Motor_Speed   speed;  
} ms;

inline void
motion_polar(int s, int div, int start)
{
  char cmd[16];
  int len=0;
  const int fd=motorBoard.fd;

  switch (div) {
  case 0:  
    // BB
    cmd[len]='0'+s; len++;
    cmd[len]='b'; len++;
    break;
  case 1:
    // bB
    cmd[len]='l'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='0'+(s/2); len++;
    cmd[len]='b'; len++; 
    break;
  case 2:
    // 0B
    cmd[len]='l'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='-'; len++;
    cmd[len]='l'; len++; cmd[len]='b'; len++;
    break;
  case 3:
    // fB
    cmd[len]='l'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='0'+(s/2); len++;
    cmd[len]='l'; len++; cmd[len]='b'; len++;
    cmd[len]='r'; len++; cmd[len]='f'; len++;
    break;
  case 4:
    // FB
    cmd[len]='l'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='0'+s; len++;
    cmd[len]='l'; len++; cmd[len]='f'; len++;
    cmd[len]='r'; len++; cmd[len]='b'; len++;
    break;
  case 5:
    // Fb
    cmd[len]='l'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='0'+(s/2); len++;
    cmd[len]='l'; len++; cmd[len]='f'; len++;
    cmd[len]='r'; len++; cmd[len]='b'; len++;
    break;
  case 6:
    // F0
    cmd[len]='l'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='-'; len++;
    cmd[len]='l'; len++; cmd[len]='f'; len++;
    break;
  case 7:
    // Ff
    cmd[len]='l'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='0'+(s/2); len++;
    cmd[len]='f'; len++;
    break;
  case 8:
    // FF
    cmd[len]='0'+s; len++;
    cmd[len]='f'; len++;
    break;
  case 9:
    // fF
    cmd[len]='l'; len++; cmd[len]='0'+(s/2); len++;
    cmd[len]='r'; len++; cmd[len]='0'+s; len++;
    cmd[len]='f'; len++;
    break;
  case 10:
    // 0F
    cmd[len]='l'; len++; cmd[len]='-'; len++;
    cmd[len]='r'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='f'; len++;
    break;
  case 11:
    // bF
    cmd[len]='l'; len++; cmd[len]='0'+(s/2); len++;
    cmd[len]='r'; len++; cmd[len]='0'+s; len++;
    cmd[len]='l'; len++; cmd[len]='b'; len++;
    cmd[len]='r'; len++; cmd[len]='f'; len++;
    break;
  case 12:
    // BF
    cmd[len]='l'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='0'+s; len++;
    cmd[len]='l'; len++; cmd[len]='b'; len++;
    cmd[len]='r'; len++; cmd[len]='f'; len++;
    break;
  case 13:
    // Bf
    cmd[len]='l'; len++; cmd[len]='0'+(s/2); len++;
    cmd[len]='r'; len++; cmd[len]='0'+s; len++;
    cmd[len]='l'; len++; cmd[len]='f'; len++;
    cmd[len]='r'; len++; cmd[len]='b'; len++;
    break;
  case 14:
    // B0
    cmd[len]='l'; len++; cmd[len]='-'; len++;
    cmd[len]='r'; len++; cmd[len]='0'+s; len++;
    cmd[len]='r'; len++; cmd[len]='b'; len++;
    break;
  case 15:
    // Bb:
    cmd[len]='l'; len++; cmd[len]='0'+(s/2); len++;
    cmd[len]='r'; len++; cmd[len]='0'+s; len++;
    cmd[len]='b'; len++; 
    break;
  }
  if (len>1) {
    if (start) { cmd[len]='g'; len++;}
    cmd[len]='\n'; len++;
#ifdef __TRACE__
    write(1, cmd, len);
#endif
#ifndef SUPPRESS_MOTORCMDS
    write(fd, cmd, len);
#endif
  }
}

inline void
motion_begin(char d, int s, int o) 
{
  char cmd[8];
  int len=0;
  const int fd=motorBoard.fd;

  if ((s>=0 && s<=9) && (o>=0 && o<=4))  {
    cmd[len]='0'+s; len++;
    if (ms.mstate == STOPPED) {
      switch (d) {
      case FORWARD: 
	cmd[len]='f'; len++;
	ms.mstate = STRAIGHT_FORWARD;
	break;
      case BACKWARD:
	cmd[len]='b'; len++; 
	ms.mstate = STRAIGHT_BACKWARD;
      break;
      case LEFT:
	cmd[len]='l'; len++; cmd[len]='b'; len++;
	cmd[len]='r'; len++; cmd[len]='f'; len++;
	ms.mstate = ROTATE_LEFT;
	break;
      case RIGHT:
	cmd[len]='l'; len++; cmd[len]='f'; len++;
	cmd[len]='r'; len++; cmd[len]='b'; len++;
	ms.mstate = ROTATE_RIGHT;
	break;   
      }
      if (len>1) {
        cmd[len]='g'; len++;
	cmd[len]='\n'; len++;
#ifdef __TRACE__
        write(1, cmd, len);
#endif
#ifndef SUPPRESS_MOTORCMDS
	write(fd, cmd, len);
#endif
      }
    }
  }
}

inline void
motion_change(char d, int s, int o)
{
  char cmd[16];
  int len=0;
  const int fd=motorBoard.fd;

  if ((s>=0 && s<=9) && (o>=0 && o<=4))  {
    if (ms.mstate != STOPPED) {
      switch (d) {
      case FORWARD:
	cmd[len]='0'+s; len++;
	cmd[len]='f'; len++;
	ms.mstate = STRAIGHT_FORWARD;
	break;
      case BACKWARD:
	cmd[len]='0'+s; len++;
	cmd[len]='b'; len++; 
	ms.mstate = STRAIGHT_BACKWARD;
	break;
      case LEFT:
	switch (ms.mstate) {
	case FWD_TURN_LEFT:
	case STRAIGHT_FORWARD:
	  {
	    int ls = (s-(o+1));
	    if (ls>=0) {
	      ls=(ls==0) ? '-' : '0' + ls; len++;
	      cmd[len]='l'; len++;
	      cmd[len]=ls; len++;
	      cmd[len]='l'; len++;
	      cmd[len]='f'; len++;
	    } else {
	      ls='0' + -(ls); len++;
	      cmd[len]='l'; len++;
	      cmd[len]=ls; len++;
	      cmd[len]='l'; len++;
	      cmd[len]='b'; len++;	      
	    } 
	    cmd[len]='r'; len++; 
	    cmd[len]='0' + s; len++;
	    cmd[len]='r'; len++;
	    cmd[len]='f'; len++;
	    ms.mstate = FWD_TURN_LEFT;
	  }
	  break;
	case STRAIGHT_BACKWARD:
	case BWD_TURN_RIGHT:
	  {
	    int ls = (s-(o+1));
	    if (ls>=0) {
	      ls=(ls==0) ? '-' : '0' + ls; len++;
	      cmd[len]='l'; len++;
	      cmd[len]=ls; len++;
	      cmd[len]='l'; len++;
	      cmd[len]='b'; len++;
	    } else {
	      ls='0' + -(ls); len++;
	      cmd[len]='l'; len++;
	      cmd[len]=ls; len++;
	      cmd[len]='l'; len++;
	      cmd[len]='f'; len++;	      
	    } 
	    cmd[len]='r'; len++; 
	    cmd[len]='0' + s; len++;
	    cmd[len]='r'; len++;
	    cmd[len]='b'; len++;
	    ms.mstate = BWD_TURN_LEFT;
	  }
	  break;
	case ROTATE_LEFT:
	  cmd[len]='0' + s; len++;
	  cmd[len]='l'; len++; cmd[len]='b'; len++;
	  cmd[len]='r'; len++; cmd[len]='f'; len++;
	  ms.mstate = ROTATE_LEFT;
	  break;
	}
	break;
      case RIGHT:
	switch (ms.mstate) {
	case FWD_TURN_RIGHT:
	case STRAIGHT_FORWARD:
	  {
	    int rs = (s-(o+1));
	    if (rs>=0) {
	      rs=(rs==0) ? '-' : '0' + rs; len++;
	      cmd[len]='r'; len++;
	      cmd[len]=rs; len++;
	      cmd[len]='r'; len++;
	      cmd[len]='f'; len++;
	    } else {
	      rs='0' + -(rs); len++;
	      cmd[len]='r'; len++;
	      cmd[len]=rs; len++;
	      cmd[len]='r'; len++;
	      cmd[len]='b'; len++;	      
	    } 
	    cmd[len]='l'; len++; 
	    cmd[len]='0' + s; len++;
	    cmd[len]='l'; len++;
	    cmd[len]='f'; len++;
	    ms.mstate = FWD_TURN_RIGHT;
	  }
	  break;
	case STRAIGHT_BACKWARD:
	case BWD_TURN_RIGHT:
	  {
	    int rs = (s-(o+1));
	    if (rs>=0) {
	      rs=(rs==0) ? '-' : '0' + rs; len++;
	      cmd[len]='r'; len++;
	      cmd[len]=rs; len++;
	      cmd[len]='r'; len++;
	      cmd[len]='b'; len++;
	    } else {
	      rs='0' + -(rs); len++;
	      cmd[len]='r'; len++;
	      cmd[len]=rs; len++;
	      cmd[len]='r'; len++;
	      cmd[len]='f'; len++;	      
	    } 
	    cmd[len]='l'; len++; 
	    cmd[len]='0' + s; len++;
	    cmd[len]='l'; len++;
	    cmd[len]='b'; len++;
	    ms.mstate = BWD_TURN_RIGHT;
	  }
	  break;
	case ROTATE_RIGHT:
	  cmd[len]='0' + s; len++;
	  cmd[len]='l'; len++; cmd[len]='f'; len++;
	  cmd[len]='r'; len++; cmd[len]='b'; len++;
	  ms.mstate = ROTATE_RIGHT;
	  break;   
	}
      }
      if (len>0) {
	cmd[len]='\n'; len++;
#ifdef __TRACE__
        write(1, cmd, len);
#endif
#ifndef SUPPRESS_MOTORCMDS
	write(fd, cmd, len);
#endif
      }
    }
  }
}

inline void
motion_end()
{
#ifdef __TRACE__
        write(1, "H\n", 2);
#endif
#ifndef SUPPRESS_MOTORCMDS
  write(motorBoard.fd, "H\n", 2);
#endif
  ms.mstate = STOPPED;
}

inline void
motion_init()
{
  bzero(&ms, sizeof(ms));
  motion_end();
}

int processLine(struct Connection *c)
{
  char *line=c->line;
  int len = c->len;
  if (len>0) {
#ifdef __TRACE__
     fprintf(stderr, "got: ");
     write(2, line, len);
     fprintf(stderr, "\ncmd=%c\n", line[0]);
#endif
    switch (line[0]) {
    case 'M': 
      if (len>1) {
	switch(line[1]) {
	case 'P':
	  if (motorBoard.owner == NULL || motorBoard.owner == c) {
	    int s, d;
	    if (sscanf(&line[2], "%d,%d", &s, &d) == 2) {
	      motion_polar(s,d,(motorBoard.owner) ? 0 : 1);
	      motorBoard.owner=c;
	    } 
	  }
	  break;
	case 'B':
	  if (motorBoard.owner==NULL) {
	    motorBoard.owner=c;
	    // MOTION BEGIN
	    if (len>2) {
	      char d;
	      int s,o;
	      if (sscanf(&line[2], "%c%d,%d", &d, &s, &o)==3) {
	      motion_begin(d,s,o);
	      }
	    }
	  }
	  break;
	case 'C':
	  if (motorBoard.owner==c) {
	    // MOTION CHANGE
	    if (len>2) {
	      char d;
	      int s,o;
	      if (sscanf(&line[2], "%c%d,%d", &d, &s, &o)==3) {
		motion_change(d,s,o);
	      }
	    }
	  }
	  break;
	case 'E':
	  if (motorBoard.owner==c) {
	    // MOTION END
	    motion_end();
	    motorBoard.owner=NULL;
	    break;
	  }
	}
      }
      break;
    case 'V':
      if (len>1) {
	char *msg=NULL; int mlen=0;
	switch(line[1]) {
	case 'v': 
            if (len>2) {
	      float v;
	      line[len]=0;
	      sscanf(&line[2], "%f", &v);
	      //	      fprintf(stderr, "setting volume to %f\n", v);
	      voice_volume(v);
	    }
	    break;
	case 'H':
	  msg = messages[HELLOMSG].str;
	  mlen = messages[HELLOMSG].len;
	  break;
	case 'W':
	  msg = messages[WAZUPMSG].str;
	  mlen = messages[WAZUPMSG].len;
	  break;
	case 'T':
	  msg = messages[TNKUMSG].str;
	  mlen = messages[TNKUMSG].len;
	  break;
	case 'Y':
	  msg = messages[YESMSG].str;
	  mlen = messages[YESMSG].len;
	  break;
	case 'N':
	  msg = messages[NOMSG].str;
	  mlen = messages[NOMSG].len;
	  break;
	case 'P':
	  msg = messages[PLSMSG].str;
	  mlen = messages[PLSMSG].len;
	  break;
	case 'C':
	  msg = messages[COOLMSG].str;
	  mlen = messages[COOLMSG].len;
	  break;
	case 'S':
	  msg = messages[SHANTMSG].str;
	  mlen = messages[SHANTMSG].len;
	  break;
	case 'R':
	  msg = messages[RAJAMSG].str;
	  mlen = messages[RAJAMSG].len;
	  break;
        case '"':
          if (len>2) {
	    msg=&(line[2]);
            mlen=len-2;
	  }
	  break;
	}
	if (msg && mlen) voice_say(msg, mlen);
      }
    }
  }
  return 1;
}

int 
main(int argc, char **argv)
{
  int port=PORT;
  int maxfd=0;
  int rc,i;
  fd_set rfds, efds, fdset;
  char greeting[160];

  if (argc!=1) { 
    if (argc!=3) {
      fprintf(stderr, 
	      "USAGE: cbserver <motordev eg. /dev/tty.usbmodem3B11> <sensordev eg. /dev/tty.usbserial-A9003Vnd>\n");
      return -1;
    }
  }

  FD_ZERO(&fdset);
  bzero(cons,sizeof(cons));

  if (argc == 3) {
    motorBoard.fd = serialport_init(argv[1],BAUD);
    if (motorBoard.fd < 0) {
      fprintf(stderr, "ERROR: failed to open motordev=%s\n", argv[1]);
      return -1;
    }
    FD_SET(motorBoard.fd, &fdset);
    if (motorBoard.fd>maxfd) maxfd=motorBoard.fd;
    
    sensorBoard.fd = serialport_init(argv[2],BAUD);
    if (sensorBoard.fd < 0) {
      fprintf(stderr, "ERROR: failed to open sensordev=%s\n", argv[1]);
      return -1;
    }
    FD_SET(sensorBoard.fd, &fdset);
    if (sensorBoard.fd>maxfd) maxfd=sensorBoard.fd;
  }

  if  (net_setup_listen_socket(&netfd, &port)<0) {
    fprintf(stderr, "ERROR: failed to open netfd=%d port=%d\n", netfd, port);
    return -1;
  }

  if  (net_listen(netfd)<0) {
    fprintf(stderr, "ERROR: failed to open netfd=%d port=%d\n", netfd, port);
    return -1;
  }

  FD_SET(netfd, &fdset);
  if (netfd>maxfd) maxfd=netfd;


  printf("motorfd=%d sensorfd=%d netfd=%d port=%d\n", 
	 motorBoard.fd, sensorBoard.fd, netfd, port);

  voice_init();
  voice_volume(0.1);

  snprintf(greeting, 160, "ClaraBell is ready and listening on port %d.", port);
  voice_say(greeting, strlen(greeting));

  //  sight_init();

  while (1)  {
    FD_COPY(&fdset, &rfds);
    FD_COPY(&fdset, &efds);
    rc = select(maxfd+1, &rfds, NULL, &efds, NULL);
    //    printf("rc=%d\n", rc);

    if (rc<0) {
      if (errno==EINTR) {
      } else {
	fprintf(stderr, "Error: pselect failed (%d)\n", errno);
	perror("select");
	return -1;
      }
    }
    
    if ((FD_ISSET(sensorBoard.fd, &efds) || (FD_ISSET(sensorBoard.fd, &rfds)))) {
      //     write(1,"+",1);
      sn=read(sensorBoard.fd, sbuf, BUFLEN);
      //     write(1, sbuf, sn);
      for (i=netfd+1; i<=maxfd; i++) {
	//	printf("i=%d n=%d\n", i, n);
	if (FD_ISSET(i, &fdset)) {
	  write(i, sbuf, sn);
	}
      }
    }
    
    if ((FD_ISSET(motorBoard.fd, &efds) || (FD_ISSET(motorBoard.fd, &rfds)))) {
      //      fprintf(stderr, "activity on motorfd=%d\n", motorfd);
      mn=read(motorBoard.fd, mbuf, BUFLEN);
      write(1, mbuf, mn);
    }
    
    if ((FD_ISSET(netfd, &efds) || (FD_ISSET(netfd, &rfds)))) {
      fprintf(stderr, "connection on netfd=%d\n", netfd);
      int fd = net_accept(netfd);
      FD_SET(fd, &fdset);
      if (fd > maxfd) maxfd = fd;
      assert(cons[fd]==NULL);
      cons[fd]=(struct Connection *)malloc(sizeof(struct Connection));
      cons[fd]->fd = fd;
      cons[fd]->len = 0;
      fprintf(stderr, "new connection on fd=%d\n", fd);
    }
    
    for (i = netfd+1; i <= maxfd; i++) {
      if (FD_ISSET(i, &rfds) || FD_ISSET(i, &efds)) { 
	//	printf("activity on fd=%d", i);
	nn=read(i, nbuf, BUFLEN);
	//	printf(" nn=%d\n", nn);
	if (nn>0) {
	  struct Connection *c = cons[i];
	  assert(c!=NULL && c->fd==i);
	  //	  fprintf(stderr, "got data %d on %d:\n", nn, i);
	  for (i=0; i<nn; i++) {
	    c->line[c->len] = nbuf[i];
	    c->len++;
	    if (c->len == LINELEN-1 || c->line[c->len-1]=='\n') {
	      c->line[c->len]=0;
	      processLine(c); 
	      c->len=0;
	    }
	  }
	} else { 
	  //	  printf("errno=%d\n", errno); perror("read <=0");
          if (errno != EWOULDBLOCK || nn==0) {
	    fprintf(stderr, "ERROR on %d closing it nn=%d errno=%d\n", i, nn, errno);
	    close(i);
	    FD_CLR(i, &fdset);
	    if (motorBoard.owner==cons[i]) {
	      motion_end();
	      motorBoard.owner=NULL;
	    }
	    if (sensorBoard.owner==cons[i]) {
	      sensorBoard.owner=NULL;
	    }
	    free(cons[i]);
	    cons[i]=NULL;
	  }
	}
      }
    }
    
  } 
  return 1;
}

