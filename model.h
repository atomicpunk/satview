/*
 * Copyright (c) 2013, Todd E Brandt <tebrandt@frontier.com>.
 *
 * This program is licensed under the terms and conditions of the
 * GPL License, version 2.0.  The full text of the GPL License is at
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 */

#ifndef MODEL_H
#define MODEL_H

#include <string>

class Polygon {
public:
    Polygon() {};
    ~Polygon() {};
    float facet[3];
    float vertex[4][3];
    float tex[4][2];
};

class Texture {
public:
    Texture() {
        height=width=id=0;
        buffer=NULL;
    };
    ~Texture() {};
    unsigned int id;
    unsigned char *buffer;
    int height;
    int width;
};

class Model {
public:
    enum ModelType {
        STLFILE=0,
        EARTH
    };

    Model(ModelType t, const char* filename=NULL);
    ~Model();

    void draw();
    void userInput(char ch);
    float scale;
    ModelType type;
private:
    bool loadSTL(const char* filename);
    bool loadJpeg(const char* filename, unsigned int *tex);
    void constructEarth();
    void centerObject();
    void calculateInitialScale();
    void calculateUnitVectors();
    float axismin[3];
    float axismax[3];
    int polygon_count;
    Polygon *polygons;
    unsigned int earth_day;
    unsigned int earth_night;
};

#endif //MODEL_H

