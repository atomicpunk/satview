/*
 * Copyright (c) 2013, Todd E Brandt <tebrandt@frontier.com>.
 *
 * This program is licensed under the terms and conditions of the
 * GPL License, version 2.0.  The full text of the GPL License is at
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 */

#include <stdio.h>
#include "display.h"
#include "GL/gl.h"
#include "GL/glext.h"
#include <math.h>
#include <glib.h>

#define LINEY (((float)(LINESIZE)/(float)WINDOW_HEIGHT)-1.0)
#define RPIX(c, n) ((((c)>>2)&0x1)?(n):0)
#define GPIX(c, n) ((((c)>>1)&0x1)?(n):0)
#define BPIX(c, n) (((c)&0x1)?(n):0)
//#define MODELFILE "spaceshuttle.stl"
//#define MODELFILE "VESTA.STL"
//#define MODELFILE "Earth2.stl"
extern float sundec[366];

enum {
    MOUSE_LEFT = 0,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_WHEEL_UP,
    MOUSE_WHEEL_DOWN
};

Display *display = NULL;
MouseState mousestate;

static void keyPress(unsigned char, int, int);
static void mousePress(int, int, int, int);
static void mouseMove(int, int);
static void drawScreen();
static void reshapeScreen(int, int);

Display::Display()
{
    paused = false;
    redraw = false;
    rotation[0] = 0;
    rotation[1] = 0;
    winWidth = 1280;
    winHeight = 720;
    light_sun.orientnow();

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(WINDOW_X, WINDOW_Y);
    window = glutCreateWindow("Musualizer Spectrum");

    glutDisplayFunc(drawScreen);
    glutIdleFunc(drawScreen);
    glutReshapeFunc(reshapeScreen);
    glutKeyboardFunc(keyPress);
    glutMouseFunc(mousePress);
    glutMotionFunc(mouseMove);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearDepth(1.0);
    glLineWidth(LINESIZE);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glutFullScreen();

//    gluPerspective(30, (GLfloat)1920/(GLfloat)1080,0.1f,5);

    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHTING);
    glLightfv(GL_LIGHT0, GL_POSITION, light_sun.frontpos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_sun.ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_sun.diffuse);
    glEnable (GL_LIGHT0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_sun.backpos);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_sun.ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_sun.diffuse);
    glEnable (GL_LIGHT1);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

#ifdef MODELFILE
    modelearth = new Model(Model::STLFILE, MODELFILE);
#else
    modelearth = new Model(Model::EARTH);
    modelearth->orient(light_sun.frontpos[0],
                       light_sun.frontpos[1],
                       light_sun.frontpos[2]);
#endif
    redraw = true;
}

Display::~Display()
{
    glutDestroyWindow(window);
}

void Display::draw()
{
    if(light_sun.orientnow())
        redraw = true;

    if(!redraw || paused)
        return;

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0, 0, -3);

    if(mousestate.buttons[MOUSE_LEFT])
    {
        float ax = (float)mousestate.dy*180.0/winHeight;
        float ay = (float)mousestate.dx*180.0/winWidth;
        glRotatef(rotation[0]-ax, 1.0, 0.0, 0.0);
        glRotatef(rotation[1]-ay, 0.0, 1.0, 0.0);
    }
    else
    {
        glRotatef(rotation[0], 1.0, 0.0, 0.0);
        glRotatef(rotation[1], 0.0, 1.0, 0.0);
    }
    glLightfv(GL_LIGHT0, GL_POSITION, light_sun.frontpos);
    glLightfv(GL_LIGHT1, GL_POSITION, light_sun.backpos);
    modelearth->draw();
    glutSwapBuffers();
    redraw = false;
}

void Display::drawPolygon(
    float x1, float y1, float z1,
    float x2, float y2, float z2,
    float x3, float y3, float z3)
{
    float vx = (y3-y1)*(z2-z1) - (z3-z1)*(y2-y1);
    float vy = (z3-z1)*(x2-x1) - (x3-x1)*(z2-z1);
    float vz = (x3-x1)*(y2-y1) - (y3-y1)*(x2-x1);

    float a = pow(vx, 2) + pow(vy, 2) + pow(vz, 2);
    if(a > 0)
    {
        a = pow((1 / a), 0.5);
        vx *= a;
        vy *= a;
        vz *= a;
    }

    glBegin(GL_POLYGON);
    glNormal3f(vx, vy, vz);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glEnd();
}

void Display::changeRotation(float xval, float yval)
{
    rotation[0] -= xval;
    rotation[1] -= yval;
    redraw = true;
}

void Display::changeScale(float val)
{
    modelearth->scale *= val;
    redraw = true;
}

static void keyPress(unsigned char key, int x, int y)
{
    if(display == NULL)
        return;

    switch(key) {
    case 'p': display->paused = !display->paused; break;
    case 'e': display->modelearth->userInput('e'); break;
    case 'q': exit(0); break;
    default:
        if(verbose)
            printf("KEY: %c, %x\n", key, key);
    }
    display->redraw = true;
}

static void mousePress(int button, int state, int x, int y)
{
    if(display == NULL)
        return;

    if(button < 5)
    {
        mousestate.buttons[button] = (state==GLUT_DOWN);
        if(state==GLUT_DOWN)
        {
            mousestate.px = x;
            mousestate.py = y;
        }
    }

    if(((button == MOUSE_WHEEL_UP)||(button == MOUSE_WHEEL_DOWN))&&
       (state==GLUT_DOWN))
    {
        display->changeScale((button == MOUSE_WHEEL_UP)?1.1:0.9);
    }

    if((button == MOUSE_LEFT)&&(state==GLUT_UP))
    {
        float ax = (float)mousestate.dy*180.0/display->winHeight;
        float ay = (float)mousestate.dx*180.0/display->winWidth;
        mousestate.dy = 0;
        mousestate.dx = 0;
        display->changeRotation(ax, ay);
    }
}

static void mouseMove(int x, int y)
{
    if(mousestate.buttons[MOUSE_LEFT])
    {
        mousestate.dx = mousestate.px - x;
        mousestate.dy = mousestate.py - y;
        display->redraw = true;
    }
}

static void reshapeScreen(int Width, int Height)
{
    /* sanity check */
    if(Height < 1) Height = 1;
    if(Width < 1) Width = 1;

    display->winWidth = Width;
    display->winHeight = Height;
    glViewport(0, 0, Width, Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30, (GLfloat)Width/(GLfloat)Height,0.1f,5);
    glMatrixMode(GL_MODELVIEW);

    display->redraw = true;
    display->draw();
}

static void drawScreen()
{
    display->draw();
}

void Display::doRedraw()
{
    display->redraw = true;
}

void Display::create()
{
    if(display == NULL)
        display = new Display();
}

LightSource::LightSource()
{
    frontpos[3] = backpos[3] = 0;
    azimuth = 1;
    orient(0, 0);

    ambient[4] = 1.0;
    for(int i = 0; i < 3; i++)
        ambient[i] = 0.0;

    diffuse[4] = 1.0;
    for(int i = 0; i < 3; i++)
        diffuse[i] = 2.0;
};

bool LightSource::orient(float azi, float inc)
{
    if((azi == azimuth)&&(inc == inclination))
        return false;

    azimuth = azi;
    inclination = inc;

    frontpos[0] = 10.0*cos(azi);
    frontpos[2] = 10.0*sin(azi);
    frontpos[1] = 10.0*tan(inc);

    backpos[0] = -1*frontpos[0];
    backpos[2] = -1*frontpos[2];
    backpos[1] = -1*frontpos[1];

    return true;
}

bool LightSource::orientnow()
{
    time_t ct = time(NULL);
    struct tm *t = gmtime(&ct);
    float inc = sundec[t->tm_yday]*M_PI/180;
    float azi = ((float)(60*t->tm_hour+t->tm_min)/1440)*2.0*M_PI;
    return orient(azi, inc);
}
