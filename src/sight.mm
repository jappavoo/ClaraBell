#import <AppKit/AppKit.h>
#import <QTKit/QTKit.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SIGHT_REPEAT_DELAY 6

#ifdef __ppc__
#import "isight/CocoaSequenceGrabber.h"
#endif

#include "sight.h"
#include "net.h"

void
sight_send_image(int len, char *bytes)
{
  if (len==0 || bytes==NULL) return;
  /*
   * Write out picture to to destinations
   */
  // under the assumption that the list is relatively stable
  // terminate send to the number of destinations that exist at the 
  // start
  int num=sight.dst_cnt;
  for (int i=0; i<SIGHT_MAX_DESTINATIONS; i++) {
    int fd = sight.destinations[i];
    if (fd > 0 ) {
      int n;
      //      fprintf(stderr, "sending picture %d bytes to fd=%d\n", len, fd);
      int nlen = htonl(len);
      if ((n=net_writen(fd, &nlen, sizeof(nlen)))!=sizeof(len)) {
	fprintf(stderr, "net_writen: failed writing %ld bytes n=%d closing fd=%d\n", sizeof(len), n, fd);
	sight_remove_destination(i,fd);
      } else {
	if ((n=net_writen(fd, bytes, len))!=len) {
	  fprintf(stderr, "net_writen: failed writing %d bytes n=%d closing fd=%d\n", len, n, fd);
	  sight_remove_destination(i,fd);
	}
      }
      num--;
    }
    if (num==0) break;
  }
}

#ifdef __ppc__

CFMutableDataRef picture;
CSGCamera *camera;

BOOL shouldKeepRunning = YES; 

// copied from isight code found on the net (see isight subdirectory).  Modelled on main
/*
 * This delegate handles the didReceiveFrame callback from CSGCamera,
 * which we use to convert the image to a JPEG.
 */
@interface CSGCameraDelegate : CSGCamera
{
    CFMutableDataRef data;
}

/*
 * Assign a CFMutableDataRef to receive JPEG image data
 */
- (void)setDataRef:(CFMutableDataRef)dataRef;

/*
 * Convert captured frame into a JPEG datastream, stored in a CFDataRef
 */
- (void)camera:(CSGCamera *)aCamera didReceiveFrame:(CSGImage *)aFrame;

@end

@implementation CSGCameraDelegate

- (void)setDataRef:(CFMutableDataRef)dataRef
{
    data = dataRef;
}

- (void)camera:(CSGCamera *)aCamera didReceiveFrame:(CSGImage *)aFrame;
{
    // First, we must convert to a TIFF bitmap
    NSBitmapImageRep *imageRep = 
        [NSBitmapImageRep imageRepWithData: [aFrame TIFFRepresentation]];
    
    NSNumber *quality = [NSNumber numberWithFloat: 1.0];
    
    NSDictionary *props = 
        [NSDictionary dictionaryWithObject:quality
                      forKey:NSImageCompressionFactor];

    // Now convert TIFF bitmap to JPEG compressed image
    NSData *jpeg = 
        [imageRep representationUsingType: NSJPEGFileType properties:props];

    // Store JPEG image in a CFDataRef
    CFIndex jpegLen = CFDataGetLength((CFDataRef)jpeg);
    CFDataSetLength(data, jpegLen);
    CFDataReplaceBytes(data, CFRangeMake((CFIndex)0, jpegLen), 
        CFDataGetBytePtr((CFDataRef)jpeg), jpegLen);

    // Stop the camera and signal that we should exit the run loop    
    [aCamera stop];
    shouldKeepRunning = NO;
}
@end

CSGCameraDelegate *delegate;

void 
sight_run(void)
{
  
  //    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
  shouldKeepRunning = YES;

  // initiate the capture
  [camera startWithSize:NSMakeSize(320, 240)]; 
  // [camera start]; 
  /*
   * Execute RunLoop until global flag is cleared
   */
  NSRunLoop *theRL = [NSRunLoop currentRunLoop];
  while (shouldKeepRunning && [theRL runMode:NSDefaultRunLoopMode 
			       beforeDate:[NSDate distantFuture]]);
  
  sight_send_image(CFDataGetLength(picture), (char *)CFDataGetBytePtr(picture));
}

#endif

struct Sight sight; 

void
sight_take(void)
{
  int rc;
  pthread_mutex_lock(&sight.mutex);
  if (!sight.take_pic) { 
    sight.take_pic=1;
    rc = pthread_cond_signal(&sight.cond);
    assert(rc==0);
  }
  pthread_mutex_unlock(&sight.mutex);
}


void
sight_start_repeat(void)
{
  int rc;
  pthread_mutex_lock(&sight.mutex);
  if (!sight.repeat) { 
    //    fprintf(stderr, "setting sight.repeat=1\n");
    sight.repeat=1;
    rc = pthread_cond_signal(&sight.cond);
    assert(rc==0);
  }
  pthread_mutex_unlock(&sight.mutex);
}


void
sight_stop_repeat(void)
{
  int rc;
  pthread_mutex_lock(&sight.mutex);
  sight.repeat=0;
  sight.take_pic=0;
  pthread_mutex_unlock(&sight.mutex);
}


void *sight_loop(void *arg)
{
  int rc;

  rc = pthread_mutex_lock(&sight.mutex);
  assert(rc==0);

#ifdef __ppc__
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  /*
   * Use CocoaSequenceGrabber to capture a single image from the
   * iSight camera and store it as a JPEG data stream in picture.
   */
  picture = CFDataCreateMutable(NULL, 0);
  delegate = [[CSGCameraDelegate alloc] init];
  [delegate setDataRef:picture];
  
  camera = [[CSGCamera alloc] init];
  [camera setDelegate:delegate];

  //  [camera setupWithSize:NSMakeSize(320, 240)]; 
#endif

  while (1) {
    if (sight.repeat) {
      //      fprintf(stderr, "sight.repeat=%d sleeping ...\n", sight.repeat);
      sleep(SIGHT_REPEAT_DELAY);
    } else {
      while (!sight.take_pic) {
	//      printf("sight_loop:  blocking\n");
	rc = pthread_cond_wait(&sight.cond, &sight.mutex); // unlocks mutex on entry 
	// reacquires lock before return
	assert(rc==0);
	if (sight.repeat) break;
      }
      
      rc = pthread_mutex_unlock(&sight.mutex);
      assert(rc==0);
    }
#ifndef __ppc__
  char *ibytes=NULL;
  int ilen=0;
  struct stat ss;
  fprintf(stderr, "checking for cb.img\n");
  if (stat("cb.img", &ss)==0 && ss.st_size) {
    fprintf(stderr, "stat succeded\n");
    int fd = open("cb.img", O_RDONLY);
    fprintf(stderr, "open succeded\n");
    if (fd!=-1) {
      ibytes = (char *) malloc(ss.st_size);
      if (read(fd, ibytes, ss.st_size)==ss.st_size) {
	ilen=ss.st_size;
	fprintf(stderr, "read succeded %d\n", ilen);
      } else { perror("read"); free(ibytes); ibytes=NULL; }
      close(fd);
      sight_send_image(ilen, ibytes);
      free(ibytes); ilen=0;
    } else perror("open");  
  } else perror("stat");


#else
    sight_run();
#endif

    assert(rc==0);
    if (!sight.repeat) {
      rc=pthread_mutex_lock(&sight.mutex);
      sight.take_pic = 0;
    }
  }
#ifdef __ppc__
  [pool release];
#endif
  return NULL;
}


int
sight_init()
{
  int rc;
  bzero(&sight, sizeof(sight));
  
  memset((void *)&(sight.destinations), -1, sizeof(sight.destinations));

  rc = pthread_cond_init(&sight.cond, NULL);
  assert(rc==0);
  rc = pthread_mutex_init(&sight.mutex, NULL);
  assert(rc==0);

  if (pthread_create(&(sight.tid), NULL, sight_loop, NULL)!=0) return -1;
  return 1;
}



