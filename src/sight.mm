#import <AppKit/AppKit.h>
#import <QTKit/QTKit.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#ifdef __ppc__
#import "isight/CocoaSequenceGrabber.h"
#endif

#include "sight.h"



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
sight_run(int fd)
{
  
  //    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    
  shouldKeepRunning = YES;

  // initiate the capture
  [camera startWithSize:NSMakeSize(320, 240)]; 
  /*
   * Execute RunLoop until global flag is cleared
   */
  NSRunLoop *theRL = [NSRunLoop currentRunLoop];
  while (shouldKeepRunning && [theRL runMode:NSDefaultRunLoopMode 
			       beforeDate:[NSDate distantFuture]]);
  
  /*
   * Write out picture to to socket
   */
  int file=0;
  if (fd == 0)  { 
    fd=open("/tmp/cb.jpg", O_CREAT|O_TRUNC|O_WRONLY, 0666);
    if (fd>0) { 
	fprintf(stderr, "opened: /tmp/cb.jpg on %d\n", fd);
	file=1;
      } else {
	fd=0;
	perror("opening /tmp/cb.jpg");
      }
    }

    if (fd >0 ) {
      size_t len = CFDataGetLength(picture);
      int n;
      //        write(socket, &len, sizeof(len));
      n=write(fd, CFDataGetBytePtr(picture), len);
      if (n<=0) perror("write of image data failed");
      fprintf(stderr, "written %d bytes image of len=%d on fd=%d\n",
	      n, len, fd);
      if (file) { 
	fprintf(stderr, "closing /tmp/cb.jpg\n");
	close(fd);
      }
    } else {
      fprintf(stderr, "Failed to send the picture anywhere\n");
    }
}

#endif







#define SIGHT_IMAGE_LEN 200 * 1024
char sight_image[SIGHT_IMAGE_LEN];

struct Sight sight; 


void
sight_take(int sendfd)
{
  int rc;
  pthread_mutex_lock(&sight.mutex);
  if (!sight.take_pic) { 
    sight.send_fd = sendfd;
    sight.take_pic=1;
    rc = pthread_cond_signal(&sight.cond);
    assert(rc==0);
  }
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
  
#endif

  while (1) {
    while (!sight.take_pic) {
      printf("sight_loop:  blocking\n");
      rc = pthread_cond_wait(&sight.cond, &sight.mutex); // unlocks mutex on entry 
                                                         // requires before return
      assert(rc==0);
    }

    rc = pthread_mutex_unlock(&sight.mutex);
    assert(rc==0);

#ifndef __ppc__
    fprintf(stderr, "sight_loop: taking picture\n");
    sleep(20);
    fprintf(stderr, "sight_loop: picture ready\n");
#else
    sight_run(sight.send_fd);
#endif

    rc=pthread_mutex_lock(&sight.mutex);
    assert(rc==0);
    sight.take_pic = 0;
  }
#ifdef __ppc__
  [pool release];
#endif
  return NULL;
}

#ifdef __ppc__
void
sight_init_camera(void)
{
  
}
#endif

int
sight_init()
{
  int rc;
  bzero(&sight, sizeof(sight));
  sight.img = sight_image;
  
  rc = pthread_cond_init(&sight.cond, NULL);
  assert(rc==0);
  rc = pthread_mutex_init(&sight.mutex, NULL);
  assert(rc==0);

#ifdef __ppc__
  sight_init_camera();
#endif
  if (pthread_create(&(sight.tid), NULL, sight_loop, NULL)!=0) return -1;
  return 1;
}



#if 0
int
sight_init(void)	
{	
  int rc;
  QTCaptureDevice *dev = NULL;
  QTCaptureDeviceInput *camera = NULL;
  NSError *error;

  cameraSession = [[QTCaptureSession alloc] init];
  dev = [QTCaptureDevice defaultInputDeviceWithMediaType:QTMediaTypeVideo];
  if (dev) {
    rc = [dev open:&error];
    if (!rc) {
      fprintf(stderr, "Error opening camera\n");
      return -1;
    }
    camera = [[QTCaptureDeviceInput alloc] initWithDevice:dev];
    rc = [cameraSession addInput:camera error:&error];
    if (!rc) {
      fprintf(stderr, "Error establishing a capture session from the camera\n");
      return -1;
    }
    cameraOutput = [[QTCaptureDecompressedVideoOutput alloc] init];

  } else {
    fprintf(stderr, "No Camera found\n");
    return -1;
  }
  return 1;
}

#if 0
int sight_pic(void)
{
	
    CVImageBufferRef frame = nil;               // Hold frame we find
    while( frame == nil ){                      // While waiting for a frame
		
		//verbose( "\tEntering synchronized block to see if frame is captured yet...");
        @synchronized(self){                    // Lock since capture is on another thread
            frame = mCurrentImageBuffer;        // Hold current frame
            CVBufferRetain(frame);              // Retain it (OK if nil)
        }   // end sync: self
		//verbose( "Done.\n" );
		
        if( frame == nil ){                     // Still no frame? Wait a little while.
            [[NSRunLoop currentRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow: 0.1]];
        }   // end if: still nothing, wait
		
    }   // end while: no frame yet
    
    // Convert frame to an NSImage
    NSCIImageRep *imageRep = [NSCIImageRep imageRepWithCIImage:[CIImage imageWithCVImageBuffer:frame]];
    NSImage *image = [[[NSImage alloc] initWithSize:[imageRep size]] autorelease];
    [image addRepresentation:imageRep];
}
#endif

#endif
