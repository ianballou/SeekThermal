/* A simple program to show how to set up an X window for OpenGL rendering.
 * X86 compilation: gcc -o -L/usr/X11/lib   main main.c -lGL -lX11
 * X64 compilation: gcc -o -L/usr/X11/lib64 main main.c -lGL -lX11
 */
#include <stdio.h>
#include <stdlib.h>

#include <GL/glut.h>
#include <GL/gl.h>

#include <GL/glx.h>    /* this includes the necessary X headers */

#include <X11/X.h>    /* X11 constant (e.g. TrueColor) */
#include <X11/keysym.h>

#include "SeekThermalCamera.h"

#include <signal.h>
#include <iostream>

static int snglBuf[] = {GLX_RGBA, GLX_DEPTH_SIZE, 16, None};
static int dblBuf[]  = {GLX_RGBA, GLX_DEPTH_SIZE, 16, GLX_DOUBLEBUFFER, None};

uint8_t pic[MAX_THERMAL_PIXELS * 3];

Display   *dpy;
Window     win;
GLfloat    xAngle = 42.0, yAngle = 82.0, zAngle = 112.0;
GLboolean  doubleBuffer = GL_TRUE;

bool shouldQuit = false;

void fatalError(char *message)
{
  fprintf(stderr, "main: %s\n", message);
  exit(1);
}

void redraw(void)
{
  static GLboolean   displayListInited = GL_FALSE;

  /*if (displayListInited)
  {
    // if display list already exists, just execute it
    glCallList(1);
  }
  else
  {*/
    // otherwise compile and execute to create the display list
    glNewList(1, GL_COMPILE_AND_EXECUTE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 208, 156, 0, GL_RGB, GL_UNSIGNED_BYTE, pic);

    /* front face */
    glBegin(GL_QUADS);
    glTexCoord2f (0.0f, 0.0f);   glVertex2f (-1.0f, 1.0f);
    glTexCoord2f (1.0f, 0.0f);   glVertex2f (1.0f, 1.0f);
    glTexCoord2f (1.0f, 1.0f);   glVertex2f (1.0f, -1.0f);
    glTexCoord2f (0.0f, 1.0f);   glVertex2f (-1.0f, -1.0f);
    glEnd();
    glEndList();
    displayListInited = GL_TRUE;
  //}
  if (doubleBuffer)
    glXSwapBuffers(dpy, win);/* buffer swap does implicit glFlush */
  else
    glFlush();  /* explicit flush for single buffered case */
}

void signal_handler(int signum)
{
	shouldQuit = true;
}

/*float map(float val, float istart, float istop, float ostart, float ostop)
{
	return ostart + (ostop - ostart) * ((val - istart) / (istop - istart));
}*/

int main(int argc, char **argv)
{
    //SeekThermalCamera cam;

    //Set signal handler function.
	signal(SIGINT, signal_handler);

	SeekThermalCamera camera;

	//Try to initialize camera.
	try
	{
        camera.initialize();
	}
	catch(SeekThermalCamera_Exception e)
	{
        std::cout << "Error initializing: " << getSeekThermalCameraExceptionString(e) << std::endl;
        return EXIT_FAILURE;
	}

  XVisualInfo         *vi;
  Colormap             cmap;
  XSetWindowAttributes swa;
  GLXContext           cx;
  XEvent               event;
  GLboolean            needRedraw = GL_FALSE, recalcModelView = GL_TRUE;
  int                  dummy;

  /*** (1) open a connection to the X server ***/

  dpy = XOpenDisplay(NULL);
  if (dpy == NULL)
    fatalError("could not open display");

  /*** (2) make sure OpenGL's GLX extension supported ***/

  if(!glXQueryExtension(dpy, &dummy, &dummy))
    fatalError("X server has no OpenGL GLX extension");

  /*** (3) find an appropriate visual ***/

  /* find an OpenGL-capable RGB visual with depth buffer */
  vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);
  if (vi == NULL)
  {
    vi = glXChooseVisual(dpy, DefaultScreen(dpy), snglBuf);
    if (vi == NULL) fatalError("no RGB visual with depth buffer");
    doubleBuffer = GL_FALSE;
  }
  if(vi->c_class != TrueColor)
    fatalError("TrueColor visual required for this program");

  /*** (4) create an OpenGL rendering context  ***/

  /* create an OpenGL rendering context */
  cx = glXCreateContext(dpy, vi, /* no shared dlists */ None,
                        /* direct rendering if possible */ GL_TRUE);
  if (cx == NULL)
    fatalError("could not create rendering context");

  /*** (5) create an X window with the selected visual ***/

  /* create an X colormap since probably not using default visual */
  cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.border_pixel = 0;
  swa.event_mask = KeyPressMask    | ExposureMask
                 | ButtonPressMask | StructureNotifyMask;
  win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0,
                      208, 156, 0, vi->depth, InputOutput, vi->visual,
                      CWBorderPixel | CWColormap | CWEventMask, &swa);
  XSetStandardProperties(dpy, win, "main", "main", None,
                         argv, argc, NULL);

  /*** (6) bind the rendering context to the window ***/

  glXMakeCurrent(dpy, win, cx);

  /*** (7) request the X window to be displayed on the screen ***/

  XMapWindow(dpy, win);

  /*** (8) configure the OpenGL context for rendering ***/

  //glEnable(GL_DEPTH_TEST); /* enable depth buffering */
  //glDepthFunc(GL_LESS);    /* pedantic, GL_LESS is the default */
  //glClearDepth(1.0);       /* pedantic, 1.0 is the default */

    GLuint textureId;

   glGenTextures(1, &textureId);
   glBindTexture(GL_TEXTURE_2D, textureId);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

   glEnable(GL_TEXTURE_2D);

  /* frame buffer clears should be to black */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  /* set up projection transform */
  /* establish initial viewport */
  /* pedantic, full window size is default viewport */
  glViewport(0, 0, 208, 156);

  printf( "Press left mouse button to rotate around X axis\n" );
  printf( "Press middle mouse button to rotate around Y axis\n" );
  printf( "Press right mouse button to rotate around Z axis\n" );
  printf( "Press ESC to quit the application\n" );

  /*** (9) dispatch X events ***/
  uint16_t centerTemp;
  while (!shouldQuit)
  {
    /*if (needRedraw)
    {
      redraw();
      needRedraw = GL_FALSE;
    }*/
    int i;
    /*for(i = 0; i < 9; ++i)
    {
        pic[i] = rand();
    }*/

    createBitmap(pic, camera.getFrame(), false);
	//centerTemp = camera.getFrame()->getData()[(int)(MAX_THERMAL_PIXELS / 2)];
	//std::cout << map(centerTemp, 0, 65535, -40, 330);
	//std::cout << centerTemp << std::endl;
    redraw();
  }

  return 0;
}
