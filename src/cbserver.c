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
    fprintf(stderr, "%c: %s", line[0], &line[1]);
    switch (line[0]) {
    case 'M': 
      if (len>1) write(motorfd, &line[1], len-1);
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
  char *hello[] = { "Hello" };

  voice_say(1, hello);

  if (argc==1) { 
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


  while (1)  {
    FD_COPY(&fdset, &rfds);
    FD_COPY(&fdset, &efds);
    rc = select(maxfd+1, &rfds, NULL, &efds, NULL);
    
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
      if ((FD_ISSET(i, &rfds)) || (( FD_ISSET(i, &efds) ))) {
	//	printf("activity on fd=%d\n", i);
	nn=read(i, nbuf, BUFLEN);
	if (nn>0) {
	  fprintf(stderr, "got data %d on %d:\n", nn, i);
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
          if (errno != EWOULDBLOCK) {
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

