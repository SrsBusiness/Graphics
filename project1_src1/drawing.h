#ifndef _DRAWING_H_
#define _DRAWING_H_

/* All variables / contants declared in this file are global. */

/*
 * Different drawing modes.  The drawing modes should begin at 0.
 * and should be consecutive integers.
 * 
 * DM_MAX should be the total number of drawing modes.
 */
#define DM_CUBE_GLUT            0
#define DM_CUBE_QUAD            1
#define DM_CUBE_QUAD_ARRAYS     2
#define DM_CONE_GLUT            3
#define DM_CONE_TRI             4
#define DM_CONE_TRI_ARRAYS      5
#define DM_CONE_TRI_CALC        6
#define DM_VRML                 7
#define DM_FREE_SCENE           8
#define DM_MAX                  9

/* The current display mode */
extern int disp_mode;
extern double h;
extern double r;
extern int n;



/* Styles of drawing glut objects, either solid or wire-frame */
#define DS_SOLID        0
#define DS_WIRE         1

/* The current display style */
extern int disp_style;


/* Function Declarations */
void draw_cube_glut(void);
void draw_cube_quad(void);
void draw_cube_quad_arrays(void);
void draw_cone_glut(void);
void draw_cone_tri(void);
void draw_cone_tri_arrays(void);
void draw_cone_tri_calc(double height, double radius, int base_tri);
void draw_vrml(void);
void draw_free_scene(void);
void print_disp_mode(void);
void print_disp_style(void);

#endif	/* _DRAWING_H_ */

