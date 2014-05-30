#ifndef _PENGUIN_
#define _PENGUIN_
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <math.h>

void drawCube(float rad, float height);
void drawBody();
void drawBeak(float height);
void drawArm();
void drawHand();
void drawFoot();
void drawCircle(float radius);

double norm(double a, double b, double c);

void drawTree(float trunk, float width, float height);
void drawPyramid(float width, float height);
void drawCylinder(float radius, float height);

#endif