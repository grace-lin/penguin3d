#include "equipment.h"

void drawSSPlank() {
    float a, b, c, d;
    a = 5.0;  b = 0.3;  c = 0.8;  d = 0.0;
    glBegin(GL_QUADS);
        // front
        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(-a, b, c);
        glVertex3f(-a, d, c);
        glVertex3f( a, d, c);
        glVertex3f( a, b, c);
        
        // back
        glNormal3f(0.0, 0.0, -1.0);
        glVertex3f(-a, b, -c);
        glVertex3f(-a, d, -c);
        glVertex3f( a, d, -c);
        glVertex3f( a, b, -c);
        
        // right
        glNormal3f(1.0, 0.0, 0.0);
        glVertex3f(a, b,  c);
        glVertex3f(a, d,  c);
        glVertex3f(a, d, -c);
        glVertex3f(a, b, -c);
        
        //left
        glNormal3f(-1.0, 0.0, 0.0);
        glVertex3f(-a, b,  c);
        glVertex3f(-a, d,  c);
        glVertex3f(-a, d, -c);
        glVertex3f(-a, b, -c);
        
        // top
        glNormal3f(0.0, 1.0, 0.0);
        glVertex3f(-a, b, -c);
        glVertex3f(-a, b,  c);
        glVertex3f( a, b,  c);
        glVertex3f( a, b, -c);
        
        // bottom
        glNormal3f(0.0, -1.0, 0.0);
        glVertex3f(-a, d, -c);
        glVertex3f(-a, d,  c);
        glVertex3f( a, d,  c);
        glVertex3f( a, d, -c);
    glEnd();
}


// default is right leg; scale by (-1.0, 1.0, 1.0) for left leg
void drawSSLegs() {
    float a, b, c, d, e;
    //a = 0.0;  b = 0.1;  c = 0.2;  d = 1.4;  e = 1.6;
    a = 0.0;  b = 0.2;  c = 0.4;  d = 2;  e = 2.5;
    glBegin(GL_QUADS);
        // front
        glNormal3f(0.0, 0.0, 1.0);
        glVertex3f(-c,  a, b);
        glVertex3f(-c, -d, b);
        glVertex3f( a, -e, b);
        glVertex3f( a,  a, b);
        
        // back
        glNormal3f(0.0, 0.0, -1.0);
        glVertex3f(-c,  a, b);
        glVertex3f(-c, -d, b);
        glVertex3f( a, -e, b);
        glVertex3f( a,  a, b);
        
        // right
        glNormal3f(1.0, 0.0, 0.0);
        glVertex3f(a,  a,  b);
        glVertex3f(a, -e,  b);
        glVertex3f(a, -e, -b);
        glVertex3f(a,  a, -b);
        
        //left
        glNormal3f(-1.0, 0.0, 0.0);
        glVertex3f(-c,  a,  b);
        glVertex3f(-c, -d,  b);
        glVertex3f(-c, -d, -b);
        glVertex3f(-c,  a, -b);
        
        // top
        glNormal3f(0.0, 1.0, 0.0);
        glVertex3f(-c, a, -b);
        glVertex3f(-c, a,  b);
        glVertex3f(a,  a,  b);
        glVertex3f(a,  a, -b);
        
        // bottom
        glNormal3f(-0.7071, -0.7071, 0.0);
        glVertex3f(-c, -d, -b);
        glVertex3f(-c, -d,  b);
        glVertex3f( a, -e,  b);
        glVertex3f( a, -e, -b);
    glEnd();
}