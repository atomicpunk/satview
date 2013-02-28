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
    unsigned int texid;
};

class Model {
public:
    enum ModelType {
        STLFILE=0,
        EARTH
    };

    Model(ModelType t, const char* filename=NULL);
    ~Model();

    void orient(float x, float y, float z);
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
    float vectorAngle(float x1, float y1, float z1,
                      float x2, float y2, float z2);
};

#endif //MODEL_H

