/*
 * drawplant.cpp
 * -------------
 * Contains the drawing routine and related helper functions for the
 * L-system fractal plant.  Starter code for Project 2.
 *
 * Group Members: <FILL IN>
 */

#ifdef _WIN32
#include <windows.h>
#endif
#include <cmath>
#include <assert.h>
#include <iostream>

#include "common.h"
#include "drawplant.h"
#include "readppm.h"
#include <stdio.h>
#include <vector>

using namespace std;

int depth = 4;
GLfloat orientation = 0;
vector<Matrix3f> stack;

void load_matrix(Matrix3f);
Matrix3f current_matrix();
void push(){
    stack.push_back(current_matrix());
}

void print_matrix(const char *title, Matrix3f m){
    printf("%s\n", title);
    for(int i = 0; i < 4; i++){
        printf("%f, %f, %f, %f\n", m.m[i], m.m[i + 4], m.m[i + 8], m.m[i + 12]);
    }
}

void print_vector(const char *title, Vector3f v){
    printf("%s\n%f, %f, %f, %f\n", title, v.v[0], v.v[1], v.v[2], v.v[3]);
}

void pop(){
    if(stack.size() == 0)
        return; 
    else{
        Matrix3f result = stack.back();
        stack.pop_back();
        load_matrix(result);
    }
}

Matrix3f current_matrix(){
    Matrix3f result;
    glGetDoublev(GL_MODELVIEW_MATRIX, result.m);
    return result;
}

Vector3f mvmult(Matrix3f m, Vector3f v){
    GLdouble array[4];
    for(int i = 0; i < 4; i++){
        array[i] = m.m[i] * v.v[0] + m.m[i + 4] *  v.v[1] + m.m[i + 8] * v.v[2] + m.m[i + 12] * v.v[3];
        //printf("m: %f, %f, %f, %f\n", m.m[i], m.m[i + 4], m.m[i + 8], m.m[i + 12]);
        //printf("v: %f, %f, %f, %f\n", v.v[0], v.v[1], v.v[2], v.v[3]);
    }
    return (Vector3f){{array[0], array[1], array[2], array[3]}};
}

Matrix3f mmmult(Matrix3f m1, Matrix3f m2){
    GLdouble array[16];
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            array[i * 4 + j] = m1.m[j] * m2.m[i * 4] + 
                m1.m[4 + j] * m2.m[i * 4 + 1] + 
                m1.m[8 + j] * m2.m[i * 4 + 2] + 
                m1.m[12 + j] * m2.m[i * 4 + 3];
        }
    }
    return (Matrix3f){{array[0], array[1], array[2], array[3], 
        array[4], array[5], array[6], array[7], 
        array[8], array[9], array[10], array[11], 
        array[12], array[13], array[14], array[15]}};
}

Matrix3f scale_matrix(GLdouble x, GLdouble y, GLdouble z){
    return (Matrix3f){{x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, z, 0,
        0, 0, 0, 1}};
}

Matrix3f translate_matrix(GLdouble x, GLdouble y, GLdouble z){
    return (Matrix3f){{1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, z, 1}}; 
}

Matrix3f quaternion(GLdouble theta, Vector3f v, Vector3f point){
    //normalize v
    GLdouble mag = sqrt(v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2]);
    for(int i = 0; i < 3; i++)
        v.v[1] /= mag;
    GLdouble c = cos(theta);
    GLdouble s = sin(theta);
    GLdouble x = v.v[0];
    GLdouble y = v.v[1];
    GLdouble z = v.v[2];
    Matrix3f quat = {{c + x *  x * (1 - c), x * y * (1 - c) + z * s, x * z * (1 - c) - y * s, 0,
        x * y * (1 - c) - z * s, c + y * y * (1 - c), y * z * (1 - c) + x * s, 0,
        x * z * (1 - c) + y * s, y * z * (1 - c) - x * s, c + z * z * (1 - c), 0,
        0, 0, 0, 1
    }
    };
    //return mmmult(translate(point.v[0], point.v[1], point.v[2]), quat);
    return mmmult(translate_matrix(point.v[0], point.v[1], point.v[2]), mmmult(quat,translate_matrix(-1 * point.v[0], -1 * point.v[1], -1 * point.v[2])));
    return quat;
}

void scale(GLdouble x, GLdouble y, GLdouble z){
    load_matrix(mmmult(scale_matrix(x, y, z), current_matrix()));
}

void translate(GLdouble x, GLdouble y, GLdouble z){
    load_matrix(mmmult(translate_matrix(x, y, z), current_matrix()));
}

void rotate(GLdouble theta, GLdouble x, GLdouble y, GLdouble z, GLdouble px, GLdouble py, GLdouble pz){
    load_matrix(mmmult(quaternion(theta, (Vector3f){{x, y, z, 0}}, (Vector3f){{px, py, pz}}), current_matrix()));
}
/* Takes a 2D matrix in row-major order, and loads the 3D matrix which
   does the same trasformation into the OpenGL MODELVIEW matrix, in
   column-major order. */

void load2DMatrix(
        GLdouble m00, GLdouble m01, GLdouble m02,
        GLdouble m10, GLdouble m11, GLdouble m12,
        GLdouble m20, GLdouble m21, GLdouble m22) {

    GLdouble M3D [16];  /* three dimensional matrix doing same transform */

    M3D[0] = m00;  M3D[1] = m10; M3D[2] = 0.0; M3D[3] = m20;
    M3D[4] = m01;  M3D[5] = m11; M3D[6] = 0.0; M3D[7] = m21;
    M3D[8] = 0.0;  M3D[9] = 0.0; M3D[10] = 1.0; M3D[11] = 0.0;
    M3D[12] = m02; M3D[13] = m12; M3D[14] = 0.0; M3D[15] = m22;

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(M3D);
}

// returns Matrix3f containing the parameter values

Matrix3f Matrix2D(
        GLdouble m00, GLdouble m01, GLdouble m02,
        GLdouble m10, GLdouble m11, GLdouble m12,
        GLdouble m20, GLdouble m21, GLdouble m22) {

    GLdouble M3D [16];  /* three dimensional matrix doing same transform */
    M3D[0] = m00;  M3D[1] = m10; M3D[2] = 0.0; M3D[3] = m20;
    M3D[4] = m01;  M3D[5] = m11; M3D[6] = 0.0; M3D[7] = m21;
    M3D[8] = 0.0;  M3D[9] = 0.0; M3D[10] = 1.0; M3D[11] = 0.0;
    M3D[12] = m02; M3D[13] = m12; M3D[14] = 0.0; M3D[15] = m22;
    return (Matrix3f){{M3D[0], M3D[1], M3D[2], M3D[3],
        M3D[4], M3D[5], M3D[6], M3D[7],
        M3D[8], M3D[9], M3D[10], M3D[11],
        M3D[12], M3D[13], M3D[14], M3D[15]}}; 
}

Matrix3f Matrix3D(
        GLdouble m00, GLdouble m01, GLdouble m02, GLdouble m03,
        GLdouble m10, GLdouble m11, GLdouble m12, GLdouble m13,
        GLdouble m20, GLdouble m21, GLdouble m22, GLdouble m23,
        GLdouble m30, GLdouble m31, GLdouble m32, GLdouble m33) {

    GLdouble M3D[16];
    M3D[0] = m00;  M3D[1] = m10; M3D[2] = m20; M3D[3] = m30;
    M3D[4] = m01;  M3D[5] = m11; M3D[6] = m21; M3D[7] = m31;
    M3D[8] = m02;  M3D[9] = m12; M3D[10] = m22; M3D[11] = m32;
    M3D[12] = m03; M3D[13] = m13; M3D[14] = m23; M3D[15] = m33;
    return (Matrix3f){{M3D[0], M3D[1], M3D[2], M3D[3],
        M3D[4], M3D[5], M3D[6], M3D[7],
        M3D[8], M3D[9], M3D[10], M3D[11],
        M3D[12], M3D[13], M3D[14], M3D[15]}}; 
}

/* Takes a 3D matrix in row-major order, and loads the 3D matrix which
   does the same trasformation into the OpenGL MODELVIEW matrix, in
   column-major order. */
void load3DMatrix(
        GLdouble m00, GLdouble m01, GLdouble m02, GLdouble m03,
        GLdouble m10, GLdouble m11, GLdouble m12, GLdouble m13,
        GLdouble m20, GLdouble m21, GLdouble m22, GLdouble m23,
        GLdouble m30, GLdouble m31, GLdouble m32, GLdouble m33) {

    /* ADD YOUR CODE */
    GLdouble M3D[16];
    M3D[0] = m00;  M3D[1] = m10; M3D[2] = m20; M3D[3] = m30;
    M3D[4] = m01;  M3D[5] = m11; M3D[6] = m21; M3D[7] = m31;
    M3D[8] = m02;  M3D[9] = m12; M3D[10] = m22; M3D[11] = m32;
    M3D[12] = m03; M3D[13] = m13; M3D[14] = m23; M3D[15] = m33;

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(M3D);
}

void load_matrix(Matrix3f m){
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixd(m.m);
}

// always "vertical" cylinder
void draw_cylinder(GLdouble radius, GLdouble height, Vector3f base){
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.54,0.27,0.07); 
    GLdouble theta = 2 * PI / 8;
    GLdouble r = cos(theta);
    GLdouble tan_fact = tan(theta);
    GLdouble x, y, z;
    x = radius;
    y = base.v[1];
    z = 0;
    glVertex3f(base.v[0], y, base.v[2]);
    for(int i = 0; i < 9; i++){
        glVertex3f(x + base.v[0], y, z + base.v[2]);
        GLdouble tx = -1 * z;
        GLdouble tz = x;
        x += tx * tan_fact;
        z += tz * tan_fact;
        x *= r;
        z *= r;
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.54,0.27,0.07); 
    x = radius;
    y += height;
    z = 0;
    glVertex3f(base.v[0], y, base.v[2]);
    for(int i = 0; i < 9; i++){
        glVertex3f(x + base.v[0], y, z + base.v[2]);
        GLdouble tx = -1 * z;
        GLdouble tz = x;
        x += tx * tan_fact;
        z += tz * tan_fact;
        x *= r;
        z *= r;
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.54,0.27,0.07); 
    x = radius; 
    y = base.v[1];
    z = 0;
    glVertex3f(base.v[0], y, base.v[2]);
    glVertex3f(base.v[0], y + height, base.v[2]);
    for(int i = 0; i < 9; i++){
        glVertex3f(x + base.v[0], y, z + base.v[2]);
        glVertex3f(x + base.v[0], y + height, z + base.v[2]);
        GLdouble tx = -1 * z;
        GLdouble tz = x;
        x += tx * tan_fact;
        z += tz * tan_fact;
        x *= r;
        z *= r;
    }
    glEnd();
}



void drawLeaf(void) {
    /* ADD YOUR CODE to make the 2D leaf a 3D extrusion */

    glColor3d(0.1,0.9,0.1); 
    glBegin(GL_POLYGON);
    glVertex3d(0.0,0.0, 0);
    glVertex3d(1.0,0.7, 0);
    glVertex3d(1.3,1.8, 0);
    glVertex3d(1.0,2.8, 0);
    glVertex3d(0.0,4.0, 0);
    glVertex3d(-1.0,2.8, 0);
    glVertex3d(-1.3,1.8, 0);
    glVertex3d(-1.0,0.7, 0);

    glEnd();
}
/*
   Vector3f seek_vector(Vector3f start, Vector3f end, GLdouble f){
//Vector3f v = {{end.v[0] - start.v[0], end.v[1] - start.v[1], end.v[2] - start.v[2], end.v[3] - start.v[3]}};
return (Vector3f){{start.v[0] + (end.v[0] - start.v[0]) * f, start.v[1] + (end.v[1] - start.v[1]) * f, start.v[2] + (end.v[2] - start.v[2]) * f, 1}};
}
*/
void drawBranch(GLdouble radius, GLdouble length, Vector3f base, int level) {

    /* ADD YOUR CODE to make the 2D branch a 3D extrusion */
    //printf("base: %f, %f. %f\n", base.v[0], base.v[1], base.v[2]);
    print_matrix("current matrix before draw", current_matrix());
    push();
    rotate(orientation, 0, 1, 0, 0, 0, 0);
    draw_cylinder(radius, length, base);
    drawLeaf();
    pop();
    /*    
          push();
          Matrix3f quat = quaternion(30, (Vector3f){{0, 0, 1, 0}}, base);
          load_matrix(mmmult(quat, current_matrix()));
          load_matrix(mmmult(translate_matrix(0,  2 * length / 3, 0), current_matrix()));
          print_matrix("current matrix before recursive call", current_matrix());
          load_matrix(mmmult(scale_matrix(.75, .75, .75), current_matrix()));
          draw_cylinder(radius, length, base);

          pop();
          */
    //Matrix3f current = current_matrix();
    //printf("current\n");
    //print_matrix(current_matrix());
    /* 
       glTranslatef(0, 2 * length / 3, 0); // object relative rotate
       glRotatef(30, 0, 0, 1); // object relative rotate
       */ 
    //Vector3f new_base = {{base.v[0], base.v[1] + 2 * length / 3, base.v[2], base.v[3]}};
    for(int i = -1; i <= 1; i+= 2){
        //theta = 2 * PI / 3 * rand() / (GLdouble)RAND_MAX - PI / 3;
        GLdouble theta = i * PI / 6; 
        printf("theta: %f\n", theta * 180 / PI);
        //theta = PI / 6;
        //glPushMatrix();
        push();
        /*
           glTranslatef(.0, 2 * length / 3, 0);
           glRotatef(theta * 180 / PI, 0, 0, 1);
           glScalef(.75, .75, .75);
           */

        //print_vector("base", base);
        //print_matrix("current_matrix", current_matrix());
        //Vector3f new_base = mvmult(current_matrix(), mvmult(current_matrix(), base));
        //print_vector("new_base", new_base);
        //Matrix3f quat = quaternion(theta, (Vector3f){{0, 0, 1, 0}}, new_base);
        //print_matrix("quaternion", quat);
        //load_matrix(mmmult(scale_matrix(.75, .75, .75), mmmult(translate_matrix(0, length / .75, 0), mmmult(quat, current_matrix())))); 

        //load_matrix(mmmult(quat, current_matrix()));
        rotate(theta, 0, 0, 1, base.v[0], base.v[1], base.v[2]);
        //draw_cylinder(radius, length, base);
        //load_matrix(mmmult(translate_matrix(0, length / 3, 0), current_matrix()));
        translate(0, 2 * length / 3, 0);
        //load_matrix(mmmult(scale_matrix(.75, .75, .75), current_matrix()));
        scale(.75, .75, .75);
        print_matrix("current matrix before recursive call", current_matrix());
        //mmmult(quaternion(PI / 6, (Vector3f){{0, 0, 1, 0}}, base), current_matrix());
        //load_matrix(quaternion(PI / 6, (Vector3f){{0, 0, 1, 0}}, new_base));
        if(level > 0)
            drawBranch(radius, length, base, level - 1);
        /*
        else{
            draw_cylinder(radius, length, base);
            drawLeaf();
        }
        */
        //glPopMatrix();
        pop();
    }
    /*
       glColor3f(0.54,0.27,0.07); 
       glBegin(GL_POLYGON);
       glVertex2f(1.0,0.0);
       glVertex2f(1.0,6.0);
       glVertex2f(-1.0,6.0);
       glVertex2f(-1.0,0.0);
       glEnd();
       */
}

/*
 * Draws the plant.
 *
 * ADD YOUR CODE and modify the function to take an L-system depth and
 * any other necessary arguments.
 */

void drawPlant(GLdouble rotate) {
    printf("New Tree\n");
    orientation += rotate;
    drawBranch(1, 20, (Vector3f){{0, -20, 0, 1}}, depth);
    /*
       glPushMatrix(); 
       draw_cylinder(1, 20, (Vector3f){{0, 0, 0, 1}});
       glRotatef(30, 0, 0, 1);
       glTranslatef(0, 20, 0);
       draw_cylinder(1, 20, (Vector3f){{0, 0, 0, 1}});
       glRotatef(30, 0, 0, 1);
       glTranslatef(0, 20, 0);
       draw_cylinder(1, 20, (Vector3f){{0, 0, 0, 1}});
       glPopMatrix();
       */
    /*
       Matrix3f trans =translate_matrix(1, 5, -7);
       for(int i = 0; i < 16; i++)
       printf("%f\n", trans.m[i]);
       Vector3f original = {{4 ,6, 3, 1}};
       Vector3f original1 = mvmult(quaternion(PI / 2, (Vector3f){{0, 0, 1, 0}}, (Vector3f){{0, 0, 0, 1}}), original);
       printf("x: %f, y: %f, z: %f\n", original1.v[0], original1.v[1], original1.v[2]);
       */
}
/* end of drawplant.c */
