#import <AppKit/AppKit.h>
#import <QTKit/QTKit.h>
//#include <stdio.h>

QTCaptureSession *cameraSession;
QTCaptureDecompressedVideoOutput *cameraOutput;
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
