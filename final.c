//Ryan Than - Final Project
#include "CSCIx229.h"


// Variables:
//********************************************************************************************************************************************************************************
//Global Variables
int axes      =   0;  // Variable to toggle axes
int proj      =   1;  // Variable to keep track of projection mode
int th        = -25;  // Azimuth of view angle
int ph        =  25;  // Elevation of view angle
int fov       =  55;  // Field of view
int obj       =  12;  // Scene/object selection
double asp    =   1;  // Aspect Ratio
double dim    =  55;  // Size of World
int box       =   1;  //Toggle Skybox
const char* view[] = {"Orthogonal","Perspective","First-Person"}; // Projection mode text
unsigned int texture[120]; // Array to hold texture names

//Light Variables:
int move      =   1;  // Move light
int light     =   1;  // Lighting (1 = ON, 0 = OFF)
int distance  =  35;  // Light distance
int inc       =   5;  // Ball increment
int smooth    =   1;  // Smooth/Flat shading
int local     =   0;  // Local Viewer Model
int emission  =   0;  // Emission intensity (%)
int ambient   =  40;  // Ambient intensity (%)
int diffuse   =  50;  // Diffuse intensity (%)
int specular  =   0;  // Specular intensity (%)
int shininess =   0;  // Shininess (power of two)
float shiny   =   1;  // Shininess (value)
int zh        =  90;  // Light azimuth
float ylight  =   7;  // Elevation of light

//Animation Variables:
float car1right = -8;
float car1bottom = -9.5;
float car1left = 20;
float car1top = -30;

float car2right = 14;
float car2bottom = -8;
float car2left = -11.5;
float car2top = 8;


//Variables to help draw cone:
typedef struct {float x,y,z;} vtx;
typedef struct {int A,B,C;} tri;
#define n 500
vtx is[n];

//First-Person Global Variables (Reused from HW4)
//Position Variables
double pos_x = 1;
double pos_y = 1.1;
double pos_z = 15;

//Camera Variables
double camera_x = 0;
double camera_z = 0;
//Since we are not moving the camera up, we don't need camera_y

int rotation_angle = 0; //Angle of rotation
//********************************************************************************************************************************************************************************



// Primitive Functions: Functions that draw simple shapes and objects that can be used to create other objects
//********************************************************************************************************************************************************************************
//Draw vertex in polar coordinates with normal (borrowed from Example 13 on Canvas)
static void Vertex(double th,double ph)
{
   double x = Sin(th)*Cos(ph);
   double y = Cos(th)*Cos(ph);
   double z =         Sin(ph);
   //  For a sphere at the origin, the position
   //  and normal vectors are the same
   glNormal3d(x,y,z);
   glVertex3d(x,y,z);
}

//Draw a ball at (x,y,z) with radius (r) (borrowed from Example 13 on Canvas)
static void ball(double x,double y,double z,double r)
{
   int th,ph;
   float yellow[] = {1.0,1.0,0.0,1.0};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,1);
   glMaterialf(GL_FRONT,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT,GL_SPECULAR,yellow);
   glMaterialfv(GL_FRONT,GL_EMISSION,Emission);
   //  Bands of latitude
   for (ph=-90;ph<90;ph+=inc)
   {
      glBegin(GL_QUAD_STRIP);
      for (th=0;th<=360;th+=2*inc)
      {
         Vertex(th,ph);
         Vertex(th,ph+inc);
      }
      glEnd();
   }
   //  Undo transformations
   glPopMatrix();
}

//Function to draw triangles (borrowed from Example 13 on Canvas)
static void triangle(vtx A,vtx B,vtx C)
{
   //Planar vector 0
   float dx0 = A.x-B.x;
   float dy0 = A.y-B.y;
   float dz0 = A.z-B.z;
   //Planar vector 1
   float dx1 = C.x-A.x;
   float dy1 = C.y-A.y;
   float dz1 = C.z-A.z;
   // Calculate the normal vectors
   float Nx = dy0*dz1 - dy1*dz0;
   float Ny = dz0*dx1 - dz1*dx0;
   float Nz = dx0*dy1 - dx1*dy0;
   //  Draw triangle
   glNormal3f(Nx,Ny,Nz);
   glBegin(GL_TRIANGLES);
   glTexCoord2f(1,0); glVertex3f(A.x,A.y,A.z);
   glTexCoord2f(0,1); glVertex3f(B.x,B.y,B.z);
   glTexCoord2f(-1,0); glVertex3f(C.x,C.y,C.z);
   glEnd();
}

//Cone Data (modified from icosahedron data in Example 13 to form a low-poly cone)
const int N=16; //My cone has 16 faces
//Triangle index list
const tri idx[] =
   {
      {2, 1, 0},   {3, 2, 0},   {4, 3, 0},   {5, 4, 0},   {6, 5, 0},
      {7, 6, 0},   {8, 7, 0},   {9, 8, 0},   {10, 9, 0},  {11, 10, 0},
      {12, 11, 0}, {13, 12, 0}, {14, 13, 0}, {15, 14, 0}, {16, 15, 0},
      {1, 16, 0}
   };
//Vertex coordinates
const vtx xyz[] =
   {
      {0.0, 1.5, 0.0},   {0.0, 0.0, 1.1},   {0.4, 0.0, 1.0},
      {0.8, 0.0, 0.8},   {1.0, 0.0, 0.4},   {1.1, 0.0, 0.0},
      {1.0, 0.0, -0.4},  {0.8, 0.0, -0.8},  {0.4, 0.0, -1.0},
      {0.0, 0.0, -1.1},  {-0.4, 0.0, -1.0}, {-0.8, 0.0, -0.8},
      {-1.0, 0.0, -0.4}, {-1.1, 0.0, 0.0},  {-1.0, 0.0, 0.4},
      {-0.8, 0.0, 0.8},  {-0.4, 0.0, 1.0}
   };

//Function to draw a low-poly cone at (x,y,z) with scale (s) and rotation (th) (modified icosahedron function from Example 13)
//Mode: 0 = Tree, 1 = Metal
static void cone(float x, float y, float z, float dx, float dy, float dz, float th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

   //Save transformation
   glPushMatrix();

   //Offset, scale, and rotation
   glTranslatef(x,y,z);
   glScalef(dx,dy,dz);
   glRotatef(th,0,1,0);

   //Enable textures
   glEnable(GL_TEXTURE_2D);
   if (mode == 0) { //Tree
      glColor3f(0.5, 1, 0.5); //Give the texture some green color underneath
      glBindTexture(GL_TEXTURE_2D,texture[0]);
   } else if (mode == 1) { //Metal
      glColor3f(0.7, 0.7, 0.7); //Gray
      glBindTexture(GL_TEXTURE_2D,texture[8]);
   }
   
   //Draw the bottom of the cone:
   glBegin(GL_POLYGON);
   glNormal3f(0.0,-1.0, 0.0); //Set normal of bottom to be -y
   glTexCoord2f(0.5,0);     glVertex3f(0.0, 0.0, 1.1);
   glTexCoord2f(0.31,0.05); glVertex3f(-0.4, 0.0, 1.0);
   glTexCoord2f(0.12,0.12); glVertex3f(-0.8, 0.0, 0.8);
   glTexCoord2f(0.05,0.31); glVertex3f(-1.0, 0.0, 0.4);
   glTexCoord2f(0,0.5);     glVertex3f(-1.1, 0.0, 0.0);
   glTexCoord2f(0.05,0.69); glVertex3f(-1.0, 0.0, -0.4);
   glTexCoord2f(0.12,0.86); glVertex3f(-0.8, 0.0, -0.8);
   glTexCoord2f(0.31,0.95); glVertex3f(-0.4, 0.0, -1.0);
   glTexCoord2f(0.5,1);     glVertex3f(0.0, 0.0, -1.1);
   glTexCoord2f(0.79,0.95); glVertex3f(0.4, 0.0, -1.0);
   glTexCoord2f(0.86,0.86); glVertex3f(0.8, 0.0, -0.8);
   glTexCoord2f(0.95,0.69); glVertex3f(1.0, 0.0, -0.4);
   glTexCoord2f(1,0.5);     glVertex3f(1.1, 0.0, 0.0);
   glTexCoord2f(0.95,0.31); glVertex3f(1.0, 0.0, 0.4);
   glTexCoord2f(0.86,0.12); glVertex3f(0.8, 0.0, 0.8);
   glTexCoord2f(0.79,0.05); glVertex3f(0.4, 0.0, 1.0);
   glTexCoord2f(0.5,0);     glVertex3f(0.0, 0.0, 1.1);
   glEnd();

   //Draw the top of the cone:
   for (int i=0;i<N;i++)
      triangle(xyz[idx[i].A],xyz[idx[i].B],xyz[idx[i].C]);

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw a octagonal cylinder at (x,y,z) with dimensions (dx,dy,dz), rotation (th) around the y-axis, and rotation (ph) around the x-axis
//Mode: 0 = Tree Trunk, 1/2 = Metal, 4/11 = White Metal, 5/6/7 = Red/White/Yellow Column, 8/9/10 = Red/White/Yellow Stone, 12/13/14 = Black Marble
//      15/16 = Red/Blue Metal, 17 = Solid Red Color, 18 = Solid Brown Color
static void cylinder(float x, float y, float z, float dx, float dy, float dz, float th, float ph, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

   //Save transformation
   glPushMatrix();
   //Offset, scale, and rotation
   glTranslatef(x,y,z);
   glScalef(dx,dy,dz);
   glRotatef(th,0,1,0);
   glRotatef(ph,1,0,0);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[1]); //Tree Trunk
   else if (mode == 1 || mode == 2) glBindTexture(GL_TEXTURE_2D,texture[8]); //Metal
   else if (mode == 4 || mode == 11) glBindTexture(GL_TEXTURE_2D,texture[31]); //White Metal
   else if (mode == 5) glBindTexture(GL_TEXTURE_2D,texture[51]); //Red Column
   else if (mode == 6) glBindTexture(GL_TEXTURE_2D,texture[52]); //White Column
   else if (mode == 7) glBindTexture(GL_TEXTURE_2D,texture[53]); //Yellow Column
   else if (mode == 8) glBindTexture(GL_TEXTURE_2D,texture[48]); //Red Stone
   else if (mode == 9) glBindTexture(GL_TEXTURE_2D,texture[49]); //White Stone
   else if (mode == 10) glBindTexture(GL_TEXTURE_2D,texture[50]); //Yellow Stone
   else if (mode == 12 || mode == 13  || mode == 14) glBindTexture(GL_TEXTURE_2D,texture[80]); //Black Marble
   else if (mode == 15) glBindTexture(GL_TEXTURE_2D,texture[81]); //Red Metal
   else if (mode == 16) glBindTexture(GL_TEXTURE_2D,texture[83]); //Blue Metal
   else if (mode == 17) { //Red
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 0.1, 0.1);
   } else if (mode == 18) { //Brown
      glDisable(GL_TEXTURE_2D);
      glColor3f(0.804, 0.522, 0.247);
   } 

   //Draw the sides of the cylinder:
   glBegin(GL_QUADS);
   //Front Left Rectangle
   //Planar vector 1 (p1) = v3 - v2 = (-0.2, 1.0, 0.4) - (-0.4, 1.0, 0.2) = (0.2, 0.0, 0.2)
   //Planar vector 2 (p2) = v2 - v1 = (-0.4, 1.0, 0.2) - (-0.4, 0.0, 0.2) = (0.0, 1.0, 0.0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0.0*0.0 - 0.2*1.0, 0.2*0.0 - 0.2*0.0, 0.2*1.0 - 0.0*0.0); //Calculate normal
   glTexCoord2f(0,1); glVertex3f(-0.2, 0.0, 0.4); //v4
   glTexCoord2f(0,0); glVertex3f(-0.2, 1.0, 0.4); //v3
   glTexCoord2f(1,0); glVertex3f(-0.4, 1.0, 0.2); //v2
   glTexCoord2f(1,1); glVertex3f(-0.4, 0.0, 0.2); //v1

   //Left Rectangle
   glNormal3f(-1.0, 0.0, 0.0); //Set normal of left side to be -x
   glTexCoord2f(0,1); glVertex3f(-0.4, 0.0, 0.2);
   glTexCoord2f(0,0); glVertex3f(-0.4, 1.0, 0.2);
   glTexCoord2f(1,0); glVertex3f(-0.4, 1.0, -0.2);
   glTexCoord2f(1,1); glVertex3f(-0.4, 0.0, -0.2);

   //Back Left Rectangle
   //Planar vector 1 (p1) = v3 - v2 = (-0.4, 1.0, -0.2) - (-0.2, 1.0, -0.4) = (-0.2, 0.0, 0.2)
   //Planar vector 2 (p2) = v2 - v1 = (-0.2, 1.0, -0.4) - (-0.2, 0.0, -0.4) = (0.0, 1.0, 0.0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0.0*0.0 - 0.2*1.0, 0.2*0.0 - (-0.2)*0.0, (-0.2)*1.0 - 0.0*0.0); //Calculate normal
   glTexCoord2f(0,1); glVertex3f(-0.4, 0.0, -0.2); //v4
   glTexCoord2f(0,0); glVertex3f(-0.4, 1.0, -0.2); //v3
   glTexCoord2f(1,0); glVertex3f(-0.2, 1.0, -0.4); //v2
   glTexCoord2f(1,1); glVertex3f(-0.2, 0.0, -0.4); //v1

   //Back Rectangle
   glNormal3f(0.0, 0.0, -1.0); //Set normal of back to be -z
   glTexCoord2f(0,1); glVertex3f(-0.2, 0.0, -0.4);
   glTexCoord2f(0,0); glVertex3f(-0.2, 1.0, -0.4);
   glTexCoord2f(1,0); glVertex3f(0.2, 1.0, -0.4);
   glTexCoord2f(1,1); glVertex3f(0.2, 0.0, -0.4);

   //Back Right Rectangle
   //Planar vector 1 (p1) = v3 - v2 = (0.2, 1.0, -0.4) - (0.4, 1.0, -0.2) = (-0.2, 0.0, -0.2)
   //Planar vector 2 (p2) = v2 - v1 = (0.4, 1.0, -0.2) - (0.4, 0.0, -0.2) = (0.0, 1.0, 0.0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0.0*0.0 - (-0.2)*1.0, (-0.2)*0.0 - (-0.2)*0.0, (-0.2)*1.0 - 0.0*0.0); //Calculate normal
   glTexCoord2f(0,1); glVertex3f(0.2, 0.0, -0.4); //v4
   glTexCoord2f(0,0); glVertex3f(0.2, 1.0, -0.4); //v3
   glTexCoord2f(1,0); glVertex3f(0.4, 1.0, -0.2); //v2
   glTexCoord2f(1,1); glVertex3f(0.4, 0.0, -0.2); //v1

   //Right Rectangle
   glNormal3f(+1.0, 0.0, 0.0); //Set normal of right side to be +x
   glTexCoord2f(0,1); glVertex3f(0.4, 0.0, -0.2);
   glTexCoord2f(0,0); glVertex3f(0.4, 1.0, -0.2);
   glTexCoord2f(1,0); glVertex3f(0.4, 1.0, 0.2);
   glTexCoord2f(1,1); glVertex3f(0.4, 0.0, 0.2);

   //Front Right Rectangle
   //Planar vector 1 (p1) = v3 - v2 = (0.4, 1.0, 0.2) - (0.2, 1.0, 0.4) = (0.2, 0.0, -0.2)
   //Planar vector 2 (p2) = v2 - v1 = (0.2, 1.0, 0.4) - (0.2, 0.0, 0.4) = (0.0, 1.0, 0.0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0.0*0.0 - (-0.2)*1.0, (-0.2)*0.0 - 0.2*0.0, 0.2*1.0 - 0.0*0.0); //Calculate normal
   glTexCoord2f(0,1); glVertex3f(0.4, 0.0, 0.2); //v4
   glTexCoord2f(0,0); glVertex3f(0.4, 1.0, 0.2); //v3
   glTexCoord2f(1,0); glVertex3f(0.2, 1.0, 0.4); //v2
   glTexCoord2f(1,1); glVertex3f(0.2, 0.0, 0.4); //v1

   //Front Rectangle
   glNormal3f(0.0, 0.0, +1.0); //Set normal of front to be +z
   glTexCoord2f(0,1); glVertex3f(0.2, 0.0, 0.4);
   glTexCoord2f(0,0); glVertex3f(0.2, 1.0, 0.4);
   glTexCoord2f(1,0); glVertex3f(-0.2, 1.0, 0.4);
   glTexCoord2f(1,1); glVertex3f(-0.2, 0.0, 0.4);
   glEnd();

   if (mode == 0) { //Wooden Rings
      glBindTexture(GL_TEXTURE_2D,texture[24]);
   } else if (mode == 1) { //Stop Sign
      glBindTexture(GL_TEXTURE_2D,texture[9]);
   } else if (mode == 2) { //Metal
      glBindTexture(GL_TEXTURE_2D,texture[8]);
   } else if (mode == 3) { //Wood Sign
      glBindTexture(GL_TEXTURE_2D,texture[5]);
   } else if (mode == 4) { //Roof Fan
      glBindTexture(GL_TEXTURE_2D,texture[32]);
   } else if (mode == 5 || mode == 11) { //White Metal
      glBindTexture(GL_TEXTURE_2D,texture[31]);
   } else if (mode == 12) { //Black Marble
      glBindTexture(GL_TEXTURE_2D,texture[80]);
   } else if (mode == 13) { //Car Wheel
      glBindTexture(GL_TEXTURE_2D,texture[89]);
   } else if (mode == 14 || mode == 15 || mode == 16) { //Back Car Lights
      glBindTexture(GL_TEXTURE_2D,texture[88]);
   } else if (mode == 17) { //Red
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 0.1, 0.1);
   } else if (mode == 18) { //Brown
      glDisable(GL_TEXTURE_2D);
      glColor3f(0.804, 0.522, 0.247);
   } 
   //Draw the top of the cylinder:
   glBegin(GL_POLYGON);
   glNormal3f(0.0, 1.0, 0.0); //Set normal of top to be +y
   glTexCoord2f(0.75,0); glVertex3f(0.2, 1.0, 0.4);
   glTexCoord2f(0.25,0); glVertex3f(-0.2, 1.0, 0.4);
   glTexCoord2f(0,0.25); glVertex3f(-0.4, 1.0, 0.2);
   glTexCoord2f(0,0.75); glVertex3f(-0.4, 1.0, -0.2);
   glTexCoord2f(0.25,1); glVertex3f(-0.2, 1.0, -0.4);
   glTexCoord2f(0.75,1); glVertex3f(0.2, 1.0, -0.4);
   glTexCoord2f(1,0.75); glVertex3f(0.4, 1.0, -0.2);
   glTexCoord2f(1,0.25); glVertex3f(0.4, 1.0, 0.2);
   glTexCoord2f(0.75,0); glVertex3f(0.2, 1.0, 0.4);
   glEnd();

   if (mode == 0) { //Wooden Rings
      glBindTexture(GL_TEXTURE_2D,texture[24]);
   } else if (mode == 1 || mode == 2) { //Metal
      glBindTexture(GL_TEXTURE_2D,texture[8]);
   } else if (mode == 3) { //Wood Sign
      glBindTexture(GL_TEXTURE_2D,texture[5]);
   } else if (mode == 11) { //White Metal
      glBindTexture(GL_TEXTURE_2D,texture[31]);
   } else if (mode == 12) { //Black Marble
      glBindTexture(GL_TEXTURE_2D,texture[80]);
   } else if (mode == 13) { //Car Wheel
      glBindTexture(GL_TEXTURE_2D,texture[89]);
   } else if (mode == 14 || mode == 15 || mode == 16) { //Front Car Lights
      glBindTexture(GL_TEXTURE_2D,texture[87]);
   } else if (mode == 17) { //Red
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 0.1, 0.1);
   } else if (mode == 18) { //Brown
      glDisable(GL_TEXTURE_2D);
      glColor3f(0.804, 0.522, 0.247);
   } 
   //Draw the bottom of the cylinder:
   glBegin(GL_POLYGON);
   glNormal3f(0.0,-1.0, 0.0); //Set normal of bottom to be -y
   glTexCoord2f(0.75,0); glVertex3f(0.2, 0.0, 0.4);
   glTexCoord2f(0.25,0); glVertex3f(-0.2, 0.0, 0.4);
   glTexCoord2f(0,0.25); glVertex3f(-0.4, 0.0, 0.2);
   glTexCoord2f(0,0.75); glVertex3f(-0.4, 0.0, -0.2);
   glTexCoord2f(0.25,1); glVertex3f(-0.2, 0.0, -0.4);
   glTexCoord2f(0.75,1); glVertex3f(0.2, 0.0, -0.4);
   glTexCoord2f(1,0.75); glVertex3f(0.4, 0.0, -0.2);
   glTexCoord2f(1,0.25); glVertex3f(0.4, 0.0, 0.2);
   glTexCoord2f(0.75,0); glVertex3f(0.2, 0.0, 0.4);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw a uniform triangular prism at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
//Mode: 0 = White Wood, 1/2/3 = Red/White/Yellow Stone, 4 = Marble
static void triangular_prism(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);

   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[15]); //White Wood
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[48]); //Red Stone
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[49]); //White Stone
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[50]); //Yellow Stone
   else if (mode == 4) glBindTexture(GL_TEXTURE_2D,texture[60]); //Marble

   //Draw the rectangular sides of the prism
   glBegin(GL_QUADS);
   //Top Left Side:
   //Planar vector 1 (p1) = v3 - v2 = (0, 1, -1) - (-1, 0, -1) = (1, 1, 0)
   //Planar vector 2 (p2) = v2 - v1 = (-1, 0, -1) - (-1, 0, 1) = (0, 0, -2)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   // glNormal3f((-1.0)*(-1.0) - 0.0*0.0, 0.0*0.0 - (-0.5)*(-1.0), (-0.5)*0.0 - (-1.0)*0.0); //Calculate normal
   glNormal3f(1*(-2) - 0*0, 0*0 - 1*(-2), 1*0 - 1*0); //Calculate normal
   glTexCoord2f(1,1); glVertex3f(0, 1, 1); //v4
   glTexCoord2f(0,1); glVertex3f(0, 1, -1); //v3
   glTexCoord2f(0,0); glVertex3f(-1, 0, -1); //v2
   glTexCoord2f(1,0); glVertex3f(-1, 0, 1); //v1

   //Top Right Side:
   //Planar vector 1 (p1) = v3 - v2 = (0, 1, -1) - (0, 1, 1) = (0, 0, -2)
   //Planar vector 2 (p2) = v2 - v1 = (0, 1, 1) - (1, 0, 1) = (-1, 1, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0*0 - (-2)*1, (-2)*(-1) - 0*0, 0*1 - 0*(-1)); //Calculate normal
   glTexCoord2f(1,0); glVertex3f(1, 0, -1); //v4
   glTexCoord2f(1,1); glVertex3f(0, 1, -1); //v3
   glTexCoord2f(0,1); glVertex3f(0, 1, 1); //v2
   glTexCoord2f(0,0); glVertex3f(1, 0, 1); //v1
   glEnd();

   //Bottom:
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[15]); //White Wood
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[48]); //Red Stone
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[49]); //White Stone
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[50]); //Yellow Stone
   else if (mode == 4) glBindTexture(GL_TEXTURE_2D,texture[60]); //Marble
   glBegin(GL_QUADS);
   glNormal3f(0.0,-1.0, 0.0); //Set normal of the bottom rectangle to be -y
   glTexCoord2f(0,0); glVertex3f(1, 0, -1);
   glTexCoord2f(1,0); glVertex3f(-1, 0, -1);
   glTexCoord2f(1,1); glVertex3f(-1, 0, 1);
   glTexCoord2f(0,1); glVertex3f(1, 0, 1);
   glEnd();

   //Draw the triangles
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[37]); //Triangle Window
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[48]); //Red Stone
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[49]); //White Stone
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[50]); //Yellow Stone
   else if (mode == 4) glBindTexture(GL_TEXTURE_2D,texture[62]); //Front/back of town hall
   glBegin(GL_TRIANGLES);
   //Back Triangle
   glNormal3f(0, 0, -2); //Set normal to be opposite of front triangle
   glTexCoord2f(0.5,1); glVertex3f(0, 1, -1); //v3
   glTexCoord2f(0,0); glVertex3f(-1, 0, -1); //v2
   glTexCoord2f(1,0); glVertex3f(1, 0, -1); //v1

   //Front Triangle
   //Planar vector 1 (p1) = v3 - v2 = (0, 1, 1) - (-1, 0, 1) = (1, 1, 0)
   //Planar vector 2 (p2) = v2 - v1 = (-1, 0, 1) - (1, 0, 1) = (-2, 0, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x) = (1*0 - 0*0, 0*(-2) - 1*0, 1*0 - 1*(-2)) = (0, 0, 2)
   glNormal3f(0, 0, 2); //Calculate normal
   glTexCoord2f(0.5,1); glVertex3f(0, 1, 1); //v3
   glTexCoord2f(0,0); glVertex3f(-1, 0, 1); //v2
   glTexCoord2f(1,0); glVertex3f(1, 0, 1); //v1
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//A multipurpose cube function for use with house/building features (doors, windows, chimney), fence connections, mailbox legs, building tops, and more
//The cube is placed at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
//Mode: 1/2 = White/Yellow Door, 3/4 = White/Yellow Window, 5/6 = White/Yellow House, 7 = Mailbox, 8/9 = White/Brown Fence, 10 = White Metal
//      11 = Concrete, 13/14/15 = Red/White/Orange Brick, 16 = Yellow Stone, 17 = Marble, 18/19 = SNES Switches, 20 = SNES Cartridge, 21 = White Tile
//      22 = Black Window, 23 = Square Window (burger shop), 24 = Police Car Door, 25 = Front of Car, 26 = Red Car Door, 27 = Red Car Body, 28 = Blue Car Door
//      29 = Blue Car Body, 30 = Ambulance Body, 31 = Solid Yellow, 32 = Grass, 33 = Road
static void cube(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   if (mode == 1) { //White Door
      glBindTexture(GL_TEXTURE_2D,texture[14]);
   } else if (mode == 2) { //Yellow Door
      glBindTexture(GL_TEXTURE_2D,texture[18]);
   } else if (mode == 3) { //White Window
      glBindTexture(GL_TEXTURE_2D,texture[13]);
   } else if (mode == 4) { //Yellow Window
      glBindTexture(GL_TEXTURE_2D,texture[17]);
   } else if (mode == 5) { //White House
      glBindTexture(GL_TEXTURE_2D,texture[15]);
   } else if (mode == 6) { //Yellow House
      glBindTexture(GL_TEXTURE_2D,texture[19]);
   } else if (mode == 7) { //Mailbox
      glBindTexture(GL_TEXTURE_2D,texture[23]);
   } else if (mode == 8) { //White Fence
      glBindTexture(GL_TEXTURE_2D,texture[2]);
   } else if (mode == 9) { //Brown Fence
      glBindTexture(GL_TEXTURE_2D,texture[3]);
   } else if (mode == 10 || mode == 25) { //White Metal
      glBindTexture(GL_TEXTURE_2D,texture[31]);
   } else if (mode == 11) { //Concrete
      glBindTexture(GL_TEXTURE_2D,texture[56]);
   } else if (mode == 13) { //Red Brick
      glBindTexture(GL_TEXTURE_2D,texture[34]);
   } else if (mode == 14) { //White Brick
      glBindTexture(GL_TEXTURE_2D,texture[36]);
   } else if (mode == 15) { //Orange Brick
      glBindTexture(GL_TEXTURE_2D,texture[35]);
   } else if (mode == 16) { //Yellow Stone
      glBindTexture(GL_TEXTURE_2D,texture[50]);
   } else if (mode == 17) { //Marble
      glBindTexture(GL_TEXTURE_2D,texture[60]);
   } else if (mode == 18 || mode == 19) { //Purple Switch
      glBindTexture(GL_TEXTURE_2D,texture[70]);
   } else if (mode == 20) { //Cartridge
      glBindTexture(GL_TEXTURE_2D,texture[73]);
   } else if (mode == 21) { //White Tile
      glBindTexture(GL_TEXTURE_2D,texture[67]);
   } else if (mode == 22) { //Black
      glBindTexture(GL_TEXTURE_2D,texture[63]);
   } else if (mode == 23) { //Square Window
      glBindTexture(GL_TEXTURE_2D,texture[79]);
   } else if (mode == 24) { //Police Car Door
      glBindTexture(GL_TEXTURE_2D,texture[86]);
   } else if (mode == 26) { //Red Car Door
      glBindTexture(GL_TEXTURE_2D,texture[82]);
   } else if (mode == 27) { //Red Metal
      glBindTexture(GL_TEXTURE_2D,texture[81]);
   } else if (mode == 28) { //Blue Car Door
      glBindTexture(GL_TEXTURE_2D,texture[84]);
   } else if (mode == 29) { //Blue Metal
      glBindTexture(GL_TEXTURE_2D,texture[83]);
   } else if (mode == 30) { //Ambulance
      glBindTexture(GL_TEXTURE_2D,texture[98]);
   } else if (mode == 31) { //Yellow
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 1, 0);
   } else if (mode == 32) { //Grass
      glBindTexture(GL_TEXTURE_2D,texture[116]);
   } else if (mode == 33) { //Road
      glBindTexture(GL_TEXTURE_2D,texture[117]);
   }
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
   glTexCoord2f(1,0); glVertex3f(+1,-1, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   glEnd();

   if (mode == 1) { //White Door
      glBindTexture(GL_TEXTURE_2D,texture[14]);
   } else if (mode == 2) { //Yellow Door
      glBindTexture(GL_TEXTURE_2D,texture[18]);
   } else if (mode == 3) { //White Window
      glBindTexture(GL_TEXTURE_2D,texture[13]);
   } else if (mode == 4) { //Yellow Window
      glBindTexture(GL_TEXTURE_2D,texture[17]);
   } else if (mode == 5) { //White House
      glBindTexture(GL_TEXTURE_2D,texture[15]);
   } else if (mode == 6) { //Yellow House
      glBindTexture(GL_TEXTURE_2D,texture[19]);
   } else if (mode == 7) { //Mailbox
      glBindTexture(GL_TEXTURE_2D,texture[23]);
   } else if (mode == 8) { //White Fence
      glBindTexture(GL_TEXTURE_2D,texture[2]);
   } else if (mode == 9) { //Brown Fence
      glBindTexture(GL_TEXTURE_2D,texture[3]);
   } else if (mode == 10 || mode == 25) { //White Metal
      glBindTexture(GL_TEXTURE_2D,texture[31]);
   } else if (mode == 11) { //Concrete
      glBindTexture(GL_TEXTURE_2D,texture[56]);
   } else if (mode == 13) { //Red Brick
      glBindTexture(GL_TEXTURE_2D,texture[34]);
   } else if (mode == 14) { //White Brick
      glBindTexture(GL_TEXTURE_2D,texture[36]);
   } else if (mode == 15) { //Orange Brick
      glBindTexture(GL_TEXTURE_2D,texture[35]);
   } else if (mode == 16) { //Yellow Stone
      glBindTexture(GL_TEXTURE_2D,texture[50]);
   } else if (mode == 17) { //Marble
      glBindTexture(GL_TEXTURE_2D,texture[60]);
   } else if (mode == 18 || mode == 19) { //Purple Switch
      glBindTexture(GL_TEXTURE_2D,texture[70]);
   } else if (mode == 20) { //Back of Cartridge
      glBindTexture(GL_TEXTURE_2D,texture[74]);
   } else if (mode == 21 || mode == 23) { //White Tile
      glBindTexture(GL_TEXTURE_2D,texture[67]);
   } else if (mode == 22) { //Black
      glBindTexture(GL_TEXTURE_2D,texture[63]);
   } else if (mode == 24) { //Police Car Door
      glBindTexture(GL_TEXTURE_2D,texture[86]);
   } else if (mode == 26) { //Red Car Door
      glBindTexture(GL_TEXTURE_2D,texture[82]);
   } else if (mode == 27) { //Red Metal
      glBindTexture(GL_TEXTURE_2D,texture[81]);
   } else if (mode == 28) { //Blue Car Door
      glBindTexture(GL_TEXTURE_2D,texture[84]);
   } else if (mode == 29) { //Blue Metal
      glBindTexture(GL_TEXTURE_2D,texture[83]);
   } else if (mode == 30) { //Ambulance
      glBindTexture(GL_TEXTURE_2D,texture[105]);
   } else if (mode == 31) { //Yellow
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 1, 0);
   } else if (mode == 32) { //Grass
      glBindTexture(GL_TEXTURE_2D,texture[116]);
   } else if (mode == 33) { //Road
      glBindTexture(GL_TEXTURE_2D,texture[117]);
   }
   glBegin(GL_QUADS);
   //  Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   glEnd();

   if (mode == 1 || mode == 3 || mode == 5 || mode == 22) { //White House
      glBindTexture(GL_TEXTURE_2D,texture[15]);
   } else if (mode == 2 || mode == 4 || mode == 6) { //Yellow House
      glBindTexture(GL_TEXTURE_2D,texture[19]);
   } else if (mode == 7) { //Mailbox
      glBindTexture(GL_TEXTURE_2D,texture[23]);
   } else if (mode == 8) { //White Fence
      glBindTexture(GL_TEXTURE_2D,texture[2]);
   } else if (mode == 9) { //Brown Fence
      glBindTexture(GL_TEXTURE_2D,texture[3]);
   } else if (mode == 10 || mode == 25) { //White Metal
      glBindTexture(GL_TEXTURE_2D,texture[31]);
   } else if (mode == 11) { //Concrete
      glBindTexture(GL_TEXTURE_2D,texture[56]);
   } else if (mode == 13) { //Red Brick
      glBindTexture(GL_TEXTURE_2D,texture[34]);
   } else if (mode == 14) { //White Brick
      glBindTexture(GL_TEXTURE_2D,texture[36]);
   } else if (mode == 15) { //Orange Brick
      glBindTexture(GL_TEXTURE_2D,texture[35]);
   } else if (mode == 16) { //Yellow Stone
      glBindTexture(GL_TEXTURE_2D,texture[50]);
   } else if (mode == 17) { //Marble
      glBindTexture(GL_TEXTURE_2D,texture[60]);
   } else if (mode == 18 || mode == 19) { //Purple Switch
      glBindTexture(GL_TEXTURE_2D,texture[70]);
   } else if (mode == 20) { //Sides of Cartridge
      glBindTexture(GL_TEXTURE_2D,texture[75]);
   } else if (mode == 21 || mode == 23) { //White Tile
      glBindTexture(GL_TEXTURE_2D,texture[67]);
   } else if (mode == 24) { //Black Marble
      glBindTexture(GL_TEXTURE_2D,texture[80]);
   } else if (mode == 26 || mode == 27) { //Red Metal
      glBindTexture(GL_TEXTURE_2D,texture[81]);
   } else if (mode == 28 || mode == 29) { //Blue Car Door
      glBindTexture(GL_TEXTURE_2D,texture[84]);
   } else if (mode == 30) { //Ambulance
      glBindTexture(GL_TEXTURE_2D,texture[104]);
   } else if (mode == 31) { //Yellow
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 1, 0);
   } else if (mode == 32) { //Grass
      glBindTexture(GL_TEXTURE_2D,texture[116]);
   } else if (mode == 33) { //Road
      glBindTexture(GL_TEXTURE_2D,texture[117]);
   }
   glBegin(GL_QUADS);
   //  Right
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,+1);
   glEnd();

   if (mode == 1 || mode == 3 || mode == 5 || mode == 22) { //White House
      glBindTexture(GL_TEXTURE_2D,texture[15]);
   } else if (mode == 2 || mode == 4 || mode == 6) { //Yellow House
      glBindTexture(GL_TEXTURE_2D,texture[19]);
   } else if (mode == 7) { //Mailbox
      glBindTexture(GL_TEXTURE_2D,texture[23]);
   } else if (mode == 8) { //White Fence
      glBindTexture(GL_TEXTURE_2D,texture[2]);
   } else if (mode == 9) { //Brown Fence
      glBindTexture(GL_TEXTURE_2D,texture[3]);
   } else if (mode == 10) { //White Metal
      glBindTexture(GL_TEXTURE_2D,texture[31]);
   } else if (mode == 11) { //Concrete
      glBindTexture(GL_TEXTURE_2D,texture[56]);
   } else if (mode == 13) { //Red Brick
      glBindTexture(GL_TEXTURE_2D,texture[34]);
   } else if (mode == 14) { //White Brick
      glBindTexture(GL_TEXTURE_2D,texture[36]);
   } else if (mode == 15) { //Orange Brick
      glBindTexture(GL_TEXTURE_2D,texture[35]);
   } else if (mode == 16) { //Yellow Stone
      glBindTexture(GL_TEXTURE_2D,texture[50]);
   } else if (mode == 17) { //Marble
      glBindTexture(GL_TEXTURE_2D,texture[60]);
   } else if (mode == 18 || mode == 19) { //Purple Switch
      glBindTexture(GL_TEXTURE_2D,texture[70]);
   } else if (mode == 20) { //Sides of Cartridge
      glBindTexture(GL_TEXTURE_2D,texture[75]);
   } else if (mode == 21 || mode == 23) { //White Tile
      glBindTexture(GL_TEXTURE_2D,texture[67]);
   } else if (mode == 24 || mode == 25 || mode == 27 || mode == 29 || mode == 30) { //Black Marble
      glBindTexture(GL_TEXTURE_2D,texture[80]);
   } else if (mode == 26) { //Red Metal
      glBindTexture(GL_TEXTURE_2D,texture[81]);
   } else if (mode == 28) { //Blue Metal
      glBindTexture(GL_TEXTURE_2D,texture[83]);
   } else if (mode == 31) { //Yellow
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 1, 0);
   } else if (mode == 32 || mode == 33) { //Black
      glBindTexture(GL_TEXTURE_2D,texture[63]);
   }
   glBegin(GL_QUADS);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
   glTexCoord2f(1,1); glVertex3f(+1,-1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
   glEnd();

   if (mode == 1 || mode == 3 || mode == 5 || mode == 22) { //White House
      glBindTexture(GL_TEXTURE_2D,texture[15]);
   } else if (mode == 2 || mode == 4 || mode == 6) { //Yellow House
      glBindTexture(GL_TEXTURE_2D,texture[19]);
   } else if (mode == 7) { //Mailbox
      glBindTexture(GL_TEXTURE_2D,texture[23]);
   } else if (mode == 8) { //White Fence
      glBindTexture(GL_TEXTURE_2D,texture[2]);
   } else if (mode == 9) { //Brown Fence
      glBindTexture(GL_TEXTURE_2D,texture[3]);
   } else if (mode == 11) { //Concrete
      glBindTexture(GL_TEXTURE_2D,texture[56]);
   } else if (mode == 13) { //Red Brick
      glBindTexture(GL_TEXTURE_2D,texture[34]);
   } else if (mode == 14) { //White Brick
      glBindTexture(GL_TEXTURE_2D,texture[36]);
   } else if (mode == 15) { //Orange Brick
      glBindTexture(GL_TEXTURE_2D,texture[35]);
   } else if (mode == 16) { //Yellow Stone
      glBindTexture(GL_TEXTURE_2D,texture[50]);
   } else if (mode == 17) { //Marble
      glBindTexture(GL_TEXTURE_2D,texture[60]);
   } else if (mode == 18 || mode == 19) { //Purple Switch
      glBindTexture(GL_TEXTURE_2D,texture[70]);
   } else if (mode == 20) { //Sides of Cartridge
      glBindTexture(GL_TEXTURE_2D,texture[75]);
   } else if (mode == 21 || mode == 23) { //White Tile
      glBindTexture(GL_TEXTURE_2D,texture[67]);
   } else if (mode == 24) { //Black Marble
      glBindTexture(GL_TEXTURE_2D,texture[80]);
   } else if (mode == 25 || mode == 27 || mode == 29) { //Grate
      glBindTexture(GL_TEXTURE_2D,texture[85]);
   } else if (mode == 26) { //Red Metal
      glBindTexture(GL_TEXTURE_2D,texture[81]);
   } else if (mode == 28) { //Blue Metal
      glBindTexture(GL_TEXTURE_2D,texture[83]);
   } else if (mode == 30) { //Ambulance Back
      glBindTexture(GL_TEXTURE_2D,texture[99]);
   } else if (mode == 31) { //Yellow
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 1, 0);
   } else if (mode == 32) { //Grass
      glBindTexture(GL_TEXTURE_2D,texture[116]);
   } else if (mode == 33) { //Road
      glBindTexture(GL_TEXTURE_2D,texture[117]);
   }
   glBegin(GL_QUADS);
   //  Left
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
   glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();

   if (mode == 1 || mode == 3 || mode == 5 || mode == 22) { //White House
      glBindTexture(GL_TEXTURE_2D,texture[15]);
   } else if (mode == 2 || mode == 4 || mode == 6) { //Yellow House
      glBindTexture(GL_TEXTURE_2D,texture[19]);
   } else if (mode == 7) { //Mailbox
      glBindTexture(GL_TEXTURE_2D,texture[23]);
   } else if (mode == 8) { //White Fence
      glBindTexture(GL_TEXTURE_2D,texture[2]);
   } else if (mode == 9) { //Brown Fence
      glBindTexture(GL_TEXTURE_2D,texture[3]);
   } else if (mode == 10 || mode == 25 || mode == 30) { //White Metal
      glBindTexture(GL_TEXTURE_2D,texture[31]);
   } else if (mode == 11) { //Concrete
      glBindTexture(GL_TEXTURE_2D,texture[56]);
   } else if (mode == 13 || mode == 14 || mode == 15) { //Soil (for flowers)
      glBindTexture(GL_TEXTURE_2D,texture[58]);
   } else if (mode == 16) { //Yellow Stone
      glBindTexture(GL_TEXTURE_2D,texture[50]);
   } else if (mode == 17) { //Marble
      glBindTexture(GL_TEXTURE_2D,texture[60]);
   } else if (mode == 18) { ////SNES Power Switch
      glBindTexture(GL_TEXTURE_2D,texture[71]);
   } else if (mode == 19) { //SNES Reset Switch
      glBindTexture(GL_TEXTURE_2D,texture[72]);
   } else if (mode == 20) { //Sides of Cartridge
      glBindTexture(GL_TEXTURE_2D,texture[75]);
   } else if (mode == 21 || mode == 23) { //White Tile
      glBindTexture(GL_TEXTURE_2D,texture[67]);
   } else if (mode == 24) { //Black Marble
      glBindTexture(GL_TEXTURE_2D,texture[80]);
   } else if (mode == 26 || mode == 27) { //Red Metal
      glBindTexture(GL_TEXTURE_2D,texture[81]);
   } else if (mode == 28 || mode == 29) { //Blue Metal
      glBindTexture(GL_TEXTURE_2D,texture[83]);
   } else if (mode == 31) { //Yellow
      glDisable(GL_TEXTURE_2D);
      glColor3f(1, 1, 0);
   } else if (mode == 32) { //Grass
      glBindTexture(GL_TEXTURE_2D,texture[116]);
   } else if (mode == 33) { //Road
      glBindTexture(GL_TEXTURE_2D,texture[117]);
   }
   //  Top
   glBegin(GL_QUADS);
   glNormal3f(0, 1, 0); //Set the normal to +y
   glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
   glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
   glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
   glEnd();
   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw a textured hemisphere at (x,y,z) with dimensions (dx,dy,dz) and rotation around the y-axis
//Got help with making a textured sphere from here: https://users.cs.jmu.edu/bernstdh/web/common/lectures/summary_opengl-texture-mapping.php
//Got help with glClipPlane here: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glClipPlane.xml
//Mode: 0 = Black Marble (street lamp), 1 = Burger Bun
static void hollowHemiSphere(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   if (mode == 0) glBindTexture(GL_TEXTURE_2D, texture[80]); //Black Marble
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D, texture[113]); //Burger Bun
   
   //Activate a clip plane to cut the sphere in half
   double equation[] = {0, 1, 0, 0};
   glClipPlane(GL_CLIP_PLANE0, equation);
   glEnable(GL_CLIP_PLANE0);

   //Draw a sphere
   GLUquadric *quadric = gluNewQuadric();
   gluQuadricDrawStyle(quadric, GLU_FILL);
   gluQuadricNormals(quadric, GLU_SMOOTH);
   gluQuadricTexture(quadric, GL_TRUE);
   gluSphere(quadric, 1.0, 20, 20);
   gluDeleteQuadric(quadric);

   //  Undo transformations, textures, and clip plane
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_CLIP_PLANE0);
}

// Draw the sky box (modified from Example 25 on Canvas to account for 6 textures)
// Got the skybox texture from: https://www.humus.name/index.php?page=Textures&start=88
static void skyBox(double D)
{
   glColor3f(1,1,1);
   glEnable(GL_TEXTURE_2D);

   //  Sides
   glBindTexture(GL_TEXTURE_2D,texture[107]); //Back (negative z)
   glBegin(GL_QUADS);
   glTexCoord2f(0,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(1,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(1,1); glVertex3f(-D,+D,-D);
   glTexCoord2f(0,1); glVertex3f(+D,+D,-D);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[108]); //Right (positive x)
   glBegin(GL_QUADS);
   glTexCoord2f(1,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(0,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0,1); glVertex3f(+D,+D,+D);
   glTexCoord2f(1,1); glVertex3f(+D,+D,-D);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[106]); //Front (positive z)
   glBegin(GL_QUADS);
   glTexCoord2f(1,0); glVertex3f(+D,-D,+D);
   glTexCoord2f(0,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(0,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(1,1); glVertex3f(+D,+D,+D);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[109]); //Left (negative x)
   glBegin(GL_QUADS);
   glTexCoord2f(0,0); glVertex3f(-D,-D,-D);
   glTexCoord2f(1,0); glVertex3f(-D,-D,+D);
   glTexCoord2f(1,1); glVertex3f(-D,+D,+D);
   glTexCoord2f(0,1); glVertex3f(-D,+D,-D);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[110]); //Top (positive y)
   glBegin(GL_QUADS);
   glTexCoord2f(1,0); glVertex3f(+D,+D,+D);
   glTexCoord2f(1,1); glVertex3f(+D,+D,-D);
   glTexCoord2f(0,1); glVertex3f(-D,+D,-D);
   glTexCoord2f(0,0); glVertex3f(-D,+D,+D);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[111]); //Bottom (negative y)
   glBegin(GL_QUADS);
   glTexCoord2f(1,1); glVertex3f(-D,-D,+D);
   glTexCoord2f(0,1); glVertex3f(+D,-D,+D);
   glTexCoord2f(0,0); glVertex3f(+D,-D,-D);
   glTexCoord2f(1,0); glVertex3f(-D,-D,-D);
   glEnd();

   glDisable(GL_TEXTURE_2D);
}
//********************************************************************************************************************************************************************************



// Building Block Functions: Functions that draw base objects that can be combined into more complicated objects
//********************************************************************************************************************************************************************************
//Function to draw quad with partially transparent textures at (x,y,z), with dimensions (dx,dy,dz), rotation (th) about the y axis, and rotation (ph) about the x axis
//Mode: 0 = Weathervane, 1 = Lettuce (for burger)
static void transparentObject(double x,double y,double z, double dx,double dy,double dz, double th, double ph, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glRotated(ph,1,0,0);
   glScaled(dx,dy,dz);
   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[4]); //Weathervane
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[112]); //Lettuce
   
   //  Enable blending
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
   glTexCoord2f(1,0); glVertex3f(+1,-1, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   glEnd();

   //  Undo transformations, textures, and blending
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
}

//Function to draw quad with transparent flowers and grass textures at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
//Mode: 0 = Grass, 1 = Red Flowers, 2 = Blue Flowers, 3 = Green Flowers, 4 = Shrub, 5 = Bush
static void flowersGrass(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[46]); //Grass
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[42]); //Red flowers
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[43]); //Blue flowers
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[44]); //Green flowers
   else if (mode == 4) glBindTexture(GL_TEXTURE_2D,texture[114]); //Shrub
   else if (mode == 5) glBindTexture(GL_TEXTURE_2D,texture[115]); //Bush
   
   //  Enable blending
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-1,-1, 1);
   glTexCoord2f(1,0); glVertex3f(+1,-1, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
}

//Function to draw a singular fence at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
//Mode: 0 = White Fence, 1 = Brown Fence
static void fence(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[2]); // White Fence
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[3]); // Brown Fence

   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-1, 0.0, 1);
   glTexCoord2f(1,0); glVertex3f(+1, 0.0, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   //  Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,0); glVertex3f(+1, 0.0,-1);
   glTexCoord2f(1,0); glVertex3f(-1, 0.0,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-1,0.0,-1);
   glTexCoord2f(1,0); glVertex3f(+1,0.0,-1);
   glTexCoord2f(1,1); glVertex3f(+1,0.0,+1);
   glTexCoord2f(0,1); glVertex3f(-1,0.0,+1);
   glEnd();

   //  Right
   glBegin(GL_POLYGON);
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(1, 0); glVertex3f(+1,0.0,-1);
   glTexCoord2f(1, 0.5); glVertex3f(+1,+1,-1);
   glTexCoord2f(0.5, 1); glVertex3f(+1,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(+1,1,+1);
   glTexCoord2f(0,0); glVertex3f(+1,0.0,+1);
   glEnd();

   //  Left
   glBegin(GL_POLYGON);
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(1, 0); glVertex3f(-1,0.0,+1);
   glTexCoord2f(1, 0.5); glVertex3f(-1,+1,+1);
   glTexCoord2f(0.5, 1); glVertex3f(-1,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,0); glVertex3f(-1,0.0,-1);
   glEnd();


   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[2]); // White Fence
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[3]); // Brown Fence
   //Draw the rectangular sides of the prism
   glBegin(GL_QUADS);
   //Top Back Side:
   //Planar vector 1 (p1) = v3 - v2 = (1, 2, 0) - (1, 1, -1) = (0, 1, 1)
   //Planar vector 2 (p2) = v2 - v1 = (1, 1, -1) - (-1, 1, -1) = (2, 0, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0, 2, -2); //Calculate normal
   glTexCoord2f(1,1); glVertex3f(-1, 2, 0); //v4
   glTexCoord2f(0,1); glVertex3f(1, 2, 0); //v3
   glTexCoord2f(0,0); glVertex3f(1, 1, -1); //v2
   glTexCoord2f(1,0); glVertex3f(-1, 1, -1); //v1

   //Top Front Side:
   //Planar vector 1 (p1) = v3 - v2 = (-1, 2, 0) - (-1, 1, 1) = (0, 1, -1)
   //Planar vector 2 (p2) = v2 - v1 = (-1, 1, 1) - (1, 1, 1) = (-2, 0, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0, 2, 2); //Calculate normal
   glTexCoord2f(0,0); glVertex3f(1, 2, 0); //v4
   glTexCoord2f(1,0); glVertex3f(-1, 2, 0); //v3
   glTexCoord2f(1,1); glVertex3f(-1, 1, 1); //v2
   glTexCoord2f(0,1); glVertex3f(1, 1, 1); //v1
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw the base of a house at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
//Mode: 0 = White House w/ Purple Roof, 1 = Yellow House, 2 = White House w/ Blue Roof
static void basicHouse(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   if (mode == 0 || mode == 2) { //White House
      //Windows
      cube(0.75,0.5,1, 0.1,0.3,0.05, 0, 3); // Front Right Window
      cube(-0.75,0.5,1, 0.1,0.3,0.05, 0, 3); // Front Left Window
      cube(0.75,0.5,-1, 0.1,0.3,0.05, 0, 3); // Back Right Window
      cube(-0.75,0.5,-1, 0.1,0.3,0.05, 0, 3); // Back Left Window

      cube(0.98,0.5,-0.5, 0.3,0.2,0.05, 90, 3); // Right Side Right Window
      cube(0.98,0.5,0.5, 0.3,0.2,0.05, 90, 3); // Right Side Left Window
      cube(-0.98,0.5,-0.5, 0.3,0.2,0.05, 90, 3); // Left Side Right Window
      cube(-0.98,0.5,0.5, 0.3,0.2,0.05, 90, 3); // Left Side Left Window
   } else if (mode == 1) { //Yellow House
      //Windows
      cube(0.75,0.5,1, 0.1,0.3,0.05, 0, 4); // Front Right Window
      cube(-0.75,0.5,1, 0.1,0.3,0.05, 0, 4); // Front Left Window
      cube(0.75,0.5,-1, 0.1,0.3,0.05, 0, 4); // Back Right Window
      cube(-0.75,0.5,-1, 0.1,0.3,0.05, 0, 4); // Back Left Window

      cube(0.98,0.5,-0.5, 0.3,0.2,0.05, 90, 4); // Right Side Right Window
      cube(0.98,0.5,0.5, 0.3,0.2,0.05, 90, 4); // Right Side Left Window
      cube(-0.98,0.5,-0.5, 0.3,0.2,0.05, 90, 4); // Left Side Right Window
      cube(-0.98,0.5,0.5, 0.3,0.2,0.05, 90, 4); // Left Side Left Window
   }

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   if (mode == 0 || mode == 2) glBindTexture(GL_TEXTURE_2D,texture[15]); //White House
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[19]); //Yellow House
   
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-1, 0.0, 1);
   glTexCoord2f(1,0); glVertex3f(+1, 0.0, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   //  Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,0); glVertex3f(+1, 0.0,-1);
   glTexCoord2f(1,0); glVertex3f(-1, 0.0,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-1,0.0,-1);
   glTexCoord2f(1,0); glVertex3f(+1,0.0,-1);
   glTexCoord2f(1,1); glVertex3f(+1,0.0,+1);
   glTexCoord2f(0,1); glVertex3f(-1,0.0,+1);
   glEnd();

   //  Right Side of House
   glBegin(GL_POLYGON);
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(1, 0); glVertex3f(+1,0.0,-1);
   glTexCoord2f(1, 0.5); glVertex3f(+1,+1,-1);
   glTexCoord2f(0.5, 1); glVertex3f(+1,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(+1,1,+1);
   glTexCoord2f(0,0); glVertex3f(+1,0.0,+1);
   glEnd();

   //  Left Side of House
   glBegin(GL_POLYGON);
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(1, 0); glVertex3f(-1,0.0,+1);
   glTexCoord2f(1, 0.5); glVertex3f(-1,+1,+1);
   glTexCoord2f(0.5, 1); glVertex3f(-1,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,0); glVertex3f(-1,0.0,-1);
   glEnd();


   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[12]); //Purple Roof
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[16]); //Red Roof
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[26]); //Blue Roof

   //Draw the rectangular sides of the prism
   glBegin(GL_QUADS);
   //Top Back Side:
   //Planar vector 1 (p1) = v3 - v2 = (1, 2, 0) - (1, 1, -1) = (0, 1, 1)
   //Planar vector 2 (p2) = v2 - v1 = (1, 1, -1) - (-1, 1, -1) = (2, 0, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0, 2, -2); //Calculate normal
   glTexCoord2f(1,1); glVertex3f(-1, 2, 0); //v4
   glTexCoord2f(0,1); glVertex3f(1, 2, 0); //v3
   glTexCoord2f(0,0); glVertex3f(1, 1, -1); //v2
   glTexCoord2f(1,0); glVertex3f(-1, 1, -1); //v1

   //Top Front Side:
   //Planar vector 1 (p1) = v3 - v2 = (-1, 2, 0) - (-1, 1, 1) = (0, 1, -1)
   //Planar vector 2 (p2) = v2 - v1 = (-1, 1, 1) - (1, 1, 1) = (-2, 0, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0, 2, 2); //Calculate normal
   glTexCoord2f(0,0); glVertex3f(1, 2, 0); //v4
   glTexCoord2f(1,0); glVertex3f(-1, 2, 0); //v3
   glTexCoord2f(1,1); glVertex3f(-1, 1, 1); //v2
   glTexCoord2f(0,1); glVertex3f(1, 1, 1); //v1
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw the base of a sign at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
//Mode: 0 = Big Sign, 1 = Small Sign, 2 = Very Big Sign
static void signBase(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   if (mode == 0) { //Big Sign
      glBindTexture(GL_TEXTURE_2D,texture[7]);
   } else if (mode == 1) { //Small Sign
      glBindTexture(GL_TEXTURE_2D,texture[6]);
   } else if (mode == 2) { //Very Big Sign
      glBindTexture(GL_TEXTURE_2D,texture[20]);
   }
   glBegin(GL_QUADS);
   // Actual Sign Portion
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-0.8,0.4, 0.11);
   glTexCoord2f(1,1); glVertex3f(0.8,0.4, 0.11);
   glTexCoord2f(1,0); glVertex3f(0.8,-0.4, 0.11);
   glTexCoord2f(0,0); glVertex3f(-0.8,-0.4,0.11);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[5]); //Wood texture
   glBegin(GL_QUADS);
   //  Right
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(0,0); glVertex3f(1,-0.4, -0.1);
   glTexCoord2f(1,0); glVertex3f(1,0.4, -0.1);
   glTexCoord2f(1,1); glVertex3f(1,0.4, 0.1);
   glTexCoord2f(0,1); glVertex3f(1,-0.4, 0.1);
   //  Left
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(0,0); glVertex3f(-1, -0.4,-0.1);
   glTexCoord2f(1,0); glVertex3f(-1, 0.4,-0.1);
   glTexCoord2f(1,1); glVertex3f(-1,0.4,0.1);
   glTexCoord2f(0,1); glVertex3f(-1,-0.4,0.1);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-0.8,-0.6,-0.1);
   glTexCoord2f(1,0); glVertex3f(0.8,-0.6,-0.1);
   glTexCoord2f(1,1); glVertex3f(0.8,-0.6,0.1);
   glTexCoord2f(0,1); glVertex3f(-0.8,-0.6,0.1);

   // Bottom Right
   //Planar vector 1 (p1) = v3 - v2 = (0.8,-0.6,-0.1) - (0.8,-0.6,0.1) = (0, 0, -0.2)
   //Planar vector 2 (p2) = v2 - v1 = (0.8,-0.6,0.1) - (1,-0.4,0.1) = (-0.2, -0.2, 0)
   glNormal3f(0.04, -0.04, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(1,-0.4,-0.1);
   glTexCoord2f(1,0); glVertex3f(0.8,-0.6,-0.1);
   glTexCoord2f(1,1); glVertex3f(0.8,-0.6,0.1);
   glTexCoord2f(0,1); glVertex3f(1,-0.4,0.1);

   // Bottom Left
   glNormal3f(-0.04, -0.04, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(-1,-0.4,-0.1);
   glTexCoord2f(1,0); glVertex3f(-0.8,-0.6,-0.1);
   glTexCoord2f(1,1); glVertex3f(-0.8,-0.6,0.1);
   glTexCoord2f(0,1); glVertex3f(-1,-0.4,0.1);

   //  Top
   glNormal3f(0, +1, 0); //Set the normal to +y
   glTexCoord2f(0,0); glVertex3f(-0.8,0.6,-0.1);
   glTexCoord2f(1,0); glVertex3f(0.8,0.6,-0.1);
   glTexCoord2f(1,1); glVertex3f(0.8,0.6,0.1);
   glTexCoord2f(0,1); glVertex3f(-0.8,0.6,0.1);

   // Top Right
   glNormal3f(0.04, 0.04, 0); //Set the normal to +y
   glTexCoord2f(0,0); glVertex3f(1,0.4,-0.1);
   glTexCoord2f(1,0); glVertex3f(0.8,0.6,-0.1);
   glTexCoord2f(1,1); glVertex3f(0.8,0.6,0.1);
   glTexCoord2f(0,1); glVertex3f(1,0.4,0.1);
   // Top Left
   glNormal3f(-0.04, 0.04, 0); //Set the normal to +y
   glTexCoord2f(0,0); glVertex3f(-1,0.4,-0.1);
   glTexCoord2f(1,0); glVertex3f(-0.8,0.6,-0.1);
   glTexCoord2f(1,1); glVertex3f(-0.8,0.6,0.1);
   glTexCoord2f(0,1); glVertex3f(-1,0.4,0.1);
   glEnd();

   // Front
   glBegin(GL_POLYGON);
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0, 1.6); glVertex3f(-1,-0.4,0.1);
   glTexCoord2f(0, 0.8); glVertex3f(-1,0.4,0.1);
   glTexCoord2f(0.1,1); glVertex3f(-0.8,0.6,0.1);
   glTexCoord2f(0.9, 1); glVertex3f(0.8,0.6,0.1);
   glTexCoord2f(1, 0.8); glVertex3f(+1,0.4,0.1);
   glTexCoord2f(1, 0.16); glVertex3f(1,-0.4,0.1);
   glTexCoord2f(0.9, 0); glVertex3f(0.8,-0.6,0.1);
   glTexCoord2f(0.1,0); glVertex3f(-0.8,-0.6,0.1);
   glEnd();

   //Back
   glBegin(GL_POLYGON);
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0, 1.6); glVertex3f(-1,-0.4,-0.1);
   glTexCoord2f(0, 0.8); glVertex3f(-1,0.4,-0.1);
   glTexCoord2f(0.1,1); glVertex3f(-0.8,0.6,-0.1);
   glTexCoord2f(0.9, 1); glVertex3f(0.8,0.6,-0.1);
   glTexCoord2f(1, 0.8); glVertex3f(+1,0.4,-0.1);
   glTexCoord2f(1, 0.16); glVertex3f(1,-0.4,-0.1);
   glTexCoord2f(0.9, 0); glVertex3f(0.8,-0.6,-0.1);
   glTexCoord2f(0.1,0); glVertex3f(-0.8,-0.6,-0.1);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a pizza (for the pizza shop), allowing it to be scaled, translated, and rotated around the y-axis
static void pizzaSign(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,0,1);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   glBindTexture(GL_TEXTURE_2D,texture[30]); //Pizza Crust
   glBegin(GL_QUADS);
   //  Right
   //Planar vector 1 (p1) = v3 - v2 = (0,1, -0.3) - (0,1, 0.3) = (0, 0, -0.6)
   //Planar vector 2 (p2) = v2 - v1 = (0,1, 0.3) - (0.4,0.2, 0.3) = (-0.4, 0.8, 0)
   glNormal3f(0.48, 0.24, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(0.4,0.2, -0.3);
   glTexCoord2f(1,0); glVertex3f(0,1, -0.3);
   glTexCoord2f(1,1); glVertex3f(0,1, 0.3);
   glTexCoord2f(0,1); glVertex3f(0.4,0.2, 0.3);
   //  Left
   glNormal3f(-0.48, 0.24, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(-0.4,0.2, -0.3);
   glTexCoord2f(1,0); glVertex3f(0,1, -0.3);
   glTexCoord2f(1,1); glVertex3f(0,1, 0.3);
   glTexCoord2f(0,1); glVertex3f(-0.4,0.2, 0.3);

   //  Bottom Right
   //Planar vector 1 (p1) = v3 - v2 = (0.4,0.2,-0.3) - (0.4,0.2,0.3) = (0, 0, -0.6)
   //Planar vector 2 (p2) = v2 - v1 = (0.4,0.2,0.3) - (0.3,0.1,0.3) = (0.1, 0.1, 0)
   glNormal3f(0.06, -0.06, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(0.3,0.1,-0.3);
   glTexCoord2f(1,0); glVertex3f(0.4,0.2,-0.3);
   glTexCoord2f(1,1); glVertex3f(0.4,0.2,0.3);
   glTexCoord2f(0,1); glVertex3f(0.3,0.1,0.3);

   //  Very Bottom Right
   //Planar vector 1 (p1) = v3 - v2 = (0.3,0.1,-0.3) - (0.3,0.1,0.3) = (0, 0, -0.6)
   //Planar vector 2 (p2) = v2 - v1 = (0.3,0.1,0.3) - (0.1,0,0.3) = (0.2, 0.1, 0)
   glNormal3f(0.06, -0.12, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(0.1,0,-0.3);
   glTexCoord2f(1,0); glVertex3f(0.3,0.1,-0.3);
   glTexCoord2f(1,1); glVertex3f(0.3,0.1,0.3);
   glTexCoord2f(0,1); glVertex3f(0.1,0,0.3);

   // Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-0.1,0,-0.3);
   glTexCoord2f(1,0); glVertex3f(0.1,0,-0.3);
   glTexCoord2f(1,1); glVertex3f(0.1,0,0.3);
   glTexCoord2f(0,1); glVertex3f(-0.1,0,0.3);

   //  Bottom Left
   glNormal3f(-0.06, -0.06, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(-0.3,0.1,-0.3);
   glTexCoord2f(1,0); glVertex3f(-0.4,0.2,-0.3);
   glTexCoord2f(1,1); glVertex3f(-0.4,0.2,0.3);
   glTexCoord2f(0,1); glVertex3f(-0.3,0.1,0.3);

   //  Very Bottom Right
   glNormal3f(-0.06, -0.12, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(-0.1,0,-0.3);
   glTexCoord2f(1,0); glVertex3f(-0.3,0.1,-0.3);
   glTexCoord2f(1,1); glVertex3f(-0.3,0.1,0.3);
   glTexCoord2f(0,1); glVertex3f(-0.1,0,0.3);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[29]); //Pizza
   //Back
   glBegin(GL_POLYGON);
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0.4, 0); glVertex3f(-0.1,0,-0.3);
   glTexCoord2f(0.2,0.1); glVertex3f(-0.3,0.1,-0.3);
   glTexCoord2f(0.1, 0.2); glVertex3f(-0.4,0.2,-0.3);
   glTexCoord2f(0.5, 1); glVertex3f(0,1,-0.3);
   glTexCoord2f(0.9, 0.2); glVertex3f(0.4,0.2,-0.3);
   glTexCoord2f(0.8, 0.1); glVertex3f(0.3,0.1,-0.3);
   glTexCoord2f(0.6,0); glVertex3f(0.1,0,-0.3);
   glEnd();

   //Front
   glBegin(GL_POLYGON);
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0.4, 0); glVertex3f(-0.1,0,0.3);
   glTexCoord2f(0.2,0.1); glVertex3f(-0.3,0.1,0.3);
   glTexCoord2f(0.1, 0.2); glVertex3f(-0.4,0.2,0.3);
   glTexCoord2f(0.5, 1); glVertex3f(0,1,0.3);
   glTexCoord2f(0.9, 0.2); glVertex3f(0.4,0.2,0.3);
   glTexCoord2f(0.8, 0.1); glVertex3f(0.3,0.1,0.3);
   glTexCoord2f(0.6,0); glVertex3f(0.1,0,0.3);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of the top of buildings, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = Standard (White Gravel), 1 = Standard (Brown Gravel), 2 = Plain Top (no fan/chimneys)
static void buildingTop(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   if (mode == 0) { //Standard (White Gravel)
      glBindTexture(GL_TEXTURE_2D,texture[33]); //Gravel
      glBegin(GL_QUADS);
      //  Floor
      glNormal3f(0, 1, 0); //Set the normal to +y
      glTexCoord2f(0,0); glVertex3f(-2,0,-1);
      glTexCoord2f(1,0); glVertex3f(2,0,-1);
      glTexCoord2f(1,1); glVertex3f(2,0,1);
      glTexCoord2f(0,1); glVertex3f(-2,0,1);
      glEnd();
      glDisable(GL_TEXTURE_2D);

      cone(0.7,0.6,0, 0.2,0.2,0.2, 0,1); //Right Chimney Hat
      cylinder(0.7,0,0, 0.4,0.2,0.4, 0,0, 2); //Right Chimney Bottom Part
      cylinder(0.7,0,0, 0.3,0.6,0.3, 0,0, 4); //Right Chimney Pipe
      
      cone(0.2,1,0, 0.2,0.2,0.2, 0,1); //Left Chimney Hat
      cylinder(0.2,0,0, 0.4,0.2,0.4, 0,0, 2); //Left Chimney Bottom Part
      cylinder(0.2,0,0, 0.3,1,0.3, 0,0, 4); //Left Chimney Pipe

      cylinder(-0.8,0,0, 1,0.7,1, 0,0, 4); //Fan

      cube(-1.9,0,0, 1,0.1,0.1, 90, 10); //Left
      cube(1.9,0,0, 1,0.1,0.1, 90, 10); //Right

      cube(0,0,-1, 2,0.1,0.1, 0, 10); //Back
      cube(0,0,1, 2,0.1,0.1, 0, 10); //Front
   } else if (mode == 1) { //Alternate (Brown Gravel)
      glBindTexture(GL_TEXTURE_2D,texture[76]); //Brown Gravel
      glBegin(GL_QUADS);
      //  Floor
      glNormal3f(0, 1, 0); //Set the normal to +y
      glTexCoord2f(0,0); glVertex3f(-2,0,-1);
      glTexCoord2f(1,0); glVertex3f(2,0,-1);
      glTexCoord2f(1,1); glVertex3f(2,0,1);
      glTexCoord2f(0,1); glVertex3f(-2,0,1);
      glEnd();
      glDisable(GL_TEXTURE_2D);

      cone(0.7,0.6,0, 0.2,0.2,0.2, 0,1); //Right Chimney Hat
      cylinder(0.7,0,0, 0.4,0.2,0.4, 0,0, 2); //Right Chimney Bottom Part
      cylinder(0.7,0,0, 0.3,0.6,0.3, 0,0, 4); //Right Chimney Pipe
      
      cone(0.2,1,0, 0.2,0.2,0.2, 0,1); //Left Chimney Hat
      cylinder(0.2,0,0, 0.4,0.2,0.4, 0,0, 2); //Left Chimney Bottom Part
      cylinder(0.2,0,0, 0.3,1,0.3, 0,0, 4); //Left Chimney Pipe

      cylinder(-0.8,0,0, 1,0.7,1, 0,0, 4); //Fan

      cube(-1.9,0,0, 1,0.1,0.1, 90, 10); //Left
      cube(1.9,0,0, 1,0.1,0.1, 90, 10); //Right

      cube(0,0,-1, 2,0.1,0.1, 0, 10); //Back
      cube(0,0,1, 2,0.1,0.1, 0, 10); //Front
   } else if (mode == 2) { //Plain Top (White Gravel)
      glBindTexture(GL_TEXTURE_2D,texture[33]); //Gravel
      glBegin(GL_QUADS);
      //  Floor
      glNormal3f(0, 1, 0); //Set the normal to +y
      glTexCoord2f(0,0); glVertex3f(-2,0,-1);
      glTexCoord2f(1,0); glVertex3f(2,0,-1);
      glTexCoord2f(1,1); glVertex3f(2,0,1);
      glTexCoord2f(0,1); glVertex3f(-2,0,1);
      glEnd();
      glDisable(GL_TEXTURE_2D);

      cube(-1.9,0,0, 1,0.1,0.1, 90, 10); //Left
      cube(1.9,0,0, 1,0.1,0.1, 90, 10); //Right

      cube(0,0,-1, 2,0.1,0.1, 0, 10); //Back
      cube(0,0,1, 2,0.1,0.1, 0, 10); //Front
   }
  
   //  Undo transformations and textures
   glPopMatrix();
}

//Function to draw the base of a building at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
//Mode: 0 = Red Bricks, 1 = Orange Bricks, 2 = White Bricks, 3 = Yellow Bricks
static void baseBuilding(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[34]); //Red Bricks
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[35]); //Orange Bricks
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[36]); //White Bricks
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[47]); //Yellow Bricks

   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-2,3,1);
   glTexCoord2f(1,1); glVertex3f(2,3,1);
   glTexCoord2f(1,0); glVertex3f(2,0,1);
   glTexCoord2f(0,0); glVertex3f(-2,0,1);
   //  Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,1); glVertex3f(-2,3,-1);
   glTexCoord2f(1,1); glVertex3f(2,3,-1);
   glTexCoord2f(1,0); glVertex3f(2,0,-1);
   glTexCoord2f(0,0); glVertex3f(-2,0,-1);
   //  Right
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(0,1); glVertex3f(2,3,1);
   glTexCoord2f(1,1); glVertex3f(2,3,-1);
   glTexCoord2f(1,0); glVertex3f(2,0,-1);
   glTexCoord2f(0,0); glVertex3f(2,0,1);
   //  Left
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(0,1); glVertex3f(-2,3,1);
   glTexCoord2f(1,1); glVertex3f(-2,3,-1);
   glTexCoord2f(1,0); glVertex3f(-2,0,-1);
   glTexCoord2f(0,0); glVertex3f(-2,0,1);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,1); glVertex3f(-2,0,1);
   glTexCoord2f(1,1); glVertex3f(2,0,1);
   glTexCoord2f(1,0); glVertex3f(2,0,-1);
   glTexCoord2f(0,0); glVertex3f(-2,0,-1);
   //  Top
   glNormal3f(0, 1, 0); //Set the normal to +y
   glTexCoord2f(0,1); glVertex3f(-2,3,1);
   glTexCoord2f(1,1); glVertex3f(2,3,1);
   glTexCoord2f(1,0); glVertex3f(2,3,-1);
   glTexCoord2f(0,0); glVertex3f(-2,3,-1);
   glEnd();
   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a window (for the buildings), allowing it to be scaled, translated, and rotated around the y-axis
static void buildingWindow(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   triangular_prism(0,1,0, 1,0.6,1, 0, 0);
   cube(0,0,0, 1,1,1, 0, 3);

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a business entrance, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 1 = Bakery, 2 = Pizza Shop, 3 = Drug Store
static void businessEntrance(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[39]); //Bakery
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[40]); //Pizza
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[41]); //Drug store

   glBegin(GL_QUADS);
   // Name Sign
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-2,3,0);
   glTexCoord2f(1,1); glVertex3f(2,3,0);
   glTexCoord2f(1,0); glVertex3f(2,2,0);
   glTexCoord2f(0,0); glVertex3f(-2,2,0);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[38]); //Green Stripes
   glBegin(GL_QUADS);
   // Door Cover
   //Planar vector 1 (p1) = v3 - v2 = (2,2,0) - (2,1.7,2) = (0, 0.3, -2)
   //Planar vector 2 (p2) = v2 - v1 = (2,1.7,2) - (-2,1.7,2) = (4, 0, 0)
   glNormal3f(0, 8, 1.2); //Calculate normal
   glTexCoord2f(1,0); glVertex3f(-2,2,0);
   glTexCoord2f(1,1); glVertex3f(2,2,0);
   glTexCoord2f(0,1); glVertex3f(2,1.7,1);
   glTexCoord2f(0,0); glVertex3f(-2,1.7,1);

   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(1,0); glVertex3f(-2,1.7,1);
   glTexCoord2f(1,1); glVertex3f(2,1.7,1);
   glTexCoord2f(0,1); glVertex3f(2,1.4,1);
   glTexCoord2f(0,0); glVertex3f(-2,1.4,1);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[61]); //Glass Door
   //Door
   glBegin(GL_QUADS);
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-2,2,0);
   glTexCoord2f(1,0); glVertex3f(2,2,0);
   glTexCoord2f(1,1); glVertex3f(2,0,0);
   glTexCoord2f(0,1); glVertex3f(-2,0,0);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a staircase handle, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = Red Stone, 1 = White Stone, 2 = Yellow Stone
static void staircaseHandle(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[48]); //Red Stone
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[49]); //White Stone
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[50]); //Yellow Stone
   
   glBegin(GL_QUADS);
   //  Right
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(1,0); glVertex3f(0.8,0.2, 0.2);
   glTexCoord2f(1,1); glVertex3f(0.8,0.2, -0.2);
   glTexCoord2f(0,1); glVertex3f(0.8,0, -0.2);
   glTexCoord2f(0,0); glVertex3f(0.8,0, 0.2);
   //  Left
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(1,0); glVertex3f(-0.8,0.8, -0.2);
   glTexCoord2f(1,1); glVertex3f(-0.8,0.8, 0.2);
   glTexCoord2f(0,1); glVertex3f(-0.8,0, 0.2);
   glTexCoord2f(0,0); glVertex3f(-0.8,0, -0.2);

   //  Slant
   //Planar vector 1 (p1) = v3 - v2 = (-0.4,0.8, -0.2) - (0.8,0.2, -0.2) = (-1.2, 0.6, 0)
   //Planar vector 2 (p2) = v2 - v1 = (0.8,0.2, -0.2) - (0.8,0.2, 0.2) = (0, 0, -0.4)
   glNormal3f(0.24, 0.48, 0); //Set the normal
   glTexCoord2f(1,0); glVertex3f(-0.4,0.8, 0.2);
   glTexCoord2f(1,1); glVertex3f(-0.4,0.8, -0.2);
   glTexCoord2f(0,1); glVertex3f(0.8,0.2, -0.2);
   glTexCoord2f(0,0); glVertex3f(0.8,0.2, 0.2);

   // Top
   glNormal3f(0, 1, 0); //Set the normal tp be +y
   glTexCoord2f(1,0); glVertex3f(-0.8,0.8, 0.2);
   glTexCoord2f(1,1); glVertex3f(-0.8,0.8, -0.2);
   glTexCoord2f(0,1); glVertex3f(-0.4,0.8, -0.2);
   glTexCoord2f(0,0); glVertex3f(-0.4,0.8, 0.2);

   // Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-0.8,0,0.2);
   glTexCoord2f(1,0); glVertex3f(0.8,0,0.2);
   glTexCoord2f(1,1); glVertex3f(0.8,0,-0.2);
   glTexCoord2f(0,1); glVertex3f(-0.8,0,-0.2);
   glEnd();

   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[48]); //Red Stone
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[49]); //White Stone
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[50]); //Yellow Stone

   //Back
   glBegin(GL_POLYGON);
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0, 1); glVertex3f(-0.8,0.8,-0.2);
   glTexCoord2f(0.25, 1); glVertex3f(-0.4,0.8,-0.2);
   glTexCoord2f(1, 0.25); glVertex3f(0.8,0.2,-0.2);
   glTexCoord2f(1, 0); glVertex3f(0.8,0,-0.2);
   glTexCoord2f(0,0); glVertex3f(-0.8,0,-0.2);
   glEnd();

   //Front
   glBegin(GL_POLYGON);
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0, 1); glVertex3f(-0.8,0.8,0.2);
   glTexCoord2f(0.25, 1); glVertex3f(-0.4,0.8,0.2);
   glTexCoord2f(1, 0.25); glVertex3f(0.8,0.2,0.2);
   glTexCoord2f(1, 0); glVertex3f(0.8,0,0.2);
   glTexCoord2f(0,0); glVertex3f(-0.8,0,0.2);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a column, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = Red Column, 1 = White Column, 2 = Yellow Column, 3 = Black Column
static void column(double x,double y,double z, double dx,double dy,double dz, double th, double ph, int mode)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glRotated(ph,1,0,0);
   glScaled(dx,dy,dz);

   if (mode == 0) { //Red Column
      cylinder(0,2.3,0, 0.5,0.2,0.5, 0,0, 8); //Very Top
      cylinder(0,2.2,0, 0.35,0.1,0.35, 0,0, 8); //Top
      cylinder(0,0.3,0, 0.3,2,0.3, 0,0, 5); //Column
      cylinder(0,0.2,0, 0.35,0.1,0.35, 0,0, 8); //Bottom
      cylinder(0,0,0, 0.5,0.2,0.5, 0,0, 8); //Very Bottom

   } else if (mode == 1) { //White Column
      cylinder(0,2.3,0, 0.5,0.2,0.5, 0,0, 9); //Very Top
      cylinder(0,2.2,0, 0.35,0.1,0.35, 0,0, 9); //Top
      cylinder(0,0.3,0, 0.3,2,0.3, 0,0, 6); //Column
      cylinder(0,0.2,0, 0.35,0.1,0.35, 0,0, 9); //Bottom
      cylinder(0,0,0, 0.5,0.2,0.5, 0,0, 9); //Very Bottom
   } else if (mode == 2) { //Yellow Column
      cylinder(0,2.3,0, 0.5,0.2,0.5, 0,0, 10); //Very Top
      cylinder(0,2.2,0, 0.35,0.1,0.35, 0,0, 10); //Top
      cylinder(0,0.3,0, 0.3,2,0.3, 0,0, 7); //Column
      cylinder(0,0.2,0, 0.35,0.1,0.35, 0,0, 10); //Bottom
      cylinder(0,0,0, 0.5,0.2,0.5, 0,0, 10); //Very Bottom
   }  else if (mode == 3) { //Black Column
      cylinder(0,2.3,0, 0.5,0.2,0.5, 0,0, 12); //Very Top
      cylinder(0,2.2,0, 0.35,0.1,0.35, 0,0, 12); //Top
      cylinder(0,0.3,0, 0.3,2,0.3, 0,0, 12); //Column
      cylinder(0,0.2,0, 0.35,0.1,0.35, 0,0, 12); //Bottom
      cylinder(0,0,0, 0.5,0.2,0.5, 0,0, 12); //Very Bottom
   }
   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of an entrance to a building, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = Red Door, 1 = White Door, 2 = Yellow Door
static void buildingEntrance(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   if (mode == 0) { //Red Door
      staircaseHandle(0.7,0,0.5, 0.7,0.8,0.7, -70, 0); //Right handle
      column(0.5,0,0, 0.8,0.8,0.8, 0,0, 0); //Right Column
      triangular_prism(0,2,0, 0.2,0.2,0.8, 90, 1); //Top Part
      column(-0.5,0,0, 0.8,0.8,0.8, 0,0, 0); //Left Column
      staircaseHandle(-0.7,0,0.5, 0.7,0.8,0.7, -110, 0); //Left handle
      
      //Stairs
      cube(0,0.35,0.2, 0.5,0.05,0.2, 0, 11);
      cube(0,0.25,0.4, 0.6,0.05,0.2, 0, 11);
      cube(0,0.15,0.6, 0.7,0.05,0.2, 0, 11);
      cube(0,0.05,0.8, 0.85,0.05,0.2, 0, 11);
   } else if (mode == 1) { //White Door
      staircaseHandle(0.7,0,0.5, 0.7,0.8,0.7, -70, 1); //Right handle
      column(0.5,0,0, 0.8,0.8,0.8, 0,0, 1); //Right Column
      triangular_prism(0,2,0, 0.2,0.2,0.8, 90, 2); //Top Part
      column(-0.5,0,0, 0.8,0.8,0.8, 0,0, 1); //Left Column
      staircaseHandle(-0.7,0,0.5, 0.7,0.8,0.7, -110, 1); //Left handle
      
      //Stairs
      cube(0,0.35,0.2, 0.5,0.05,0.2, 0, 11);
      cube(0,0.25,0.4, 0.6,0.05,0.2, 0, 11);
      cube(0,0.15,0.6, 0.7,0.05,0.2, 0, 11);
      cube(0,0.05,0.8, 0.85,0.05,0.2, 0, 11);
   } else if (mode == 2) { //Yellow Door
      staircaseHandle(0.7,0,0.5, 0.7,0.8,0.7, -70, 2); //Right handle
      column(0.5,0,0, 0.8,0.8,0.8, 0,0, 2); //Right Column
      triangular_prism(0,2,0, 0.2,0.2,0.8, 90, 3); //Top Part
      column(-0.5,0,0, 0.8,0.8,0.8, 0,0, 2); //Left Column
      staircaseHandle(-0.7,0,0.5, 0.7,0.8,0.7, -110, 2); //Left handle
      
      //Stairs
      cube(0,0.35,0.2, 0.5,0.05,0.2, 0, 11);
      cube(0,0.25,0.4, 0.6,0.05,0.2, 0, 11);
      cube(0,0.15,0.6, 0.7,0.05,0.2, 0, 11);
      cube(0,0.05,0.8, 0.85,0.05,0.2, 0, 11);
   }

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   if (mode == 0 || mode == 2) glBindTexture(GL_TEXTURE_2D,texture[54]); //Blue Door
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[55]); //Green Door

   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-0.4,2,0);
   glTexCoord2f(1,1); glVertex3f(0.4,2,0);
   glTexCoord2f(1,0); glVertex3f(0.4,0.4,0);
   glTexCoord2f(0,0); glVertex3f(-0.4,0.4,0);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of an SNES system, allowing it to be scaled, translated, and rotated around the y-axis
static void snes(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   // SNES
   cube(0,2.8,-0.45, 0.7,0.5,0.1, 0, 20); //Cartridge
   cube(0,2.66,-0.45, 0.8,0.05,0.12, 0, 11); //Cartridge Holder
   cylinder(-0.95,2.5,-0.4, 1.9,0.5,1, 90, 90, 11); //Cylinder

   cube(-0.45,2.4,0.5, 0.3,0.25,0.15, 0, 18); //Power Switch
   cube(0,2.35,0.7, 0.2,0.25,0.3, 0, 11); //Eject Button
   cube(0.45,2.4,0.5, 0.3,0.25,0.15, 0, 19); //Reset Switch

   cube(-0.45,2.35,0.5, 0.8,0.25,0.25, 90, 10); //Left
   cube(0.45,2.35,0.5, 0.8,0.25,0.25, 90, 10); //Right 

   cube(0,2.35,0, 1,0.2,1.2, 0, 10); //Main Base
   cube(0,2.1,0, 0.8,0.05,1, 0, 10); //Bottom Part

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of an open window (for the hospital), allowing it to be scaled, translated, and rotated around the y-axis
static void hospitalWindow(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   cube(0,0,0, 1,1,1, 0, 22);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[13]); //Window
   glBegin(GL_QUADS);
   //  Window
   //Planar vector 1 (p1) = v3 - v2 = (1,1,1) - (1,-1,1) = (0, 2, 0)
   //Planar vector 2 (p2) = v2 - v1 = (1,-1,1) - (-0.7,-1,3) = (1.7, 0, -2)
   glNormal3f(4, 0, 3.4); //Set the normal
   glTexCoord2f(0,1); glVertex3f(-0.7,1,3);
   glTexCoord2f(1,1); glVertex3f(1,1,1);
   glTexCoord2f(1,0); glVertex3f(1,-1,1);
   glTexCoord2f(0,0); glVertex3f(-0.7,-1,3);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a flat car top, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = Police Car Top, 1 = Red Car Top, 2 = Blue Car Top
static void flatCarTop(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[80]); //Black Marble
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[81]); //Red Metal
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[83]); //Blue Metal
   
   glBegin(GL_QUADS);
   //  Front
   //Planar vector 1 (p1) = v3 - v2 = (1.3,0.1,0.5) - (1.5,0,0.7) = (-0.2, 0.1, -0.2)
   //Planar vector 2 (p2) = v2 - v1 = (1.5,0,0.7) - (-1.5,0,0.7) = (3, 0, 0)
   glNormal3f(0, 0.6, 0.3); //Set the normal
   glTexCoord2f(0,1); glVertex3f(-1.3,0.1,0.5);
   glTexCoord2f(1,1); glVertex3f(1.3,0.1,0.5);
   glTexCoord2f(1,0); glVertex3f(1.5,0,0.7);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,0.7);
   //Back
   glNormal3f(0, 0.6, -0.3); //Set the normal
   glTexCoord2f(0,1); glVertex3f(-1.3,0.1,-0.5);
   glTexCoord2f(1,1); glVertex3f(1.3,0.1,-0.5);
   glTexCoord2f(1,0); glVertex3f(1.5,0,-0.7);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,-0.7);
   //Right
   //Planar vector 1 (p1) = v3 - v2 = (1.3,0.1,-0.5) - (1.5,0,-0.7) = (-0.2, 0.1, 0.2)
   //Planar vector 2 (p2) = v2 - v1 = (1.5,0,-0.7) - (1.5,0,0.7) = (0, 0, -1.4)
   glNormal3f(0.14, 0.28, 0); //Set the normal
   glTexCoord2f(0,1); glVertex3f(1.3,0.1,0.5);
   glTexCoord2f(1,1); glVertex3f(1.3,0.1,-0.5);
   glTexCoord2f(1,0); glVertex3f(1.5,0,-0.7);
   glTexCoord2f(0,0); glVertex3f(1.5,0,0.7);
   //Left
   glNormal3f(-0.14, 0.28, 0); //Set the normal
   glTexCoord2f(0,1); glVertex3f(-1.3,0.1,-0.5);
   glTexCoord2f(1,1); glVertex3f(-1.3,0.1,0.5);
   glTexCoord2f(1,0); glVertex3f(-1.5,0,0.7);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,-0.7);
   //Top
   glNormal3f(0, 1, 0); //Set the normal to +y
   glTexCoord2f(0,1); glVertex3f(-1.3,0.1,-0.5);
   glTexCoord2f(1,1); glVertex3f(1.3,0.1,-0.5);
   glTexCoord2f(1,0); glVertex3f(1.3,0.1,0.5);
   glTexCoord2f(0,0); glVertex3f(-1.3,0.1,0.5);
   //Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,1); glVertex3f(-1.5,0,-0.7);
   glTexCoord2f(1,1); glVertex3f(1.5,0,-0.7);
   glTexCoord2f(1,0); glVertex3f(1.5,0,0.7);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,0.7);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a standard car top, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = Police Car Top, 1 = Red Car Top, 2 = Blue Car Top, 3 = Ambulance Top
static void carTop(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[90]); //Police Car Side Windows
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[92]); //Red Car Side Windows
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[94]); //Blue Car Side Windows
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[101]); //Ambulance Side Windows

   glBegin(GL_QUADS);
   //  Front
   //Planar vector 1 (p1) = v3 - v2 = (0.8,1,0.6) - (1,0,0.7) = (-0.2, 1, -0.1)
   //Planar vector 2 (p2) = v2 - v1 = (1,0,0.7) - (-1.5,0,0.7) = (2.5, 0, 0)
   glNormal3f(0, 0.25, 2.5); //Set the normal
   glTexCoord2f(0.42,1); glVertex3f(-0.4,1,0.6);
   glTexCoord2f(0.9,1);  glVertex3f(0.8,1,0.6);
   glTexCoord2f(1,0);    glVertex3f(1,0,0.7);
   glTexCoord2f(0,0);    glVertex3f(-1.5,0,0.7);
   //Back
   glNormal3f(0, -0.25, -2.5); //Set the normal
   glTexCoord2f(0.42,1); glVertex3f(-0.4,1,-0.6);
   glTexCoord2f(0.9,1);  glVertex3f(0.8,1,-0.6);
   glTexCoord2f(1,0);    glVertex3f(1,0,-0.7);
   glTexCoord2f(0,0);    glVertex3f(-1.5,0,-0.7);
   glEnd();

   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[91]); //Police Car Front Back Windows
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[93]); //Red Car Front Back Windows
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[95]); //Blue Car Front Back Windows
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[102]); //Ambulance Front Back Windows

   glBegin(GL_QUADS);
   //Right
   //Planar vector 1 (p1) = v3 - v2 = (0.8,1,-0.6) - (1,0,-0.7) = (-0.2, 1, 0.1)
   //Planar vector 2 (p2) = v2 - v1 = (1,0,-0.7) - (1,0,0.7) = (0, 0, -1.4)
   glNormal3f(1.4, 0.28, 0); //Set the normal
   glTexCoord2f(0,1); glVertex3f(0.8,1,0.6);
   glTexCoord2f(1,1); glVertex3f(0.8,1,-0.6);
   glTexCoord2f(1,0); glVertex3f(1,0,-0.7);
   glTexCoord2f(0,0); glVertex3f(1,0,0.7);
   //Left
   //Planar vector 1 (p1) = v3 - v2 = (-0.4,1,0.6) - (-1.5,0,0.7) = (1.1, 1, -0.1)
   //Planar vector 2 (p2) = v2 - v1 = (-1.5,0,0.7) - (-1.5,0,-0.7) = (0, 0, 1.4)
   glNormal3f(-1.4, 1.54, 0); //Set the normal
   glTexCoord2f(0,1); glVertex3f(-0.4,1,-0.6);
   glTexCoord2f(1,1); glVertex3f(-0.4,1,0.6);
   glTexCoord2f(1,0); glVertex3f(-1.5,0,0.7);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,-0.7);
   glEnd();

   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[80]); //Black
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D,texture[81]); //Red
   else if (mode == 2) glBindTexture(GL_TEXTURE_2D,texture[83]); //Blue
   else if (mode == 3) glBindTexture(GL_TEXTURE_2D,texture[31]); //White Metal

   glBegin(GL_QUADS);
   //Top
   glNormal3f(0, 1, 0); //Set the normal to +y
   glTexCoord2f(0,1); glVertex3f(-0.4,1,-0.6);
   glTexCoord2f(1,1); glVertex3f(0.8,1,-0.6);
   glTexCoord2f(1,0); glVertex3f(0.8,1,0.6);
   glTexCoord2f(0,0); glVertex3f(-0.4,1,0.6);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[63]); //Black
   glBegin(GL_QUADS);
   //Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,1); glVertex3f(-1.5,0,-0.7);
   glTexCoord2f(1,1); glVertex3f(1,0,-0.7);
   glTexCoord2f(1,0); glVertex3f(1,0,0.7);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,0.7);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw a cylinder at (x,y,z), with radius (r), thickness (t), rotation (th) about the y axis, rotation (ph) about the x axis (Modified from Example 19 on Canvas)
//Mode: 0 = Car Wheel, 1 = Sewer Cover
static void roundCylinder(double x,double y,double z,double r,double t, double th, double ph, int mode)
{
   int i,k;
   glEnable(GL_TEXTURE_2D);
   //  Save transformation
   glPushMatrix();
   //  Offset and scale
   glTranslated(x,y,z);
   glRotated(ph,1,0,0);
   glRotated(th,0,1,0);
   glScaled(r,r,t);
   // Draw the front and back of the wheel
   glColor3f(1,1,1);
   for (i=1;i>=-1;i-=2)
   {
      if (mode == 0) glBindTexture(GL_TEXTURE_2D, texture[89]); //Car Wheel
      else if (mode == 1) glBindTexture(GL_TEXTURE_2D, texture[118]); //Manhole Cover
      glNormal3f(0,0,i);
      glBegin(GL_TRIANGLE_FAN);
      glTexCoord2f(0.5,0.5);
      glVertex3f(0,0,i);
      for (k=0;k<=360;k+=10)
      {
         glTexCoord2f(0.5*Cos(k)+0.5,0.5*Sin(k)+0.5);
         glVertex3f(i*Cos(k),Sin(k),i);
      }
      glEnd();
   }

   // Draw the edge of the cylinder
   if (mode == 0) glBindTexture(GL_TEXTURE_2D,texture[97]); //Care tire treads
   else if (mode == 1) glBindTexture(GL_TEXTURE_2D, texture[56]); //Concrete
   glBegin(GL_QUAD_STRIP);
   for (k=0;k<=360;k+=10)
   {
      glNormal3f(Cos(k),Sin(k),0);
      glTexCoord2f(0,0.5*k); glVertex3f(Cos(k),Sin(k),+1);
      glTexCoord2f(1,0.5*k); glVertex3f(Cos(k),Sin(k),-1);
   }
   glEnd();

   //  Undo transformations
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of the front bottom part of an ambulance, allowing it to be scaled, translated, and rotated around the y-axis
static void ambulanceFront(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   

   glBindTexture(GL_TEXTURE_2D,texture[100]); //Ambulance Sides
   glBegin(GL_POLYGON);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1);   glVertex3f(-0.8,1,0.7);
   glTexCoord2f(0.7,1); glVertex3f(0.5,1,0.7);
   glTexCoord2f(1,0.5); glVertex3f(1,0.5,0.7);
   glTexCoord2f(1,0);   glVertex3f(1,0,0.7);
   glTexCoord2f(0,0);   glVertex3f(-0.8,0,0.7);
   glEnd();

   glBegin(GL_POLYGON);
   //Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,1);   glVertex3f(-0.8,1,-0.7);
   glTexCoord2f(0.7,1); glVertex3f(0.5,1,-0.7);
   glTexCoord2f(1,0.5); glVertex3f(1,0.5,-0.7);
   glTexCoord2f(1,0);   glVertex3f(1,0,-0.7);
   glTexCoord2f(0,0);   glVertex3f(-0.8,0,-0.7);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[100]); //Ambulance Sides
   glBegin(GL_QUADS);
   //Right Slant
   //Planar vector 1 (p1) = v3 - v2 = (0.5,1,-0.7) - (1,0.5,-0.7) = (-0.5, 0.5, 0)
   //Planar vector 2 (p2) = v2 - v1 = (1,0.5,-0.7) - (1,0.5,0.7) = (0, 0, -1.4)
   glNormal3f(0.7, 0.7, 0); //Set the normal
   glTexCoord2f(0,1); glVertex3f(0.5,1,0.7);
   glTexCoord2f(1,1); glVertex3f(0.5,1,-0.7);
   glTexCoord2f(1,0); glVertex3f(1,0.5,-0.7);
   glTexCoord2f(0,0); glVertex3f(1,0.5,0.7);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[103]); //Ambulance Front
   glBegin(GL_QUADS);
   //Right
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(0,1); glVertex3f(1,0.5,0.7);
   glTexCoord2f(1,1); glVertex3f(1,0.5,-0.7);
   glTexCoord2f(1,0); glVertex3f(1,0,-0.7);
   glTexCoord2f(0,0); glVertex3f(1,0,0.7);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[80]); //Black Marble
   glBegin(GL_QUADS);
   //Left
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(0,1); glVertex3f(-0.8,1,-0.7);
   glTexCoord2f(1,1); glVertex3f(-0.8,1,0.7);
   glTexCoord2f(1,0); glVertex3f(-0.8,0,0.7);
   glTexCoord2f(0,0); glVertex3f(-0.8,0,-0.7);
   //Top
   glNormal3f(0, 1, 0); //Set the normal to +y
   glTexCoord2f(0,1); glVertex3f(-0.8,1,-0.7);
   glTexCoord2f(1,1); glVertex3f(0.5,1,-0.7);
   glTexCoord2f(1,0); glVertex3f(0.5,1,0.7);
   glTexCoord2f(0,0); glVertex3f(-0.8,1,0.7);
   //Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,1); glVertex3f(-0.8,0,-0.7);
   glTexCoord2f(1,1); glVertex3f(1,0,-0.7);
   glTexCoord2f(1,0); glVertex3f(1,0,0.7);
   glTexCoord2f(0,0); glVertex3f(-0.8,0,0.7);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//to draw one instance of a burger, allowing it to be scaled, translated, and rotated around the y-axis
static void burger(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   hollowHemiSphere(0,0.5,0, 0.45,0.3,0.45, 0, 1); //Top Bun
   cylinder(-0.1,0.4,-0.1, 0.7,0.1,0.7, 0,0, 17); //Tomato
   cylinder(0.2,0.4,0.15, 0.7,0.1,0.7, 0,0, 17); //Tomato
   cylinder(0,0.3,0, 1.1,0.1,1.1, 0,0, 0); //Top Meat
   cylinder(0,0.22,0, 1,0.1,1, 0,0, 18); //Middle Bun
   cube(0,0.2,0, 0.4,0.02,0.4, 0, 31); //Bottom Cheese
   cylinder(0,0.1,0, 1.1,0.1,1.1, 0,0, 0); //Bottom Meat
   cylinder(0,0,0, 1,0.1,1, 0,0, 18); //Bottom Bun
   transparentObject(0,-0.19,0.2, 0.6,0.6,0.6, 180,-90, 1); //Lettuce

   //  Undo transformations
   glPopMatrix();
}
//********************************************************************************************************************************************************************************



// Instance Functions: Functions that draw complete objects that can be scaled and rotated as necessary
//********************************************************************************************************************************************************************************
//Function to draw one instance of a tree, allowing the tree to be scaled, translated, and rotation (th) around the y-axis
static void tree(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Draw objects such that they build a tree
   cylinder(0, 0, 0, 0.2, 0.3, 0.2, 0,0, 0); //Tree Trunk
   cone(0, 0.3, 0, 0.3, 0.4, 0.3, 0, 0); //Bottom Cone
   cone(0, 0.6, 0, 0.25, 0.3, 0.25, 0, 0); //Middle Cone
   cone(0, 0.9, 0, 0.15, 0.2, 0.15, 0, 0); //Top Cone

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a big tree, allowing the tree to be scaled, translated, and rotation (th) around the y-axis
static void bigTree(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Draw objects such that they build a tree
   cylinder(0, 0, 0, 0.3, 0.4, 0.3, 0,0, 0); //Tree Trunk
   cone(0, 0.4, 0, 0.6, 0.5, 0.6, 0, 0); //Very Bottom Cone
   cone(0, 0.7, 0, 0.5, 0.4, 0.5, 0, 0); //Bottom Cone
   cone(0, 1, 0, 0.4, 0.3, 0.4, 0, 0); //Middle Cone
   cone(0, 1.25, 0, 0.3, 0.3, 0.3, 0, 0); //Top Cone

   //  Undo transformations
   glPopMatrix();
}

//Function to draw a chain of fences at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
//Mode: 0 = White Fence, 1/2 = Brown Fence
static void fenceChain(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   if (mode == 0) { //White Fence
         cube(-1.2,0.6,0, 0.1,0.1,0.2, 90, 8);
         cube(-0.4,0.6,0, 0.1,0.1,0.2, 90, 8);
         cube(0.4,0.6,0, 0.1,0.1,0.2, 90, 8);
         cube(1.2,0.6,0, 0.1,0.1,0.2, 90, 8);
         cube(2,0.6,0, 0.1,0.1,0.2, 90, 8);
   } else if (mode == 1 || mode == 2) { //Brown Fence
         cube(-1.2,0.6,0, 0.1,0.1,0.2, 90, 9);
         cube(-0.4,0.6,0, 0.1,0.1,0.2, 90, 9);
         cube(0.4,0.6,0, 0.1,0.1,0.2, 90, 9);
         cube(1.2,0.6,0, 0.1,0.1,0.2, 90, 9);
         cube(2,0.6,0, 0.1,0.1,0.2, 90, 9);
   }
   //Create a chain of fences
   fence(2.4,0,0, 0.1,1,0.2, 90, mode);
   fence(1.6,0,0, 0.1,1,0.2, 90, mode);
   fence(0.8,0,0, 0.1,1,0.2, 90, mode);
   fence(0,0,0, 0.1,1,0.2, 90, mode);
   fence(-0.8,0,0, 0.1,1,0.2, 90, mode);
   fence(-1.6,0,0, 0.1,1,0.2, 90, mode);

   //  Undo transformations
   glPopMatrix();
}

//Function to draw a chain of wooden logs at (x,y,z), with dimensions (dx,dy,dz), and rotation (th) about the y axis
static void woodFenceChain(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Create a chain of fences
   cylinder(1.6,0,0, 0.3,0.6,0.3, 0,0, 0);
   cylinder(1.4,0,0, 0.3,1,0.3, 0,0, 0);
   cylinder(1.2,0,0, 0.3,1.2,0.3, 0,0, 0);
   cylinder(1,0,0, 0.3,0.8,0.3, 0,0, 0);
   cylinder(0.8,0,0, 0.3,1,0.3, 0,0, 0);
   cylinder(0.6,0,0, 0.3,1.3,0.3, 0,0, 0);
   cylinder(0.4,0,0, 0.3,0.6,0.3, 0,0, 0);
   cylinder(0.2,0,0, 0.3,0.9,0.3, 0,0, 0);

   cylinder(0,0,0, 0.3,1.4,0.3, 0,0, 0);
   
   cylinder(-1.6,0,0, 0.3,0.5,0.3, 0,0, 0);
   cylinder(-1.4,0,0, 0.3,1.3,0.3, 0,0, 0);
   cylinder(-1.2,0,0, 0.3,1.2,0.3, 0,0, 0);
   cylinder(-1,0,0, 0.3,0.7,0.3, 0,0, 0);
   cylinder(-0.8,0,0, 0.3,0.9,0.3, 0,0, 0);
   cylinder(-0.6,0,0, 0.3,1.1,0.3, 0,0, 0);
   cylinder(-0.4,0,0, 0.3,0.6,0.3, 0,0, 0);
   cylinder(-0.2,0,0, 0.3,1.2,0.3, 0,0, 0);

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a small house, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = White House w/ Purple Roof, 1 = Yellow House, 2 = White House w/ Blue Roof
static void smallHouse(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   // weatherVane(-0.8,2.3,0, 0.2,0.2,0.2, 90);
   if (mode == 0 || mode == 2) { //White House
      cube(-0.6,2,0, 0.1,0.1,0.1, 0, 5); // Chimney
      //Windows
      cube(0.75,0.5,1, 0.1,0.3,0.05, 0, 3); // Front Right Window
      cube(-0.75,0.5,1, 0.1,0.3,0.05, 0, 3); // Front Left Window
      cube(0.75,0.5,-1, 0.1,0.3,0.05, 0, 3); // Back Right Window
      cube(-0.75,0.5,-1, 0.1,0.3,0.05, 0, 3); // Back Left Window

      cube(0.98,0.5,-0.5, 0.3,0.2,0.05, 90, 3); // Right Side Right Window
      cube(0.98,0.5,0.5, 0.3,0.2,0.05, 90, 3); // Right Side Left Window

      cube(-1,0.4,0, 0.2,0.4,0.05, 90, 1); // Door
   } else if (mode == 1) { //Yellow House
      cube(-0.6,2,0, 0.1,0.1,0.1, 0, 6); // Chimney
      //Windows
      cube(0.75,0.5,1, 0.1,0.3,0.05, 0, 4); // Front Right Window
      cube(-0.75,0.5,1, 0.1,0.3,0.05, 0, 4); // Front Left Window
      cube(0.75,0.5,-1, 0.1,0.3,0.05, 0, 4); // Back Right Window
      cube(-0.75,0.5,-1, 0.1,0.3,0.05, 0, 4); // Back Left Window

      cube(0.98,0.5,-0.5, 0.3,0.2,0.05, 90, 4); // Right Side Right Window
      cube(0.98,0.5,0.5, 0.3,0.2,0.05, 90, 4); // Right Side Left Window

      cube(-1,0.4,0, 0.2,0.4,0.05, 90, 2); // Door
   }

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   if (mode == 0 || mode == 2) { //White House
      glBindTexture(GL_TEXTURE_2D,texture[15]);
   } else if (mode == 1) { //Yellow House
      glBindTexture(GL_TEXTURE_2D,texture[19]);
   }
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-1, 0.0, 1);
   glTexCoord2f(1,0); glVertex3f(+1, 0.0, 1);
   glTexCoord2f(1,1); glVertex3f(+1,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1,+1, 1);
   //  Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,0); glVertex3f(+1, 0.0,-1);
   glTexCoord2f(1,0); glVertex3f(-1, 0.0,-1);
   glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-1,0.0,-1);
   glTexCoord2f(1,0); glVertex3f(+1,0.0,-1);
   glTexCoord2f(1,1); glVertex3f(+1,0.0,+1);
   glTexCoord2f(0,1); glVertex3f(-1,0.0,+1);
   glEnd();

   //  Right Side of House
   glBegin(GL_POLYGON);
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(1, 0); glVertex3f(+1,0.0,-1);
   glTexCoord2f(1, 0.5); glVertex3f(+1,+1,-1);
   glTexCoord2f(0.5, 1); glVertex3f(+1,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(+1,1,+1);
   glTexCoord2f(0,0); glVertex3f(+1,0.0,+1);
   glEnd();

   //  Left Side of House
   glBegin(GL_POLYGON);
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(1, 0); glVertex3f(-1,0.0,+1);
   glTexCoord2f(1, 0.5); glVertex3f(-1,+1,+1);
   glTexCoord2f(0.5, 1); glVertex3f(-1,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(-1,+1,-1);
   glTexCoord2f(0,0); glVertex3f(-1,0.0,-1);
   glEnd();


   if (mode == 0) { //Purple Stripes
      glBindTexture(GL_TEXTURE_2D,texture[10]);
   } else if (mode == 1) { //Red Stripes
      glBindTexture(GL_TEXTURE_2D,texture[11]);
   } else if (mode == 2) { //Blue Stripes
      glBindTexture(GL_TEXTURE_2D,texture[25]);
   }
   glBegin(GL_QUADS);
   // Door Cover
   //Planar vector 1 (p1) = v3 - v2 = (-1.5,0.8,0.4) - (-1.5,0.8,-0.4) = (0, 0, 0.8)
   //Planar vector 2 (p2) = v2 - v1 = (-1.5,0.8,-0.4) - (-1,1,-0.4) = (-0.5, -0.2, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(-0.16, 0.4, 0); //Calculate normal
   glTexCoord2f(1,0); glVertex3f(-1.5,0.8,0.4);
   glTexCoord2f(1,1); glVertex3f(-1.5,0.8,-0.4);
   glTexCoord2f(0,1); glVertex3f(-1,1,-0.4);
   glTexCoord2f(0,0); glVertex3f(-1,1,0.4);

   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(1,1); glVertex3f(-1.5,0.8,0.4);
   glTexCoord2f(0,1); glVertex3f(-1.5,0.7,0.4);
   glTexCoord2f(0,0); glVertex3f(-1.5,0.7,-0.4);
   glTexCoord2f(1,0); glVertex3f(-1.5,0.8,-0.4);
   glEnd();


   if (mode == 0) { //Purple Roof
      glBindTexture(GL_TEXTURE_2D,texture[12]);
   } else if (mode == 1) { //Red Roof
      glBindTexture(GL_TEXTURE_2D,texture[16]);
   } else if (mode == 2) { //Blue Roof
      glBindTexture(GL_TEXTURE_2D,texture[26]);
   }
   //Draw the rectangular sides of the prism
   glBegin(GL_QUADS);
   //Top Back Side:
   //Planar vector 1 (p1) = v3 - v2 = (1, 2, 0) - (1, 1, -1) = (0, 1, 1)
   //Planar vector 2 (p2) = v2 - v1 = (1, 1, -1) - (-1, 1, -1) = (2, 0, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0, 2, -2); //Calculate normal
   glTexCoord2f(1,1); glVertex3f(-1, 2, 0); //v4
   glTexCoord2f(0,1); glVertex3f(1, 2, 0); //v3
   glTexCoord2f(0,0); glVertex3f(1, 1, -1); //v2
   glTexCoord2f(1,0); glVertex3f(-1, 1, -1); //v1

   //Top Front Side:
   //Planar vector 1 (p1) = v3 - v2 = (-1, 2, 0) - (-1, 1, 1) = (0, 1, -1)
   //Planar vector 2 (p2) = v2 - v1 = (-1, 1, 1) - (1, 1, 1) = (-2, 0, 0)
   //Normal = p1 x p2 = (p1y*p2z - p1z*p2y, p1z*p2x - p1x*p2z, p1x*p2y - p1y*p2x)
   glNormal3f(0, 2, 2); //Calculate normal
   glTexCoord2f(0,0); glVertex3f(1, 2, 0); //v4
   glTexCoord2f(1,0); glVertex3f(-1, 2, 0); //v3
   glTexCoord2f(1,1); glVertex3f(-1, 1, 1); //v2
   glTexCoord2f(0,1); glVertex3f(1, 1, 1); //v1
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a big house with fences, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = White House w/ Purple Roof, 1 = Yellow House, 2 = White House w/ Blue Roof
static void bigHouse(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Left Back
   fenceChain(-1,0,-0.1 , 0.15,0.15,0.15 , 0, mode);
   //Left
   fenceChain(-1.3,0,0.3 , 0.15,0.15,0.15 , 90, mode);
   fenceChain(-1.3,0,1 , 0.15,0.15,0.15 , 90, mode);
   fenceChain(-1.3,0,1.7 , 0.15,0.15,0.15 , 90, mode);
   //Left Front
   fenceChain(-1,0,2, 0.15,0.15,0.15 , 0, mode);

   //Right Back
   fenceChain(1.3,0,-0.1 , 0.15,0.15,0.15 , 0, mode);

   //Right
   fenceChain(1.7,0,0.3 , 0.15,0.15,0.15 , 90, mode);
   fenceChain(1.7,0,1 , 0.15,0.15,0.15 , 90, mode);
   fenceChain(1.7,0,1.7 , 0.15,0.15,0.15 , 90, mode);

   //Right Front
   fenceChain(1.3,0,2 , 0.15,0.15,0.15 , 0, mode);
   fenceChain(0.8,0,2 , 0.15,0.15,0.15 , 0, mode);

   //Combine the houses to form a big house
   basicHouse(0,0,0 , 1,0.5,0.5 , 0, mode);
   smallHouse(0,0,0.5 , 0.5,0.5,0.5 , 90, mode);

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a stop sign, allowing it to be scaled, translated, and rotated around the y-axis
static void stopSign(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   cylinder(0,0,0, 0.2,0.2,0.2, 0,0, 2); //Bottom Part
   cylinder(0,0,0, 0.1,1.5,0.1, 0,0, 2); //Bar
   cylinder(0,1.5,0, 1,1,0.1, 0,90, 1); //Sign

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a very big sign, allowing it to be scaled, translated, and rotated around the y-axis
static void veryBigSign(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   cylinder(0.7,0,0, 0.2,0.2,0.2, 0,0, 0); //Right Bottom Part
   cylinder(0.7,0,0, 0.1,1,0.1, 0,0, 3); //Right Bar

   cylinder(0,0,0, 0.2,0.2,0.2, 0,0, 0); //Middle Bottom Part
   cylinder(0,0,0, 0.1,1,0.1, 0,0, 3); //Middle Bar

   cylinder(-0.7,0,0, 0.2,0.2,0.2, 0,0, 0); //Left Bottom Part
   cylinder(-0.7,0,0, 0.1,1,0.1, 0,0, 3); //Left Bar

   signBase(0,1,0, 1.2,1.5,1, 0, 2); //Sign

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a big sign, allowing it to be scaled, translated, and rotated around the y-axis
static void bigSign(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   cylinder(0.7,0,0, 0.2,0.2,0.2, 0,0, 0); //Right Bottom Part
   cylinder(0.7,0,0, 0.1,1,0.1, 0,0, 3); //Right Bar

   cylinder(0,0,0, 0.2,0.2,0.2, 0,0, 0); //Middle Bottom Part
   cylinder(0,0,0, 0.1,1,0.1, 0,0, 3); //Middle Bar

   cylinder(-0.7,0,0, 0.2,0.2,0.2, 0,0, 0); //Left Bottom Part
   cylinder(-0.7,0,0, 0.1,1,0.1, 0,0, 3); //Left Bar

   signBase(0,1,0, 1,1,1, 0, 0); //Sign

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a small sign, allowing it to be scaled, translated, and rotated around the y-axis
static void smallSign(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   cylinder(0.4,0,0, 0.2,0.2,0.2, 0,0, 0); //Right Bottom Part
   cylinder(0.4,0,0, 0.1,1,0.1, 0,0, 3); //Right Bar

   cylinder(-0.4,0,0, 0.2,0.2,0.2, 0,0, 0); //Left Bottom Part
   cylinder(-0.4,0,0, 0.1,1,0.1, 0,0, 3); //Left Bar

   signBase(0,1,0, 0.7,1,1, 0, 1); //Sign

   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a mailbox, allowing it to be scaled, translated, and rotated around the y-axis
static void mailbox(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Legs of mailbox
   cube(-0.55,-0.05,0.17, 0.15,0.06,0.03, 0, 7);
   cube(-0.55,-0.05,-0.17, 0.15,0.06,0.03, 0, 7);
   cube(0.55,-0.05,-0.17, 0.15,0.06,0.03, 0, 7);
   cube(0.55,-0.05,0.17, 0.15,0.06,0.03, 0, 7);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[23]); //Mailbox metal
   glBegin(GL_QUADS);
   //  Right
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(0,0); glVertex3f(0.7,0, -0.2);
   glTexCoord2f(1,0); glVertex3f(0.7,0.5, -0.2);
   glTexCoord2f(1,1); glVertex3f(0.7,0.5, 0.2);
   glTexCoord2f(0,1); glVertex3f(0.7,0, 0.2);
   //  Left
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(0,0); glVertex3f(-0.7,0,-0.2);
   glTexCoord2f(1,0); glVertex3f(-0.7, 0.5,-0.2);
   glTexCoord2f(1,1); glVertex3f(-0.7,0.5,0.2);
   glTexCoord2f(0,1); glVertex3f(-0.7,0,0.2);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-0.7,0,-0.2);
   glTexCoord2f(1,0); glVertex3f(0.7,0,-0.2);
   glTexCoord2f(1,1); glVertex3f(0.7,0,0.2);
   glTexCoord2f(0,1); glVertex3f(-0.7,0,0.2);
   // Very Top Right
   //Planar vector 1 (p1) = v3 - v2 = (0.1,0.8,-0.2) - (0.1,0.8,0.2) = (0, 0, -0.4)
   //Planar vector 2 (p2) = v2 - v1 = (0.1,0.8,0.2) - (0.5,0.7,0.2) = (-0.4, 0.1, 0)
   glNormal3f(0.04, 0.16, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(0.5,0.7,-0.2);
   glTexCoord2f(1,0); glVertex3f(0.1,0.8,-0.2);
   glTexCoord2f(1,1); glVertex3f(0.1,0.8,0.2);
   glTexCoord2f(0,1); glVertex3f(0.5,0.7,0.2);

   //  Top
   glNormal3f(0, +1, 0); //Set the normal to +y
   glTexCoord2f(0,0); glVertex3f(0.1,0.8,-0.2);
   glTexCoord2f(1,0); glVertex3f(-0.1,0.8,-0.2);
   glTexCoord2f(1,1); glVertex3f(-0.1,0.8,0.2);
   glTexCoord2f(0,1); glVertex3f(0.1,0.8,0.2);

   // Top Left
   glNormal3f(-0.08, 0.08, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(-0.7,0.5,-0.2);
   glTexCoord2f(1,0); glVertex3f(-0.5,0.7,-0.2);
   glTexCoord2f(1,1); glVertex3f(-0.5,0.7,0.2);
   glTexCoord2f(0,1); glVertex3f(-0.7,0.5,0.2);

   // Very Top Left
   //Planar vector 1 (p1) = v3 - v2 = (-0.5,0.7,-0.2) - (-0.5,0.7,0.2) = (0, 0, -0.4)
   //Planar vector 2 (p2) = v2 - v1 = (-0.5,0.7,0.2) - (-0.1,0.8,0.2) = (-0.4, -0.1, 0)
   glNormal3f(-0.04, 0.16, 0); //Set the normal
   glTexCoord2f(0,0); glVertex3f(-0.1,0.8,-0.2);
   glTexCoord2f(1,0); glVertex3f(-0.5,0.7,-0.2);
   glTexCoord2f(1,1); glVertex3f(-0.5,0.7,0.2);
   glTexCoord2f(0,1); glVertex3f(-0.1,0.8,0.2);
   glEnd();


   glBindTexture(GL_TEXTURE_2D,texture[21]); //Mailbox Hole
   glBegin(GL_QUADS);
   // Top Right
   //Planar vector 1 (p1) = v3 - v2 = (0.5,0.7,-0.2) - (0.5,0.7,0.2) = (0, 0, -0.4)
   //Planar vector 2 (p2) = v2 - v1 = (0.5,0.7,0.2) - (0.7,0.5,0.2) = (-0.2, 0.2, 0)
   glNormal3f(0.08, 0.08, 0); //Set the normal
   glTexCoord2f(1,0); glVertex3f(0.7,0.5,-0.2);
   glTexCoord2f(1,1); glVertex3f(0.5,0.7,-0.2);
   glTexCoord2f(0,1); glVertex3f(0.5,0.7,0.2);
   glTexCoord2f(0,0); glVertex3f(0.7,0.5,0.2);
   glEnd();


   glBindTexture(GL_TEXTURE_2D,texture[22]); //Mailbox Sides
   // Front
   glBegin(GL_POLYGON);
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0, 0.625); glVertex3f(-0.7,0.5,0.2);
   glTexCoord2f(0.142, 0.875); glVertex3f(-0.5,0.7,0.2);
   glTexCoord2f(0.428,1); glVertex3f(-0.1,0.8,0.2);
   glTexCoord2f(0.571, 1); glVertex3f(0.1,0.8,0.2);
   glTexCoord2f(0.857, 0.875); glVertex3f(0.5,0.7,0.2);
   glTexCoord2f(1, 0.625); glVertex3f(0.7,0.5,0.2);
   glTexCoord2f(1, 0); glVertex3f(0.7,0,0.2);
   glTexCoord2f(0,0); glVertex3f(-0.7,0,0.2);
   glEnd();

   //Back
   glBegin(GL_POLYGON);
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0, 0.625); glVertex3f(-0.7,0.5,-0.2);
   glTexCoord2f(0.142, 0.875); glVertex3f(-0.5,0.7,-0.2);
   glTexCoord2f(0.428,1); glVertex3f(-0.1,0.8,-0.2);
   glTexCoord2f(0.571, 1); glVertex3f(0.1,0.8,-0.2);
   glTexCoord2f(0.857, 0.875); glVertex3f(0.5,0.7,-0.2);
   glTexCoord2f(1, 0.625); glVertex3f(0.7,0.5,-0.2);
   glTexCoord2f(1, 0); glVertex3f(0.7,0,-0.2);
   glTexCoord2f(0,0); glVertex3f(-0.7,0,-0.2);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of an entrance/exit sign, allowing it to be scaled, translated, and rotated around the y-axis
static void entranceExitSign(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Left fencing
   cylinder(-1.5,0,0, 0.3,1,0.3, 0,0, 0); //Tree Trunk
   cylinder(-1.3,0,0, 0.3,0.9,0.3, 0,0, 0); //Tree Trunk
   cylinder(-1.1,0,0, 0.3,1,0.3, 0,0, 0); //Tree Trunk

   //Right fencing
   cylinder(1.5,0,0, 0.3,1,0.3, 0,0, 0); //Tree Trunk
   cylinder(1.3,0,0, 0.3,0.9,0.3, 0,0, 0); //Tree Trunk
   cylinder(1.1,0,0, 0.3,1,0.3, 0,0, 0); //Tree Trunk

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   //  Enable blending
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glBindTexture(GL_TEXTURE_2D,texture[28]); //Exit
   glBegin(GL_QUADS);
   // Back of Sign
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,1); glVertex3f(-1,0.8, -0.01);
   glTexCoord2f(1,1); glVertex3f(1,0.8, -0.01);
   glTexCoord2f(1,0); glVertex3f(1,0.05, -0.01);
   glTexCoord2f(0,0); glVertex3f(-1,0.05,-0.01);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[27]); //Entrance
   glBegin(GL_QUADS);
   // Front of Sign
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-1,0.8, 0);
   glTexCoord2f(1,1); glVertex3f(1,0.8, 0);
   glTexCoord2f(1,0); glVertex3f(1,0.05, 0);
   glTexCoord2f(0,0); glVertex3f(-1,0.05,0);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
}

//Function to draw one instance of a small building (generic, bakery, pizza shop, or drugstore), allowing it to be scaled, translated, and rotated around the y-axis
//Type: 0 = Generic Building, 1 = Bakery, 2 = Pizza Shop, 3 = Drugstore
//Mode (only for use with generic buildings): 0 = Red Bricks, 1 = Orange Bricks, 2 = White Bricks, 3 = Yellow Bricks
static void smallBuilding(double x,double y,double z, double dx,double dy,double dz, double th, int type, int mode)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   if (type == 0) { //Generic Building
      //Back Windows
      buildingWindow(0.9,0.6,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
      buildingWindow(0,0.6,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
      buildingWindow(-0.9,0.6,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

      buildingWindow(0.9,1.6,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
      buildingWindow(0,1.6,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
      buildingWindow(-0.9,1.6,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

      //Front Windows
      buildingWindow(0.9,0.6,0.55, 0.2,0.25,0.2, 0); //Right Bottom Window
      buildingWindow(0,0.6,0.55, 0.2,0.25,0.2, 0); //Middle Bottom Window
      buildingWindow(-0.9,0.6,0.55, 0.2,0.25,0.2, 0); //Left Bottom Window

      buildingWindow(0.9,1.6,0.55, 0.2,0.25,0.2, 0); //Right Top Window
      buildingWindow(0,1.6,0.55, 0.2,0.25,0.2, 0); //Middle Top Window
      buildingWindow(-0.9,1.6,0.55, 0.2,0.25,0.2, 0); //Left Top Window

      buildingTop(0,2.3,0, 0.7,0.5,0.7, 0, 0);
      baseBuilding(0,0,0, 0.7,0.75,0.7, 0, mode);
   } else if (type == 1) { //Bakery
      //Back Windows
      buildingWindow(0.9,0.6,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
      buildingWindow(0,0.6,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
      buildingWindow(-0.9,0.6,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

      buildingWindow(0.9,1.6,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
      buildingWindow(0,1.6,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
      buildingWindow(-0.9,1.6,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

      //Front Windows
      buildingWindow(0.9,1.6,0.55, 0.2,0.25,0.2, 0); //Right Window
      buildingWindow(0,1.6,0.55, 0.2,0.25,0.2, 0); //Middle Window
      buildingWindow(-0.9,1.6,0.55, 0.2,0.25,0.2, 0); //Left Window

      businessEntrance(0,0,0.71, 0.5,0.4,0.5, 0, 1); //Entrance
      buildingTop(0,2.3,0, 0.7,0.5,0.7, 0, 0); //Top of Building
      baseBuilding(0,0,0, 0.7,0.75,0.7, 0, 0); //Building
   } else if (type == 2) { //Pizza Shop
      //Back Windows
      buildingWindow(0.9,0.6,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
      buildingWindow(0,0.6,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
      buildingWindow(-0.9,0.6,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

      buildingWindow(0.9,1.6,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
      buildingWindow(0,1.6,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
      buildingWindow(-0.9,1.6,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

      //Front Windows
      buildingWindow(0.9,1.6,0.55, 0.2,0.25,0.2, 0); //Right Window
      buildingWindow(-0.9,1.6,0.55, 0.2,0.25,0.2, 0); //Left Window

      pizzaSign(0.4,2.1,0.7, 1.5,1.5,0.3, 130); //Pizza Sign
      businessEntrance(0,0,0.71, 0.5,0.4,0.5, 0, 2); //Entrance
      buildingTop(0,2.3,0, 0.7,0.5,0.7, 0, 1); //Top of Building
      baseBuilding(0,0,0, 0.7,0.75,0.7, 0, 1); //Building
   } else if (type == 3) { //Drugstore
      //Back Windows
      buildingWindow(0.9,0.6,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
      buildingWindow(0,0.6,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
      buildingWindow(-0.9,0.6,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

      buildingWindow(0.9,1.6,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
      buildingWindow(0,1.6,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
      buildingWindow(-0.9,1.6,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

      //Front Windows
      buildingWindow(0.9,1.6,0.55, 0.2,0.25,0.2, 0); //Right Window
      buildingWindow(0,1.6,0.55, 0.2,0.25,0.2, 0); //Middle Window
      buildingWindow(-0.9,1.6,0.55, 0.2,0.25,0.2, 0); //Left Window

      businessEntrance(0,0,0.71, 0.5,0.4,0.5, 0, 3); //Entrance
      buildingTop(0,2.3,0, 0.7,0.5,0.7, 0, 1); //Top of Building
      baseBuilding(0,0,0, 0.7,0.75,0.7, 0, 0); //Building
   }
   //  Undo transformations
   glPopMatrix();
}

//Function to draw one instance of a tall building (generic, generic with door, or hotel), allowing it to be scaled, translated, and rotated around the y-axis
//Type: 0 = Generic Building, 1 = Generic Building w/ Door, 2 = Hotel
//Mode (for generic buildings): 0 = Red Bricks, 1 = Orange Bricks, 2 = White Bricks, 3 = Yellow Bricks
//Mode (for generic buildings w/ door): 0 = Red Building, 1 = White Building, 2 = Yellow Building
static void tallBuilding(double x,double y,double z, double dx,double dy,double dz, double th, int type, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   if (type == 0) { //Generic Building
      //Back Windows
      buildingWindow(0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
      buildingWindow(0,0.5,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
      buildingWindow(-0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

      buildingWindow(0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Right Middle Window
      buildingWindow(0,1.4,-0.7, 0.2,0.25,0.05, 0); //Middle Middle Window
      buildingWindow(-0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Left Middle Window

      buildingWindow(0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
      buildingWindow(0,2.3,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
      buildingWindow(-0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

      //Front Windows
      buildingWindow(0.9,0.5,0.55, 0.2,0.25,0.2, 0); //Right Bottom Window
      buildingWindow(0,0.5,0.55, 0.2,0.25,0.2, 0); //Middle Bottom Window
      buildingWindow(-0.9,0.5,0.55, 0.2,0.25,0.2, 0); //Left Bottom Window

      buildingWindow(0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Right Middle Window
      buildingWindow(0,1.4,0.55, 0.2,0.25,0.2, 0); //Middle Middle Window
      buildingWindow(-0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Left Middle Window

      buildingWindow(0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Right Top Window
      buildingWindow(0,2.3,0.55, 0.2,0.25,0.2, 0); //Middle Top Window
      buildingWindow(-0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Left Top Window

      buildingTop(0,3.01,0, 0.73,0.5,0.7, 0, 0); //Top of Building
      baseBuilding(0,0,0, 0.7,1,0.7, 0, mode); //Building
   } else if (type == 1) { //With Door
      if (mode == 0) { //Red Building
         //Back Windows
         buildingWindow(0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
         buildingWindow(0,0.5,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
         buildingWindow(-0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

         buildingWindow(0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Right Middle Window
         buildingWindow(0,1.4,-0.7, 0.2,0.25,0.05, 0); //Middle Middle Window
         buildingWindow(-0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Left Middle Window

         buildingWindow(0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
         buildingWindow(0,2.3,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
         buildingWindow(-0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

         //Front Windows
         buildingWindow(0.9,0.5,0.55, 0.2,0.25,0.2, 0); //Right Bottom Window

         buildingWindow(0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Right Middle Window
         buildingWindow(0,1.4,0.55, 0.2,0.25,0.2, 0); //Middle Middle Window
         buildingWindow(-0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Left Middle Window

         buildingWindow(0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Right Top Window
         buildingWindow(0,2.3,0.55, 0.2,0.25,0.2, 0); //Middle Top Window
         buildingWindow(-0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Left Top Window

         buildingEntrance(-0.3,0,0.71, 0.7,0.45,0.5, 0, 0); //Building Entrance
         buildingTop(0,3.01,0, 0.73,0.5,0.7, 0, 0); //Top of Building
         baseBuilding(0,0,0, 0.7,1,0.7, 0, 0); //Building
      } else if (mode == 1) { //White Building
         //Back Windows
         buildingWindow(0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
         buildingWindow(0,0.5,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
         buildingWindow(-0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

         buildingWindow(0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Right Middle Window
         buildingWindow(0,1.4,-0.7, 0.2,0.25,0.05, 0); //Middle Middle Window
         buildingWindow(-0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Left Middle Window

         buildingWindow(0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
         buildingWindow(0,2.3,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
         buildingWindow(-0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

         //Front Windows
         buildingWindow(0.9,0.5,0.55, 0.2,0.25,0.2, 0); //Right Bottom Window

         buildingWindow(0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Right Middle Window
         buildingWindow(0,1.4,0.55, 0.2,0.25,0.2, 0); //Middle Middle Window
         buildingWindow(-0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Left Middle Window

         buildingWindow(0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Right Top Window
         buildingWindow(0,2.3,0.55, 0.2,0.25,0.2, 0); //Middle Top Window
         buildingWindow(-0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Left Top Window

         buildingEntrance(-0.3,0,0.71, 0.7,0.45,0.5, 0, 1); //Building Entrance
         buildingTop(0,3.01,0, 0.73,0.5,0.7, 0, 3); //Top of Building
         baseBuilding(0,0,0, 0.7,1,0.7, 0, 2); //Building
      } else if (mode == 2) { //Yellow Building
         //Back Windows
         buildingWindow(0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
         buildingWindow(0,0.5,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
         buildingWindow(-0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

         buildingWindow(0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Right Middle Window
         buildingWindow(0,1.4,-0.7, 0.2,0.25,0.05, 0); //Middle Middle Window
         buildingWindow(-0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Left Middle Window

         buildingWindow(0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
         buildingWindow(0,2.3,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
         buildingWindow(-0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

         //Front Windows
         buildingWindow(0.9,0.5,0.55, 0.2,0.25,0.2, 0); //Right Bottom Window

         buildingWindow(0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Right Middle Window
         buildingWindow(0,1.4,0.55, 0.2,0.25,0.2, 0); //Middle Middle Window
         buildingWindow(-0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Left Middle Window

         buildingWindow(0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Right Top Window
         buildingWindow(0,2.3,0.55, 0.2,0.25,0.2, 0); //Middle Top Window
         buildingWindow(-0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Left Top Window

         buildingEntrance(-0.3,0,0.71, 0.7,0.45,0.5, 0, 2); //Building Entrance
         buildingTop(0,3.01,0, 0.73,0.5,0.7, 0, 0); //Top of Building
         baseBuilding(0,0,0, 0.7,1,0.7, 0, 3); //Building
      }
   } else if (type == 2) { //Hotel
         //Back Windows
         buildingWindow(0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
         buildingWindow(0,0.5,-0.7, 0.2,0.25,0.05, 0); //Middle Bottom Window
         buildingWindow(-0.9,0.5,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

         buildingWindow(0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Right Middle Window
         buildingWindow(0,1.4,-0.7, 0.2,0.25,0.05, 0); //Middle Middle Window
         buildingWindow(-0.9,1.4,-0.7, 0.2,0.25,0.05, 0); //Left Middle Window

         buildingWindow(0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
         buildingWindow(0,2.3,-0.7, 0.2,0.25,0.05, 0); //Middle Top Window
         buildingWindow(-0.9,2.3,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

         //Front Windows
         buildingWindow(0.9,0.5,0.55, 0.2,0.25,0.2, 0); //Right Bottom Window
         buildingWindow(0.9,1.4,0.55, 0.2,0.25,0.2, 0); //Right Middle Window
         buildingWindow(0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Right Top Window
         buildingWindow(0,2.3,0.55, 0.2,0.25,0.2, 0); //Middle Top Window
         buildingWindow(-0.9,2.3,0.55, 0.2,0.25,0.2, 0); //Left Top Window

         buildingEntrance(-0.3,0,0.71, 0.7,0.45,0.5, 0, 0); //Building Entrance
         buildingTop(0,3.01,0, 0.73,0.5,0.7, 0, 1); //Top of Building
         baseBuilding(0,0,0, 0.7,1,0.7, 0, 0); //Building

         //  Enable textures
         glEnable(GL_TEXTURE_2D);
         glBindTexture(GL_TEXTURE_2D,texture[57]); //Hotel Sign
         glBegin(GL_QUADS);
         //  Front
         glNormal3f(0, 0, 1); //Set the normal to +z
         glTexCoord2f(0,1); glVertex3f(-1.2,1.6,0.71);
         glTexCoord2f(1,1); glVertex3f(0.4,1.6,0.71);
         glTexCoord2f(1,0); glVertex3f(0.4,1.1,0.71);
         glTexCoord2f(0,0); glVertex3f(-1.2,1.1,0.71);
         glEnd();
   }
   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of the police station, allowing it to be scaled, translated, and rotated around the y-axis
static void policeStation(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Roof
   cube(0,3.5,0, 1.5,0.05,0.9, 0, 16);
   cube(0,3.4,0, 1.3,0.05,0.9, 0, 16);
   cube(0,3.3,0, 1,0.1,0.7, 0, 16);
   cube(0,3.2,0, 1.3,0.05,0.9, 0, 16);
   cube(0,3.1,0, 1.2,0.05,0.9, 0, 16);
   cube(0,3,0, 1,0.05,0.9, 0, 16);

   //Back Windows
   buildingWindow(0.5,0.5,-0.7, 0.2,0.25,0.05, 0); //Right Bottom Window
   buildingWindow(-0.5,0.5,-0.7, 0.2,0.25,0.05, 0); //Left Bottom Window

   buildingWindow(0.5,1.4,-0.7, 0.2,0.25,0.05, 0); //Right Middle Window
   buildingWindow(-0.5,1.4,-0.7, 0.2,0.25,0.05, 0); //Left Middle Window

   buildingWindow(0.5,2.3,-0.7, 0.2,0.25,0.05, 0); //Right Top Window
   buildingWindow(-0.5,2.3,-0.7, 0.2,0.25,0.05, 0); //Left Top Window

   //Front Windows
   buildingWindow(0.5,2.2,0.55, 0.2,0.25,0.2, 0); //Right Top Window
   buildingWindow(-0.5,2.2,0.55, 0.2,0.25,0.2, 0); //Left Top Window

   column(-0.9,0,-0.6, 1,1.2,1, 0,0, 2); //Left Back Column
   column(0.9,0,-0.6, 1,1.2,1, 0,0, 2); //Right Back Column

   column(-0.9,0,0.6, 1,1.2,1, 0,0, 2); //Left Front Column
   column(0.9,0,0.6, 1,1.2,1, 0,0, 2); //Right Front Column

   buildingEntrance(0,0,0.71, 0.7,0.6,0.5, 0, 2); //Building Entrance
   baseBuilding(0,0,0, 0.5,1,0.7, 0, 3); //Building

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,texture[59]); //Police Sign
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-0.8,1.9,0.71);
   glTexCoord2f(1,1); glVertex3f(0.8,1.9,0.71);
   glTexCoord2f(1,0); glVertex3f(0.8,1.4,0.71);
   glTexCoord2f(0,0); glVertex3f(-0.8,1.4,0.71);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of long building (in the top left corner), allowing it to be scaled, translated, and rotated around the y-axis
static void longBuilding(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //Right + Left Windows
   cube(1.98,0.7,-0.5, 0.2,0.1,0.05, 90, 3); // Right Side Top Right Window
   cube(1.98,0.7,0.5, 0.2,0.1,0.05, 90, 3); // Right Side Top Left Window
   cube(1.98,0.3,-0.5, 0.2,0.1,0.05, 90, 3); // Right Side Bottom Right Window
   cube(1.98,0.3,0.5, 0.2,0.1,0.05, 90, 3); // Right Side Bottom Left Window

   cube(-1.98,0.7,-0.5, 0.2,0.1,0.05, 90, 3); // Left Side Top Right Window
   cube(-1.98,0.7,0.5, 0.2,0.1,0.05, 90, 3); // Left Side Top Left Window
   cube(-1.98,0.3,-0.5, 0.2,0.1,0.05, 90, 3); // Left Side Bottom Right Window
   cube(-1.98,0.3,0.5, 0.2,0.1,0.05, 90, 3); // Left Side Bottom Left Window

   //Back Windows
   buildingWindow(-1.6,0.5,-0.95, 0.15,0.25,0.1, 0); //Very Left Window
   buildingWindow(-1,0.5,-0.95, 0.15,0.25,0.1, 0); //Left Window
   buildingWindow(-0.4,0.5,-0.95, 0.15,0.25,0.1, 0); //Left of Middle Window
   buildingWindow(0.4,0.5,-0.95, 0.15,0.25,0.1, 0); //Right of Middle Window
   buildingWindow(1.6,0.5,-0.95, 0.15,0.25,0.1, 0); //Very Right Window
   buildingWindow(1,0.5,-0.95, 0.15,0.25,0.1, 0); //Right Window

   //Front Windows
   buildingWindow(-1.6,0.5,0.95, 0.15,0.25,0.1, 0); //Very Left Window
   buildingWindow(-1,0.5,0.95, 0.15,0.25,0.1, 0); //Left of Door Window

   buildingWindow(1.6,0.5,0.95, 0.15,0.25,0.1, 0); //Very Right Window
   buildingWindow(1,0.5,0.95, 0.15,0.25,0.1, 0); //Right of Door Window


   buildingEntrance(0,0,1.01, 0.6,0.45,0.5, 0, 1); //Entrance

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[36]); //White Bricks
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-2, 0, 1);
   glTexCoord2f(1,0); glVertex3f(+2, 0, 1);
   glTexCoord2f(1,1); glVertex3f(+2,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-2,+1, 1);
   //  Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,0); glVertex3f(+2, 0.0,-1);
   glTexCoord2f(1,0); glVertex3f(-2, 0.0,-1);
   glTexCoord2f(1,1); glVertex3f(-2,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+2,+1,-1);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-2,0.0,-1);
   glTexCoord2f(1,0); glVertex3f(+2,0.0,-1);
   glTexCoord2f(1,1); glVertex3f(+2,0.0,+1);
   glTexCoord2f(0,1); glVertex3f(-2,0.0,+1);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[15]); //White House
   //  Right Side of House
   glBegin(GL_POLYGON);
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(1, 0); glVertex3f(+2,0.0,-1);
   glTexCoord2f(1, 0.5); glVertex3f(+2,+1,-1);
   glTexCoord2f(0.5, 1); glVertex3f(+2,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(+2,1,+1);
   glTexCoord2f(0,0); glVertex3f(+2,0.0,+1);
   glEnd();

   //  Left Side of House
   glBegin(GL_POLYGON);
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(1, 0); glVertex3f(-2,0.0,+1);
   glTexCoord2f(1, 0.5); glVertex3f(-2,+1,+1);
   glTexCoord2f(0.5, 1); glVertex3f(-2,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(-2,+1,-1);
   glTexCoord2f(0,0); glVertex3f(-2,0.0,-1);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[26]); //Blue Roof
   //Draw the rectangular sides of the prism
   glBegin(GL_QUADS);
   //Top Back Side:
   //Planar vector 1 (p1) = v3 - v2 = (2, 2, 0) - (2, 1, -1) = (0, 1, 1)
   //Planar vector 2 (p2) = v2 - v1 = (2, 1, -1) - (-2, 1, -1) = (4, 0, 0)
   glNormal3f(0, 4, -4); //Calculate normal
   glTexCoord2f(1,1); glVertex3f(-2, 2, 0); //v4
   glTexCoord2f(0,1); glVertex3f(2, 2, 0); //v3
   glTexCoord2f(0,0); glVertex3f(2, 1, -1); //v2
   glTexCoord2f(1,0); glVertex3f(-2, 1, -1); //v1

   //Top Front Side:
   //Planar vector 1 (p1) = v3 - v2 = (-2, 2, 0) - (-2, 1, 1) = (0, 1, -1)
   //Planar vector 2 (p2) = v2 - v1 = (-2, 1, 1) - (2, 1, 1) = (-2, 0, 0)
   glNormal3f(0, 4, 4); //Calculate normal
   glTexCoord2f(0,0); glVertex3f(2, 2, 0); //v4
   glTexCoord2f(1,0); glVertex3f(-2, 2, 0); //v3
   glTexCoord2f(1,1); glVertex3f(-2, 1, 1); //v2
   glTexCoord2f(0,1); glVertex3f(2, 1, 1); //v1
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of the smaller long building (in the top left corner), allowing it to be scaled, translated, and rotated around the y-axis
static void smallLongBuilding(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);
   //Right + Left Windows
   cube(1.5,0.7,-0.5, 0.2,0.1,0.05, 90, 3); // Right Side Top Right Window
   cube(1.5,0.7,0.5, 0.2,0.1,0.05, 90, 3); // Right Side Top Left Window
   cube(1.5,0.3,-0.5, 0.2,0.1,0.05, 90, 3); // Right Side Bottom Right Window
   cube(1.5,0.3,0.5, 0.2,0.1,0.05, 90, 3); // Right Side Bottom Left Window

   cube(-1.5,0.7,-0.5, 0.2,0.1,0.05, 90, 3); // Left Side Top Right Window
   cube(-1.5,0.7,0.5, 0.2,0.1,0.05, 90, 3); // Left Side Top Left Window
   cube(-1.5,0.3,-0.5, 0.2,0.1,0.05, 90, 3); // Left Side Bottom Right Window
   cube(-1.5,0.3,0.5, 0.2,0.1,0.05, 90, 3); // Left Side Bottom Left Window

   //Back Windows
   buildingWindow(-1.2,0.5,-0.95, 0.15,0.25,0.1, 0); //Very Left Window
   buildingWindow(-0.6,0.5,-0.95, 0.15,0.25,0.1, 0); //Left Window
   buildingWindow(0,0.5,-0.95, 0.15,0.25,0.1, 0); //Middle Window
   buildingWindow(1.2,0.5,-0.95, 0.15,0.25,0.1, 0); //Very Right Window
   buildingWindow(0.6,0.5,-0.95, 0.15,0.25,0.1, 0); //Right Window

   //Front Windows
   buildingWindow(-1,0.5,0.95, 0.15,0.25,0.1, 0); //Left of Door Window
   buildingWindow(1,0.5,0.95, 0.15,0.25,0.1, 0); //Right of Door Window

   buildingEntrance(0,0,1.01, 0.6,0.45,0.5, 0, 1); //Entrance

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[36]); //White Bricks
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-1.5, 0, 1);
   glTexCoord2f(1,0); glVertex3f(+1.5, 0, 1);
   glTexCoord2f(1,1); glVertex3f(+1.5,+1, 1);
   glTexCoord2f(0,1); glVertex3f(-1.5,+1, 1);
   //  Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,0); glVertex3f(+1.5, 0.0,-1);
   glTexCoord2f(1,0); glVertex3f(-1.5, 0.0,-1);
   glTexCoord2f(1,1); glVertex3f(-1.5,+1,-1);
   glTexCoord2f(0,1); glVertex3f(+1.5,+1,-1);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,0); glVertex3f(-1.5,0.0,-1);
   glTexCoord2f(1,0); glVertex3f(+1.5,0.0,-1);
   glTexCoord2f(1,1); glVertex3f(+1.5,0.0,+1);
   glTexCoord2f(0,1); glVertex3f(-1.5,0.0,+1);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[15]); //White House
   //  Right Side of House
   glBegin(GL_POLYGON);
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(1, 0); glVertex3f(+1.5,0.0,-1);
   glTexCoord2f(1, 0.5); glVertex3f(+1.5,+1,-1);
   glTexCoord2f(0.5, 1); glVertex3f(+1.5,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(+1.5,1,+1);
   glTexCoord2f(0,0); glVertex3f(+1.5,0.0,+1);
   glEnd();

   //  Left Side of House
   glBegin(GL_POLYGON);
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(1, 0); glVertex3f(-1.5,0.0,+1);
   glTexCoord2f(1, 0.5); glVertex3f(-1.5,+1,+1);
   glTexCoord2f(0.5, 1); glVertex3f(-1.5,+2,0.0);
   glTexCoord2f(0, 0.5); glVertex3f(-1.5,+1,-1);
   glTexCoord2f(0,0); glVertex3f(-1.5,0.0,-1);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[12]); //Purple Roof
   //Draw the rectangular sides of the prism
   glBegin(GL_QUADS);
   //Top Back Side:
   //Planar vector 1 (p1) = v3 - v2 = (2, 2, 0) - (2, 1, -1) = (0, 1, 1)
   //Planar vector 2 (p2) = v2 - v1 = (2, 1, -1) - (-2, 1, -1) = (4, 0, 0)
   glNormal3f(0, 4, -4); //Calculate normal
   glTexCoord2f(1,1); glVertex3f(-1.5, 2, 0); //v4
   glTexCoord2f(0,1); glVertex3f(1.5, 2, 0); //v3
   glTexCoord2f(0,0); glVertex3f(1.5, 1, -1); //v2
   glTexCoord2f(1,0); glVertex3f(-1.5, 1, -1); //v1

   //Top Front Side:
   //Planar vector 1 (p1) = v3 - v2 = (-2, 2, 0) - (-2, 1, 1) = (0, 1, -1)
   //Planar vector 2 (p2) = v2 - v1 = (-2, 1, 1) - (2, 1, 1) = (-2, 0, 0)
   glNormal3f(0, 4, 4); //Calculate normal
   glTexCoord2f(0,0); glVertex3f(1.5, 2, 0); //v4
   glTexCoord2f(1,0); glVertex3f(-1.5, 2, 0); //v3
   glTexCoord2f(1,1); glVertex3f(-1.5, 1, 1); //v2
   glTexCoord2f(0,1); glVertex3f(1.5, 1, 1); //v1
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of the town hall, allowing it to be scaled, translated, and rotated around the y-axis
static void townHall(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   triangular_prism(0,3.15,0, 1.95,1,1.2, 0, 4); //Top Triangle

   //Roof
   cube(0,3.1,0, 3.5,0.05,0.8, 0, 17);
   cube(0,3,0, 3.3,0.05,0.8, 0, 17);
   cube(0,2.9,0, 3,0.1,0.6, 0, 17);
   cube(0,2.8,0, 3.3,0.05,0.8, 0, 17);
   cube(0,2.7,0, 3.2,0.05,0.8, 0, 17);
   cube(0,2.6,0, 3,0.05,0.8, 0, 17);

   cube(0,3.1,0, 1.9,0.05,1.2, 0, 17);
   cube(0,3,0, 1.7,0.05,1.2, 0, 17);
   cube(0,2.9,0, 1.4,0.1,1, 0, 17);
   cube(0,2.8,0, 1.7,0.05,1.2, 0, 17);
   cube(0,2.7,0, 1.6,0.05,1.2, 0, 17);
   cube(0,2.6,0, 1.4,0.05,1.2, 0, 17);

   //Back Windows
   buildingWindow(2.5,1,-0.55, 0.25,0.3,0.2, 0); //Right Bottom Window
   buildingWindow(1.6,1,-0.55, 0.25,0.3,0.2, 0); //Right Bottom Window

   buildingWindow(2.5,2,-0.55, 0.25,0.3,0.2, 0); //Right Top Window
   buildingWindow(1.6,2,-0.55, 0.25,0.3,0.2, 0); //Right Top Window

   buildingWindow(0.8,2,-0.55, 0.25,0.3,0.2, 0); //Left Top Window
   buildingWindow(-0.8,2,-0.55, 0.25,0.3,0.2, 0); //Left Top Window

   buildingWindow(0,2,-0.55, 0.25,0.3,0.2, 0); //Left Bottom Window
   buildingWindow(0,1,-0.55, 0.25,0.3,0.2, 0); //Left Bottom Window

   buildingWindow(0.8,1,-0.55, 0.25,0.3,0.2, 0); //Left Bottom Window
   buildingWindow(-0.8,1,-0.55, 0.25,0.3,0.2, 0); //Left Bottom Window

   buildingWindow(-2.5,1,-0.55, 0.25,0.3,0.2, 0); //Left Bottom Window
   buildingWindow(-1.6,1,-0.55, 0.25,0.3,0.2, 0); //Left Bottom Window

   buildingWindow(-2.5,2,-0.55, 0.25,0.3,0.2, 0); //Left Top Window
   buildingWindow(-1.6,2,-0.55, 0.25,0.3,0.2, 0); //Left Top Window

   //Front Windows
   buildingWindow(2.5,1,0.55, 0.25,0.3,0.2, 0); //Right Bottom Window
   buildingWindow(1.8,1,0.55, 0.25,0.3,0.2, 0); //Right Bottom Window

   buildingWindow(2.5,2,0.55, 0.25,0.3,0.2, 0); //Right Top Window
   buildingWindow(1.8,2,0.55, 0.25,0.3,0.2, 0); //Right Top Window

   buildingWindow(-2.5,1,0.55, 0.25,0.3,0.2, 0); //Left Bottom Window
   buildingWindow(-1.8,1,0.55, 0.25,0.3,0.2, 0); //Left Bottom Window

   buildingWindow(-2.5,2,0.55, 0.25,0.3,0.2, 0); //Left Top Window
   buildingWindow(-1.8,2,0.55, 0.25,0.3,0.2, 0); //Left Top Window

   //Columns
   column(-1.2,0.6,1, 1,0.8,1, 0,0, 1); //Very Left Column
   column(-0.5,0.6,1, 1,0.8,1, 0,0, 1); //Left Column
   column(0.5,0.6,1, 1,0.8,1, 0,0, 1); //Right Column
   column(1.2,0.6,1, 1,0.8,1, 0,0, 1); //Very Right Column

   //Stairs:
   cube(0,0.5,1, 0.95,0.05,0.7, 0, 11);
   cube(0,0.4,1.2, 0.95,0.05,0.7, 0, 11);
   cube(0,0.3,1.4, 0.95,0.05,0.7, 0, 11);
   cube(0,0.2,1.6, 0.95,0.05,0.7, 0, 11);
   cube(0,0.1,1.8, 0.95,0.05,0.7, 0, 11);
   cube(0,0,2, 0.95,0.05,0.7, 0, 11);

   //Left Staircase Handle
   cube(-1.2,0.6,1.5, 0.25,0.05,0.25, 0, 17); //Top Square
   staircaseHandle(-1.2,0.15,2, 1,0.5,1.3, -90, 1); //Handle
   cube(-1.2,0.1,1.8, 0.25,0.1,1, 0, 17); //Bottom Part

   //Right Staircase Handle
   cube(1.2,0.6,1.5, 0.25,0.05,0.25, 0, 17); //Top Square
   staircaseHandle(1.2,0.15,2, 1,0.5,1.3, -90, 1); //Handle
   cube(1.2,0.1,1.8, 0.25,0.1,1, 0, 17); //Bottom Part

   cube(0,0.15,0.4, 3.3,0.15,1.1, 0, 17); //Very Bottom
   cube(0,0.4,0.4, 3,0.18,1.1, 0, 17); //Top of Bottom Floor
   baseBuilding(0,0.5,0, 1.5,0.7,0.7, 0, 2); //Building

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glBindTexture(GL_TEXTURE_2D,texture[64]); //Town Hall Sign
   glBegin(GL_QUADS);
   //  Sign
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-1.2,3.1,1.3);
   glTexCoord2f(1,1); glVertex3f(1.2,3.1,1.3);
   glTexCoord2f(1,0); glVertex3f(1.2,2.8,1.3);
   glTexCoord2f(0,0); glVertex3f(-1.2,2.8,1.3);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[63]); //Black Texture
   glBegin(GL_QUADS);
   //  Door
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-1,3,0.71);
   glTexCoord2f(1,1); glVertex3f(1,3,0.71);
   glTexCoord2f(1,0); glVertex3f(1,0.5,0.71);
   glTexCoord2f(0,0); glVertex3f(-1,0.5,0.71);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a wire fence, allowing it to be scaled, translated, and rotated around the y-axis
static void wireFence(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   cube(0,1.9,0, 2.2,0.1,0.2, 0, 10); //Top Part
   cube(-2.1,0.9,0, 0.1,0.9,0.2, 0, 10);
   cube(2.1,0.9,0, 0.1,0.9,0.2, 0, 10);
   cube(-1.3,0.9,0, 0.1,0.9,0.2, 0, 10);
   cube(1.3,0.9,0, 0.1,0.9,0.2, 0, 10);
   cube(0,0.9,0, 0.1,0.9,0.2, 0, 10);
   cube(0,0,0, 2.5,0.2,0.25, 0, 13); //Bottom Part

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);

   //  Enable blending
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glBindTexture(GL_TEXTURE_2D,texture[65]); //Wirefence
   //Door
   glBegin(GL_QUADS);
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,0); glVertex3f(-2,1.8,0);
   glTexCoord2f(1,0); glVertex3f(2,1.8,0);
   glTexCoord2f(1,1); glVertex3f(2,0,0);
   glTexCoord2f(0,1); glVertex3f(-2,0,0);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
   glDisable(GL_BLEND);
}

//Function to draw one instance of the arcade, allowing it to be scaled, translated, and rotated around the y-axis
static void arcade(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   snes(0,-0.05,0, 1,1,1, -40); //SNES System

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[68]); //Yellow Tile
   //  Cube
   glBegin(GL_QUADS);
   //  Front
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-1.5,2,1);
   glTexCoord2f(1,1); glVertex3f(1.5,2,1);
   glTexCoord2f(1,0); glVertex3f(1.5,0,1);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,1);
   //  Back
   glNormal3f(0, 0, -1); //Set the normal to -z
   glTexCoord2f(0,1); glVertex3f(-1.5,2,-1);
   glTexCoord2f(1,1); glVertex3f(1.5,2,-1);
   glTexCoord2f(1,0); glVertex3f(1.5,0,-1);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,-1);
   //  Right
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(0,1); glVertex3f(1.5,2,1);
   glTexCoord2f(1,1); glVertex3f(1.5,2,-1);
   glTexCoord2f(1,0); glVertex3f(1.5,0,-1);
   glTexCoord2f(0,0); glVertex3f(1.5,0,1);
   //  Left
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(0,1); glVertex3f(-1.5,2,1);
   glTexCoord2f(1,1); glVertex3f(-1.5,2,-1);
   glTexCoord2f(1,0); glVertex3f(-1.5,0,-1);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,1);
   //  Top
   glNormal3f(0, 1, 0); //Set the normal to +y
   glTexCoord2f(0,1); glVertex3f(-1.5,2,1);
   glTexCoord2f(1,1); glVertex3f(1.5,2,1);
   glTexCoord2f(1,0); glVertex3f(1.5,2,-1);
   glTexCoord2f(0,0); glVertex3f(-1.5,2,-1);
   //  Bottom
   glNormal3f(0, -1, 0); //Set the normal to -y
   glTexCoord2f(0,1); glVertex3f(-1.5,0,1);
   glTexCoord2f(1,1); glVertex3f(1.5,0,1);
   glTexCoord2f(1,0); glVertex3f(1.5,0,-1);
   glTexCoord2f(0,0); glVertex3f(-1.5,0,-1);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[66]); //Sign
   glBegin(GL_QUADS);
   //  Sign
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-1,1.4,1.01);
   glTexCoord2f(1,1); glVertex3f(1,1.4,1.01);
   glTexCoord2f(1,0); glVertex3f(1,1.1,1.01);
   glTexCoord2f(0,0); glVertex3f(-1,1.1,1.01);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[61]); //Door
   glBegin(GL_QUADS);
   //  Door
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-0.6,1,1.01);
   glTexCoord2f(1,1); glVertex3f(0.6,1,1.01);
   glTexCoord2f(1,0); glVertex3f(0.6,0,1.01);
   glTexCoord2f(0,0); glVertex3f(-0.6,0,1.01);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of the hospital, allowing it to be scaled, translated, and rotated around the y-axis
static void hospital(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Front Windows:
   for (double ft = -2; ft <= 2; ft+=0.4) { //Top Windows:
      hospitalWindow(ft,0.85,1, 0.1,0.2,0.05, 0);
   }
   for (double fm = -2; fm <= 2; fm+=0.4) { //Middle Windows:
      hospitalWindow(fm,0.15,1, 0.1,0.2,0.05, 0);
   }
   //Bottom Windows:
   hospitalWindow(-2,-0.55,1, 0.1,0.2,0.05, 0);
   hospitalWindow(-1.6,-0.55,1, 0.1,0.2,0.05, 0);
   hospitalWindow(2,-0.55,1, 0.1,0.2,0.05, 0);
   hospitalWindow(1.6,-0.55,1, 0.1,0.2,0.05, 0);

   //Back Windows:
   for (double bt = -2; bt <= 2; bt+=0.4) { //Top Windows:
      hospitalWindow(bt,0.85,-1, 0.1,0.2,0.05, 180);
   }
   for (double fm = -2; fm <= 2; fm+=0.4) { //Middle Windows:
      hospitalWindow(fm,0.15,-1, 0.1,0.2,0.05, 180);
   }
   for (double bb = -2; bb <= 2; bb+=0.4) { //Bottom Windows:
      hospitalWindow(bb,-0.55,-1, 0.1,0.2,0.05, 180);
   }

   cube(-0.7,-1,1, 0.25,0.5,0.29, 0, 21); //Left Column of Arch Sign
   cube(0.7,-1,1, 0.25,0.5,0.29, 0, 21); //Right Column of Arch Sign

   cube(0,-0.5,1, 1.3,0.2,0.3, 0, 17); //Top of Arch Sign

   cube(0,1.2,0, 2.3,0.04,1.1, 0, 11); //Top Separator
   cube(0,0.5,0, 2.3,0.04,1.1, 0, 11); //Middle Separator
   cube(0,-0.2,0, 2.3,0.04,1.1, 0, 11); //Bottom Separator
   cube(0,-0.9,-0.1, 2.3,0.04,1, 0, 11); //Back Bottom Separator

   buildingTop(1.3,2.11,0, 0.4,0.4,0.45, 0, 0); //Top of Small Building
   cube(1.3,1.8,0, 0.8,0.3,0.5, 0, 14); //Small Building
   buildingTop(0,1.51,0, 1.25,0.6,0.9, 0, 2); //Top of Main Building
   cube(-0.8,1.5,0.9, 1.4,0.3,0.1, 0, 17); //Sign
   cube(0,0,0, 2.5,1.5,1, 0, 17); //Main Building

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[77]); //Top Sign
   glBegin(GL_QUADS);
   //  Top Sign
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-2.1,1.7,1.01);
   glTexCoord2f(1,1); glVertex3f(0.5,1.7,1.01);
   glTexCoord2f(1,0); glVertex3f(0.5,1.3,1.01);
   glTexCoord2f(0,0); glVertex3f(-2.1,1.3,1.01);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[78]); //Entrance Sign
   glBegin(GL_QUADS);
   //  Top Sign
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-1.3,-0.3,1.31);
   glTexCoord2f(1,1); glVertex3f(1.3,-0.3,1.31);
   glTexCoord2f(1,0); glVertex3f(1.3,-0.7,1.31);
   glTexCoord2f(0,0); glVertex3f(-1.3,-0.7,1.31);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[61]); //Door
   glBegin(GL_QUADS);
   //  Door
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(1,1); glVertex3f(-0.5,-0.7,1.01);
   glTexCoord2f(0,1); glVertex3f(0.5,-0.7,1.01);
   glTexCoord2f(0,0); glVertex3f(0.5,-1.5,1.01);
   glTexCoord2f(1,0); glVertex3f(-0.5,-1.5,1.01);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of the burger shop, allowing it to be scaled, translated, and rotated around the y-axis
static void burgerShop(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Front Windows:
   cube(-1.9,-0.3,1, 0.3,0.3,0.1, 0, 23);
   cube(-0.9,-0.3,1, 0.3,0.3,0.1, 0, 23);
   cube(1.65,-0.3,1, 0.3,0.3,0.1, 0, 23);

   //Back  Windows:
   cube(-1.65,-0.3,-1, 0.3,0.3,0.1, 180, 23);
   cube(-0.65,-0.3,-1, 0.3,0.3,0.1, 180, 23);
   cube(0.65,-0.3,-1, 0.3,0.3,0.1, 180, 23);
   cube(1.65,-0.3,-1, 0.3,0.3,0.1, 180, 23);

   cube(0.3,0,1, 0.7,0.2,0.3, 0, 17); //Top of Doorway
   buildingTop(1.3,1.61,0, 0.4,0.4,0.45, 0, 0); //Top of Small Building
   cube(1.3,1.3,0, 0.8,0.3,0.5, 0, 21); //Small Building
   buildingTop(0,1.01,0, 1.25,0.6,0.9, 0, 2); //Top of Main Building
   cube(-0.8,1,0.91, 1.4,0.3,0.1, 0, 17); //Sign
   cube(0,0,0, 2.5,1,1, 0, 21); //Main Building
   burger(-0.7,1,-0.1, 2,2,2, 0);

   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[45]); //Top Sign
   glBegin(GL_QUADS);
   //  Top Sign
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(0,1); glVertex3f(-2.2,1.3,1.1);
   glTexCoord2f(1,1); glVertex3f(0.6,1.3,1.1);
   glTexCoord2f(1,0); glVertex3f(0.6,0.3,1.1);
   glTexCoord2f(0,0); glVertex3f(-2.2,0.3,1.1);
   glEnd();

   glBindTexture(GL_TEXTURE_2D,texture[61]); //Door
   glBegin(GL_QUADS);
   //  Door
   glNormal3f(0, 0, 1); //Set the normal to +z
   glTexCoord2f(1,0); glVertex3f(-0.3,-0.2,1.01);
   glTexCoord2f(0,0); glVertex3f(0.8,-0.2,1.01);
   glTexCoord2f(0,1); glVertex3f(0.8,-1,1.01);
   glTexCoord2f(1,1); glVertex3f(-0.3,-1,1.01);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of a street lamp, allowing it to be scaled, translated, and rotated around the y-axis
static void streetLamp(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Back:
   ball(0,2.25,-0.65, 0.15); //Light
   hollowHemiSphere(0,2.25,-0.65, 0.2,0.3,0.2, 0, 0); //Light Holder
   column(0,2.6,-0.5, 0.5,0.15,0.5, 180,90, 3); //Third Curve
   column(0,2.4,-0.2, 0.5,0.15,0.5, 180,60, 3); //Second Curve
   column(0,2,0,-0.5, 0.2,0.5, 180,30, 3); //First Curve

   //Front:
   ball(0,2.25,0.65, 0.15); //Light
   hollowHemiSphere(0,2.25,0.65, 0.2,0.3,0.2, 0, 0); //Light Holder
   column(0,2.6,0.5, 0.5,0.15,0.5, 0,90, 3); //Third Curve
   column(0,2.4,0.2, 0.5,0.15,0.5, 0,60, 3); //Second Curve
   column(0,2,0, 0.5,0.2,0.5, 0,30, 3); //First Curve

   column(0,0,0, 0.5,1,0.5, 0,0, 3); //Base
   cylinder(0,0,0, 0.4,0.1,0.4, 0,0, 12); //Very Bottom of Base

   //  Undo transformations and textures
   glPopMatrix();
}

//Function to draw one instance of a car, allowing it to be scaled, translated, and rotated around the y-axis
//Mode: 0 = Police Car, 1 = Red Car, 2 = Blue Car
static void car(double x,double y,double z, double dx,double dy,double dz, double th, int mode)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   roundCylinder(0.8,-0.3,-0.7, 0.4, 0.2, 0,0, 0); //Back Right Wheel
   roundCylinder(0.8,-0.3,0.7, 0.4, 0.2, 0,0, 0); //Back Left Wheel
   roundCylinder(-0.8,-0.3,-0.7, 0.4, 0.2, 0,0, 0); //Front Right Wheel
   roundCylinder(-0.8,-0.3,0.7, 0.4, 0.2, 0,0, 0); //Front Left Wheel
   cube(1.4,-0.2,0, 0.02,0.18,0.8, 0, 11); //Back Bottom Bumper
   cube(1.45,-0.1,0, 0.05,0.18,0.8, 0, 11); //Back Top Bumper
   cube(-1.45,-0.1,0, 0.05,0.18,0.8, 0, 11); //Front Bumper
   
   if (mode == 0) { //Police Car
      cube(0.5,1,-0.35, 0.2,0.1,0.15, 90, 29); //Police Light Blue
      cube(0.5,1,0, 0.15,0.1,0.15, 90, 25); //Police Light White
      cube(0.5,1,0.35, 0.2,0.1,0.15, -90, 27); //Police Light Red

      cube(0.05,-0.04,0, 0.4,0.2,0.82, 0, 24); //Door
      cylinder(-1.5,0.25,-0.65, 3,0.4,0.4, 90,90, 14); //Right Light
      cylinder(-1.5,0.25,0.65, 3,0.4,0.4, 90,90, 14); //Left Light
      carTop(0.45,0.4,0, 0.8,0.5,0.9, 0, 0); //Very Top of Car
      flatCarTop(0,0.3,0, 0.95,1,1.1, 0, 0); //Car Top
      cube(0,0,0, 1.4,0.3,0.8, 0, 25); //Car Body
   } else if (mode == 1) { //Red Car
      cube(0.05,-0.04,0, 0.4,0.2,0.82, 0, 26); //Door
      cylinder(-1.5,0.25,-0.65, 3,0.4,0.4, 90,90, 15); //Right Light
      cylinder(-1.5,0.25,0.65, 3,0.4,0.4, 90,90, 15); //Left Light
      carTop(0.45,0.4,0, 0.8,0.5,0.9, 0, 1); //Very Top of Car
      flatCarTop(0,0.3,0, 0.95,1,1.1, 0, 1); //Car Top
      cube(0,0,0, 1.4,0.3,0.8, 0, 27); //Car Body
   } else if (mode == 2) { //Blue Car
      cube(0.05,-0.04,0, 0.4,0.2,0.82, 0, 28); //Door
      cylinder(-1.5,0.25,-0.65, 3,0.4,0.4, 90,90, 16); //Right Light
      cylinder(-1.5,0.25,0.65, 3,0.4,0.4, 90,90, 16); //Left Light
      carTop(0.45,0.4,0, 0.8,0.5,0.9, 0, 2); //Very Top of Car
      flatCarTop(0,0.3,0, 0.95,1,1.1, 0, 2); //Car Top
      cube(0,0,0, 1.4,0.3,0.8, 0, 29); //Car Body
   }


   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[96]); //License Plate
   glBegin(GL_QUADS);
   //  Front License Plate
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(0,1); glVertex3f(-1.51,0,-0.2);
   glTexCoord2f(1,1); glVertex3f(-1.51,0,0.2);
   glTexCoord2f(1,0); glVertex3f(-1.51,-0.2,0.2);
   glTexCoord2f(0,0); glVertex3f(-1.51,-0.2,-0.2);
   glEnd();

   glBegin(GL_QUADS);
   //  Back License Plate
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(0,1); glVertex3f(1.51,0,0.2);
   glTexCoord2f(1,1); glVertex3f(1.51,0,-0.2);
   glTexCoord2f(1,0); glVertex3f(1.51,-0.2,-0.2);
   glTexCoord2f(0,0); glVertex3f(1.51,-0.2,0.2);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}

//Function to draw one instance of an ambulance, allowing it to be scaled, translated, and rotated around the y-axis
static void ambulance(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //Set specular color to white
   float white[] = {1,1,1,1};
   float Emission[]  = {0.0,0.0,0.01*emission,1.0};
   glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,shiny);
   glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
   glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,Emission);
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   roundCylinder(-1.2,-0.3,-0.7, 0.4, 0.2, 0,0, 0); //Back Right Wheel
   roundCylinder(-1.2,-0.3,0.7, 0.4, 0.2, 0,0, 0); //Back Left Wheel
   roundCylinder(0.7,-0.3,-0.6, 0.4, 0.2, 0,0, 0); //Front Right Wheel
   roundCylinder(0.7,-0.3,0.6, 0.4, 0.2, 0,0, 0); //Front Left Wheel

   cube(-2.3,-0.35,0, 0.05,0.1,0.85, 0, 11); //Back Bumper
   cube(1.5,-0.35,0, 0.05,0.1,0.65, 0, 11); //Front Bumper
   
   cube(0.55,1,-0.35, 0.2,0.1,0.15, 90, 27); //Red Light
   cube(0.55,1,0, 0.15,0.1,0.15, 90, 11); //Gray Light
   cube(0.55,1,0.35, 0.2,0.1,0.15, -90, 27); //Red Light

   cube(-1,0.5,0, 1.3,0.9,0.8, 0, 30); //Main Ambulance
   carTop(0.35,0.4,0, 0.8,0.5,0.9, 0, 3); //Top of Ambulance
   ambulanceFront(0.7,-0.4,0, 0.8,0.8,0.9, 0); //Front of Ambulance


   //  Enable textures
   glEnable(GL_TEXTURE_2D);
   glColor3f(1,1,1);
   glBindTexture(GL_TEXTURE_2D,texture[96]); //License Plate
   glBegin(GL_QUADS);
   //  Back License Plate
   glNormal3f(-1, 0, 0); //Set the normal to -x
   glTexCoord2f(0,1); glVertex3f(-2.4,-0.3,-0.2);
   glTexCoord2f(1,1); glVertex3f(-2.4,-0.3,0.2);
   glTexCoord2f(1,0); glVertex3f(-2.4,-0.5,0.2);
   glTexCoord2f(0,0); glVertex3f(-2.4,-0.5,-0.2);
   glEnd();

   glBegin(GL_QUADS);
   //  Front License Plate
   glNormal3f(1, 0, 0); //Set the normal to +x
   glTexCoord2f(0,1); glVertex3f(1.6,-0.3,0.2);
   glTexCoord2f(1,1); glVertex3f(1.6,-0.3,-0.2);
   glTexCoord2f(1,0); glVertex3f(1.6,-0.5,-0.2);
   glTexCoord2f(0,0); glVertex3f(1.6,-0.5,0.2);
   glEnd();

   //  Undo transformations and textures
   glPopMatrix();
   glDisable(GL_TEXTURE_2D);
}
//********************************************************************************************************************************************************************************



// Scene Functions: Functions that draw different, fully assembled, sections of the city or the entire city
//********************************************************************************************************************************************************************************
//Function to move the car around the top right section of the city
static void moveRedCar() {
   //Move the car in a square
   if (car1right <= 20) {
      car1right += 0.25;
      car(-9.5,0.4,car1right, 1,1,1, 90, 1);
   } else if (car1bottom >= -30) {
      car1bottom -= 0.25;
      car(car1bottom,0.4,20, 1,1,1, 0, 1);
   } else if (car1left >= -8) {
      car1left -= 0.25;
      car(-30,0.4,car1left, 1,1,1,-90, 1);
   }  else if (car1top <= -9.5) {
      car1top += 0.25;
      car(car1top,0.4,-8, 1,1,1,-180, 1);
   } else {
      car1right = -8;
      car1bottom = -9.5;
      car1left = 20;
      car1top = -30;
   }
   glutPostRedisplay(); // Tell GLUT it is necessary to redisplay the scene
}

//Function to move the police car around the bottom left section of the city
static void movePoliceCar() {
   //Move the car in a square
   if (car2top >= -8) {
      car2top -= 0.25;
      car(car2top,0.4,-11.5, 1,1,1, 0, 0);
   } else if (car2left <= 14) {
      car2left += 0.25;
      car(-8,0.4,car2left, 1,1,1, 90, 0);
   } else if (car2bottom <= 8) {
      car2bottom += 0.25;
      car(car2bottom,0.4,14, 1,1,1, -180, 0);
   } else if (car2right >= -11) {
      car2right -= 0.25;
      car(8,0.4,car2right, 1,1,1, -90, 0);
   } else {
      car2right = 14;
      car2bottom = -8;
      car2left = -11.5;
      car2top = 8;
   }
   glutPostRedisplay(); // Tell GLUT it is necessary to redisplay the scene
}

static void pizzaArcadeSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   for (float i = -6.25; i <= 6.25; i+=1.25) {
      cube(i,-0.25,10, 0.6,0.05,0.6, 0, 11);
      cube(i,-0.25,-10, 0.6,0.05,0.6, 0, 11);  
   }
   for (float i = -8.75; i <= 8.75; i+=1.25) {
      cube(-6.3,-0.25,i, 0.6,0.05,0.6, 0, 11);
      cube(6.3,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }
   //Grass
   cube(0,-0.3,5.3, 6.8,0.05,5.3, 0, 32);
   cube(0,-0.3,-5.3, 6.8,0.05,5.3, 0, 32);

   //Objects
   arcade(3.8,-0.3,8, 1.3,1.3,1.3, 0);
   smallBuilding(-1.8,-0.3,8, 1.6,1.6,1.6, 0, 2,0);
   cube(-3,-0.15,9.5, 0.4,0.1,0.4, 0, 15);
   flowersGrass(-3,0.2,9.3, 0.3,0.3,0.3, 0, 3);
   mailbox(-5,-0.1,10, 0.3,0.5,0.7, -90);
   stopSign(1,-0.2,-10, 1,1,1, -90);
   streetLamp(-6.3,-0.25,-9.8, 1,1,1, -45);
   car(0,0.4,11.5, 1,1,1, 0, 1);

   //If the user is viewing fullscreen, animate the car. Otherwise, have it stay still.
   if (obj == 12) movePoliceCar();
   else car(-2,0.4,-11.5, 1,1,1, 0, 0);

   //Front Fences
   woodFenceChain(2,-0.33,6, 1.5,1.5,1.5, 0);
   woodFenceChain(-2,-0.33,6, 1.5,1.5,1.5, 0);
   
   //Back Fences
   woodFenceChain(2,-0.33,-5.7, 1.5,1.5,1.5, 0);
   woodFenceChain(-2,-0.33,-5.7, 1.5,1.5,1.5, 0);

   //Left Fences
   woodFenceChain(-4.5,-0.33,-3.5, 1.5,1.5,1.5, 90);
   woodFenceChain(-4.5,-0.33,0, 1.5,1.5,1.5, 90);
   woodFenceChain(-4.5,-0.33,3.5, 1.5,1.5,1.5, 90);

   //Right Fences
   woodFenceChain(4.5,-0.33,-3.5, 1.5,1.5,1.5, 90);
   woodFenceChain(4.5,-0.33,0, 1.5,1.5,1.5, 90);
   woodFenceChain(4.5,-0.33,3.5, 1.5,1.5,1.5, 90);

   //Buildings Inside Fence
   smallBuilding(-2,-0.3,-4, 1.5,1.5,1.5, 0, 0,0);
   smallBuilding(-2.5,-0.3,-1, 1,1.5,1.5, 0, 0,2);

   glColor3f(1,1,1);
   //Foliage:
   flowersGrass(2,0.7,-9, 1,1,1, 0, 5);
   flowersGrass(-2,0.7,-9, 1,1,1, 0, 5);
   flowersGrass(2,0.2,-8, 0.5,0.5,0.5, 0, 3);
   flowersGrass(-1,0.7,-8, 1,1,1, 0, 5);
   flowersGrass(1,0.2,-7.5, 0.5,0.5,0.5, 0, 1);
   flowersGrass(5,0.7,-7.5, 1,1,1, 0, 5);
   flowersGrass(-1,0.7,-3.5, 1,1,1, 0, 5);
   flowersGrass(2,0.2,-3, 0.5,0.5,0.5, 0, 3);
   flowersGrass(3,0.7,-2, 1,1,1, 0, 5);
   flowersGrass(1,0.2,-1, 0.5,0.5,0.5, 0, 1);
   flowersGrass(-3,0.7,0, 1,1,1, 0, 5);
   flowersGrass(-1,0.7,3, 1,1,1, 0, 5);
   flowersGrass(-5,0,8, 0.7,0.6,0.7, 0, 4);
   flowersGrass(1.2,0,8.5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(5.5,0,9, 0.7,0.6,0.7, 0, 4);

   //  Undo transformations and textures
   glPopMatrix();
}

static void bakeryPoliceSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   cube(1.25,-0.25,10, 0.6,0.05,0.6, 0, 11);
   cube(1.25,-0.25,8.75, 0.6,0.05,0.6, 0, 11);
   cube(1.25,-0.25,7.5, 0.6,0.05,0.6, 0, 11);
   cube(1.25,-0.25,6.25, 0.6,0.05,0.6, 0, 11);
   cube(2.5,-0.25,6.25, 0.6,0.05,0.6, 0, 11);
   cube(3.75,-0.25,6.25, 0.6,0.05,0.6, 0, 11);
   cube(5,-0.25,6.25, 0.6,0.05,0.6, 0, 11);
   for (float i = -6.25; i <= 0; i+=1.25) { //Bottom
      cube(i,-0.25,10, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -6.25; i <= 6.25; i+=1.25) { //Top
      cube(i,-0.25,-10, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -8.75; i <= 6.25; i+=1.25) { //Right
      cube(6.3,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -8.75; i <= 8.75; i+=1.25) { //Left
      cube(-6.3,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }

   //Grass
   cube(-3,-0.3,7.7, 3.8,0.05,1.7, 0, 32);
   cube(0,-0.3,-2, 6.8,0.05,8, 0, 32);

   //Objects
   policeStation(3.8,-0.3,4, 1.3,1.3,1.3, 0);
   car(3,0.4,8.5, 1,1,1, 90, 0);
   car(5.5,0.4,8.5, 1,1,1, 90, 0);
   smallBuilding(-1.8,-0.3,8, 1.6,1.6,1.6, 0, 1,0);
   smallBuilding(-1.2,-0.3,4, 1.6,2,1.6, 0, 0,2);
   cube(1,-0.15,6, 0.4,0.1,0.4, 0, 14);
   flowersGrass(1,0.2,5.8, 0.3,0.3,0.3, 0, 1);
   cube(-3,-0.15,9.5, 0.4,0.1,0.4, 0, 13);
   flowersGrass(-3,0.2,9.3, 0.3,0.3,0.3, 0, 1);
   mailbox(-5,-0.1,10, 0.3,0.5,0.7, -90);
   stopSign(1,-0.2,-10, 1,1,1, -90);
   streetLamp(6.3,-0.25,-9.8, 1,1,1, 45);
   smallSign(-3,-0.3,-5, 1,1,1, 0);
   
   //Foliage & Fences:
   tree(5,-0.2,1, 2.5,2.5,2.5, 0);

   glColor3f(1,1,1);
   flowersGrass(2,0.7,-9, 1,1,1, 0, 5);
   flowersGrass(-2,0.7,-9, 1,1,1, 0, 5);
   flowersGrass(-1,0.7,-8, 1,1,1, 0, 5);
   wireFence(3,0,-7, 1,0.7,1, 0); // Right
   flowersGrass(5,0.7,-6.5, 1,1,1, 0, 5);
   flowersGrass(2,0.2,-6, 0.5,0.5,0.5, 0, 1);
   flowersGrass(-1,0.7,-5, 1,1,1, 0, 5);
   flowersGrass(3,0.2,-5, 0.5,0.5,0.5, 0, 3);
   wireFence(0.5,0,-4.5, 1,0.7,1, 90); // Middle
   flowersGrass(-3,0.7,-4, 1,1,1, 0, 5);
   flowersGrass(5,0.7,-3, 1,1,1, 0, 5);
   wireFence(-2,0,-2, 1,0.7,1, 0); // Left
   flowersGrass(2,0.7,5, 1,1,1, 0, 5);
   flowersGrass(5.5,0,5, 0.7,0.6,0.7, 0, 4);
   wireFence(-3,0,6, 1,0.7,1, 0); // Front Fence
   flowersGrass(-5,0,8, 0.7,0.6,0.7, 0, 4);
   flowersGrass(1,0,8.5, 0.7,0.6,0.7, 0, 4);

   
   

   //  Undo transformations and textures
   glPopMatrix();
}

static void hotelSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   for (float i = -7.5; i <= 7.5; i+=1.25) {
      cube(i,-0.25,11.25, 0.6,0.05,0.6, 0, 11);
      cube(i,-0.25,-11.25, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -10; i <= 10; i+=1.25) {
      cube(-7.5,-0.25,i, 0.6,0.05,0.6, 0, 11);
      cube(7.5,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }
   //Grass
   cube(0,-0.3,5.3, 7,0.05,5.5, 0, 32);
   cube(0,-0.3,-5.3, 7,0.05,5.5, 0, 32);

   //Objects:
   tallBuilding(-1.8,-0.3,7, 1.6,2,1.6, 0, 2,0);
   tallBuilding(3.2,-0.3,6, 1.6,2,1.6, 0, 1,1);
   tallBuilding(4.5,-0.3,2, 1.6,2,1.6, 90, 1,0);
   tallBuilding(4.5,-0.3,-3.5, 1.6,2,1.6, 90, 1,2);
   tallBuilding(-1.8,-0.3,-9, 1.6,2,1.6, 180, 1,0);
   tallBuilding(3.2,-0.3,-8, 1.6,2,1.6, 180, 1,1);
   tallBuilding(-4.5,-0.3,2, 1.6,2,1.6, -90, 1,2);
   tallBuilding(-4.5,-0.3,-3.5, 1.6,2,1.6, -90, 1,0);
   tallBuilding(0,-0.3,3, 1.6,2,1.6, 0, 0,2);
   tallBuilding(0,-0.3,-1, 1.6,2,1.6, 0, 0,0);
   tallBuilding(0,-0.3,-4.5, 1.6,2,1.6, 0, 0,1);
   mailbox(5,-0.1,11, 0.3,0.5,0.7, -90);
   smallSign(6,-0.3,9, 1,1,1, 0);
   stopSign(-0.5,-0.2,-11.5, 1,1,1, -90);
   streetLamp(7.5,-0.25,11, 1,1,1, -45);

   //Foliage:
   glColor3f(1,1,1);
   flowersGrass(1,0.7,-9, 1,1,1, 160, 5);
   flowersGrass(-5,0.7,-7.5, 1,1,1, -110, 5);
   flowersGrass(5,0.7,-6, 1,1,1, 100, 5);
   flowersGrass(-5,0.7,-1, 1,1,1, -90, 5);
   flowersGrass(6,0.7,0, 1,1,1, 90, 5);
   flowersGrass(5,0.7,5, 1,1,1, 90, 5);
   flowersGrass(-6,0.7,5, 1,1,1, -45, 5);
   flowersGrass(4,0,5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-5,0,7, 0.7,0.6,0.7, -45, 4);
   flowersGrass(-5,0.7,8, 1,1,1, 0, 5);
   flowersGrass(1.2,0,8.5, 0.7,0.6,0.7, 0, 4);

   //  Undo transformations and textures
   glPopMatrix();
}

static void townHallSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   for (float i = -8.75; i <= 8.75; i+=1.25) {
      cube(i,-0.25,11.25, 0.6,0.05,0.6, 0, 11);
      cube(i,-0.25,-11.25, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -10; i <= 10; i+=1.25) {
      cube(-8.75,-0.25,i, 0.6,0.05,0.6, 0, 11);
      cube(8.75,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }
   // Grass
   cube(0,-0.3,5.3, 9,0.05,5.5, 0, 32);
   cube(0,-0.3,-5.3, 9,0.05,5.5, 0, 32);

   //Objects:
   townHall(0,-0.3,0, 1.6,1.6,1.6, 0);
   mailbox(5,-0.1,11, 0.3,0.5,0.7, -90);
   bigSign(6,-0.3,9, 1,1,1, 0);
   veryBigSign(-5,-0.3,5, 1,1,1, 0);
   stopSign(-5,-0.2,-11.5, 1,1,1, -90);
   stopSign(5,-0.2,-11.5, 1,1,1, 90);
   streetLamp(9,-0.25,11, 1,1,1, -45);
   streetLamp(-9,-0.25,11, 1,1,1, 45);
   streetLamp(-9,-0.25,-11, 1,1,1, -45);
   streetLamp(9,-0.25,-11, 1,1,1, 45);

   //Trees:
   for (float i = 0; i >= -8; i-=2) {
      tree(7.5,-0.3,i, 2.5,2.5,2.5, 0);
      tree(-7.5,-0.3,i, 2.5,2.5,2.5, 0);
   }

   for (float i = -7; i <= 7; i+=2) {
      tree(i,-0.3,-10, 2.5,2.5,2.5, 0);
      tree(i,-0.3,-10, 2.5,2.5,2.5, 0);
   }
   bigTree(-4,-0.3,-4, 3,3,3, 0);
   bigTree(4,-0.3,-7, 3,3,3, 0);

   tree(3.5,-0.3,3.5, 3,3,3, 0);
   tree(-3.5,-0.3,3.5, 3,3,3, 0);
   bigTree(6,-0.3,6, 2,2,2, 0);
 
   //Foliage:
   glColor3f(1,1,1);
   flowersGrass(-4,0,-2.5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-2,0,-2.5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(0,0,-2.5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(2,0,-2.5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(4,0,-2.5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-2,0,4, 0.7,0.6,0.7, 0, 4);
   flowersGrass(2,0,4, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-2,0,5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(2,0,5, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-2,0,6, 0.7,0.6,0.7, 0, 4);
   flowersGrass(2,0,6, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-2,0,7, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-6,0.7,6, 1,1,1, 0, 5);
   flowersGrass(-4,0,7, 0.6,0.6,0.6, 0, 3);
   flowersGrass(-7,0,7, 0.6,0.6,0.6, 0, 2);
   flowersGrass(2,0,7, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-2,0,8, 0.7,0.6,0.7, 0, 4);
   flowersGrass(2,0,8, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-2,0,9, 0.7,0.6,0.7, 0, 4);
   flowersGrass(-5,0,9, 0.6,0.6,0.6, 0, 1);
   flowersGrass(2,0,9, 0.7,0.6,0.7, 0, 4);

   //  Undo transformations and textures
   glPopMatrix();
}

static void hospitalSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   cube(-6.25,-0.25,15, 0.6,0.05,0.6, 0, 11);
   cube(-6.25,-0.25,13.75, 0.6,0.05,0.6, 0, 11);
   cube(-6.25,-0.25,12.5, 0.6,0.05,0.6, 0, 11);
   cube(-6.25,-0.25,11.25, 0.6,0.05,0.6, 0, 11);
   for (float i = -0; i <= 6.25; i+=1.25) {
      cube(i,-0.25,-10, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -6.25; i <= 6.25; i+=1.25) {
      cube(i,-0.25,10, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -8.75; i <= 8.75; i+=1.25) {
      cube(6.3,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }
   //Grass
   cube(0,-0.3,5.1, 6.6,0.05,5.2, 0, 32);
   cube(0,-0.3,-5.3, 6.6,0.05,5.2, 0, 32);
   cube(0,-0.3,18.8, 6.8,0.05,3.2, 0, 32);

   //Objects:
   hospital(0,2.8,7, 2,2,2, 0);
   ambulance(0,0.4,12, 1,1,1, 0);
   ambulance(8,0.4,6, 1,1,1, -90);
   stopSign(1,-0.2,-10, 1,1,1, 90);
   streetLamp(6.3,-0.25,9.8, 1,1,1, -45);
   smallSign(5,-0.3,2, 1,1,1, 90);
   bigSign(0,-0.3,20, 1,1,1, 180);

   //Foliage:
   tree(2,0,-5, 2,2,2, 0);
   tree(-4,0,-1, 2,2,2, 0);

   glColor3f(1,1,1);
   flowersGrass(2,0.7,-9, 1,1,1, 0, 5);
   flowersGrass(-2,0.7,-9, 1,1,1, 0, 5);
   flowersGrass(2,0.2,-8, 0.5,0.5,0.5, 0, 3);
   flowersGrass(-1,0.7,-8, 1,1,1, 0, 5);
   flowersGrass(1,0.2,-7.5, 0.5,0.5,0.5, 0, 1);
   flowersGrass(5,0.7,-7.5, 1,1,1, 0, 5);
   flowersGrass(-1,0.7,-3.5, 1,1,1, 0, 5);
   flowersGrass(2,0.2,-3, 0.5,0.5,0.5, 0, 3);
   flowersGrass(3,0.7,-2, 1,1,1, 0, 5);
   flowersGrass(1,0.2,-1, 0.5,0.5,0.5, 0, 1);
   flowersGrass(-3,0.7,0, 1,1,1, 0, 5);
   flowersGrass(-1,0.7,3, 1,1,1, 0, 5);
   flowersGrass(-3,0,9, 0.7,0.6,0.7, 0, 4);
   flowersGrass(5.5,0,9, 0.7,0.6,0.7, 0, 4);
   flowersGrass(1.5,0,18, 0.7,0.6,0.7, 40, 5);
   flowersGrass(-1.5,0,18, 0.7,0.6,0.7, -40, 5);

   //  Undo transformations and textures
   glPopMatrix();
}

static void topLeftSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   cube(-6.25,-0.25,-15, 0.6,0.05,0.6, 0, 11);
   cube(-6.25,-0.25,-13.75, 0.6,0.05,0.6, 0, 11);
   cube(-6.25,-0.25,-12.5, 0.6,0.05,0.6, 0, 11);
   cube(-6.25,-0.25,-11.25, 0.6,0.05,0.6, 0, 11);
   for (float i = -6.25; i <= 6.25; i+=1.25) {
      cube(i,-0.25,-10, 0.6,0.05,0.6, 0, 11); 
   }
   for (float i = -10; i <= 17.5; i+=1.25) {
      cube(6.3,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }
   //Grass
   cube(-4,-0.3,11, 10.2,0.05,7, 0, 32);
   cube(-4,-0.3,-3, 10.2,0.05,7, 0, 32);
   cube(-10.3,-0.3,-13, 4,0.05,2.9, 0, 32);

   //Objects:
   tallBuilding(-7,-0.3,-4.5, 1.5,2,2, 90, 1,2);
   tallBuilding(-11,-0.3,-8, 1.5,2,2, 180, 1,2);
   tallBuilding(-11,-0.3,-1, 1.5,2,2, 0, 1,0);
   bigHouse(-1,-0.3,2, 3,3,3, 0, 2);
   smallHouse(3,-0.3,13, 1.5,1.5,1.5, 90, 1);
   smallLongBuilding(0,0,-5, 2,1.5,1.5, 0);
   car(8.5,0.4,6, 1,1,1, 90, 2);
   car(-2,0.4,-11.5, 1,1,1, 180, 1);
   stopSign(1,-0.2,-10, 1,1,1, 90); //Stop Sign
   streetLamp(6.3,-0.25,-9.8, 1,1,1, 45);

   //Foliage:
   tree(-8,-0.3,-1, 3,3,3, 0);
   tree(-8,-0.3,-8, 3,3,3, 0);

   glColor3f(1,1,1);
   flowersGrass(2,0,-8, 0.5,0.5,0.5, 0, 2);
   flowersGrass(-1,0.7,-9, 1,1,1, 0, 5);
   flowersGrass(4.5,0.7,-8.5, 1,1,1, -45, 5);
   flowersGrass(3,0.7,-4, 1,1,1, 45, 5);
   flowersGrass(-3,0.7,-3.5, 1,1,1, -45, 5);
   flowersGrass(2,0.2,-3, 0.5,0.5,0.5, 0, 3);
   flowersGrass(-10,0.7,0, 1,1,1, 0, 5);
   flowersGrass(2,0,5, 0.6,0.6,0.6, 45, 1);
   flowersGrass(-5,0,8, 0.7,0.6,0.7, 0, 4);
   flowersGrass(2,0,8, 0.7,0.6,0.7, 0, 4);
   transparentObject(-1,3,3.9, 0.5,0.5,0.5, 0,0, 0);
   transparentObject(3,3,13.5, 0.5,0.5,0.5, 0,0, 0);
   flowersGrass(2,0.7,14, 1,1,1, -45, 5);

   //  Undo transformations and textures
   glPopMatrix();
}

static void burgerDrugstoreSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   cube(15,-0.25,-5, 0.6,0.05,0.6, 0, 11);
   cube(15,-0.25,-3.75, 0.6,0.05,0.6, 0, 11);
   cube(15,-0.25,-2.5, 0.6,0.05,0.6, 0, 11);
   cube(15,-0.25,-1.25, 0.6,0.05,0.6, 0, 11);
   for (float i = -15; i <= 15; i+=1.25) {
      cube(i,-0.25,0, 0.6,0.05,0.6, 0, 11);
   }
   //Grass
   cube(7.5,-0.3,-6, 8,0.05,6, 0, 32);
   cube(-7.5,-0.3,-6, 8,0.05,6, 0, 32);

   //Objects:
   burgerShop(10.5,1,-2, 1.5,1.5,1.5, 0);
   smallBuilding(3,-0.3,-2.5, 1.6,2,1.6, 0, 3,0);
   cube(2,-0.15,-0.2, 0.4,0.1,0.4, 0, 13);
   flowersGrass(2,0.2,-0.5, 0.3,0.3,0.3, 0, 1);
   longBuilding(-8,-0.3,-8, 2,1.5,1.5, 0);
   smallSign(-13.5,-0.3,-8, 1,1,1, 0);

   //Foliage & Fences:
   tree(0,-0.3,-1, 2.5,2.5,2.5, 0);
   bigTree(-2,-0.3,-9, 2.5,2.5,2.5, 0);
   bigTree(8,-0.3,-9, 2.5,2.5,2.5, 0);
   bigTree(13,-0.3,-7, 2.5,3,2.5, 0);

   glColor3f(1,1,1);
   flowersGrass(-10,0.7,-6, 1,1,1, 0, 5);
   flowersGrass(-6,0.7,-6, 1,1,1, 0, 5);
   wireFence(-1,0,-4.5, 1,1,1, 90);
   wireFence(-4,0,-2, 1.5,1,1, 0);
   wireFence(-13,0,-2, 1,1,1, 0);
   flowersGrass(6,0.3,-2, 0.7,0.6,0.7, 0, 5);

   //  Undo transformations and textures
   glPopMatrix();
}

static void topSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   cube(-20,-0.25,-5, 0.6,0.05,0.6, 0, 11);
   cube(-20,-0.25,-3.75, 0.6,0.05,0.6, 0, 11);
   cube(-20,-0.25,-2.5, 0.6,0.05,0.6, 0, 11);
   cube(-20,-0.25,-1.25, 0.6,0.05,0.6, 0, 11);
   for (float i = -20; i <= 2.5; i+=1.25) {
      cube(i,-0.25,0, 0.6,0.05,0.6, 0, 11);
   }
   //Grass
   cube(-2,-0.3,-6, 6,0.05,6, 0, 32);
   cube(-14,-0.3,-6, 6,0.05,6, 0, 32);

   //Objects:
   smallHouse(-5,-0.3,-4, 1.2,1.2,1.2, 90, 2);
   smallHouse(-1,-0.3,-4, 1.2,1.2,1.2, 90, 1);
   veryBigSign(-9,-0.3,-4, 1.5,1.5,1.5, 0);
   woodFenceChain(-14,-0.33,-3, 0.8,0.6,0.7, 0);
   woodFenceChain(-14,-0.33,-5.7, 0.8,0.6,0.7, 0);
   woodFenceChain(-15.5,-0.33,-4.4, 0.8,0.6,0.7, 90);
   woodFenceChain(-12.5,-0.33,-4.4, 0.8,0.6,0.7, 90);
   woodFenceChain(-18,-0.33,-3, 0.8,0.6,0.7, 0);
   woodFenceChain(-18,-0.33,-5.7, 0.8,0.6,0.7, 0);
   woodFenceChain(-19.5,-0.33,-4.4, 0.8,0.6,0.7, 90);
   woodFenceChain(-16.5,-0.33,-4.4, 0.8,0.6,0.7, 90);
   streetLamp(-20,-0.25,0, 1,1,1, 45);

   //Foliage & Entrance/Exit Signs:
   tree(-3,-0.3,-2, 2.5,2.5,2.5, 0);
   bigTree(-8,-0.3,-9, 2.5,2.5,2.5, 0);
   bigTree(-3,-0.3,-8, 2.5,2.5,2.5, 0);

   glColor3f(1,1,1);
   flowersGrass(-18,0.3,-10, 0.7,0.6,0.7, 45, 5);
   flowersGrass(-16,0.3,-10, 0.7,0.6,0.7, 0, 5);
   flowersGrass(-14,0.3,-10, 0.7,0.6,0.7, 0, 5);
   flowersGrass(-12,0.3,-10, 0.7,0.6,0.7, -45, 5);
   entranceExitSign(-15,-0.3,-8, 2,2,2, 180);
   flowersGrass(-18,0.7,-6, 1,1,1, 0, 1);
   flowersGrass(-14,0.7,-6, 1,1,1, 0, 1);
   flowersGrass(-13.5,0.7,-5, 1,1,1, 0, 2);
   flowersGrass(-17.5,0.7,-5, 1,1,1, 0, 3);
   transparentObject(-5,2.5,-3.8, 0.5,0.5,0.5, 0,0, 0);
   transparentObject(-1,2.5,-3.8, 0.5,0.5,0.5, 0,0, 0);
   flowersGrass(0,0.3,-2.5, 0.7,0.6,0.7, 0, 5);

   //  Undo transformations and textures
   glPopMatrix();
}

static void topRightSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   cube(7.5,-0.25,22.5, 0.6,0.05,0.6, 0, 11);
   cube(7.5,-0.25,21.25, 0.6,0.05,0.6, 0, 11);
   cube(7.5,-0.25,20, 0.6,0.05,0.6, 0, 11);
   cube(7.5,-0.25,18.75, 0.6,0.05,0.6, 0, 11);
   for (float i = -7.5; i <= 7.5; i+=1.25) {
      cube(i,-0.25,17.5, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -10; i <= 17.5; i+=1.25) {
      cube(-7.5,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }
   //Grass
   cube(0,-0.3,10, 8,0.05,7, 0, 32);
   cube(0,-0.3,-3, 8,0.05,7, 0, 32);

   //Objects:
   bigHouse(0,-0.3,7, 3,3,3, 0, 2);
   bigHouse(0,-0.3,-5, 3,3,3, 0, 0);
   veryBigSign(-5,-0.2,-8, 1,1,1, -45);
   streetLamp(-7.5,-0.25,17.5, 1,1,1, 45);
   car(0,0.4,19.5, 1,1,1, 0, 2);

   //If the user is viewing fullscreen, animate the car. Otherwise, have it stay still.
   if (obj == 12) moveRedCar();
   else car(-9.5,0.4,0, 1,1,1, 90, 1);

   woodFenceChain(-5,-0.33,2, 0.8,0.6,0.7, 0);
   woodFenceChain(-5,-0.33,4.7, 0.8,0.6,0.7, 0);
   woodFenceChain(-6.5,-0.33,3.4, 0.8,0.6,0.7, 90);
   woodFenceChain(-3.5,-0.33,3.4, 0.8,0.6,0.7, 90);

   // Foliage:
   bigTree(5,-0.3,-7, 2.5,3,2.5, 0);

   glColor3f(1,1,1);
   flowersGrass(-5,0.7,-5.5, 1,1,1, -110, 5);
   flowersGrass(5,0.7,-4, 1,1,1, 100, 5);
   transparentObject(0,3,-3, 0.5,0.5,0.5, 0,0, 0);
   flowersGrass(-3,0.3,-3, 0.7,0.7,0.7, 0, 1);
   flowersGrass(3,0.7,-3, 1,1,1, 0, 5);
   flowersGrass(-5.5,0.3,2, 0.7,0.7,0.7, 0, 1);
   flowersGrass(-4.5,0.3,2.5, 0.7,0.7,0.7, 0, 2);
   flowersGrass(-5.5,0.3,3, 0.7,0.7,0.7, 0, 3);
   flowersGrass(-6,0.7,7, 1,1,1, -45, 5);
   flowersGrass(4,0,7, 0.7,0.6,0.7, 0, 4);
   transparentObject(0,3,9, 0.5,0.5,0.5, 0,0, 0);
   flowersGrass(-5,0.7,10, 1,1,1, 0, 5);
   flowersGrass(1.2,0,10.5, 0.7,0.6,0.7, 0, 4);

   //  Undo transformations and textures
   glPopMatrix();
}

static void bottomRightSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   for (float i = -7.5; i <= 6.25; i+=1.25) {
      cube(i,-0.25,-11.25, 0.6,0.05,0.6, 0, 11);
   }
   for (float i = -10; i <= 15; i+=1.25) {
      cube(-7.5,-0.25,i, 0.6,0.05,0.6, 0, 11);
   }
   // Grass
   cube(0,-0.3,8.5, 8,0.05,7, 0, 32);
   cube(0,-0.3,-4.5, 8,0.05,7, 0, 32);

   //Objects:
   bigHouse(0,-0.3,5, 3,3,3, 0, 0);
   bigHouse(0,-0.3,-7, 3,3,3, 0, 1);
   bigSign(4,-0.3,0, 1,1,1, 0);
   stopSign(-0.5,-0.2,-11.5, 1,1,1, -90);
   car(-9.5,0.4,7.5, 1,1,1, -90, 2);

   // Foliage:
   bigTree(5,-0.3,-9, 2.5,3,2.5, 0);

   glColor3f(1,1,1);
   flowersGrass(-5,0.7,-7.5, 1,1,1, -110, 5);
   flowersGrass(5,0.7,-6, 1,1,1, 100, 5);
   transparentObject(0,3,-5, 0.5,0.5,0.5, 0,0, 0);
   flowersGrass(-3,0.3,-5, 0.7,0.7,0.7, 0, 1);
   flowersGrass(3,0.7,-5, 1,1,1, 0, 5);
   flowersGrass(3,0.3,8, 0.7,0.7,0.7, 0, 1);
   flowersGrass(4,0.3,8.5, 0.7,0.7,0.7, 0, 2);
   flowersGrass(3,0.3,9, 0.7,0.7,0.7, 0, 3);
   flowersGrass(-6,0.7,5, 1,1,1, -45, 5);
   flowersGrass(4,0,5, 0.7,0.6,0.7, 0, 4);
   transparentObject(0,3,7, 0.5,0.5,0.5, 0,0, 0);
   flowersGrass(-5,0.7,8, 1,1,1, 0, 5);
   flowersGrass(1.2,0,8.5, 0.7,0.6,0.7, 0, 4);

   //  Undo transformations and textures
   glPopMatrix();
}

static void bottomSection(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Sidewalks:
   for (float i = -20; i <= 20; i+=1.25) {
      cube(i,-0.25,5, 0.6,0.05,0.6, 0, 11);
   }
   //Grass
   cube(10,-0.3,8, 12,0.05,3, 0, 32);
   cube(-13,-0.3,8, 12,0.05,3, 0, 32);

   //Objects:
   smallHouse(-5,-0.3,8, 1.2,1.2,1.2, -90, 2);
   smallHouse(-1,-0.3,8, 1.2,1.2,1.2, -90, 1);
   smallSign(18,-0.3,9, 1.5,1.5,1.5, 0);

   //Foliage & Entrance/Exit Signs:
   tree(5,-0.3,7, 2.5,2.5,2.5, 0);
   bigTree(2,-0.3,9, 2,2,2, 0);
   bigTree(-10,-0.3,9, 2,2,2, 0);
   bigTree(-15,-0.3,7, 2,2,2, 0);

   glColor3f(1,1,1);
   transparentObject(-5,2.5,6.8, 0.5,0.5,0.5, 0,0, 0);
   transparentObject(-1,2.5,6.8, 0.5,0.5,0.5, 0,0, 0);
   flowersGrass(-14,0.3,7, 0.7,0.6,0.7, 45, 5);
   flowersGrass(-12,0.3,7, 0.7,0.6,0.7, 0, 5);
   flowersGrass(-10,0.3,7, 0.7,0.6,0.7, 0, 5);
   flowersGrass(-8,0.3,7, 0.7,0.6,0.7, -45, 5);
   entranceExitSign(10,-0.3,8, 1.5,1.5,1.5, 0);

   //  Undo transformations and textures
   glPopMatrix();
}

//Draw the entire city of Onett
static void onett(double x,double y,double z, double dx,double dy,double dz, double th)
{
   //  Save transformation
   glPushMatrix();
   //  Offset
   glTranslated(x,y,z);
   glRotated(th,0,1,0);
   glScaled(dx,dy,dz);

   //Town Sections:
   burgerDrugstoreSection(-20,0,-20, 1,1,1, 0);
   topLeftSection(-30,0,-5, 1,1,1, 0);
   townHallSection(-10,0,-4, 1,1,1, 0);
   hotelSection(11,0,-4, 1,1,1, 0);
   topSection(20,0,-20, 1,1,1, 0);
   topRightSection(31,0,-10, 1,1,1, 0);
   bottomRightSection(31,0,24, 1,1,1, 0);
   bakeryPoliceSection(12,0,23, 1,1,1, 0);
   pizzaArcadeSection(-6,0,23, 1,1,1, 0);
   hospitalSection(-24,0,23, 1,1,1, 0);
   bottomSection(3,0,34, 1,1,1, 0);

   //Sewers:
   //Top
   roundCylinder(-12,-0.35,-17.5, 0.8, 0.1, 0, 90, 1);
   roundCylinder(13,-0.35,-17.5, 0.8, 0.1, 0, 90, 1);
   
   //Middle
   roundCylinder(-8,-0.35,10, 0.8, 0.1, 0, 90, 1);
   roundCylinder(13,-0.35,10, 0.8, 0.1, 0, 90, 1);
   roundCylinder(35,-0.35,10, 0.8, 0.1, 0, 90, 1);

   //Bottom
   roundCylinder(-4,-0.35,36, 0.8, 0.1, 0, 90, 1);
   roundCylinder(17,-0.35,31, 0.8, 0.1, 0, 90, 1);

   //Grass:
   cube(31,-0.3,-26, 8,0.05,6, 0, 32); //Top Right Corner Grass
   cube(0,-0.3,-28, 7,0.05,4, 0, 32); //Top Middle Corner Grass
   cube(-37.5,-0.3,-26, 7,0.05,6, 0, 32); //Top Left Corner Grass
   cube(-37,-0.3,27, 7,0.05,18, 0, 32); //Bottom Left Corner Grass
   cube(32,-0.3,41, 7,0.05,4, 0, 32); //Bottom Right Corner Grass

   //Roads:
   //Bottom:
   cube(15,-0.3,36, 8,0.01,2.5, 0, 33);
   cube(0,-0.3,36, 8,0.01,2.5, 0, 33);
   cube(-17.5,-0.3,36, 12.5,0.01,2.5, 0, 33);

   //Middle
   cube(28,-0.3,10.1, 10,0.01,2.3, 0, 33);
   cube(8,-0.3,10.1, 10,0.01,2.3, 0, 33);
   cube(-9,-0.3,10.1, 14,0.01,2.3, 0, 33);

   //Top
   cube(18,-0.3,-17.5, 10,0.01,2.3, 0, 33);
   cube(-2,-0.3,-17.5, 10,0.01,2.3, 0, 33);
   cube(-22,-0.3,-17.5, 14,0.01,2.3, 0, 33);

   //Top 3 Vertical
   cube(21,-0.3,-4, 12,0.01,2, 90, 33);
   cube(1,-0.3,-4, 12,0.01,2, 90, 33);
   cube(-21,-0.3,-4, 12,0.01,2, 90, 33);

   //Bottom 3 Vertical
   cube(21,-0.3,23, 10.7,0.01,2.3, 90, 33);
   cube(3,-0.3,23, 10.7,0.01,2.3, 90, 33);
   cube(-15,-0.3,23, 10.7,0.01,2.3, 90, 33);

   cube(-2.5,-0.3,-22, 2.5,0.01,2, 90, 33); //Top Little Part
   cube(16.2,-0.3,31.6, 2.7,0.01,2, 0, 33); //Police Station Part
   
   //Trees:
   for(int i = -44; i <= 38; i+= 2) {
      tree(i,-0.3,45, 3,3,3, 0);
      tree(i,-0.3,-32, 3,3,3, 0);
   }
   for (int i = -32; i <= 44; i+=2) {
      tree(39,-0.3,i, 3,3,3, 0);
      tree(-44,-0.3,i, 3,3,3, 0);
   }

   // Extra Trees:
   // These make the scene look nice but cause slowdown due to the high number. If you would like to see them, just uncomment the for loops below.

   // for (int i = 0; i <= 44; i+=2) {
   //    tree(-42,-0.3,i, 3,3,3, 0);
   // }
   // for (int i = 10; i <= 44; i+=2) {
   //    tree(-40,-0.3,i, 3,3,3, 0);
   //    tree(-38,-0.3,i, 3,3,3, 0);
   // }
   // for (int i = 16; i <= 44; i+=2) {
   //    tree(-36,-0.3,i, 3,3,3, 0);
   //    tree(-34,-0.3,i, 3,3,3, 0);
   // }
   // for (int i = 20; i <= 44; i+=2) {
   //    tree(-32,-0.3,i, 3,3,3, 0);
   // }

   // for(int i = 26; i <= 38; i+= 2) {
   //    tree(i,-0.3,43, 3,3,3, 0);
   //    tree(i,-0.3,41, 3,3,3, 0);
   //    tree(i,-0.3,39, 3,3,3, 0);

   //    tree(i,-0.3,-30, 3,3,3, 0);
   //    tree(i,-0.3,-28, 3,3,3, 0);
   //    tree(i,-0.3,-26, 3,3,3, 0);
   //    tree(i,-0.3,-24, 3,3,3, 0);
   //    tree(i,-0.3,-22, 3,3,3, 0);
   // }

   //  Undo transformations and textures
   glPopMatrix();
}
//********************************************************************************************************************************************************************************



//OpenGL (GLUT) calls this routine to display the scene
void display()
{
   const double len = 20.0; //Length of axes
   //  Erase the window and the depth buffer
   glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   //  Enable Z-buffering in OpenGL
   glEnable(GL_DEPTH_TEST);
   //  Undo previous transformations
   glLoadIdentity();

   //  Orthogonal - set world orientation
   if (proj == 0)
   {
      glRotatef(ph,1,0,0);
      glRotatef(th,0,1,0);
   }
   //  Perspective - set eye position
   else if (proj == 1)
   {
      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);
   }
   //First Person View - Set camera position/point of view
   else if (proj == 2) {
      //Calculate the camera's new position based on the rotation angle
      camera_x = Sin(rotation_angle);
      camera_z = -Cos(rotation_angle);

      //First three values are the camera position, next three values are where the camera is looking, and the last three values change the camera’s tilt
      gluLookAt(pos_x,pos_y,pos_z,  pos_x+camera_x,pos_y,pos_z+camera_z,  0,1,0);
   }

   //  Flat or smooth shading
   glShadeModel(smooth ? GL_SMOOTH : GL_FLAT);
   
   if (box) skyBox(3.5*dim); //Draw the skybox

   //Light switch (from Example 13)
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light position
      float Position[]  = {distance*Cos(zh),ylight,distance*Sin(zh),1.0};
      //  Draw light position as ball (still no lighting here)
      glColor3f(1,1,1);
      ball(Position[0],Position[1],Position[2] , 0.1);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  Location of viewer for specular calculations
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,local);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
      glDisable(GL_LIGHTING);

   //Draw each scene:
   if(obj==1)
      burgerDrugstoreSection(0,0,0, 1,1,1, 0);
   else if (obj==2)
      topLeftSection(0,0,0, 1,1,1, 0);
   else if (obj==3)
      townHallSection(0,0,0, 1,1,1, 0);
   else if (obj==4)
      hotelSection(0,0,0, 1,1,1, 0);
   else if (obj==5)
      topSection(0,0,0, 1,1,1, 0);
   else if (obj==6)
      topRightSection(0,0,0, 1,1,1, 0);
   else if (obj==7)
      bottomRightSection(0,0,0, 1,1,1, 0);
   else if (obj==8)
      bakeryPoliceSection(0,0,0, 1,1,1, 0);
   else if (obj==9)
      pizzaArcadeSection(0,0,0, 1,1,1, 0);
   else if (obj==10)
      hospitalSection(0,0,0, 1,1,1, 0);
   else if (obj==11)
      bottomSection(0,0,0, 1,1,1, 0);
   else if (obj==12)
      onett(0,-0.5,0, 1,1,1, 0);
   

   glDisable(GL_LIGHTING); //Disable lighting - no lighting from here on
   glColor3f(1,1,1); //White
   if (axes) //Draw axes
   {
      glBegin(GL_LINES);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(len,0.0,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,len,0.0);
      glVertex3d(0.0,0.0,0.0);
      glVertex3d(0.0,0.0,len);
      glEnd();
      //  Label axes
      glRasterPos3d(len,0.0,0.0);
      Print("X");
      glRasterPos3d(0.0,len,0.0);
      Print("Y");
      glRasterPos3d(0.0,0.0,len);
      Print("Z");
   }

   //Print relevant information based on which view the user is in:
   if (proj == 0) { //Orthogonal
      // Display parameters
      glWindowPos2i(5, 5);
      Print("Angle=%d,%d  Dim=%.1f  Projection=%s  Light=%s",
      th,ph,dim,view[proj],light?"On":"Off");

      if (light)
      {
         glWindowPos2i(5, 45);
         Print("Model=%s  LocalViewer=%s  Distance=%d  Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
         glWindowPos2i(5, 25);
         Print("Ambient=%d  Diffuse=%d  Specular=%d  Emission=%d  Shininess=%.0f",ambient,diffuse,specular,emission,shiny);
      }
   } else if (proj == 1) { //Perspective
      // Display parameters
      glWindowPos2i(5, 5);
      Print("Angle=%d,%d  Dim=%.1f  FOV=%d  Projection=%s  Light=%s",
      th,ph,dim,fov,view[proj],light?"On":"Off");

      if (light)
      {
         glWindowPos2i(5, 45);
         Print("Model=%s  LocalViewer=%s  Distance=%d  Elevation=%.1f",smooth?"Smooth":"Flat",local?"On":"Off",distance,ylight);
         glWindowPos2i(5, 25);
         Print("Ambient=%d  Diffuse=%d  Specular=%d  Emission=%d  Shininess=%.0f",ambient,diffuse,specular,emission,shiny);
      }
   }

   // Check for any errors
   ErrCheck("display");
   // Render the scene
   glFlush();
   // Make the rendered scene visible
   glutSwapBuffers();
}


void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when any special keys are pressed
 */
void special(int key,int x,int y)
{
   if (proj == 2) { //If the user is in First-Person mode:
      //  Right arrow key - Increase the rotation angle by 5
      if (key == GLUT_KEY_RIGHT) {
         rotation_angle += 5;
      }
      //  Left arrow key - Decrease the rotation angle by 5
      else if (key == GLUT_KEY_LEFT) {
         rotation_angle -= 5;
      }
      //  Up arrow key - Move camera forward
      else if (key == GLUT_KEY_UP) {
         pos_x += camera_x*0.5;
         pos_z += camera_z*0.5;
      }
      //  Down arrow key - Move camera backward
      else if (key == GLUT_KEY_DOWN) {
         pos_x -= camera_x*0.5;
         pos_z -= camera_z*0.5;
      }
      //  Keep the rotation angle to +/-360 degrees
      rotation_angle %= 360;
   } else { //If the user is in Orthogonal or Perspective modes:
      //  Right arrow key - increase azimuth by 5 degrees
      if (key == GLUT_KEY_RIGHT)
         th += 5;
      //  Left arrow key - decrease azimuth by 5 degrees
      else if (key == GLUT_KEY_LEFT)
         th -= 5;
      //  Up arrow key - increase elevation by 5 degrees
      else if (key == GLUT_KEY_UP)
         ph += 5;
      //  Down arrow key - decrease elevation by 5 degrees
      else if (key == GLUT_KEY_DOWN)
         ph -= 5;
      //  PageUp key - decrease dim + zoom in
      else if (key == GLUT_KEY_PAGE_UP && dim>1)
         dim -= 1;
      //  PageDown key - increase dim + zoom out
      else if (key == GLUT_KEY_PAGE_DOWN)
         dim += 1;
      //  Keep angles to +/-360 degrees
      th %= 360;
      ph %= 360;

      //  Smooth color model
      if (key == GLUT_KEY_F1)
         smooth = 1-smooth;
      //  Local Viewer
      else if (key == GLUT_KEY_F2)
         local = 1-local;
      // Decrease the light distance
      else if (key == GLUT_KEY_F3 && distance > 5)
         distance -= 5;
      // Increase the light distance
      else if (key == GLUT_KEY_F4 && distance < 50)
         distance += 5;
      //  Toggle ball increment
      else if (key == GLUT_KEY_F5)
         inc = (inc==10)?3:10;
   }
   //  Update projection
   Project(proj?fov:0,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0') {
      if(proj == 0) { //{Orthogonal
         if (obj == 12) {
            th = -25;
            ph = 25;
            dim = 55;
         } else {
            th = -25;
            ph = 25;
            dim = 25;
         }
      } else if (proj == 1) { //Perspective
         if (obj == 12) {
            th = -25;
            ph = 25;
            dim = 55;
            fov = 55;
         } else {
            th = -25;
            ph = 25;
            dim = 25;
            fov = 55;
         }
      } else if (proj == 2) { //First-Person
         pos_x = 1;
         pos_y = 1.1;
         pos_z = 15;
         rotation_angle = 0;
      }
   }
   else if (ch == '9') { //Reset to overhead view
      if (obj == 12) {
            th = 0;
            ph = 90;
            dim = 55;
            fov = 55;
      } else {
            th = 0;
            ph = 90;
            dim = 25;
            fov = 55;
      }
   }
   //  Toggle axes
   else if (ch == 'x' || ch == 'X')
      axes = 1-axes;
   //  Toggle skybox
   else if (ch == 'b' || ch == 'B')
      box = 1-box;
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //  Switch projection mode
   else if (ch == 'p' || ch == 'P') {
      if (obj != 12) {
         proj = (proj+1)%2;
      } else {
         proj = (proj+1)%3;
         if (proj == 2) { //If the user switches to first-person, reset to original view
            pos_x = 1;
            pos_y = 1.1;
            pos_z = 15;
            camera_x = 0;
            camera_z = 10;
            rotation_angle = 0;
            fov = 55;
            dim = 55;
         }
      }
   }
   //  Cycle through individual sections
   else if (ch == 'o' || ch == 'O') {
      proj = 0;
      obj = (obj+1)%12;
      distance = 15;
      th = -25;
      ph = 25;
      dim = 25;
   }
   //  Display the entire city
   else if (ch == 'f' || ch == 'F') {
      obj = 12;
      distance = 35;
      th = -25;
      ph = 25;
      dim = 55;
   }

   if (proj == 0 || proj == 1) { //Allow users to change light values ONLY if they are in orthogonal or perspective view
      //  Toggle light movement
      if (ch == 'm' || ch == 'M')
         move = 1-move;
      //  Move light
      else if (ch == '<')
         zh += 1;
      else if (ch == '>')
         zh -= 1;
      //  Decrease Light elevation
      else if (ch=='[')
         ylight -= 0.1;
      //  Increase Light elevation
      else if (ch==']')
         ylight += 0.1;
      //  Ambient level
      else if (ch=='a' && ambient>0)
         ambient -= 5;
      else if (ch=='A' && ambient<100)
         ambient += 5;
      //  Diffuse level
      else if (ch=='d' && diffuse>0)
         diffuse -= 5;
      else if (ch=='D' && diffuse<100)
         diffuse += 5;
      //  Specular level
      else if (ch=='s' && specular>0)
         specular -= 5;
      else if (ch=='S' && specular<100)
         specular += 5;
      //  Emission level
      else if (ch=='e' && emission>0)
         emission -= 5;
      else if (ch=='E' && emission<100)
         emission += 5;
      //  Shininess level
      else if (ch=='n' && shininess>-1)
         shininess -= 1;
      else if (ch=='N' && shininess<7)
         shininess += 1;
      //Allow users to change field of view angle ONLY if the program is in perspective view
      if(proj == 1) { 
         if (ch == '-' && ch>1)
            fov--;
         else if (ch == '+' && ch<179)
            fov++;
      }
   } else if (proj == 2) { //If the user is in first-person
      if (ch == '1') { //North
         pos_x = -2;
         pos_y = 1.1;
         pos_z = -27;
         camera_x = 0;
         camera_z = 10;
         rotation_angle = 180;
         fov = 55;
         dim = 55;
      } else if (ch == '2') { //East
         pos_x = 37;
         pos_y = 1.1;
         pos_z = 11;
         camera_x = 0;
         camera_z = 10;
         rotation_angle = 270;
         fov = 55;
         dim = 55;
      } else if (ch == '3') { //South
         pos_x = 0;
         pos_y = 1.1;
         pos_z = 43;
         camera_x = 0;
         camera_z = 10;
         rotation_angle = 0;
         fov = 55;
         dim = 55;
      } else if (ch == '4') { //West
         pos_x = -40;
         pos_y = 1.1;
         pos_z = 5;
         camera_x = 0;
         camera_z = 10;
         rotation_angle = 90;
         fov = 55;
         dim = 55;
      }
   }
   //  Translate shininess power to value (-1 => 0)
   shiny = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(proj?fov:0,asp,dim);
   //  Animate if requested
   glutIdleFunc(move?idle:NULL);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized (from Example 13 on Canvas)
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(proj?fov:0,asp,dim);
}


/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT and process user parameters
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitWindowSize(600,600);
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   //  Create the window
   glutCreateWindow("Final Project: Ryan Than");
   //  Tell GLUT to call "display" when the scene should be drawn
   glutDisplayFunc(display);
   //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
   //  Load textures - texture, useAlpha (0 = false, 1 = true)
   texture[0] = LoadTexBMP("Textures/tree.bmp", 0);
   texture[1] = LoadTexBMP("Textures/trunk.bmp", 0);
   texture[2] = LoadTexBMP("Textures/whitefence.bmp", 0);
   texture[3] = LoadTexBMP("Textures/brownfence.bmp", 0);
   texture[4] = LoadTexBMP("Textures/weathervane.bmp", 1);
   texture[5] = LoadTexBMP("Textures/woodsign.bmp", 0);
   texture[6] = LoadTexBMP("Textures/smallsign.bmp", 0);
   texture[7] = LoadTexBMP("Textures/bigsign.bmp", 0);
   texture[8] = LoadTexBMP("Textures/metal.bmp", 0);
   texture[9] = LoadTexBMP("Textures/stop.bmp", 0);
   texture[10] = LoadTexBMP("Textures/purplestripes.bmp", 0);
   texture[11] = LoadTexBMP("Textures/redstripes.bmp", 0);
   texture[12] = LoadTexBMP("Textures/purpleroof.bmp", 0);
   texture[13] = LoadTexBMP("Textures/whitewindow.bmp", 0);
   texture[14] = LoadTexBMP("Textures/whitehousedoor.bmp", 0);
   texture[15] = LoadTexBMP("Textures/whitehouse.bmp", 0);
   texture[16] = LoadTexBMP("Textures/redroof.bmp", 0);
   texture[17] = LoadTexBMP("Textures/yellowwindow.bmp", 0);
   texture[18] = LoadTexBMP("Textures/yellowhousedoor.bmp", 0);
   texture[19] = LoadTexBMP("Textures/yellowhouse.bmp", 0);
   texture[20] = LoadTexBMP("Textures/verybigsign.bmp", 0);
   texture[21] = LoadTexBMP("Textures/mailboxhole.bmp", 0);
   texture[22] = LoadTexBMP("Textures/mailboxsides.bmp", 0);
   texture[23] = LoadTexBMP("Textures/mailboxfrontback.bmp", 0);
   texture[24] = LoadTexBMP("Textures/woodrings.bmp", 0);
   texture[25] = LoadTexBMP("Textures/bluestripes.bmp", 0);
   texture[26] = LoadTexBMP("Textures/blueroof.bmp", 0);
   texture[27] = LoadTexBMP("Textures/welcome.bmp", 1);
   texture[28] = LoadTexBMP("Textures/goodbye.bmp", 1);
   texture[29] = LoadTexBMP("Textures/pizza.bmp", 0);
   texture[30] = LoadTexBMP("Textures/pizzacrust.bmp", 0);
   texture[31] = LoadTexBMP("Textures/whitemetal.bmp", 0);
   texture[32] = LoadTexBMP("Textures/rooffan.bmp", 0);
   texture[33] = LoadTexBMP("Textures/gravel.bmp", 0);
   texture[34] = LoadTexBMP("Textures/redbricks.bmp", 0);
   texture[35] = LoadTexBMP("Textures/orangebricks.bmp", 0);
   texture[36] = LoadTexBMP("Textures/whitebricks.bmp", 0);
   texture[37] = LoadTexBMP("Textures/windowsill.bmp", 0);
   texture[38] = LoadTexBMP("Textures/greenstripes.bmp", 0);
   texture[39] = LoadTexBMP("Textures/bakerytitle.bmp", 0);
   texture[40] = LoadTexBMP("Textures/pizzatitle.bmp", 0);
   texture[41] = LoadTexBMP("Textures/drugstoretitle.bmp", 0);
   texture[42] = LoadTexBMP("Textures/redflowers.bmp", 1);
   texture[43] = LoadTexBMP("Textures/blueflowers.bmp", 1);
   texture[44] = LoadTexBMP("Textures/greenflowers.bmp", 1);
   texture[45] = LoadTexBMP("Textures/burgertitle.bmp", 0);
   texture[46] = LoadTexBMP("Textures/grass.bmp", 1);
   texture[47] = LoadTexBMP("Textures/yellowbricks.bmp", 0);
   texture[48] = LoadTexBMP("Textures/redrock.bmp", 0);
   texture[49] = LoadTexBMP("Textures/whiterock.bmp", 0);
   texture[50] = LoadTexBMP("Textures/yellowrock.bmp", 0);
   texture[51] = LoadTexBMP("Textures/redcolumn.bmp", 0);
   texture[52] = LoadTexBMP("Textures/whitecolumn.bmp", 0);
   texture[53] = LoadTexBMP("Textures/yellowcolumn.bmp", 0);
   texture[54] = LoadTexBMP("Textures/bluedoor.bmp", 0);
   texture[55] = LoadTexBMP("Textures/greendoor.bmp", 0);
   texture[56] = LoadTexBMP("Textures/concrete.bmp", 0);
   texture[57] = LoadTexBMP("Textures/hoteltitle.bmp", 0);
   texture[58] = LoadTexBMP("Textures/soil.bmp", 0);
   texture[59] = LoadTexBMP("Textures/policetitle.bmp", 0);
   texture[60] = LoadTexBMP("Textures/marble.bmp", 0);
   texture[61] = LoadTexBMP("Textures/glassdoor.bmp", 0);
   texture[62] = LoadTexBMP("Textures/townhalltop.bmp", 0);
   texture[63] = LoadTexBMP("Textures/black.bmp", 0);
   texture[64] = LoadTexBMP("Textures/townhallsign.bmp", 0);
   texture[65] = LoadTexBMP("Textures/wirefence.bmp", 1);
   texture[66] = LoadTexBMP("Textures/gametitle.bmp", 0);
   texture[67] = LoadTexBMP("Textures/whitetile.bmp", 0);
   texture[68] = LoadTexBMP("Textures/yellowtile.bmp", 0);
   texture[69] = LoadTexBMP("Textures/checkerboard.bmp", 0);
   texture[70] = LoadTexBMP("Textures/purpleswitch.bmp", 0);
   texture[71] = LoadTexBMP("Textures/powerswitch.bmp", 0);
   texture[72] = LoadTexBMP("Textures/resetswitch.bmp", 0);
   texture[73] = LoadTexBMP("Textures/cartridge.bmp", 0);
   texture[74] = LoadTexBMP("Textures/cartridgeback.bmp", 0);
   texture[75] = LoadTexBMP("Textures/cartridgesides.bmp", 0);
   texture[76] = LoadTexBMP("Textures/browngravel.bmp", 0);
   texture[77] = LoadTexBMP("Textures/hospitaltitle.bmp", 0);
   texture[78] = LoadTexBMP("Textures/emergencytitle.bmp", 0);
   texture[79] = LoadTexBMP("Textures/squarewindow.bmp", 0);
   texture[80] = LoadTexBMP("Textures/blackmarble.bmp", 0);
   texture[81] = LoadTexBMP("Textures/redmetal.bmp", 0);
   texture[82] = LoadTexBMP("Textures/redcardoor.bmp", 0);
   texture[83] = LoadTexBMP("Textures/bluemetal.bmp", 0);
   texture[84] = LoadTexBMP("Textures/bluecardoor.bmp", 0);
   texture[85] = LoadTexBMP("Textures/grate.bmp", 0);
   texture[86] = LoadTexBMP("Textures/policecardoor.bmp", 0);
   texture[87] = LoadTexBMP("Textures/frontcarlight.bmp", 0);
   texture[88] = LoadTexBMP("Textures/backcarlight.bmp", 0);
   texture[89] = LoadTexBMP("Textures/carwheel.bmp", 0);
   texture[90] = LoadTexBMP("Textures/policecartopsides.bmp", 0);
   texture[91] = LoadTexBMP("Textures/policecarfrontback.bmp", 0);
   texture[92] = LoadTexBMP("Textures/redcarsides.bmp", 0);
   texture[93] = LoadTexBMP("Textures/redcarfrontback.bmp", 0);
   texture[94] = LoadTexBMP("Textures/bluecarsides.bmp", 0);
   texture[95] = LoadTexBMP("Textures/bluecarfrontback.bmp", 0);
   texture[96] = LoadTexBMP("Textures/licenseplate.bmp", 0);
   texture[97] = LoadTexBMP("Textures/tiretread.bmp", 0);
   texture[98] = LoadTexBMP("Textures/ambulancesideright.bmp", 0);
   texture[99] = LoadTexBMP("Textures/ambulanceback.bmp", 0);
   texture[100] = LoadTexBMP("Textures/ambulancefrontsides.bmp", 0);
   texture[101] = LoadTexBMP("Textures/ambulancesidewindows.bmp", 0);
   texture[102] = LoadTexBMP("Textures/ambulancefrontwindow.bmp", 0);
   texture[103] = LoadTexBMP("Textures/ambulancefront.bmp", 0);
   texture[104] = LoadTexBMP("Textures/ambulanceboxfront.bmp", 0);
   texture[105] = LoadTexBMP("Textures/ambulancesideleft.bmp", 0);
   texture[106] = LoadTexBMP("Textures/posz.bmp", 0);
   texture[107] = LoadTexBMP("Textures/negz.bmp", 0);
   texture[108] = LoadTexBMP("Textures/posx.bmp", 0);
   texture[109] = LoadTexBMP("Textures/negx.bmp", 0);
   texture[110] = LoadTexBMP("Textures/posy.bmp", 0);
   texture[111] = LoadTexBMP("Textures/negy.bmp", 0);
   texture[112] = LoadTexBMP("Textures/lettuce.bmp", 1);
   texture[113] = LoadTexBMP("Textures/burgerbun.bmp", 0);
   texture[114] = LoadTexBMP("Textures/shrub.bmp", 1);
   texture[115] = LoadTexBMP("Textures/bush.bmp", 1);
   texture[116] = LoadTexBMP("Textures/grassfloor.bmp", 0);
   texture[117] = LoadTexBMP("Textures/road.bmp", 0);
   texture[118] = LoadTexBMP("Textures/sewer.bmp", 0);

   //  Pass control to GLUT so it can interact with the user
   ErrCheck("init");
   glutMainLoop();
   return 0;
}