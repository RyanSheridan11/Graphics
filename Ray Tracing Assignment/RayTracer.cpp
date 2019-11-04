/*========================================================================
* COSC 363  Computer Graphics (2019)
* Ray tracer Assignment
* Author: Ryan Sheridan
* StudentID: 98351301
* Date Due: 31st May - 2019
*=========================================================================*/
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glut.h>

#include "SceneObject.h"
#include "Sphere.h"
#include "Plane.h"
#include "Cylinder.h"
#include "Ray.h"
#include "TextureBMP.h"

using namespace std;
const float WIDTH = 20.0;
const float HEIGHT = 20.0;
const float EDIST = 40.0;
const int NUMDIV = 500;
const int MAX_STEPS = 5;
const float XMIN = -WIDTH * 0.5;
const float XMAX =  WIDTH * 0.5;
const float YMIN = -HEIGHT * 0.5;
const float YMAX =  HEIGHT * 0.5;

float initial_display = 1;
float rectangleSize = 7;
float eyex = 0, eyey = 0, eyez = 0;
glm::vec3 eye = glm::vec3(eyex, eyey, eyez);  //The eye position (source of primary rays) is the origin
vector<SceneObject*> sceneObjects;  //A global list containing pointers to objects in the scene
TextureBMP texture;

//-------------------------------------------------------------------------------------------
glm::vec3 shadow_rendering(glm::vec3 normalVector, glm::vec3 lightPos, float lightIntensity, Ray viewRay, glm::vec3 materialCol)
{
    glm::vec3 colorSum(0);
    glm::vec3 ambientCol(0.3);   //Ambient color of light
    glm::vec3 specCol(1, 1, 1);
    float phong = 20;
    float specularTerm = 0.0;


    glm::vec3 lightVector = glm::normalize (lightPos - viewRay.xpt);       //The vector from the point of intersection towards the light source (Normalized)
    glm::vec3 reflVector = glm::reflect(-lightVector, normalVector);
    float rDotv = glm::dot(reflVector, -viewRay.dir);  //-ray.dir = viewVector
    float lDotn = glm::dot(lightVector, normalVector);
    float lightDist = glm::distance(lightPos, viewRay.xpt); //Distance between light source and the intersected point

    //Shadow
    Ray shadow(viewRay.xpt, lightVector);
    shadow.closestPt(sceneObjects);

    if ((lDotn <= 0.0) or ((shadow.xindex > -1) and (shadow.xdist < lightDist))) {   //If the point is in shadow, just return the ambient colour
        colorSum += (ambientCol*materialCol);
        if (shadow.xindex == 4) //But if the shadow is from the transparent sphere it has a light shadow with some of the objects colour bled in
            colorSum += (lDotn*materialCol*ambientCol)+(sceneObjects[shadow.xindex]->getColor()*glm::vec3(0.1));
    } else if (rDotv < 0.0) {
        colorSum += ((ambientCol*materialCol) + (lDotn*materialCol*lightIntensity));  //no specular
    } else {
        specularTerm = pow(rDotv, phong);
        colorSum += ((ambientCol*materialCol) + (lDotn*materialCol*lightIntensity) + (specularTerm*specCol*lightIntensity));
    }
    return colorSum;
}
//-------------------------------------------------------------------------------------------
glm::vec3 trace(Ray ray, int step)
{
    //---The most important function in a ray tracer! ----------------------------------
    //   Computes the colour value obtained by tracing a ray and finding its
    //     closest point of intersection with objects in the scene.
    glm::vec3 backgroundCol(0);
    glm::vec3 colorSum(0);
    glm::vec3 materialCol;

    ray.closestPt(sceneObjects);		//Compute the closest point of intersetion of objects with the ray

    //------------------------------------------------------index -1 == no intersection
    if (ray.xindex == -1) {
        return backgroundCol;
    //------------------------------------------------------index 1 == Base Plane
    } else if (ray.xindex == 1) {
        int modx = int(ray.xpt.x) % 4;
        int modz = int((ray.xpt.z) / 4) % 4;
        if((modx and modz) or (!modx and !modz))
            materialCol = glm::vec3(0, 0.6, 0); //Green
        else
            materialCol = glm::vec3(0.6, 0.6, 0);//Yellow
    //------------------------------------------------------index 2 == Cylinder
    } else if (ray.xindex == 2) {
        int modx = int((ray.xpt.x) * 5) % 4;
        int mody = int((ray.xpt.y) * 5) % 4;
        if(modx and mody)
            materialCol = glm::vec3(1, 0.27, 0); // Orange
        else
            materialCol = glm::vec3(0.57, 0.01, 0.57);  //Purple
    //-----------------------------------------------------index 3 == 2nd Sphere
    } else if (ray.xindex == 3) {
        glm::vec3 center(-9.0, 5.0, -80.0);
        glm::vec3 d = glm::normalize(ray.xpt - center);
        float textureCoordU = (0.5 - atan2(d.z, d.x) + M_PI) / (2 * M_PI);
        float textureCoordV = 0.5 + (asin(d.y) / M_PI);
        materialCol = texture.getColorAt(textureCoordU, textureCoordV);

        int mody = int((ray.xpt.y) * 20) % 8;
        if (!mody)
            materialCol = glm::vec3(1,1,1);
    //------------------------------------------------------Not specifically textured
    } else {
        materialCol = sceneObjects[ray.xindex]->getColor();
    }
    glm::vec3 normalVector = sceneObjects[ray.xindex]->normal(ray.xpt);  // normal vector on the object at the point of intersection

    //------------------------------------------------------Colour and shadows from Light 1
    glm::vec3 lightPos(10, 40, -3);
    float light1intensity = 0.8;
    colorSum = shadow_rendering(normalVector, lightPos, light1intensity, ray, materialCol);

    //------------------------------------------------------Colour and shadows from Light 2
    glm::vec3 light2Pos(30, 10, -85);
    float light2intensity = 0.6;
    colorSum += shadow_rendering(normalVector, light2Pos, light2intensity, ray, materialCol);

    //Reflection of the main Sphere, which should always be at index 0
    if(ray.xindex == 0 and step < MAX_STEPS) {
        glm::vec3 reflectedDir = glm::reflect(ray.dir, normalVector);
        Ray reflectedRay(ray.xpt, reflectedDir);
        glm::vec3 reflectedCol = trace(reflectedRay, step+1); //Recursion!
        colorSum += (0.6f * reflectedCol);
     }
    //Refractive/Transparent sphere, which should always be at index 4
    if (ray.xindex == 4 and step < MAX_STEPS) {
        float glassN = 1.012;
        float airN = 1.0;
        float transparency = 0.5;
        //Through the first layer
        glm::vec3 refractedDir = glm::refract(ray.dir, normalVector, airN/glassN);
        Ray refractedRay(ray.xpt, refractedDir);
        refractedRay.closestPt(sceneObjects);
        if (refractedRay.xindex == -1) return backgroundCol;
        //Through the second layer
        normalVector = sceneObjects[refractedRay.xindex]->normal(refractedRay.xpt);
        refractedDir = glm::refract(refractedDir, -normalVector, glassN/airN);
        Ray refractedRay2(refractedRay.xpt, refractedDir);
        refractedRay2.closestPt(sceneObjects);
//        if (refractedRay2.xindex == -1) return backgroundCol;
        //Get the second refracted rays colour
        glm::vec3 refractedCol = trace(refractedRay2, step+1); //Recursion!
        colorSum = (colorSum * transparency) + (refractedCol * ( 1 - transparency));
    }
    return colorSum;
}
//-------------------------------------------------------------------------------------------
glm::vec3 dynamic_alias_rays(glm::vec3 eyepos, float small_quarter, float xp, float yp)
{
    glm::vec3 colorSum(0);

    Ray botLeft = Ray(eyepos, glm::vec3(xp - small_quarter, yp - small_quarter, -EDIST));
    botLeft.normalize();
    colorSum += trace(botLeft, 1);

    Ray topLeft = Ray(eyepos, glm::vec3(xp - small_quarter, yp + small_quarter, -EDIST));
    topLeft.normalize();
    colorSum += trace(topLeft, 1);

    Ray botRight = Ray(eyepos, glm::vec3(xp + small_quarter, yp - small_quarter, -EDIST));
    botRight.normalize();
    colorSum += trace(botRight, 1);

    Ray topRight = Ray(eyepos, glm::vec3(xp + small_quarter, yp  + small_quarter, -EDIST));
    topRight.normalize();
    colorSum += trace(topRight, 1);

    return colorSum;
}
//-------------------------------------------------------------------------------------------
glm::vec3 anti_aliasing(glm::vec3 eyepos, float cell_size, float xp, float yp)
{
    glm::vec3 averageCol_4(4);
    glm::vec3 averageCol_3(3);
    glm::vec3 colorSum(0);
    float cell_quarter = cell_size / 4;
    float cell_3quarters = cell_quarter * 3;
    float small_quarter = cell_size / 8;

    float adaptiveOn = 0;  //Adaptive sampling!!!

    Ray botLeft = Ray(eyepos, glm::vec3(xp + cell_quarter, yp + cell_quarter, -EDIST));
    botLeft.normalize();
    glm::vec3 color0 = trace(botLeft, 1);

    Ray topLeft = Ray(eyepos, glm::vec3(xp + cell_quarter, yp + cell_3quarters, -EDIST));
    topLeft.normalize();
    glm::vec3 color1 = trace(topLeft, 1);

    Ray botRight = Ray(eyepos, glm::vec3(xp + cell_3quarters, yp + cell_quarter, -EDIST));
    botRight.normalize();
    glm::vec3 color2 = trace(botRight, 1);

    Ray topRight = Ray(eyepos, glm::vec3(xp + cell_3quarters, yp + cell_3quarters, -EDIST));
    topRight.normalize();
    glm::vec3 color3 = trace(topRight, 1);

    if (!adaptiveOn) {
        colorSum = (color0 + color1 + color2 + color3) / averageCol_4;
        return colorSum;
    }

    glm::vec3 colors[4] = {color0, color1, color2, color3};
    for(int i = 0; i <= 4; i++) { //For each color, if one color varies significantly from the other 3, create 4 new rays
        glm::vec3 colorSum(0);
        for(int j = 0; j <= 4; j++) { //This loop is for adding up the other 3 colors
            if (j != i) colorSum += colors[j];
        }
        colorSum = colorSum / averageCol_3;

        if (colorSum != colors[i]) { //If the colour differs from the other 3 colours,  ---------- Send the 4 new rays!
            if (i == 0) {  // -------------------Bottom left---------------------
                glm::vec3 colorSum = dynamic_alias_rays(eyepos, small_quarter, xp + cell_quarter, yp + cell_quarter) ;
                colors[i] = colorSum / averageCol_4;

            } else if (i == 1) { // -------------------Top left---------------------
                glm::vec3 colorSum = dynamic_alias_rays(eyepos, small_quarter, xp + cell_quarter, yp + cell_3quarters) ;
                colors[i] = colorSum / averageCol_4;

            } else if (i == 2) { // -------------------Bottom Right---------------------
                glm::vec3 colorSum = dynamic_alias_rays(eyepos, small_quarter, xp + cell_3quarters, yp + cell_quarter) ;
                colors[i] = colorSum / averageCol_4;

            } else if (i == 3) { // -------------------Top Right---------------------
                glm::vec3 colorSum = dynamic_alias_rays(eyepos, small_quarter, xp + cell_3quarters, yp + cell_3quarters) ;
                colors[i] = colorSum / averageCol_4;
            }
        }
    }
    colorSum = (colors[0] + colors[1] + colors[2] + colors[3]) / averageCol_4;
    return colorSum;
}
//-------------------------------------------------------------------------------------------
void display()
{
    // The main display module
    // In a ray tracing application, it just displays the ray traced image by drawing
    // each cell as a quad.
    float xp, yp;  //grid point
    float cellX = (XMAX-XMIN)/NUMDIV;  //cell width
    float cellY = (YMAX-YMIN)/NUMDIV;  //cell height

    eye = glm::vec3(eyex, eyey, eyez);
    float aa_on = 1; //Set to 1 to turn anti aliasing on. Set to 0 if you want no anti aliasing

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);  //Each cell is a quad.

    for(int i = 0; i < NUMDIV; i++) {  	//For each grid point xp, yp
        xp = XMIN + i*cellX;
        for(int j = 0; j < NUMDIV; j++) {
            yp = YMIN + j*cellY;
            glm::vec3 dir(xp+0.5*cellX, yp+0.5*cellY, -EDIST);	//direction of the primary ray
            Ray ray = Ray(eye, dir);		//Create a ray originating from the camera in the direction 'dir'
            glm::vec3 col(0,0,0);
            ray.normalize();				//Normalize the direction of the ray to a unit vector

            if (initial_display and aa_on) {
                col = anti_aliasing (eye, cellX, xp, yp);
            }  else {
                col = trace (ray, 1); //Trace the primary ray and get the colour value
            }

            glColor3f(col.r, col.g, col.b);
            glVertex2f(xp, yp);				//Draw each cell with its color value
            glVertex2f(xp+cellX, yp);
            glVertex2f(xp+cellX, yp+cellY);
            glVertex2f(xp, yp+cellY);
        }
    }
    glEnd();
    glFlush();
}
//-------------------------------------------------------------------------------------------
void rectangle(float x, float y, float z, float size, glm::vec3 color)
{
    float height = size;
    float width = size / 2;

    glm::vec3 A = glm::vec3(x-width, y,          z);
    glm::vec3 B = glm::vec3(x,       y,          z-width);
    glm::vec3 C = glm::vec3(x+width, y,          z);
    glm::vec3 D = glm::vec3(x,       y,          z+width);
    glm::vec3 E = glm::vec3(x-width, y + height, z);
    glm::vec3 F = glm::vec3(x,       y + height, z-width);
    glm::vec3 G = glm::vec3(x+width, y + height, z);
    glm::vec3 H = glm::vec3(x,       y + height, z+width);

//    Plane *plane0 = new Plane(A, B, C, D, color);  //Bottom
    Plane *plane1 = new Plane(E, H, G, F, color);  //Top

    Plane *plane2 = new Plane(A, E, F, B, color);  //Back Left
    Plane *plane3 = new Plane(D, H, E, A, color);  //Front Left

    Plane *plane4 = new Plane(C, G, H, D, color);  //Front Right
    Plane *plane5 = new Plane(B, F, G, C, color);  //Back Right

//    sceneObjects.push_back(plane0);//Bottom
    sceneObjects.push_back(plane1);//Top
    sceneObjects.push_back(plane2);  //Back Left
    sceneObjects.push_back(plane3);  //Front Left
    sceneObjects.push_back(plane4);  //Front Right
    sceneObjects.push_back(plane5);  //Back Right
}
//-------------------------------------------------------------------------------------------
void initialize()
{
    //---This function initializes the scene ---------------------------------------------
    //   Specifically, it creates scene objects (spheres, planes, cones, cylinders etc)
    //     and add them to the list of scene objects.
    //   It also initializes the OpenGL orthographc projection matrix for drawing the
    //     the ray traced image.
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(XMIN, XMAX, YMIN, YMAX);
    glClearColor(0, 0, 0, 1);

    //-- Create a pointer to a sphere object
    Sphere *bigBlue = new Sphere(glm::vec3(-5.0, -5.0, -110.0), 15.0, glm::vec3(0, 0, 1));         //Blue Reflective Sphere
    Sphere *world = new Sphere(glm::vec3(-9.0, 5.0, -80.0), 2.5, glm::vec3(0, 0, 0));            //Texture of the world!
    Sphere *transparentSphere = new Sphere(glm::vec3(-11.0, -10.0, -80.0), 5.0, glm::vec3(1,0.5,1));           // transparent sphere -- Pink
    Cylinder *cylinder = new Cylinder(glm::vec3(15, -20, -85),  3, 10, glm::vec3 (1, 0.27, 0));
    Plane *basePlane = new Plane (
        glm::vec3(-20., -20, -40), //Point A
        glm::vec3(20., -20, -40), //Point B
        glm::vec3(20., -20, -200), //Point C
        glm::vec3(-20., -20, -200), //Point D
        glm::vec3(0.5, 0.5, 0)); //Colour

    texture = TextureBMP((char*)("world.bmp"));

    //--Add the above to the list of scene objects.
    sceneObjects.push_back(bigBlue);
    sceneObjects.push_back(basePlane);
    sceneObjects.push_back(cylinder);
    sceneObjects.push_back(world);
    sceneObjects.push_back(transparentSphere);

    rectangle(-5, -19, -90, rectangleSize, glm::vec3(0.7, 0, 0));


}
//-------------------------------------------------------------------------------------------
void SpecialKey(int key, int x, int y)
{
    switch (key) {
      case GLUT_KEY_UP:
        initial_display = 0;
        sceneObjects.pop_back();
        sceneObjects.pop_back();
        sceneObjects.pop_back();
        sceneObjects.pop_back();
        sceneObjects.pop_back();
        rectangleSize ++;
        rectangle(-5, -19, -90, rectangleSize, glm::vec3(0.7, 0, 0));
        display();
        break;
      case GLUT_KEY_DOWN:
        initial_display = 0;
        sceneObjects.pop_back();
        sceneObjects.pop_back();
        sceneObjects.pop_back();
        sceneObjects.pop_back();
        sceneObjects.pop_back();
        rectangleSize --;
        if (rectangleSize > 1) rectangleSize = 1;
        rectangle(-5, -19, -90, rectangleSize, glm::vec3(0.7, 0, 0));
        display();
        break;
      case GLUT_KEY_LEFT:
        initial_display = 0;
        eyex -= 10;
        eye = glm::vec3(eyex, eyey, eyez);
        display();
        break;
      case GLUT_KEY_RIGHT:
        initial_display = 0;
        eyex += 10;
        eye = glm::vec3(eyex, eyey, eyez);
        display();
        break;
    }
}
//-------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB );
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(20, 20);
    glutCreateWindow("Raytracer");

    glutSpecialFunc(SpecialKey);
    glutDisplayFunc(display);
    initialize();

    glutMainLoop();
    return 0;
}
