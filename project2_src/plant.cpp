/*
 * plant.cpp
 * ---------
 * Contains function callbacks and bookkeeping used by glut.  Starter code
 * for Project 2.
 *
 * Group Members: <FILL IN>
 */

#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "common.h"
#include "drawplant.h"

/* GLOBAL VARAIBLES */
/* (storage is actually allocated here) */
int W=800;		/* window width */
int H=600;		/* window height */
int X_OFF = 10;	/* window x offset */
int Y_OFF = 10;	/* window y offset */

/* local function declarations */
void display(void);
void init(void);

GLdouble angle = 0;
int projection = 0;
void key_handler(unsigned char c, int x, int y){
    switch(c){
        case 'a':
            depth++;
            break;
        case 's':
            if(depth >= 1)
                depth--;
            else
                printf("Already at lowest depth");
            break;
        case '/':
            // positive angle around y
            angle = PI / 6;
            break;
        case '?':
            angle = -1 * PI / 6;
            // negative angle around y
            break;
        case 'p':
            projection ^= 1;       
            break;
        case 'q':
            exit(0);
            break;
        default:
            break;
    }
    display();
    angle = 0;
}

int mouse_x;
int current_button;
void mouse_handler(int button, int state, int x, int y ){
    if(state == GLUT_DOWN && button == GLUT_LEFT_BUTTON){
        mouse_x = x;
        current_button = GLUT_LEFT_BUTTON;
    }
}
void motion_handler(int x, int y){
    if(current_button == GLUT_LEFT_BUTTON){
        int dx = x - mouse_x;
        mouse_x = x;
        angle = -1 * PI * dx / 1366;
    }
    display();
}
int main (int argc, char** argv) {
    glutInit(&argc,argv);
    glutInitWindowSize(W, H);
    glutInitWindowPosition(X_OFF, Y_OFF);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("plant");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(key_handler);
    glutMotionFunc(motion_handler);
    glutMouseFunc(mouse_handler);
    glutMainLoop();
    return 0;
}

void init() {
    glClearColor(0.0, 0.0, 0.0, 0.0);  
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-40.0, 40.0, -40.0, 40.0, -10.0, 10.0);
}


void display() {
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    /* See drawplant.c for the definition of this routine */
    drawPlant(angle, projection);


    glFlush();  /* Flush all executed OpenGL ops finish */

    /*
     * Since we are using double buffers, we need to call the swap
     * function every time we are done drawing.
     */
    glutSwapBuffers();
}

/* end of plant.c */
