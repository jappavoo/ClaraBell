#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <string.h>
#include "arduino-serial.h"
#include "net.h"
#include "voice.h"

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
char line[LINELEN];
int linelen=0;

int motorfd;
int sensorfd;
int netfd;

int processLine(char *line, int len)
{
  if (len>0) {
    //    fprintf(stderr, "%c: %s", line[0], &line[1]);
    switch (line[0]) {
    case 'M': 
      if (len>1) write(motorfd, &line[1], len-1);
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
	      fprintf(stderr, "setting volume to %f\n", v);
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

  if (argc == 3) {
    motorfd = serialport_init(argv[1],BAUD);
    if (motorfd < 0) {
      fprintf(stderr, "ERROR: failed to open motordev=%s\n", argv[1]);
      return -1;
    }
    FD_SET(motorfd, &fdset);
    if (motorfd>maxfd) maxfd=motorfd;
    
    sensorfd = serialport_init(argv[2],BAUD);
    if (sensorfd < 0) {
      fprintf(stderr, "ERROR: failed to open sensordev=%s\n", argv[1]);
      return -1;
    }
    FD_SET(sensorfd, &fdset);
    if (sensorfd>maxfd) maxfd=sensorfd;
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
	 motorfd, sensorfd, netfd, port);

  voice_init();
  voice_volume(0.1);

  snprintf(greeting, 160, "ClaraBell is ready and listening on port %d.", port);
  voice_say(greeting, strlen(greeting));

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
    
    if ((FD_ISSET(sensorfd, &efds) || (FD_ISSET(sensorfd, &rfds)))) {
      //     write(1,"+",1);
      sn=read(sensorfd, sbuf, BUFLEN);
      //     write(1, sbuf, sn);
      for (i=netfd+1; i<=maxfd; i++) {
	//	printf("i=%d n=%d\n", i, n);
	if (FD_ISSET(i, &fdset)) {
	  write(i, sbuf, sn);
	}
      }
    }
    
    if ((FD_ISSET(motorfd, &efds) || (FD_ISSET(motorfd, &rfds)))) {
      //      fprintf(stderr, "activity on motorfd=%d\n", motorfd);
      mn=read(motorfd, mbuf, BUFLEN);
      write(1, mbuf, mn);
    }
    
    if ((FD_ISSET(netfd, &efds) || (FD_ISSET(netfd, &rfds)))) {
      fprintf(stderr, "connection on netfd=%d\n", netfd);
      int fd = net_accept(netfd);
      FD_SET(fd, &fdset);
      if (fd > maxfd) maxfd = fd;
      fprintf(stderr, "new connection on fd=%d\n", fd);
    }
    
    for (i = netfd+1; i <= maxfd; i++) {
      if (FD_ISSET(i, &rfds) || FD_ISSET(i, &efds)) { 
	//	printf("activity on fd=%d", i);
	nn=read(i, nbuf, BUFLEN);
	//	printf(" nn=%d\n", nn);
	if (nn>0) {
	  //	  fprintf(stderr, "got data %d on %d:\n", nn, i);
	  for (i=0; i<nn; i++) {
	    line[linelen] = nbuf[i];
	    linelen++;
	    if (linelen == LINELEN-1 || line[linelen-1]=='\n') {
	      line[linelen]=0;
	      processLine(line, linelen); 
	      linelen=0;
	    }
	  }
	} else { 
	  //	  printf("errno=%d\n", errno); perror("read <=0");
          if (errno != EWOULDBLOCK || nn==0) {
	    fprintf(stderr, "ERROR on %d closing it nn=%d errno=%d\n", i, nn, errno);
	    close(i);
	    FD_CLR(i, &fdset);
	  }
	}
      }
    }
    
  } 
  return 1;
}

