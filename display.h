/*
 * Copyright (c) 2013, Todd E Brandt <tebrandt@frontier.com>.
 *
 * This program is licensed under the terms and conditions of the
 * GPL License, version 2.0.  The full text of the GPL License is at
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include "GL/freeglut.h"
#include "defines.h"
#include "model.h"

class MouseState {
public:
    MouseState()
    {
        for(int i = 0; i < 5; i++)
            buttons[i] = false;
        px = 0;
        py = 0;
        dx = 0;
        dy = 0;
    };
    ~MouseState() {};
    bool buttons[5];
    int px;
    int py;
    int dx;
    int dy;
};

class LightSource {
public:
    LightSource();
    ~LightSource() {};

    float frontpos[4];
    float backpos[4];
    float ambient[4];
    float diffuse[4];

    void orient(float azi, float inc);
};

class Display {
public:
    Display();
    ~Display();

    static void create();
    static void doRedraw();

    void changeScale(float val);
    void changeRotation(float xval, float yval);
    void draw();
    bool paused;
    bool redraw;
    float winWidth;
    float winHeight;
    Model *modelearth;

private:
    float azimuth;
    float inclination;
    LightSource light_sun;
    int window;
    float rotation[2];
    void drawPolygon(float, float, float,
         float, float, float, float, float, float);
};

extern Display *display;

#endif //DISPLAY_H

