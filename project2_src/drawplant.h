#ifndef _DRAWPLANT_H_
#define _DRAWPLANT_H_

/* Functions implemented in drawplant.cpp */

#define PI 3.14159265358


struct Vector3f{
    GLdouble v[4];
} typedef Vector3f;

struct Matrix3f{
    GLdouble m[16];
} typedef Matrix3f;

extern int depth;
extern float orientation;
void drawPlant(GLdouble);
void push();
void pop();
void rotate(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
void translate(GLdouble, GLdouble, GLdouble);
void scale(GLdouble, GLdouble, GLdouble);
Matrix3f current_matrix(void);
Matrix3f mmmult(Matrix3f, Matrix3f);
Vector3f mvmult(Matrix3f, Vector3f);
#endif	/* _DRAWPLANT_H_ */
