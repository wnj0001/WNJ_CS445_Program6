
/***********************************************************


   This header file contains initialization function calls and set-ups
for basic 3D CS 445/545 Open GL (Mesa) programs that use the GLUT.
The initializations involve defining a callback handler (my_reshape_function)
that sets viewing parameters for orthographic 3D display.

   TSN 02/2010, updated 01/2012

 ************************************************************/

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
/* reshape callback handler - defines viewing parameters */

void my_3d_projection(int width, int height)
{
    GLfloat left_side   = (GLfloat)((width / 2) * -1);
    GLfloat right_side  = (GLfloat)(width / 2);
    GLfloat bottom_side = (GLfloat)((height / 2) * -1);
    GLfloat top_side    = (GLfloat)(height / 2);
    GLfloat front_plane = 100.0;
    GLfloat back_plane  = -1000.0;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(left_side, right_side, bottom_side, top_side, front_plane, back_plane);
    glMatrixMode(GL_MODELVIEW);
}

#define STRT_X_POS 25
#define STRT_Y_POS 25

void my_setup(int width, int height, char *window_name_str)
{
    // To get double buffering, uncomment the following line
    // glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB); 
    // below code line does single buffering - if above line is uncommented,
    // the single buffering line will have to be commented out
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);  
    glutInitWindowSize(width, height);
    glutInitWindowPosition(STRT_X_POS,STRT_Y_POS);

    glutCreateWindow(window_name_str);

    glutReshapeFunc(my_3d_projection);
}

