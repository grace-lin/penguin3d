#include "penguin.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

const float DEG2RAD = 3.14159/180;

// Draw a unit cube, centered at the current location
// README: Helper code for drawing a cube
void drawCube(float rad, float height)
{
	glBegin(GL_QUADS);
		// draw front face
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-rad, -height, rad);
		glVertex3f( rad, -height, rad);
		glVertex3f( rad,  height, rad);
		glVertex3f(-rad,  height, rad);

		// draw back face
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f( rad, -height, -rad);
		glVertex3f(-rad, -height, -rad);
		glVertex3f(-rad,  height, -rad);
		glVertex3f( rad,  height, -rad);

		// draw left face
		glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(-rad, -height, -rad);
		glVertex3f(-rad, -height,  rad);
		glVertex3f(-rad,  height,  rad);
		glVertex3f(-rad,  height, -rad);

		// draw right face
		glNormal3f(1.0, 0.0, 0.0);
		glVertex3f( rad, -height,  rad);
		glVertex3f( rad, -height, -rad);
		glVertex3f( rad,  height, -rad);
		glVertex3f( rad,  height,  rad);

		// draw top
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-rad,  height,  rad);
		glVertex3f( rad,  height,  rad);
		glVertex3f( rad,  height, -rad);
		glVertex3f(-rad,  height, -rad);

		// draw bottom
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-rad, -height, -rad);
		glVertex3f( rad, -height, -rad);
		glVertex3f( rad, -height,  rad);
		glVertex3f(-rad, -height,  rad);
	glEnd();
}


// Draw the penguin body
void drawBody() {
	float a, b, c;
	a = 0.7;  b = 1.0;  c = 0.8;
	
	glBegin(GL_QUADS);
		// draw front
		glNormal3f(0.0, 0.0, 1.0);
		glVertex3f(-b, -b,  c);
		glVertex3f( b, -b,  c);
		glVertex3f( a,  b,  c);
		glVertex3f(-a,  b,  c);
		
		// draw back
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f( b, -b, -c);
		glVertex3f(-b, -b, -c);
		glVertex3f(-a,  b, -c);
		glVertex3f( a,  b, -c);
		
		// draw left
		glNormal3f(0.99, 0.14, 0.0);
		glVertex3f( b, -b,  c);
		glVertex3f( b, -b, -c);
		glVertex3f( a,  b, -c);
		glVertex3f( a,  b,  c);
		
		// draw right
		glNormal3f(-0.99, 0.14, 0.0);
		glVertex3f(-b, -b, -c);
		glVertex3f(-b, -b,  c);
		glVertex3f(-a,  b,  c);
		glVertex3f(-a,  b, -c);
		
		//draw top
		glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(-a,  b,  c);
		glVertex3f( a,  b,  c);
		glVertex3f( a,  b, -c);
		glVertex3f(-a,  b, -c);
		
		// draw bottom
		glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(-b, -b,  c);
		glVertex3f( b, -b,  c);
		glVertex3f( b, -b, -c);
		glVertex3f(-b, -b, -c);
	glEnd();
}


//  draw upper beak
void drawBeak(float height) {
	float a, b, c;
	a = 0.05;  b = height;  c = 0.15;
    
	glBegin(GL_QUADS);
        // draw front
		glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(-a,  b,  c);
        glVertex3f( a,  b,  c);
        glVertex3f( a, -b,  c);
        glVertex3f(-a, -b,  c);
        
        // draw back
		glNormal3f(0.0, 0.0, -1.0);
        glVertex3f(-c,  b, -c);
        glVertex3f( c,  b, -c);
        glVertex3f( c, -b, -c);
        glVertex3f(-c, -b, -c);
        
        // draw left
		glNormal3f(0.97, 0.0, 0.243);
        glVertex3f( a,  b,  c);
        glVertex3f( c,  b, -c);
        glVertex3f( c, -b, -c);
        glVertex3f( a, -b,  c);
        
        // draw right
		glNormal3f(-0.97, 0.0, 0.243);
        glVertex3f(-a,  b,  c);
        glVertex3f(-c,  b, -c);
        glVertex3f(-c, -b, -c);
        glVertex3f(-a, -b,  c);
        
        // draw top
		glNormal3f(0.0, 1.0, 0.0);
        glVertex3f(-a,  b,  c);
        glVertex3f( a,  b,  c);
        glVertex3f( c,  b,  -c);
        glVertex3f(-c,  b,  -c);
        
        // draw bottom
		glNormal3f(0.0, -1.0, 0.0);
        glVertex3f(-a, -b,  c);
        glVertex3f( a, -b,  c);
        glVertex3f( c, -b,  -c);
        glVertex3f(-c, -b,  -c);
    glEnd();
}


// draw arm; default is for left arm
void drawArm() {
	float a;
	a = 0.5;
	glBegin(GL_QUADS);
        // draw front
        glNormal3f(0.0, -0.12, 0.99);
		glVertex3f(0.1, a, 0.3);
		glVertex3f(0.1, -1.05, 0.15);
		glVertex3f(0.0, -1.05, 0.15);
		glVertex3f(0.0, a, 0.3);
		
        // draw back
		glNormal3f(0.0, 0.0, -1.0);
		glVertex3f(0.1, 0.7, -0.25);
		glVertex3f(0.1, -1.15, -0.25);
		glVertex3f(0.0, -1.15, -0.25);
		glVertex3f(0.0, 0.7, -0.25);
		
        // draw outside
        glNormal3f(1.0, 0.0, 0.0);
		glVertex3f(0.1, a, 0.3);
		glVertex3f(0.1, 0.7, -0.25);
		glVertex3f(0.1, -1.15, -0.25);
		glVertex3f(0.1, -1.05, 0.15);
        
        // draw inside
        glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(0.0, a, 0.3);
		glVertex3f(0.0, 0.7, -0.25);
		glVertex3f(0.0, -1.15, -0.25);
		glVertex3f(0.0, -1.05, 0.15);
        
        // draw top
        glNormal3f(0.0, 0.99, -0.14);
		glVertex3f(0.1, a, 0.3);
		glVertex3f(0.1, 0.7, -0.25);
		glVertex3f(0.0, 0.7, -0.25);
		glVertex3f(0.0, a, 0.3);
        
        // draw bottom
        glNormal3f(0.0, -0.97, 0.24);
		glVertex3f(0.1, -1.05, 0.15);
		glVertex3f(0.1, -1.15, -0.25);
		glVertex3f(0.0, -1.15, -0.25);
		glVertex3f(0.0, -1.05, 0.15);
    glEnd();
}


// draw hand; default is for left handle
void drawHand() {
	glBegin(GL_QUADS);
        // draw front
		glNormal3f(0.0, 0.45, 0.89);
		glVertex3f(0.1, -1.05, 0.15);
		glVertex3f(0.1, -1.25, 0.25);
		glVertex3f(0.0, -1.25, 0.25);
		glVertex3f(0.0, -1.05, 0.15);
        
        // draw back
		glNormal3f(0.0, -0.45, -0.89);
		glVertex3f(0.1, -1.15, -0.25);
		glVertex3f(0.0, -1.15, -0.25);
		glVertex3f(0.0, -1.35, -0.15);
		glVertex3f(0.1, -1.35, -0.15);
        
        // draw outside
        glNormal3f(1.0, 0.0, 0.0);
		glVertex3f(0.1, -1.15, -0.25);
		glVertex3f(0.1, -1.35, -0.15);
		glVertex3f(0.1, -1.25, 0.25);
		glVertex3f(0.1, -1.05, 0.15);
        
        // draw inside
        glNormal3f(-1.0, 0.0, 0.0);
		glVertex3f(0.1, -1.15, -0.25);
		glVertex3f(0.1, -1.35, -0.15);
		glVertex3f(0.1, -1.25, 0.25);
		glVertex3f(0.1, -1.05, 0.15);
        
        // draw top
        glNormal3f(0.0, 0.97, -0.24);
		glVertex3f(0.1, -1.05, 0.15);
		glVertex3f(0.1, -1.15, -0.25);
		glVertex3f(0.0, -1.15, -0.25);
		glVertex3f(0.0, -1.05, 0.15);
		
        // draw bottom
        glNormal3f(0.0, -0.97, 0.24);
		glVertex3f(0.0, -1.35, -0.15);
		glVertex3f(0.1, -1.35, -0.15);
		glVertex3f(0.1, -1.25, 0.25);
		glVertex3f(0.0, -1.25, 0.25);
    glEnd();
}


// draw foot; default is for left foot
void drawFoot() {
	float a, b, m, n;
	a = -0.25; b = 0.35;
	m = 0.1; n = 0.5;
	glBegin(GL_QUADS);
        // draw front
        glNormal3f(0.39, 0.0, 0.92);
		glVertex3f(-a, -0.1,  b);
		glVertex3f(-a,  0.0,  b);
		glVertex3f(-m,  0.0,  n);
		glVertex3f(-m, -0.1,  n);
		
        // draw left
        glNormal3f(0.81, 0.0, -0.58);
		glVertex3f(-a,  0.0,  b);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, -0.1, 0.0);
		glVertex3f(-a, -0.1,  b);
        
        // draw right
        glNormal3f(-0.98, 0.0, -0.2);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, -0.1, 0.0);
		glVertex3f(-m, -0.1,  n);
		glVertex3f(-m,  0.0,  n);
        
        // draw top
        glNormal3f(0.0, 1.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(-m, 0.0,  n);
		glVertex3f(-a, 0.0,  b);
        
        // draw bottom
        glNormal3f(0.0, -1.0, 0.0);
		glVertex3f(0.0, -0.1, 0.0);
		glVertex3f(0.0, -0.1, 0.0);
		glVertex3f(-m, -0.1,  n);
		glVertex3f(-a, -0.1,  b);
    glEnd();
}


// draw circle
void drawCircle(float radius)
{
   glBegin(GL_LINE_LOOP);
   for (int i=0; i < 360; i++) {
      float Rads = i*DEG2RAD;
      glVertex2f(cos(Rads)*radius,sin(Rads)*radius);
   }
   glEnd();
}

void drawCylinder(float radius, float height){
   glBegin(GL_LINE_LOOP);
   float h = -height;
   for(;h<=height;h=h+0.01)
   for (int i=0; i < 360; i++) {
      float Rads = i*DEG2RAD;
      glVertex3f(cos(Rads)*radius,h,sin(Rads)*radius);
   }
      
   glEnd();  

   glBegin(GL_QUADS);

   h = -height;
   for(int j=0; j < 360; j++)
   {
      glVertex3f(cos(j)*radius,h,sin(j)*radius);
      glVertex3f(0,h,0);
      glVertex3f(cos(j+1)*radius,h,sin(j+1)*radius);
  }
  
   h = height;
   for(int j=0; j < 360; j++)
   {
      glVertex3f(cos(j)*radius,h,sin(j)*radius);
      glVertex3f(0,h,0);
      glVertex3f(cos(j+1)*radius,h,sin(j+1)*radius);
  }
     glEnd(); 

}


// from util.cpp of part 1 of this assignment
// returns normal to divide by
double norm(double a, double b, double c) 
{
    double denom = 1.0;
	double x = (a > 0.0) ? a : -a;
	double y = (b > 0.0) ? b : -b;
	double z = (c > 0.0) ? c : -c;

	if(x > y) {
		if(x > z) {
			if(1.0 + x > 1.0) {
				y = y / x;
				z = z / x;
				denom = 1.0 / (x * sqrt(1.0 + y*y + z*z));
			}
		} else { /* z > x > y */ 
			if(1.0 + z > 1.0) {
				y = y / z;
				x = x / z;
				denom = 1.0 / (z * sqrt(1.0 + y*y + x*x));
			}
		}
	} else {
		if(y > z) {
			if(1.0 + y > 1.0) {
				z = z / y;
				x = x / y;
				denom = 1.0 / (y * sqrt(1.0 + z*z + x*x));
			}
		} else { /* x < y < z */
			if(1.0 + z > 1.0) {
				y = y / z;
				x = x / z;
				denom = 1.0 / (z * sqrt(1.0 + y*y + x*x));
			}
		}
	}

	if(1.0 + x + y + z > 1.0) {
		a *= denom;
		b *= denom;
		c *= denom;
		return 1.0 / denom;
	}

	return 0.0;
}


void drawPyramid(float width, float height)
{
    double a, b, c, normal;
    
    glBegin(GL_TRIANGLE_FAN);
        //a = -w*h; b = pow(w, 2)/2; c = 0;
        a= -height; b = width/2; c =0;
        normal = norm(a, b, c);
        glNormal3f(a/normal, b/normal, c/normal);
	    glVertex3f(-width/2,0,-width/2);
	    glVertex3f(-width/2,0,width/2);
	    glVertex3f(0,height,0);

        //a = 0; b = -pow(w, 2)/2; c = w*h;
	    a=0;b= -width/2; c= height;
        normal = norm(a, b, c);
        glNormal3f(a/normal, b/normal, c/normal);
	    glVertex3f(-width/2,0,-width/2);
	    glVertex3f(width/2,0,-width/2);
	    glVertex3f(0,height,0);

        //a = -w*h; b = -pow(w, 2)/2; c = 0;
	    a = height; b = width/2; c =0;
        normal = norm(a, b, c);
        glNormal3f(a/normal, b/normal, c/normal);
	    glVertex3f(width/2,0,-width/2);
	    glVertex3f(width/2,0,width/2);
	    glVertex3f(0,height,0);

        //a = 0; b = -pow(w, 2)/2; c = -w*h;
	    a= b; b= width/2; c= height;
        normal = norm(a, b, c);
        glNormal3f(a/normal, b/normal, c/normal);
	    glVertex3f(width/2,0,width/2);
	    glVertex3f(-width/2,0,width/2);
	    glVertex3f(0,height,0);
	      
	    glNormal3f(0.0, -1.0, 0.0);
	    glVertex3f(width/2,0,width/2);
	    glVertex3f(-width/2,0,width/2);
        glVertex3f(-width/2,0,-width/2);
        glVertex3f(width/2,0,-width/2);
    glEnd();
}

