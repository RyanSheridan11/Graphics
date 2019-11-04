/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Cylinder class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Cylinder.h"
#include <math.h>

/**
* Cylinder's intersection method.  The input is a ray (pos, dir).
*/
float Cylinder::intersect(glm::vec3 posn, glm::vec3 dir)
{

    glm::vec3 d = posn - center;
    float a = (dir.x * dir.x) + (dir.z * dir.z);
    float b = 2 * ((dir.x * d.x) + (dir.z * d.z));
    float c = d.x * d.x + d.z * d.z - (radius * radius);

    float delta = b * b - 4 * (a * c); //b^2 - 4*a*c

    if((fabs(delta)) < 0.001)//-------No intersections/solutions to the equation
        return -1.0;
    if(delta < 0.0)
        return -1.0;

    float t1 = (-b - sqrt(delta))/(2 * a);  //Intersection 1
    float t2 = (-b + sqrt(delta))/(2 * a); // Intersection 2
    float tSmall;
    float tBig;

    if(t1 < 0.01)   //this is when the intersection is behind the ray origin. So we won't display it
        t1 = -1;

    if(t2 < 0.01) //this is when the intersection is behind the ray origin. So we won't display it
        t2 = -1;

    if (t1 > t2) {
        tSmall = t2;
        tBig = t1;
    } else {
        tSmall = t1;
        tBig= t2;
    }

    float ypos = posn.y + dir.y * tSmall; //Rays y position as it goes over the closer wall/intersection
    if((ypos >= center.y) && (ypos <= center.y + height)) //Intersects with the closer wall of the Cylinder
        return tSmall;

    ypos = posn.y + dir.y * tBig;//Rays y position as it goes over the further wall/intersection
    if((ypos >= center.y) && (ypos <= center.y + height)) //Passes over the closer wall and intersects with the further one
        return tBig;
    return -1.0;  //Doesn't hit either wall
}
/**
* Returns the unit normal vector at a given point.
* Assumption: The input point p lies on the Cylinder.
*
*  Normal vector at (x, y, z)
* (un-normalized) n = (x - xc, 0, z - zc)
* (normalized) n = ( (x - xc)/R, 0, (z - zc)/R )
*/
glm::vec3 Cylinder::normal(glm::vec3 p)
{
    glm::vec3 n = glm::vec3 (p.x - center.x, 0, p.z - center.z);
    n = glm::normalize(n);
    return n;
}

