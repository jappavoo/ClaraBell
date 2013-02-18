#include <SDL.h>
#include <math.h>

int NYI(void) { fprintf(stderr, "NYI\n"); return 1; }

volatile int s0, s1, s2, s3, prox;
SDL_Rect r0, r1, r2, r3;

#define RANGE 200 
#define BORDER 25

#define W ((RANGE + BORDER)*2)
#define H ((RANGE + BORDER)*2)
#define MIDX (W/2)
#define MIDY (H/2)

struct UI_Struct {
  SDL_Surface *screen;
  int32_t depth;

  uint32_t red_c;
  uint32_t green_c;
  uint32_t blue_c;
  uint32_t white_c;
  uint32_t black_c;
  uint32_t yellow_c;
  uint32_t purple_c;
} ui;


/****
Next two routines for drawing circles and filled circles are from 
http://content.gpwiki.org/index.php/SDL:Tutorials:Drawing_and_Filling_Circles
 ****/

/*
 * This is a 32-bit pixel function created with help from this
 * website: http://www.libsdl.org/intro.en/usingvideo.html
 *
 * You will need to make changes if you want it to work with
 * 8-, 16- or 24-bit surfaces.  Consult the above website for
 * more information.
 */
void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32 *)target_pixel = pixel;
}
 
/*
 * This is an implementation of the Midpoint Circle Algorithm 
 * found on Wikipedia at the following link:
 *
 *   http://en.wikipedia.org/wiki/Midpoint_circle_algorithm
 *
 * The algorithm elegantly draws a circle quickly, using a
 * set_pixel function for clarity.
 */
void draw_circle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint32 pixel)
{
    // if the first pixel in the screen is represented by (0,0) (which is in sdl)
    // remember that the beginning of the circle is not in the middle of the pixel
    // but to the left-top from it:
 
    double error = (double)-radius;
    double x = (double)radius -0.5;
    double y = (double)0.5;
    double cx = n_cx - 0.5;
    double cy = n_cy - 0.5;
 
    while (x >= y)
    {
        set_pixel(surface, (int)(cx + x), (int)(cy + y), pixel);
        set_pixel(surface, (int)(cx + y), (int)(cy + x), pixel);
 
        if (x != 0)
        {
            set_pixel(surface, (int)(cx - x), (int)(cy + y), pixel);
            set_pixel(surface, (int)(cx + y), (int)(cy - x), pixel);
        }
 
        if (y != 0)
        {
            set_pixel(surface, (int)(cx + x), (int)(cy - y), pixel);
            set_pixel(surface, (int)(cx - y), (int)(cy + x), pixel);
        }
 
        if (x != 0 && y != 0)
        {
            set_pixel(surface, (int)(cx - x), (int)(cy - y), pixel);
            set_pixel(surface, (int)(cx - y), (int)(cy - x), pixel);
        }
 
        error += y;
        ++y;
        error += y;
 
        if (error >= 0)
        {
            --x;
            error -= x;
            error -= x;
        }
    }
}

/*
 * SDL_Surface 32-bit circle-fill algorithm without using trig
 *
 * While I humbly call this "Celdecea's Method", odds are that the 
 * procedure has already been documented somewhere long ago.  All of
 * the circle-fill examples I came across utilized trig functions or
 * scanning neighbor pixels.  This algorithm identifies the width of
 * a semi-circle at each pixel height and draws a scan-line covering
 * that width.  
 *
 * The code is not optimized but very fast, owing to the fact that it
 * alters pixels in the provided surface directly rather than through
 * function calls.
 *
 * WARNING:  This function does not lock surfaces before altering, so
 * use SDL_LockSurface in any release situation.
 */
void fill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel)
{
    // Note that there is more to altering the bitrate of this 
    // method than just changing this value.  See how pixels are
    // altered at the following web page for tips:
    //   http://www.libsdl.org/intro.en/usingvideo.html
    static const int BPP = 4;
    double dy;
    double r = (double)radius;
 
    for (dy = 1; dy <= r; dy += 1.0)
    {
        // This loop is unrolled a bit, only iterating through half of the
        // height of the circle.  The result is used to draw a scan line and
        // its mirror image below it.
 
        // The following formula has been simplified from our original.  We
        // are using half of the width of the circle because we are provided
        // with a center and we need left/right coordinates.
 
        double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
        int x = cx - dx;
 
        // Grab a pointer to the left-most pixel for each half of the circle
        Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * BPP;
        Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * BPP;
 
        for (; x <= cx + dx; x++)
        {
            *(Uint32 *)target_pixel_a = pixel;
	    *(Uint32 *)target_pixel_b = pixel;
            target_pixel_a += BPP;
            target_pixel_b += BPP;
        }
    }
}
ui_paint(void)
{
  uint32_t r0_col, r1_col, r2_col, r3_col;
#ifndef RECT
  #define TAN15 0.2679492
  int x,h;
  int y;
  double end,start;
#endif
  //  printf("%d %d %d %d %04x\n", s0, s1, s2, s3, prox);
#if 1
  if (s0 == 0) s0=2;
  if (s1 == 0) s1=2;
  if (s2 == 0) s2=2;
  if (s3 == 0) s3=2;
#endif
#ifdef RECT
  SDL_FillRect(ui.screen, &r0, ui.black_c);
  SDL_FillRect(ui.screen, &r1, ui.black_c);
  SDL_FillRect(ui.screen, &r2, ui.black_c);
  SDL_FillRect(ui.screen, &r3, ui.black_c);
#endif
#ifndef RECT
  r0_col = (prox & (1<<1)) ? ui.yellow_c : ui.green_c;
  for (h=0; h<=s1; h++) {
    end=MIDX+(double)h * TAN15;
    start=MIDX-(double)h * TAN15;
    y = MIDY - h;
    for (x=start; x<end; x++) {
      set_pixel(ui.screen, x, y, r0_col);
    }
  }
  for (h=s1+1;h<=RANGE>0;h++) {
    end=MIDX+(double)h * TAN15;
    start=MIDX-(double)h * TAN15;
    y = MIDY - h;
    for (x=start; x<end; x++) {
      set_pixel(ui.screen, x, y, ui.black_c);
    }
  }
#else
  r0.w = 20;
  r0.h = s0;
  r0.x = W/2 + 2;
  r0.y = H/2-s0;
  r0_col = (prox & (1<<0)) ? ui.yellow_c : ui.red_c;
#endif

#ifndef RECT
  r1_col = (prox & (1<<0)) ? ui.yellow_c : ui.red_c;
  for (h=0; h<s0; h++) {
    end=MIDY+(double)h * TAN15;
    start=MIDY-(double)h * TAN15;
    x = MIDX + h;
    for (y=start; y<end; y++) {
      set_pixel(ui.screen, x, y, r1_col);
    }
  }
  for (h=s0;h<RANGE>0;h++) {
    end=MIDY+(double)h * TAN15;
    start=MIDY-(double)h * TAN15;
    x = MIDX + h;
    for (y=start; y<end; y++) {
      set_pixel(ui.screen, x, y, ui.black_c);
    }
  }
#else
  r1.w = s1;
  r1.h = 20;
  r1.x = W/2;
  r1.y = H/2-10;
  r1_col = (prox & (1<<1)) ? ui.yellow_c : ui.red_c;
#endif

#ifndef RECT
  r2_col = (prox & (1<<2)) ? ui.yellow_c : ui.red_c;
  for (h=0; h<=s2; h++) {
    end=MIDY+(double)h * TAN15;
    start=MIDY-(double)h * TAN15;
    x = MIDX - h;
    for (y=start; y<end; y++) {
      set_pixel(ui.screen, x, y, r2_col);
    }
  }
  for (h=s2;h<=RANGE>0;h++) {
    end=MIDY+(double)h * TAN15;
    start=MIDY-(double)h * TAN15;
    x = MIDX - h;
    for (y=start; y<end; y++) {
      set_pixel(ui.screen, x, y, ui.black_c);
    }
  }
#else
  r2.w = s2;
  r2.h = 20;
  r2.x = W/2-s2;
  r2.y = H/2-10;
  r2_col = (prox & (1<<2)) ? ui.yellow_c : ui.green_c;
#endif

#ifndef RECT
  r3_col = (prox & (1<<3)) ? ui.yellow_c : ui.red_c;
  for (h=0; h<s3; h++) {
    end=MIDX+(double)h * TAN15;
    start=MIDX-(double)h * TAN15;
    y = MIDY + h;
    for (x=start; x<end; x++) {
      set_pixel(ui.screen, x, y, r3_col);
    }
  }
  for (h=s3;h<RANGE>0;h++) {
    end=MIDX+(double)h * TAN15;
    start=MIDX-(double)h * TAN15;
    y = MIDY + h;
    for (x=start; x<end; x++) {
      set_pixel(ui.screen, x, y, ui.black_c);
    }
  }
#else
  r3.w = 20;
  r3.h = s3;
  r3.x = W/2-22;
  r3.y = H/2-s3;
  r3_col = (prox & (1<<3)) ? ui.yellow_c : ui.green_c;
#endif

#ifdef RECT
  SDL_FillRect(ui.screen, &r0, r0_col);
  SDL_FillRect(ui.screen, &r1, r1_col);
  SDL_FillRect(ui.screen, &r2, r2_col);
  SDL_FillRect(ui.screen, &r3, r3_col);
#endif

  SDL_UpdateRect(ui.screen, 0, 0, W, H);
}

ui_init(int32_t d)
{

  atexit(SDL_Quit);
  ui.depth        = d;
  ui.screen = SDL_SetVideoMode(W, H, ui.depth, SDL_SWSURFACE);
   if ( ui.screen == NULL ) {
    fprintf(stderr, "Couldn't set %dx%dx%d video mode: %s\n", W, H, ui.depth, 
	    SDL_GetError());
    return -1;
  }


  ui.black_c      = SDL_MapRGB(ui.screen->format, 0x00, 0x00, 0x00);
  ui.white_c      = SDL_MapRGB(ui.screen->format, 0xff, 0xff, 0xff);
  ui.red_c        = SDL_MapRGB(ui.screen->format, 0xff, 0x00, 0x00);
  ui.green_c      = SDL_MapRGB(ui.screen->format, 0x00, 0xff, 0x00);
  ui.blue_c       = SDL_MapRGB(ui.screen->format, 0x00, 0x00, 0x00);
  ui.yellow_c     = SDL_MapRGB(ui.screen->format, 0xff, 0xff, 0x00);
  ui.purple_c     = SDL_MapRGB(ui.screen->format, 0xff, 0x00, 0xff);

  draw_circle(ui.screen, W/2, H/2, RANGE, ui.white_c);
  SDL_UpdateRect(ui.screen, 0, 0, W, H);
  // ui_splash();
}



void *
shell(void *arg)
{
  char buf[80];
  prox;
  SDL_Event event;

  pthread_detach(pthread_self());

  while (1) {
    fgets(buf, 80, stdin);
    sscanf(buf, "%d %d %d %d %d", &s0, &s1, &s2, &s3, &prox);
    event.type      = SDL_USEREVENT;
    event.user.code = 1;
    SDL_PushEvent(&event);
  }

  //  fprintf(stderr, "terminating\n");
  fflush(stdout);
  return NULL;
}


struct JoyStick {
  int num;
  SDL_Joystick *hdl;
  char name[80];
  int numButtons;
  int numAxis;
  int numHats;
  int numBalls;
} *sticks;

int
button(SDL_JoyButtonEvent *e)
{
  int rc = 1;
  Uint8 b = e->button;
  Uint8 s = e->state;

  switch (b) {
  case 0:
    if (s == SDL_PRESSED) printf("g\n");
    else printf("h\n");
    break;
  case 1:
    if (s == SDL_PRESSED) printf("-\n");
    break;
  case 2:
    if (s == SDL_PRESSED) printf("a\n");
    break;
  case 3:
    if (s == SDL_PRESSED) printf("d\n");
    break;
  case 4:
    if (s == SDL_PRESSED) printf("S\n");
    break;
  case 5:
    if (s == SDL_PRESSED) printf("I\n");
    break;
  case 6:
    if (s == SDL_PRESSED) printf("R\n");
    break;
  case 7:
    if (s == SDL_PRESSED) printf("m\n");
    break;
  case 8:
  case 9:
  default:
#if 0
    fprintf(stderr, "button: %d: ", b);
    if (s == SDL_PRESSED) fprintf(stderr," DOWN\n");
    else fprintf(stderr, " UP\n");
#endif
    break;
  }
}

int 
axismotion(SDL_JoyAxisEvent *e)
{
  int rc = 1;
  Uint8 axis = e->axis;
  Sint16 value = e->value;

  if ( (value < -3200 ) || (value > 3200 ) ) {
    switch (axis) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    default:
      //fprintf(stderr, "axis %d: %d\n", axis, value);
      break;
    }
    return rc;
  }
}

int 
hatmotion(SDL_JoyHatEvent *e)
{
  int rc = 1;
  Uint8 value = e->value;

  if ( value & SDL_HAT_UP ){
    printf("f\n");
  }
  if ( value & SDL_HAT_DOWN ){
    printf("b\n");
  }
  if ( value & SDL_HAT_RIGHT ){
    printf("lfrb\n");
  }
  if ( value & SDL_HAT_LEFT ){
    printf("rflb\n");
  }
  return rc;
}

int 
keypress(SDL_KeyboardEvent *e)
{
  SDLKey sym = e->keysym.sym;
  SDLMod mod = e->keysym.mod;

  if (e->type == SDL_KEYDOWN) {
    if (sym == SDLK_LEFT && mod == KMOD_NONE) 
      return NYI();
    if (sym == SDLK_RIGHT && mod == KMOD_NONE) 
      return NYI();
    if (sym == SDLK_UP && mod == KMOD_NONE)    
      return NYI();
    if (sym == SDLK_DOWN && mod == KMOD_NONE)  
      return NYI();
    if (sym == SDLK_p && mod == KMOD_NONE)    
      return NYI();
    if (sym == SDLK_d && mod == KMOD_NONE)     
      return NYI();
    if (sym == SDLK_q) return -1;
    if (sym == SDLK_z && mod == KMOD_NONE) return NYI();
    if (sym == SDLK_z && mod & KMOD_SHIFT ) return NYI();
    if (sym == SDLK_LEFT && mod & KMOD_SHIFT) return NYI();
    if (sym == SDLK_RIGHT && mod & KMOD_SHIFT) return NYI();
    if (sym == SDLK_UP && mod & KMOD_SHIFT) return NYI();
    if (sym == SDLK_DOWN && mod & KMOD_SHIFT) return NYI();
    else {
      //fprintf(stderr, "key pressed: %d\n", sym); 
    }
  } else {
    //fprintf(stderr, "key released: %d\n", sym);
  }

}

int
main(int argc, char **argv)
{
  int i, n;
  SDL_Event e;
  int rc=1;
  pthread_t tid;
  struct JoyStick *s;

  if (SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK ) < 0) {
      //fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
      exit(-1);
  }

  n = SDL_NumJoysticks();
  //fprintf(stderr, "%i joysticks were found.\n\n", n);
  //  if (n == 0) //fprintf(stderr, "No Joysticks found exiting\n");


  sticks = (struct JoyStick *)malloc(n * sizeof(struct JoyStick));

  SDL_JoystickEventState(SDL_ENABLE);  
  //fprintf(stderr, "The joysticks:\n");
  for ( i=0; i<n; i++) {
    s = &sticks[i];
    s->num = i;
    strncpy(s->name,SDL_JoystickName(i),80);
    s->hdl        = SDL_JoystickOpen(0);
    s->numButtons = SDL_JoystickNumButtons(s->hdl);
    s->numAxis    = SDL_JoystickNumButtons(s->hdl);
    s->numHats    = SDL_JoystickNumHats(s->hdl);
    s->numBalls   = SDL_JoystickNumBalls(s->hdl);
    //fprintf(stderr, "    %s: buttons: %d axis: %d hats: %d balls: %d\n", 
    //s->name, s->numButtons, s->numAxis, s->numHats, s->numBalls);
  }

  ui_init(32);
  s0 = s1 = s2 = s3 = prox = 0;
  pthread_create(&tid, NULL, shell, NULL);

  while(SDL_WaitEvent(&e)) {
    switch (e.type) {
    case SDL_QUIT:
      return -1;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      rc = keypress(&(e.key));
      break;
    case SDL_JOYHATMOTION:  /* Handle Hat Motion */
      rc = hatmotion(&e.jhat);
      break;
    case SDL_JOYAXISMOTION:
      rc = axismotion(&e.jaxis);
      break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      rc = button(&e.jbutton);
      break;
    case SDL_USEREVENT:
      //      printf("%d %d %d %d %04x\n", s0, s1, s2, s3, prox);
      ui_paint();
      break;
    default:
      //fprintf(stderr, "e.type=%d NOT Handled\n", e.type);
      break;
    }
    if (rc<0) break;
    fflush(stdout);
  }

  for ( i=0; i<n; i++) {
    SDL_JoystickClose(sticks[i].hdl);
  }
  SDL_Quit();

  exit(0);
}
