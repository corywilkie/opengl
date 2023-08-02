/**********************************************************************
 Name: Cory Wilkie         Class: CSCI3161         Assignment 0
 Run on Mac OSX 10.6.8 using OpenGL libraries from XCode 3.2.6 64-bit
 Compiled in terminal with: gcc -framework GLUT -framework OpenGL a0.c
 **********************************************************************/
#include <GLUT/glut.h>
#include <stdlib.h>
#include <math.h>

static int n = 5;    //default value of starpoints
static char m = 'a'; //default display mode

void myDisplay(){
	glClear(GL_COLOR_BUFFER_BIT);
	int i;
	double increment = 2*M_PI/n; //2*PI for full 360 degrees of circle, divided by n for the measurement between star points
	double big = M_PI_2; //the 'big circle' for the outside points of the star
	double small = M_PI_2+increment/2; //the 'small circle' for the inside points of the star
	
	if (m == 'a'){ //clear solid star
		glBegin(GL_LINE_LOOP);
		for (i=0;i<n;i++){
			glVertex2f(2.5*cos(big), 2.5*sin(big)); //draws outer point
			glVertex2f(cos(small), sin(small));     //draws inner point
			big = big+increment;
			small = small+increment;
		}
	}
	else if (m == 'b'){ //wireframe star
		glBegin(GL_LINE_LOOP);
		for (i=0;i<n;i++){ //generates outside edge
			glVertex2f(2.5*cos(big), 2.5*sin(big));
			glVertex2f(cos(small), sin(small));
			big = big+increment;
			small = small+increment;
		}
		glEnd();
		glBegin(GL_LINES);
		for (i=0;i<n;i++){ //generates inside lines
			glVertex2f(0.0, 0.0);
			glVertex2f(2.5*cos(big), 2.5*sin(big));
			glVertex2f(0.0, 0.0);
			glVertex2f(cos(small), sin(small));
			big = big+increment;
			small = small+increment;
		}
	}
	else if (m == 'c'){ //makes star a solid color
		double big2 = big;
		double small2 = small;
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(0.0, 0.0);
		for (i=0;i<n;i++){
			glVertex2f(2.5*cos(big), 2.5*sin(big));
			glVertex2f(cos(small), sin(small));
			big = big+increment;
			small = small+increment;
		}
		glVertex2f(2.5*cos(big2), 2.5*sin(big2));
	}
	else if (m == 'd'){ //adds color to stars triangles
		double big2 = big;
		double small2 = small;
		glBegin(GL_TRIANGLE_FAN);
		glColor3f(1.0, 0.0, 0.0);
		for (i=0;i<n;i++){
			if (i==0){
				glVertex2f(0.0, 0.0);
				glVertex2f(2.5*cos(big), 2.5*sin(big));
				glVertex2f(cos(small), sin(small));
			}
			else{
				glColor3f(0.0, 1.0, 0.0);
				glVertex2f(2.5*cos(big), 2.5*sin(big));
				glColor3f(1.0, 0.0, 0.0);
				glVertex2f(cos(small), sin(small));
			}
			big = big+increment;
			small = small+increment;
		}
		glColor3f(0.0, 1.0, 0.0);
		glVertex2f(2.5*cos(big2), 2.5*sin(big2));
		glColor3f(0.0, 0.0, 0.0);
	}
	glEnd();
	glFlush();
}

void myKey(unsigned char key, int x, int y){
	if (key >= '3' && key <= '8'){ //changes points on star
		n = (int)key-48;
		glutPostRedisplay();
	}
	else if (key=='a' || key=='b' || key=='c' || key=='d'){ //changes display mode
		m = key;
		glutPostRedisplay();
	}
	else if (key=='q') //quits program
		exit(0);
}

/* most code taken from example program in class */
void init(){
	glLineWidth( 3 );	// Lines are n pixels wide 
	glClearColor(1.0, 1.0, 1.0, 1.0);	/* black */
	glColor3f(0.0, 0.0, 0.0);	/* white */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-3.0, 3.0, -3.0, 3.0);
	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_FLAT); //added for color requirement
}

/* most code taken from example program in class */
int main(int argc, char *argv[]){ 
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("a0");
	glutDisplayFunc(myDisplay);
	glutKeyboardFunc(myKey);
	init(); /* initialize attributes */
	glutMainLoop();
	return 0;
}
