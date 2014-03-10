#ifndef _DRAWPLANT_H_
#define _DRAWPLANT_H_

/* Functions implemented in drawplant.cpp */

#define PI 3.14159265358
void drawPlant(void);
extern int depth;
void push();
void pop();
void rotate(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
void translate(GLdouble, GLdouble, GLdouble);
void scale(GLdouble, GLdouble, GLdouble);
#endif	/* _DRAWPLANT_H_ */
