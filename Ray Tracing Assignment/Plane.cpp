/*----------------------------------------------------------
* COSC363  Ray Tracer
*
*  The Plane class
*  This is a subclass of Object, and hence implements the
*  methods intersect() and normal().
-------------------------------------------------------------*/

#include "Plane.h"
#include <math.h>


/**
* Checks if a point pt is inside the current polygon
* Implement a point inclusion test using 
* member variables a, b, c, d.
*/
bool Plane::isInside(glm::vec3 p)
{

	//=== Complete this function ====
    glm::vec3 n = normal(n);

    glm::vec3 A = glm::cross(b-a, p-a);
    glm::vec3 B = glm::cross(c-b, p-b);
    glm::vec3 C = glm::cross(d-c, p-c);
    glm::vec3 D = glm::cross(a-d, p-d);


    float in1 = glm::dot(A, n);
    float in2 = glm::dot(B, n);
    float in3 = glm::dot(C, n);
    float in4 = glm::dot(D, n);

    if ((in1 >= 0) and (in2 >= 0) and (in3 >= 0) and (in4 >= 0))
        return true;

    return false;
}

/**
* Plane's intersection method.  The input is a ray (pos, dir). 
*/
float Plane::intersect(glm::vec3 posn, glm::vec3 dir)
{
	glm::vec3 n = normal(posn);
	glm::vec3 vdif = a - posn;
	float vdotn = glm::dot(dir, n);
	if(fabs(vdotn) < 1.e-4) return -1;
    float t = glm::dot(vdif, n)/vdotn;
	if(fabs(t) < 0.0001) return -1;
	glm::vec3 q = posn + dir*t;
	if(isInside(q)) return t;
    else return -1;
}

/**
* Returns the unit normal vector at a given point.
* Compute the plane's normal vector using 
* member variables a, b, c, d.
* The parameter pt is a dummy variable and is not used.
*/
glm::vec3 Plane::normal(glm::vec3 pt)
{
	glm::vec3 n = glm::vec3(0);

    glm::vec3 temp1 = b - a;
    glm::vec3 temp2 = d - a;
    n = glm::cross(temp1, temp2);
    n = glm::normalize(n);
    //The surface normal vector n of the plane (Fig.5) can be computed as (B - A)x(D-A)

    return n;
}



