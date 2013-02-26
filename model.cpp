/*
 * Copyright (c) 2013, Todd E Brandt <tebrandt@frontier.com>.
 *
 * This program is licensed under the terms and conditions of the
 * GPL License, version 2.0.  The full text of the GPL License is at
 * http://www.gnu.org/licenses/gpl-2.0.html
 *
 */

#include "defines.h"
#include "model.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <jpeglib.h>

#include <math.h>
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#define EARTH_JPEG "EarthMap_2500x1250.jpg"

Model::Model(ModelType t, const char *filename) :
    type(t)
{
    /* initial values */
    polygon_count = 0;
    polygons = NULL;
    scale = 1.0;
    for(int i = 0; i < 3; i++)
    {
        axismin[i] = 0;
        axismax[i] = 0;
    }

    if((t == STLFILE)&&(filename != NULL))
        loadSTL(filename);
    else if(t == EARTH)
        constructEarth();
}

Model::~Model()
{
    if(polygons)
        delete polygons;
    if(earthtexture.buffer)
        delete earthtexture.buffer;
}

bool Model::loadJpeg(const char* filename, Texture *tex)
{
    if(tex->buffer != NULL)
        return false;

    unsigned char a,r,g,b;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    FILE * jfp;
    JSAMPARRAY pJpegBuffer;
    int row_stride;

    if ((jfp = fopen(filename, "rb")) == NULL)
    {
        fprintf(stderr, "can't open %s\n", filename);
        return false;
    }
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, jfp);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    tex->width = cinfo.output_width;
    tex->height = cinfo.output_height;

    tex->buffer = new GLubyte [(tex->width)*(tex->height)*4];
    GLubyte * pDummy = tex->buffer;

    if (!pDummy)
    {
        printf("NO MEM FOR JPEG CONVERT!\n");
        return false;
    }
    row_stride = tex->width * cinfo.output_components;
    pJpegBuffer = (*cinfo.mem->alloc_sarray)
    ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    while (cinfo.output_scanline < cinfo.output_height)
    {
        (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
        for (int x=0;x<(tex->width);x++)
        {
            a = 0; // alpha value is not supported on jpg
            r = pJpegBuffer[0][cinfo.output_components*x];
            if (cinfo.output_components>2)
            {
                g = pJpegBuffer[0][cinfo.output_components*x+1];
                b = pJpegBuffer[0][cinfo.output_components*x+2];
            } else {
                g = r;
                b = r;
            }
            *(pDummy++) = r;
            *(pDummy++) = g;
            *(pDummy++) = b;
            *(pDummy++) = a;
        }
    }
    fclose(jfp);
    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    GLuint texid;
    glGenTextures(1, &texid);
    tex->id = texid;
    printf("SIZE=%dx%d, ID=%d\n", tex->width, tex->height, tex->id);

    glBindTexture(GL_TEXTURE_2D, tex->id);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, tex->width, tex->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex->buffer);
//    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
//    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
//    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    return true;
}

void Model::constructEarth()
{
    loadJpeg(EARTH_JPEG, &earthtexture);
    for(int i = 0; i < 360-1; i++)
        for(int j = 0; j < 180-1; j++)
        {
            float lon0 = ((float)i/180.0)*M_PI;
            float lon1 = ((float)(i+1)/180.0)*M_PI;
            float lat0 = ((float)j/180.0)*M_PI;
            float lat1 = ((float)(j+1)/180.0)*M_PI;
        }
}

bool Model::loadSTL(const char *filename)
{
    /* prepare to parse the file */
    std::ifstream fp(filename);
    std::string line;
    std::string field;
    if(!fp.is_open())
    {
        fprintf(stderr, "Model load failure: %s\n", filename);
        return false;
    }

    if(verbose)
        printf("Model: %s\n", filename);

    /* get the total polygon count (for memory preallocation) */
    while(fp.good())
    {
        std::getline(fp, line);
        std::stringstream parse(line);
        parse >> field;
        if(!field.compare("facet"))
            polygon_count++;
    }
    if(polygon_count <= 0)
    {
        fp.close();
        fprintf(stderr, "Model has no polygons: %s\n", filename);
        return false;
    }
    if(verbose)
        printf("\tPOLYGONS: %d\n", polygon_count);

    /* create the memory for the polygon data and reset the stream */
    fp.clear();
    fp.seekg(std::ios::beg);
    polygons = new Polygon[polygon_count];

    /* read in every polygon */
    for(int ln = 1, i = 0, j = 0; fp.good() && (i < polygon_count); ln++)
    {
        std::getline(fp, line);
        std::stringstream parse(line);
        parse >> field;
        if(!field.compare("facet"))
        {
            parse >> field;
            parse >> polygons[i].facet[0] >> polygons[i].facet[1] >> polygons[i].facet[2];
        }
        else if(!field.compare("vertex"))
        {
            /* sanity check */
            if(j > 2)
            {
                fprintf(stderr, "found more than 3 vertexes at %s:%d\n", filename, ln);
                return false;
            }

            /* pull out X, Y, Z coordinates */
            parse >> polygons[i].vertex[j][0] >> polygons[i].vertex[j][1] >> polygons[i].vertex[j][2];

            /* recalibrate the axis ranges */
            for(int k = 0; k < 3; k++)
            {
                if(polygons[i].vertex[j][k] > axismax[k])
                    axismax[k] = polygons[i].vertex[j][k];
                else if(polygons[i].vertex[j][k] < axismin[k])
                    axismin[k] = polygons[i].vertex[j][k];
            }

            /* move on to the next vertex */
            j++;
        }
        else if(!field.compare("endfacet"))
        {
            i++;
            j = 0;
        }
    }

    fp.close();

//    centerObject();
    calculateInitialScale();
    calculateUnitVectors();

    if(verbose)
    {
        printf("\tXAXIS [%f, %f]\n", axismin[0], axismax[0]);
        printf("\tYAXIS [%f, %f]\n", axismin[1], axismax[1]);
        printf("\tZAXIS [%f, %f]\n", axismin[2], axismax[2]);
    }

    return true;
}

void Model::centerObject()
{
    for(int i = 0; i < 3; i++)
    {
        float dx = ((axismax[i]-axismin[i])/2) - axismax[i];
        for(int j = 0; j < 3; j++)
            for(int k = 0; k < polygon_count; k++)
                polygons[k].vertex[j][i] += dx;
    }
}

void Model::calculateUnitVectors()
{
    for(int i = 0; i < polygon_count; i++)
    {
        float a = pow(polygons[i].facet[0], 2) +
                  pow(polygons[i].facet[1], 2) +
                  pow(polygons[i].facet[2], 2);
        if(a > 0)
        {
            a = pow((1 / a), 0.5);
            for(int j = 0; j < 3; j++)
                polygons[i].facet[j] *= a;
        }
    }
}

void Model::calculateInitialScale()
{
    float mlen = axismax[0];
    for(int i = 0; i < 3; i++)
    {
        if(axismax[i] > mlen)
            mlen = axismax[i];
        if(-1*axismin[i] > mlen)
            mlen = -1*axismin[i];
    }
    if(mlen <= 0)
    {
        fprintf(stderr, "Model has a negative or zero scale\n");
        return;
    }
    scale = 1/mlen;
}

void Model::draw()
{
    int sides = (type == STLFILE)?3:4;
    for(int i = 0; i < polygon_count; i++)
    {
        glBegin((sides == 3)?GL_TRIANGLES:GL_POLYGON);
        glNormal3f(polygons[i].facet[0], polygons[i].facet[1], polygons[i].facet[2]);
        for(int j = 0; j < sides; j++)
        {
            glVertex3f(polygons[i].vertex[j][0]*scale,
                       polygons[i].vertex[j][1]*scale,
                       polygons[i].vertex[j][2]*scale);
        }
        glEnd();
    }
}
