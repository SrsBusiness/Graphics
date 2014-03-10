/*
 * drawing.c
 * ---------
 * Drawing subroutines for each of the various display modes in the canvas.
 * Also contains the quadrilateral information for a cube and the
 * triangulation information for a cone.
 *
 * Starter code for Project 1.
 *
 * Group Members: <FILL IN>
 */

#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include "drawing.h"
#include "vrml.h"
#include <math.h>
#include <vector>
#define PI 3.14159265358

using namespace std;

/* The current display mode */
int disp_mode;

/* The current display style */
int disp_style;
struct Vector3f{
    GLdouble v[4];
} typedef Vector3f;

struct Matrix3f{
    GLdouble m[16];
} typedef Matrix3f;

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

void rotate(GLdouble theta, GLfloat x, GLfloat y, GLfloat z, GLfloat px, GLfloat py, GLfloat pz){
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
    draw_cylinder(radius, length, base);
            //drawLeaf();
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
        GLfloat theta = i * PI / 6; 
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
        else{
            draw_cylinder(radius, length, base);
            //drawLeaf();
        }
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

void drawPlant(void) {
    printf("New Tree\n");
    drawBranch(.2, 1, (Vector3f){{0, -1, 0, 1}}, 6);
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
GLfloat cube_vertices[] = {
    -0.5f,  0.5f,  0.5f,
    0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    0.5f, -0.5f, -0.5f,
};

/*
 * The colors of each vertex in the low level cube.  The index
 * into this array corresponds to the index into cube_vert.
 */
GLfloat cube_colors[] = {
    0.5f, 0.5f, 0.5f,
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,
};

/*
 * Indices into cube_vert, groups of 4 can be used to draw quadrilaterals
 * which represents a face of the cube.  For instance, the first
 * quad means that vertices 0, 1, 3, 2 of the cube_vertices[]
 * are used, in that order, to form the first face of the cube.
 *
 * Note that all front facing quads are specified in a counterclockwise order
 * (that is, if you are looking at the front of a quad, the vertices will
 * appear in counterclockwise order).
 */
GLuint cube_indices[] = {
    0, 2, 3, 1,
    2, 6, 7, 3,
    7, 6, 4, 5,
    4, 0, 1, 5,
    1, 3, 7, 5,
    0, 4, 6, 2,
};
/***********************************************************
 * End Cube Data
 ***********************************************************/


/***********************************************************
 * Begin Cone Data
 ***********************************************************/

/*
 * Data for the triangulation of the surface of a cone that is one
 * unit tall has a unit circle for its base.  The base is composed
 * of 8 equally sized triangles, and the lateral surface of the cone
 * is composed of a different set of 8 equally sized triangles.
 *
 * See documentation in the Cube Data section for information on
 * the meaning of the data in each array.
 */

GLfloat cone_vertices[] = {
    1.0, -0.5, 0.0,         /* begin unit circle at y = -0.5 */
    0.707, -0.5, 0.707,
    0.0, -0.5, 1.0,
    -0.707, -0.5, 0.707,
    -1.0, -0.5, 0.0,
    -0.707, -0.5, -0.707,
    0.0, -0.5, -1.0,
    0.707, -0.5, -0.707,    /* end unit circle at y = -0.5 */
    0.0, -0.5, 0.0,         /* center of the base */
    0.0, 0.5, 0.0,          /* top of the cone */
};

GLfloat cone_colors[] = {
    0.0f, 0.0f, 0.5f,
    0.0f, 0.5f, 0.5f,
    0.5f, 0.5f, 0.5f,
    0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,
    1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,
};

/*
 * Each set of 3 indices represent the vertices of a triangle.
 * Each triangle faces to the outside of the cone.  The vertices
 * of these forward facing triangles are specified in
 * counterclockwise order.
 */
GLuint cone_indices[] = {
    0, 1, 8,
    1, 2, 8,
    2, 3, 8,
    3, 4, 8,
    4, 5, 8,
    5, 6, 8,
    6, 7, 8,
    7, 0, 8,
    1, 0, 9,
    2, 1, 9,
    3, 2, 9,
    4, 3, 9,
    5, 4, 9,
    6, 5, 9,
    7, 6, 9,
    0, 7, 9,
};
/***********************************************************
 * End Cone Data
 ***********************************************************/


/* Uses glut to draw a cube */
void draw_cube_glut(void) {
    /* Draw the cube using glut */

    glColor3f(1.0f, 0.0f, 0.0f);
    if (disp_style == DS_SOLID) {
        glutSolidCube(1.0f);
    } else if (disp_style == DS_WIRE) {
        glutWireCube(1.0f);
    }
}

/*
 * Draws a cube using the data arrays at the top of this file.
 * Iteratively draws each quad in the cube.
 */
void draw_cube_quad(void) {
    int num_indices;
    int i;
    int index1, index2, index3, index4;

    num_indices = sizeof(cube_indices) / sizeof(GLuint);

    /*
     * Loop over all quads that need to be draen.
     * Step i by 4 because there are 4 vertices per quad.
     */
    for (i = 0; i < num_indices; i += 4) {
        /*
         * Find the index into the vertex array.  The value
         * in the cube_indices array refers to the index
         * of the ordered triples, not the index for the
         * actual GLfloats that comprise the cube_vertices array.
         * Thus, we need to multiple by 3 to get the real index.
         */
        index1 = cube_indices[i] * 3;
        index2 = cube_indices[i+1] * 3;
        index3 = cube_indices[i+2] * 3;
        index4 = cube_indices[i+3] * 3;

        glBegin(GL_QUADS);

        /* All arguments here are pointers */
        glColor3fv(  &(cube_colors[index1]) );
        glVertex3fv( &(cube_vertices[index1]) );
        glColor3fv(  &(cube_colors[index2]) );
        glVertex3fv( &(cube_vertices[index2]) );
        glColor3fv(  &(cube_colors[index3]) );
        glVertex3fv( &(cube_vertices[index3]) );
        glColor3fv(  &(cube_colors[index4]) );
        glVertex3fv( &(cube_vertices[index4]) );

        glEnd();

    }
}

/*
 * Draws a cube using the data arrays at the top of this file.
 * Uses GL's vertex arrays, index arrays, color arrays, etc.
 */
void draw_cube_quad_arrays(void) {
    int num_indices;

    num_indices = sizeof(cube_indices) / sizeof(GLuint);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, cube_vertices);
    glColorPointer(3, GL_FLOAT, 0, cube_colors);
    glDrawElements(GL_QUADS, num_indices,
            GL_UNSIGNED_INT, cube_indices);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

/*
 * Uses glut to draw a cone.  Must render in either solid and wire
 * frame modes, based on the value of the variable disp_style.
 */
void draw_cone_glut(void) {
    /* ADD YOUR CODE HERE */
    if (disp_style == DS_SOLID) {
        glutSolidCone(1.0, 1.0, 20, 20);
    } else if (disp_style == DS_WIRE) {
        glutWireCone(1.0, 1.0, 20, 20);
    }
}

/*
 * Draws a cone using the data arrays at the top of this file.
 * Iteratively draws each triangle in the cone.
 */
void draw_cylinder(GLfloat radius, GLfloat height, GLfloat xb, GLfloat yb, GLfloat zb){
    /*
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.54,0.27,0.07); 
    glVertex3f(xb, yb, zb);
    GLfloat theta = 2 * 3.14159265358 / 8;
    GLfloat r = cosf(theta);
    GLfloat tan_fact = tanf(theta);
    GLfloat x, z;
    x = radius;
    z = 0;
    int i;
    for(i = 0; i < 9; i++){
        glVertex3f(x + xb, yb, z + zb);
        GLfloat tx = -1 * z;
        GLfloat tz = x;
        x += tx * tan_fact;
        z += tz * tan_fact;
        x *= r;
        z *= r;
    }
    glEnd();
    */
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.54,0.27,0.07); 
    GLfloat theta = 2 * 3.14159265358 / 8;
    GLfloat r = cosf(theta);
    GLfloat tan_fact = tanf(theta);
    GLfloat x, y, z;
    x = radius;
    y = yb;
    z = 0;
    int i;
    glVertex3f(xb, y, zb);
    for(i = 0; i < 9; i++){
        printf("x: %f, y: %f, z: %f\n", x, y, z);
        glVertex3f(x + xb, y, z + zb);
        GLfloat tx = -1 * z;
        GLfloat tz = x;
        x += tx * tan_fact;
        z += tz * tan_fact;
        x *= r;
        z *= r;
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0.54,0.27,0.07); 
    x = radius;
    y+= height;
    z = 0;
    glVertex3f(xb, y, zb);
    for(i = 0; i < 9; i++){
        printf("x: %f, y: %f, z: %f\n", x, y, z);
        glVertex3f(x + xb, y, z + zb);
        GLfloat tx = -1 * z;
        GLfloat tz = x;
        x += tx * tan_fact;
        z += tz * tan_fact;
        x *= r;
        z *= r;
    }
    glEnd();
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.54,0.27,0.07); 
    x = radius; 
    y = yb;
    z = 0;
    glVertex3f(xb, y, zb);
    glVertex3f(xb, y + height, zb);
    for(i = 0; i < 9; i++){
        glVertex3f(x + xb, y, z + zb);
        glVertex3f(x + xb, y + height, z + zb);
        GLfloat tx = -1 * z;
        GLfloat tz = x;
        x += tx * tan_fact;
        z += tz * tan_fact;
        x *= r;
        z *= r;
    }
    glEnd();
}

void draw_cone_tri(void) {
    /* ADD YOUR CODE HERE */
    /*
    glBegin(GL_TRIANGLES);
    int i;
    int indices = sizeof(cone_indices) / sizeof(cone_indices[0]) / 3;
    for(i = 0; i < sizeof(cone_indices) / sizeof(cone_indices[0]) / 3; i++){
        int j;
        for(j = 0; j < 3; j++){
            int index = cone_vertices[3 * i + j];
            int tmp = 3 * cone_indices[3 * i + j];
            glColor3f(cone_colors[tmp], cone_colors[tmp + 1], cone_colors[tmp + 2]);
            glVertex3f(cone_vertices[tmp], cone_vertices[tmp + 1], cone_vertices[tmp + 2]);
        }
    }
    glEnd();
    */
    //draw_cylinder(1, .2, 0, 0, 0);
    drawPlant();
}

/*
 * Draws a cone using the data arrays at the top of this file.
 * Uses GL's vertex arrays, index arrays, color arrays, etc.
 */
void draw_cone_tri_arrays(void) {
    /* ADD YOUR CODE HERE */
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, cone_vertices);
    glColorPointer(3, GL_FLOAT, 0, cone_colors);
    glDrawElements(GL_TRIANGLES, sizeof(cone_indices) / sizeof(cone_indices[0]), GL_UNSIGNED_INT, cone_indices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

/*
 * Draws a cone using a calculated triangulation of the cone surface.
 *
 * Args
 * ----
 * The HEIGHT and RADIUS of the cone.
 *
 * BASE_TRI refers to the number of triangles used to represent
 * the base of the cone.  Each of these triangles should share a
 * common vertex, namely, the center of the base.
 *
 * The final triangulation of the cone surface should include
 * exactly 2 * BASE_TRI.
 */
extern double h = 1.0;
extern double r = 1.0;
extern int n = 8;


void draw_cone_tri_calc(double height, double radius, int base_tri) {
    /* ADD YOUR CODE HERE */
    double tip[] = {0, height / 2, 0};
    double center[] = {0, -1 * height / 2, 0};
    double center_y = -1 * height / 2;
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0, 1.0, 0);
    glVertex3f(0, -1 * height / 2, 0);
    //double current[] = {radius, -1 * height / 2, 0};
    double current_x, current_y, current_z;
    double next_x, next_y, next_z;
    current_x = radius; current_y = next_y = center_y; current_z = 0;
    glVertex3f(current_x, current_y, current_z);
    double next[3];
    double angle = 2 * 3.14159265358 / base_tri; 
    double cosine = cos(angle);
    double sine = sin(angle);
    int i; 
    //printf("base_tri: %d\n", base_tri);
    for(i = 0; i < base_tri; i++){
        // compute next
        //next = {current[0] * cosine + current[2] * -1 * sine, center_y, 
        //    current[0] * sine + current[2] * cos};
        next_x = current_x * cosine + current_z * -1 * sine;
        next_z = current_x * sine + current_z * cosine;
        glVertex3f(next_x, next_y, next_z);
        current_x = next_x;
        current_z = next_z;
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(0, 1.0, 0);
    glVertex3f(0, height / 2, 0);
    current_y = next_y = center_y;
    current_x = radius; current_z = 0;
    for(i = 0; i < base_tri; i++){
        // compute next
        //next = {current[0] * cosine + current[2] * -1 * sine, center_y, 
        //    current[0] * sine + current[2] * cos};
        next_x = current_x * cosine + current_z * -1 * sine;
        next_z = current_x * sine + current_z * cosine;
        glVertex3f(next_x, next_y, next_z);
        current_x = next_x;
        current_z = next_z;
    }
    glEnd();
}


/* Draw the various vrml scenes */
void draw_vrml(void) {
    /* ADD YOUR CODE HERE */
    /* NOTE: you should be calling a function or functions in vrml.c */
    //draw_vrml_cube();
    switch(vr_object){
        case VR_CUBE:
            draw_vrml_cube();
            break;
        case VR_DODECAHEDRON:
            draw_vrml_dodecahedron();
            break;
        case VR_ICOSAHEDRON:
            draw_vrml_icosahedron();
            break;
        case VR_PYRAMID:
            draw_vrml_pyramid();
            break;
        default:
            break;
    }
}

extern int seed;

void sea_urchin(){
    srand(seed);
    if(disp_style == DS_SOLID)
        glutSolidSphere(.5, 8, 8);
    else
        glutWireSphere(.5, 8, 8);
    // rotate around y axis, then z 
    int i;
    for(i = 0; i < 128; i++){
        glPushMatrix();
        glRotatef(rand() % 360, 1, 0, 0);
        glRotatef(rand() % 360, 0, 1, 0);
        glRotatef(rand() % 360, 0, 0, 1);
        if(disp_style == DS_SOLID)
            glutSolidCone(.025, (rand() % 10) / 100.0 + 1, 4, 4);
        else
            glutWireCone(.025, (rand() % 10) / 100.0 + 1, 4, 4);
        glPopMatrix();
    }
}
/* Draws a freeform scene */
void draw_free_scene(void) {
    /* ADD YOUR CODE HERE */
    /* NOTE: Modify or remove the existing code in this func, as necessary */

    /*
     * Draw a red torus.
     *
     * glutWireTorus args: (inner radius, outer radius,
     * sides per radial section, # of radial sections)
     */
    //glColor3f(1.0f, 0.0f, 0.0f);
    //glutWireTorus(0.1, 0.4, 10, 20);
    /*
     * Draw a green cube at an offset of (0, 1, 0) from the center of
     * the torus.  Note that the glPushMatrix remembers the current
     * drawing position (the center of the torus), the glTranslatef
     * translates the drawing point by and offset, and the
     * glPopMatrix restores the drawing point to the center of
     * the torus.
     */
    //glPushMatrix();
    //glTranslatef(1.0f, 0.0f, 1.0f);		/* the drawing offset */
    //glColor3f(0.0f, 1.0f, 0.0f);		/* green */
    //glutWireCube(1.0f);
    //glPopMatrix();
    //sea_urchin(0.0, 0.0, 0.0, 1); 
}



/* Prints to stdout the current display mode */
void print_disp_mode( void ) {
    switch (disp_mode) {
        case DM_CUBE_GLUT:
            printf("Display Mode: Cube using glut\n");
            break;
        case DM_CUBE_QUAD:
            printf("Display Mode: Cube using quadrilaterals\n");
            break;
        case DM_CUBE_QUAD_ARRAYS:
            printf("Display Mode: Cube using quadrilateral arrays\n");
            break;
        case DM_CONE_GLUT:
            printf("Display Mode: Cone using glut\n");
            break;
        case DM_CONE_TRI:
            printf("Display Mode: Cone using triangles\n");
            break;
        case DM_CONE_TRI_ARRAYS:
            printf("Display Mode: Cone using triangle arrays\n");
            break;
        case DM_CONE_TRI_CALC:
            printf("Display Mode: Cone using calculated triangles\n");
            break;
        case DM_VRML:
            printf("Display Mode: VRML objects\n");
            break;
        case DM_FREE_SCENE:
            printf("Attempted Sea Urchin\n");
            break;
        default:
            printf("Warning: unknown display mode\n");
            break;
    }
}


/* Prints to stdout the current display style */
void print_disp_style( void ) {
    if (disp_style == DS_SOLID) {
        printf("Display Style: solid (for glut modes only)\n");
    } else if (disp_style == DS_WIRE) {  
        printf("Display Style: wire (for glut modes only)\n");
    } else {
        printf("Warning: unknown display style\n");
    }
}

/* end of drawing.c */

