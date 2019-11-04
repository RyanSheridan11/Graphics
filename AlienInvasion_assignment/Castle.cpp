//  ========================================================================
//  COSC363: Computer Graphics (2019);  University of Canterbury.
//
//  FILE NAME: Castle.cpp
//  Authors name: Ryan Sheridan
//  Alien invasion assignement
//  ========================================================================

#include <iostream>
#include <fstream>
#include <climits>
#include <cmath> 
#include <GL/glut.h>
#include "loadTGA.h"
#include "loadBMP.h"
using namespace std;
#define GL_CLAMP_TO_EDGE 0x812F   //To get rid of seams between textures
GLuint texId[10];

//Debug and view stuff
int debug_mode = 0;
int special_view_mode = 0;

float saucer_look_x = 0;
float saucer_look_y = 12;
float saucer_look_z = 1000;
float saucer_cam_x =0;
float saucer_cam_y = 12;
float saucer_cam_z = 9.6;

//camera parameters
float angle = 0;
float cam_x = 0;
float cam_z = 57;
float cam_y = 5;
float look_x = cam_x + 100*sin(angle);
float look_z = cam_z - 100*cos(angle);
float look_y = 1;

//UFO paremeters
const int N = 17;  // Total number of vertices on the base curve
float vx[N] = {0, 18, 16, 13, 9, 7, 5, 6, 7, 8,  9,  9.5, 9,  8,  6,  3,  0};
float vy[N] = {0, 1,  3,  4,  5, 6, 7, 8, 9, 10, 11, 12,  13, 14, 15, 16, 16};
float vz[N] = {0};
int saucer_rotation = 0;
int saucer_on_ground = 1;
const int F = 18;
int saucer_pos = 0;
//The UFO's flight paremeters
float sx[F] = {0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
float sy[F] = {0, 1, 2, 3, 3, 3, 3, 3, 3,  3,  3,  4,  5,  7,  9,  11, 13, 15};
float sz[F] = {0, 0, 0, 1, 3, 5, 7, 9, 11, 13, 16, 19, 23, 27, 31, 35, 38, 42}; //hanger ends at 20
float saucer_x = 0;
float saucer_y = 0;
float saucer_z = 0;

//Variables for the cannon
float *x, *y, *z;  //vertex coordinate arrays
int *t1, *t2, *t3; //triangles
int nvrt, ntri;    //total number of vertices and triangles
float ball_pos_z = 38.88;
float ball_pos_y = 64;
float z_speed = 20;
float y_speed = 1.5;
float gravity = -9.81;
float resistance = -2;
int ball_shot = 0;

// Hanger door movement paremeters
int doors_moving=0;
int doors_open=0;
float doors_progress=-0.1;

//lighting colours
float ambient[4] = {0.5, 0.5, 0.5, 1.0};
float white[4]  = {1.0, 1.0, 1.0, 1.0};
float black[4] = {0.0,0.0,0.0,1.0};
float yellow[4] = {1.0, 1.0, 0.0, 1.0};
float green[4] = {0.0, 1.0, 0.0, 1.0};
float red[4] = {1.0, 0, 0, 1};
float blue[4] = {0,0,1,1};

//Parameters for the robots
float robot_z = 35;
float robot_bobbing = 2;
float robot_path = 3;
float armMovement = 0;
int arm_dir = 0;
int walking_forwards = 1;
int robot_dir = 0;

//===========================================================================================

//-- Function to compute the normal from 3 points -------------------------------------------
void computeNormal(float x1, float y1, float z1,
                   float x2, float y2, float z2,
                   float x3, float y3, float z3 )
{
      float nx, ny, nz;
      nx = y1*(z2-z3)+ y2*(z3-z1)+ y3*(z1-z2);
      ny = z1*(x2-x3)+ z2*(x3-x1)+ z3*(x1-x2);
      nz = x1*(y2-y3)+ x2*(y3-y1)+ x3*(y1-y2);
      glNormal3f(-nx, -ny, -nz);
}

//-- Function that draws the sweep surface for the UFO  -----------------------------------------------------------
void saucer()
{
    float rotation = 5.0*3.1415926/180.0;  //Rotate in 5 deg steps (converted to radians)
    float wx[N], wy[N], wz[N];

    //  Drawing the surface of revolution here.
    for(int j = 0; j < 72; j++) //72 slices in 5 deg steps
    {
        for(int i = 0; i < N; i++) {
            wx[i] = vx[i] * cos(rotation) + vz[i] * sin(rotation);
            wy[i] = vy[i];
            wz[i] = - vx[i] * sin(rotation) +  vz[i] * cos(rotation);
        }
        glBegin(GL_TRIANGLE_STRIP);
         for(int i = 0; i < N; i++) {
            if(i > 0) computeNormal( wx[i-1], wy[i-1], wz[i-1],
                                     vx[i-1], vy[i-1], vz[i-1],
                                     vx[i], vy[i], vz[i] );
            glVertex3f(vx[i], vy[i], vz[i]);
            if(i > 0) computeNormal( wx[i-1], wy[i-1], wz[i-1],
                                     vx[i], vy[i], vz[i],
                                     wx[i], wy[i], wz[i] );
            glVertex3f(wx[i], wy[i], wz[i]);
         }
        glEnd();
        for(int i = 0; i < N; i++) {
            vx[i] = wx[i];
            vy[i] = wy[i];
            vz[i] = wz[i];
        }
    }
}

//-- Loads mesh data in OFF format    -------------------------------------
void loadMeshFile(const char* fname)
{
    ifstream fp_in;
    int num, ne;

    fp_in.open(fname, ios::in);
    if(!fp_in.is_open())
    {
        cout << "Error opening mesh file" << endl;
        exit(1);
    }

    fp_in.ignore(INT_MAX, '\n');				//ignore first line
    fp_in >> nvrt >> ntri >> ne;			    // read number of vertices, polygons, edges

    x = new float[nvrt];                        //create arrays
    y = new float[nvrt];
    z = new float[nvrt];

    t1 = new int[ntri];
    t2 = new int[ntri];
    t3 = new int[ntri];

    for(int i=0; i < nvrt; i++)                         //read vertex list
        fp_in >> x[i] >> y[i] >> z[i];

    for(int i=0; i < ntri; i++)                         //read polygon list
    {
        fp_in >> num >> t1[i] >> t2[i] >> t3[i];
        if(num != 3)
        {
            cout << "ERROR: Polygon with index " << i  << " is not a triangle." << endl;  //not a triangle!!
            exit(1);
        }
    }

    fp_in.close();
    cout << " File successfully read." << endl;
}

// -- Load bitmaps/tgas And Convert To Textures -----------------------------------------------------
void loadGLTextures()
{
    glGenTextures(10, texId); 		// Create texture ids
    // *** left ***
    glBindTexture(GL_TEXTURE_2D, texId[0]);
    loadTGA("left.tga");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // *** front ***
    glBindTexture(GL_TEXTURE_2D, texId[1]);
    loadTGA("front.tga");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // *** right ***
    glBindTexture(GL_TEXTURE_2D, texId[2]);
    loadTGA("right.tga");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // *** back***
    glBindTexture(GL_TEXTURE_2D, texId[3]);
    loadTGA("back.tga");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // *** top ***
    glBindTexture(GL_TEXTURE_2D, texId[4]);
    loadTGA("up.tga");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // *** down ***
    glBindTexture(GL_TEXTURE_2D, texId[5]);
    loadTGA("down.tga");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // *** Concrete floor ***
    glBindTexture(GL_TEXTURE_2D, texId[6]);
    loadBMP("concrete_base.bmp");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // *** sliding metal doors***
    glBindTexture(GL_TEXTURE_2D, texId[7]);
    loadBMP("slidingMetal.bmp");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // *** corrugated aluminium walls***
    glBindTexture(GL_TEXTURE_2D, texId[8]);
    loadBMP("corrugated.bmp");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // *** Roofing metal***
    glBindTexture(GL_TEXTURE_2D, texId[9]);
    loadBMP("metalRoof.bmp");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // *** Outer Fence ***
    glBindTexture(GL_TEXTURE_2D, texId[10]);
    loadBMP("concrete_fence.bmp");
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
}

//-- Desert skybox   --------------------------------------------------------
void skybox(){
  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  ////////////////////// LEFT WALL ///////////////////////
  glBindTexture(GL_TEXTURE_2D, texId[0]);
  glBegin(GL_QUADS);
    glTexCoord2f(1 , 0.4); glVertex3f(-1000,  0, 1000);
    glTexCoord2f(0 , 0.4); glVertex3f(-1000, 0, -1000);
    glTexCoord2f(0 , 1); glVertex3f(-1000, 1000., -1000);
    glTexCoord2f(1 , 1); glVertex3f(-1000, 1000, 1000);
  glEnd();

  ////////////////////// FRONT WALL ///////////////////////
  glBindTexture(GL_TEXTURE_2D, texId[1]);
  glBegin(GL_QUADS);
    glTexCoord2f(1 , 0.4); glVertex3f(-1000,  0, -1000);
    glTexCoord2f(0 , 0.4); glVertex3f(1000, 0, -1000);
    glTexCoord2f(0 , 1); glVertex3f(1000, 1000, -1000);
    glTexCoord2f(1 , 1); glVertex3f(-1000,  1000, -1000);
  glEnd();

 ////////////////////// RIGHT WALL ///////////////////////
  glBindTexture(GL_TEXTURE_2D, texId[2]);
  glBegin(GL_QUADS);
    glTexCoord2f(1 , 0.4); glVertex3f(1000,  0, -1000);
    glTexCoord2f(0 , 0.4); glVertex3f(1000, 0, 1000);
    glTexCoord2f(0 , 1); glVertex3f(1000, 1000,  1000);
    glTexCoord2f(1 , 1); glVertex3f(1000,  1000,  -1000);
  glEnd();

  ////////////////////// REAR WALL ////////////////////////
  glBindTexture(GL_TEXTURE_2D, texId[3]);
  glBegin(GL_QUADS);
    glTexCoord2f(1 , 0.4); glVertex3f( 1000, 0, 1000);
    glTexCoord2f(0 , 0.4); glVertex3f(-1000, 0,  1000);
    glTexCoord2f(0 , 1); glVertex3f(-1000, 1000,  1000);
    glTexCoord2f(1 , 1); glVertex3f( 1000, 1000, 1000);
  glEnd();

  /////////////////////// TOP //////////////////////////
  glBindTexture(GL_TEXTURE_2D, texId[4]);
  glBegin(GL_QUADS);
    glTexCoord2f(1 , 1); glVertex3f(-1000, 1000, -1000);
    glTexCoord2f(1 , 0); glVertex3f(1000, 1000,  -1000);
    glTexCoord2f(0 , 0); glVertex3f(1000, 1000,  1000);
    glTexCoord2f(0 , 1); glVertex3f(-1000, 1000, 1000);
  glEnd();

  /////////////////////// FLOOR //////////////////////////
  glBindTexture(GL_TEXTURE_2D, texId[5]);
  glBegin(GL_QUADS);
    glTexCoord2f(1 , 1); glVertex3f(-1000, 0., 1000);
    glTexCoord2f(1 , 0); glVertex3f(1000, 0.,  1000);
    glTexCoord2f(0 , 0); glVertex3f(1000, 0., -1000);
    glTexCoord2f(0 , 1); glVertex3f(-1000, 0., -1000);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
}

//-- The floor of the whole compound, gravel textured --------------------------------------------------------
void floor()
{
    //The floor is made up of several tiny squares on a 80x95 grid. Each square has a unit size.
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId[6]);

    glNormal3f(0,1,0);
    glBegin(GL_QUADS);
        for(int i = -40; i < 40; i++) {
            for(int j = -35;  j < 60; j++) {
                glTexCoord2f(0., 0.); glVertex3f(i, 0.1, j);
                glTexCoord2f(1, 0.);glVertex3f(i, 0.1, j+1);
                glTexCoord2f(1, 1);glVertex3f(i+1, 0.1, j+1);
                glTexCoord2f(0., 1);glVertex3f(i+1, 0.1, j);
            }
        }
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

//-- textured Fence surrounding the whole compound ---------------------------------------------------
void fence()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId[10]);

    glBegin(GL_QUADS);
        glNormal3f(0,0,-1);
        glTexCoord2f(0., 1.); glVertex3f(40, 8, 60);
        glTexCoord2f(0., 0.); glVertex3f(40, 0, 60);
        glTexCoord2f(3., 0.); glVertex3f(-40, 0, 60);
        glTexCoord2f(3., 1.); glVertex3f(-40, 8, 60);
    
        glNormal3f(1,0,0);
        glTexCoord2f(0., 1.); glVertex3f(-40, 8, 60);
        glTexCoord2f(0., 0.); glVertex3f(-40, 0, 60);
        glTexCoord2f(3., 0.); glVertex3f(-40, 0, -35);
        glTexCoord2f(3., 1.); glVertex3f(-40, 8, -35);

        glNormal3f(0,0,1);
        glTexCoord2f(0., 1.); glVertex3f(-40, 8, -35);
        glTexCoord2f(0., 0.); glVertex3f(-40, 0, -35);
        glTexCoord2f(3., 0.); glVertex3f(40, 0, -35);
        glTexCoord2f(3., 1.); glVertex3f(40, 8, -35);

        glNormal3f(-1,0,0);
        glTexCoord2f(0., 1.); glVertex3f(40, 8, -35);
        glTexCoord2f(0., 0.); glVertex3f(40, 0, -35);
        glTexCoord2f(3., 0.); glVertex3f(40, 0, 60);
        glTexCoord2f(3., 1.); glVertex3f(40, 8, 60);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

//-- textured Hanger containing the saucer -----------------------------------------------------------
void hanger()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId[8]);  //Walls

    glBegin(GL_QUADS);
      glNormal3f(0,0,1);
      glTexCoord2f(0., 1.); glVertex3f(-15, 15, 20);
      glTexCoord2f(0., 0.); glVertex3f(-15, 0, 20);
      glTexCoord2f(1., 0.); glVertex3f(-30, 0, 20);
      glTexCoord2f(1., 1.); glVertex3f(-30, 15, 20);
      
      glNormal3f(1,0,0);
      glTexCoord2f(0., 1.); glVertex3f(-30, 15, 20);
      glTexCoord2f(0., 0.); glVertex3f(-30, 0, 20);
      glTexCoord2f(2., 0.); glVertex3f(-30, 0, -20);
      glTexCoord2f(2., 1.); glVertex3f(-30, 15, -20);

      glNormal3f(0,0,1);
      glTexCoord2f(0., 1.); glVertex3f(-30, 15, -20);
      glTexCoord2f(0., 0.); glVertex3f(-30, 0, -20);
      glTexCoord2f(4., 0.); glVertex3f(30, 0, -20);
      glTexCoord2f(4., 1.); glVertex3f(30, 15, -20);

      glNormal3f(-1,0,0);
      glTexCoord2f(0., 1.); glVertex3f(30, 15, -20);
      glTexCoord2f(0., 0.); glVertex3f(30, 0, -20);
      glTexCoord2f(2., 0.); glVertex3f(30, 0, 20);
      glTexCoord2f(2., 1.); glVertex3f(30, 15, 20);

      glNormal3f(0,0,1);
      glTexCoord2f(0., 1.); glVertex3f(30, 15, 20);
      glTexCoord2f(0., 0.); glVertex3f(30, 0, 20);
      glTexCoord2f(1., 0.); glVertex3f(15, 0, 20);
      glTexCoord2f(1., 1.); glVertex3f(15, 15, 20);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId[9]);  // Roof
    glBegin(GL_QUADS);
      glNormal3f(0,-1,0);
      glTexCoord2f(0., 0.);  glVertex3f(-36, 14, -20);
      glTexCoord2f(0., 1.);  glVertex3f(-36, 14, 20);
      glTexCoord2f(1., 1.);  glVertex3f(0, 20, 20);
      glTexCoord2f(1., 0.);  glVertex3f(0, 20, -20);

      glNormal3f(0,-1,0);
      glTexCoord2f(0., 0.);  glVertex3f(0, 20, -20);
      glTexCoord2f(0., 1.);  glVertex3f(0, 20, 20);
      glTexCoord2f(1., 1.);  glVertex3f(36, 14, 20);
      glTexCoord2f(1., 0.);  glVertex3f(36, 14, -20);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

//-- Doors to the Hanger --------------------------------------------------------------------
void hanger_doors()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texId[7]);

    glPushMatrix();
        glTranslatef(-doors_progress,0,0);
        glNormal3f(0,0,1);
        glBegin(GL_QUADS); // left door
          glTexCoord2f(0., 1.); glVertex3f(-16, 15, 19.8);
          glTexCoord2f(0., 0.); glVertex3f(-16, 0, 19.8);
          glTexCoord2f(1., 0.); glVertex3f(-0.1, 0, 19.8);
          glTexCoord2f(1., 1.); glVertex3f(-0.1, 15, 19.8);
        glEnd();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(doors_progress,0,0);
        glNormal3f(0,0,1);
        glBegin(GL_QUADS); // right door
          glTexCoord2f(0., 1.); glVertex3f(0.1, 15, 19.8);
          glTexCoord2f(0., 0.); glVertex3f(0.1, 0, 19.8);
          glTexCoord2f(1., 0.); glVertex3f(16, 0, 19.8);
          glTexCoord2f(1., 1.); glVertex3f(16, 15, 19.8);
        glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

//--------draws the mesh model of the cannon----------------------------
void drawCannon()
{
    glColor3f(0, 0.1, 0.4);

    //Construct the object model here using triangles read from OFF file
    glBegin(GL_TRIANGLES);
        for(int tindx = 0; tindx < ntri; tindx++)
        {
           glVertex3d(x[t1[tindx]], y[t1[tindx]], z[t1[tindx]]);
           glVertex3d(x[t2[tindx]], y[t2[tindx]], z[t2[tindx]]);
           glVertex3d(x[t3[tindx]], y[t3[tindx]], z[t3[tindx]]);
        }
    glEnd();
}

//-- Draws the cannon and cannon base ---------------------
void cannon()
{
    glPushMatrix();
        glTranslatef(-20, 30, 0); //Pivot point coordinates
        glRotatef(30, 0, 0, 1); //Rotation
        glTranslatef(20, -30, 0);
        drawCannon();
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-10, 5, 17);
        glScalef(80, 10, 6);
        glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-20, 25, 17);
        glScalef(40, 30, 6);
        glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-10, 5, -17);
        glScalef(80, 10, 6);
        glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(-20, 25, -17);
        glScalef(40, 30, 6);
        glutSolidCube(1);
    glPopMatrix();
}

//-- Draws a moving robot ----------
void robot()
{
    glColor3f(1, 1, 0);		//Headlight
    glPushMatrix();
        glTranslatef(0, 8.2, -0.8);
        glutSolidCube(0.5);
    glPopMatrix();
    
    glColor3f(1., 0.78, 0.06);		//Head
    glPushMatrix();
        glTranslatef(0, 7.7, 0);
        glutSolidCube(1.4);
    glPopMatrix();
    
    glColor3f(1., 0., 0.);			//Torso
    glPushMatrix();
        glTranslatef(0, 5.5, 0);
        glScalef(3, 3, 1.4);
        glutSolidCube(1);
    glPopMatrix();

    glColor3f(0., 0., 1.);		//Right leg
    glPushMatrix();
        glTranslatef(-0.8, 4, 0);
        glRotatef(-armMovement, 1, 0, 0);
        glTranslatef(0.8, -4, 0);
        glTranslatef(-0.8, 2.2, 0);
        glScalef(1, 4.4, 1);
        glutSolidCube(1);
    glPopMatrix();

    glColor3f(0., 0., 1);                 //Left leg
    glPushMatrix();
        glTranslatef(0.8, 4, 0);
        glRotatef(armMovement, 1, 0, 0);
        glTranslatef(-0.8, -4, 0);
        glTranslatef(0.8, 2.2, 0);
        glScalef(1, 4.4, 1);
        glutSolidCube(1);
    glPopMatrix();    
    
    glPushMatrix();	                    	//Right arm
        glTranslatef(-2, 6.5, 0);
        glRotatef(armMovement, 1, 0, 0);
        glTranslatef(2, -6.5, 0);
        glTranslatef(-2, 5, 0);
        glScalef(1, 4, 1);
        glutSolidCube(1);
    glPopMatrix();

    glPushMatrix();		//Left arm
        glTranslatef(2, 6.5, 0);
        glRotatef(-armMovement, 1, 0, 0);
        glTranslatef(-2, -6.5, 0);
        glTranslatef(2, 5, 0);
        glScalef(1, 4, 1);
        glutSolidCube(1);
    glPopMatrix();
}

//-------------------------------------------------------------------------------------------
void initialize(void)
{
    float lgt_pos[4] = {0.0f, 100.0f, 0.0f, 1.0f};  //light0 position (directly above the origin)
    loadGLTextures();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glColorMaterial(GL_FRONT, GL_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

//	Light 0's is the main ambient light
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);
    glLightfv(GL_LIGHT0, GL_POSITION, lgt_pos);   //Main light position

    glEnable(GL_LIGHT1);		    //Light 1 is the headlight for robot 1
    glLightfv(GL_LIGHT1, GL_AMBIENT, green);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, green);
    glLightfv(GL_LIGHT1, GL_SPECULAR, green);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 75);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT,20);
	
    glEnable(GL_LIGHT2);		    //Light 2 is the headlight for robot 2
    glLightfv(GL_LIGHT2, GL_AMBIENT, red);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, red);
    glLightfv(GL_LIGHT2, GL_SPECULAR, red);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 75);
    glLightf(GL_LIGHT2, GL_SPOT_EXPONENT,20);
    
    glEnable(GL_LIGHT3);		    
    glLightfv(GL_LIGHT3, GL_AMBIENT, blue);  // spotlight attached to the top of the UFO, pointing down at itself
    glLightfv(GL_LIGHT3, GL_DIFFUSE, blue);
    glLightfv(GL_LIGHT3, GL_SPECULAR, blue);
    glLightf(GL_LIGHT3, GL_SPOT_CUTOFF, 75);
    glLightf(GL_LIGHT3, GL_SPOT_EXPONENT,20);

    glEnable(GL_LIGHT4);
    glLightfv(GL_LIGHT4, GL_AMBIENT, yellow);	//spotlight attached to the bottom of the UFO, pointing out
    glLightfv(GL_LIGHT4, GL_DIFFUSE, yellow);
    glLightfv(GL_LIGHT4, GL_SPECULAR, yellow);
    glLightf(GL_LIGHT4, GL_SPOT_CUTOFF, 75);
    glLightf(GL_LIGHT4, GL_SPOT_EXPONENT,20);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(80.0, 1.0, 1.0, 2000.0);   //Perspective projection  // Was having problems with clipping??? of the skybox, so changed this lots

    special_view_mode = 0;
    debug_mode = 0;
}

//-------------------------------------------------------------------------------------------
void display(void)
{
    float robot_spotlight_pos[4] = {0.0f, 8.2f, -0.8f, 1.0f};
    float robot_spot_dir[4] = {0.0f, -1.0f, -1.0f, 0.0f};   
    float saucer_light3_pos[4] = {0.0f, 11.0f, 9.0f, 1.0f};
    float saucer_light3_dir[4] = {0.0f, -3.0f, 1.0f, 0.0f}; 
    float saucer_light4_pos[4] = {0.0f, 1.0f, 18.0f, 1.0f};
    float saucer_light4_dir[4] =  {0.0f, 0.0f, -1.0f, 0.0f};

    glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (special_view_mode) {
        gluLookAt (saucer_cam_x, saucer_cam_y, saucer_cam_z, saucer_look_x, saucer_look_y, saucer_look_z, 0.0, 1.0, 0.0);
    } else {
        gluLookAt (cam_x, cam_y, cam_z, look_x, look_y, look_z, 0.0, 1.0, 0.0);
    }
    
    glColor3f(1,1,1);
    skybox();             //Textured Skybox of area 51
    floor();              //concrete floor
    hanger_doors();             //opening and closing doors
    fence();
    hanger();             //walls and roof  of the castle/hanger

    glPushMatrix();              // ------------ Big robot
        glTranslatef(-35,robot_bobbing-4,robot_z);
        glRotatef(robot_dir,0,1,0);
        glScalef(1.4,1.4,1.4);
        glLightfv(GL_LIGHT1, GL_POSITION, robot_spotlight_pos); 
        glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, robot_spot_dir); 
        robot();
    glPopMatrix();
    
    glPushMatrix();             // ------------ Small robot
        glTranslatef(35,robot_bobbing-1,robot_z);
        glRotatef(robot_dir,0,1,0);
        glLightfv(GL_LIGHT2, GL_POSITION, robot_spotlight_pos);  
        glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, robot_spot_dir); 
        robot();
    glPopMatrix();

    glPushMatrix();           // ----------- Cannon
        glTranslatef(15,0,35);
        glRotatef(-90,0,1,0);
        glScalef(0.1,0.1,0.1);
        cannon();
    glPopMatrix();

    glPushMatrix();         // ----------- Cannon ball
        glTranslatef(15,0,35);
        glRotatef(-90,0,1,0);
        glScalef(0.1,0.1,0.1);
        glTranslatef(ball_pos_z, ball_pos_y, 0);
        glutSolidSphere(5, 36, 18);
    glPopMatrix();

    glColor3f(1,0,0);
    glPushMatrix();
        glTranslatef(saucer_x, saucer_y, saucer_z);
        glRotatef(saucer_rotation, 0, 1, 0);
        glLightfv(GL_LIGHT3, GL_POSITION, saucer_light3_pos);
        glLightfv(GL_LIGHT3, GL_SPOT_DIRECTION, saucer_light3_dir);  //spaceships flashing lights
        glLightfv(GL_LIGHT4, GL_POSITION, saucer_light4_pos);
        glLightfv(GL_LIGHT4, GL_SPOT_DIRECTION, saucer_light4_dir);
        saucer();						    //The aliens spaceship!
    glPopMatrix();

    glutSwapBuffers();   //Useful for animation
    glFlush();
}

//-------------------------------------------------------------------------------------------
void door_opening_timer(int value)
{
    if (doors_progress >= 14.7) {
        doors_open = 1;
        doors_moving = 0;
    } else {
        doors_progress+=0.3;
        display();
        glutTimerFunc(60, door_opening_timer, 0);
    }
}

//-------------------------------------------------------------------------------------------
void door_closing_timer(int value)
{
    if (doors_progress <= 0) {
        doors_open = 0;
        doors_moving = 0;
    } else {
        doors_progress-=0.3;
        display();
        glutTimerFunc(60, door_closing_timer, 0);
    }
}

// --   ----------
void robots_walking_timer(int value)
{
    //This part is for the arms and legs swinging
    if(armMovement > 30) {
        arm_dir = 0;
    } else if (armMovement < -30) {
        arm_dir = 1;
    }
    if (arm_dir) {
        armMovement+=3;
    } else if (!arm_dir) {
        armMovement-=3;
    }

    //this part is for the movement of the robot
    if (walking_forwards) {
        if (robot_z < -20) {
            walking_forwards = 0;
            robot_dir = 180;
        }
        robot_z-= 0.5;
    } else if (walking_forwards == 0) {
        if (robot_z > 30) {
            walking_forwards = 1;
            robot_dir = 0;
        }
        robot_z+= 0.5;
    }

    //Robot bobbing movement
    if (value == 1) {
        robot_bobbing += 0.05;
        glutTimerFunc(80, robots_walking_timer, 0);
    } else {
        robot_bobbing -= 0.05;
        glutTimerFunc(80, robots_walking_timer, 1);
    }
    display();
}

// --  Timer to control the firing of the cannon----------
void cannon_shot_timer(int value)
{
    ball_pos_z += 2 * z_speed;
    ball_pos_y += 2 * y_speed;

    if (ball_pos_y < 2){ //Hit the ground, can keep rolling, but will be affected by ground friction instead of air resistance
        ball_pos_y = 2;
        resistance = -8;
        y_speed = 0;
    } else {
        y_speed = y_speed + (0.01 * gravity);
    }
    
    if (ball_pos_z > 9598) {                                        //Colision with the skybox
        z_speed = 0;
    } else if (((ball_pos_y < 80) && (ball_pos_z > 220))) {         // Collision with the fence
        z_speed = 0;
    } else {
        z_speed = z_speed + (0.01 * resistance);
    }

    if ((z_speed <= 0) && (ball_pos_y <= 2)) {   // Ball has stoped, so end the timer
        //do nothing
    } else {
        glutTimerFunc(100, cannon_shot_timer, 0);
    }
    display();
}

// -- Timer to control the movement of the flying saucer-------------------------------------
void saucer_take_off_timer(int value)
{
    if (saucer_pos > 10) {
        saucer_rotation += 20;
    } else if (saucer_pos > 3) {
        saucer_rotation += 10;
    }
    
    if (saucer_pos >= F) {
        //DO what happens after the flight path, maybe just go straight up???
        saucer_x = saucer_x + 4;
        saucer_y = saucer_y + 4;
        saucer_z = saucer_z + 4;

        saucer_cam_x = saucer_x;
        saucer_cam_y = saucer_y - 2;
        saucer_cam_z = saucer_z;
        saucer_look_x = 0;
        saucer_look_y = 5;
        saucer_look_z = 40;

    } else {
        saucer_x = sx[saucer_pos];
        saucer_y = sy[saucer_pos];
        saucer_z = sz[saucer_pos];

        saucer_cam_x = saucer_x;
        saucer_cam_y = saucer_y + 12;
        saucer_cam_z = saucer_z + 9.6;

    }
    saucer_pos ++;

    display();
    glutTimerFunc(50, saucer_take_off_timer, 0);
}

//-------------------------------------------------------------------------------------------
void SpecialKey(int key, int x, int y)
{
    switch (key) {
      case GLUT_KEY_UP:
        if (!special_view_mode) {
            cam_x += 1*sin(angle);
            cam_z -= 1*cos(angle);
        }
        break;
      case GLUT_KEY_DOWN:
        if (!special_view_mode) {
            cam_x -= 1*sin(angle);
            cam_z += 1*cos(angle);
        }
        break;
      case GLUT_KEY_LEFT:
        if (!special_view_mode) {
            angle -= 0.1;
        }
        break;
      case GLUT_KEY_RIGHT:
        if (!special_view_mode) {
            angle += 0.1;
        }
        break;
    }
    if (cam_x > 39) { //Collision detection with the outer fence
        cam_x = 38;
    }
    if (cam_x < -39) {
        cam_x = -38;
    }
    if (cam_z > 59) {
        cam_z = 58;
    }
    if (cam_z < -34) {
        cam_z = -33;
    }

    look_x = cam_x + 100*sin(angle);
    look_z = cam_z - 100*cos(angle);

    display();
}

//-------------------------------------------------------------------------------------------
void keyboard (unsigned char key, int x, int y)
{
    float max_look_height = cam_y + 100;
    float min_look_height = cam_y - 100;
    switch (key) {
        //--------------------------------- debug mode only-------------------------------------
      case 'w':   //If w is pressed. look up
        if (debug_mode) {
            if  (look_y < max_look_height)
                look_y +=5;
        }
        break;
      case 's':   //If s is pressed. look down
        if (debug_mode) {
            if  (look_y > min_look_height)
                look_y -=5;
        }
        break;
      case 'e':   //If w is pressed. look up
        if (debug_mode) {
            cam_y -= 5;
        }
        break;
      case 'd':   //If s is pressed. look down
        if (debug_mode) {
            cam_y += 5;
        }
        break;
        //----------------------------- end of debug mode-------------------------------------
      case ' ':  // pressing the spacebar opens/closes the doors
        if (doors_moving) {  //doors already moving
        } else if (doors_open) {
            doors_moving = 1;
            glutTimerFunc(40, door_closing_timer, 0);
        } else {
            doors_moving = 1;
            glutTimerFunc(40, door_opening_timer, 0);
        }
        break;
      case 'c':  //pressing C shoots the cannon!
        if (ball_shot) {  //reload
            ball_shot = 0;
            ball_pos_z = 38.88;
            ball_pos_y = 64;
            z_speed = 12;
            y_speed = 8;
            resistance = -2;
        } else {            //shoot
            ball_shot = 1;
            glutTimerFunc(50, cannon_shot_timer, 0);
        }
        break;
      case 'v':  //pressing V launches the flying saucer on its trajectory only if the doors are open and it hasnt already taken off
        if(saucer_on_ground && doors_open) {
            saucer_on_ground = 0;
            glutTimerFunc(100, saucer_take_off_timer, 0);
        }
        break;
      case 'z':  //debug mode switch 
        if (debug_mode) {
            debug_mode = 0;
        } else {
            debug_mode = 1;
        }
        break;
      case 'x':  //view mode switch
        if (special_view_mode) {
            special_view_mode = 0;
        } else {
            special_view_mode = 1;
        }
        break;
    }
    display();
}

//-------------------------------------------------------------------------------------------
int main(int argc, char** argv)
{
   loadMeshFile("Cannon.off");
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE|GLUT_DEPTH);
   glutInitWindowSize (600, 600);
   glutInitWindowPosition (50, 50);
   glutCreateWindow ("Area 51");
   initialize ();

   glutTimerFunc(40, robots_walking_timer, 0);
   glutSpecialFunc(SpecialKey);
   glutKeyboardFunc(keyboard);
   glutDisplayFunc(display);

   glutMainLoop();
   return 0;
}
