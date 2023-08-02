/*********************************************************************************
 Operating instructions:
 Run on Mac OSX 10.6.8 using OpenGL libraries from XCode 3.2.6 64-bit
 Compiled in terminal with: gcc -framework GLUT -framework OpenGL rollercoaster.c
 Coordinates read from "rollercoaster.txt"
 Press 'c' to change camera view
 Press 'q' to quit the program

 Features include:
 Track drawn with b-splines
 Side rails for track
 Rail ties between track and rails
 Poles draw for track support
 Tilting of track around corners
 Tilting of camera around corners
 Acceleration and velocity
 Coordinates read in through a file
 Global lighting
 Ground plane
 Track drawn with GL_COMPILE
 Sky made with cylinder and plane on top
 *********************************************************************************/

/*********************************************************************************
 rollercoaster.txt:
 
 14
 0.404 0.1 -0.294
 1.0 0.1 0.0
 0.404 0.1 0.294
 0.309 0.1 0.951
 -0.155 0.1 0.476
 -0.809 0.1 0.588
 -0.5 1.0 0.0
 -0.809 0.1 -0.588
 -0.155 0.1 -0.476
 0.309 0.5 -0.951
 0.404 0.1 -0.294
 1.0 0.1 0.0
 0.404 0.1 0.294
 0.309 0.1 0.951
 
 **********************************************************************************/
 
#if defined (__APPLE__) || defined (MACOSX)
	#include <GLUT/glut.h>
#else
	#include <GL/glut.h>
#endif
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

static void	myDisplay(void);
static void	myTimer(int value);
static void	myKey(unsigned char key, int x, int y);
static void	myReshape(int w, int h);
static void	init(void);
void nCalc(float *q, float *n);
void uCalc(float *u, float *up, float *n, float *qp, float *qpp);
void vCalc(float *v, float *n, float *u);
void qCalc(float u, float **points, float *q);
void qpCalc(float u, float **points, float *qp);
void qppCalc(float u, float **points, float *qpp);
void calcDrawTrack(float step, float *q, float *qp, float *qpp, float *n, float *u, float *up, float *v);
void rotateVector(float *a, float *n, float theta);
float getHeightDiff(float **points, int numPoints, float inc);
GLuint createTrack(float **points, int numPoints, float inc, float poleInc, float tieInc);

static int xMax = 640;
static int yMax = 640;
static int view = 0;
static int numPoints;
static float dt = 0.01667; //60 fps
static float theta = 0.0;
static float n[3];
static float q[3];
static float qp[3];
static float qpp[3];
static float up[3] = {0.0,1.0,0.0};
static float u[3];
static float v[3];
static float **coords;
static float mass = 1;
static float workTotal;
static float workConstant = 5;
static float cameraLocation;
GLuint track = 0;

int main(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowSize(xMax, yMax);
	glutCreateWindow("Roller Coaster");
	glutDisplayFunc(myDisplay);
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(myKey);
	glutReshapeFunc(myReshape);
	glutTimerFunc(16, myTimer, 16);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	myReshape(xMax,yMax);
	int i;
	FILE *fp;
	fp = fopen("rollercoaster.txt", "r");
	fscanf(fp,"%d", &numPoints);
	float f1,f2,f3;	
	coords = (float**)malloc(sizeof(float*)*numPoints);
	for(i=0;i<numPoints;i++){
		coords[i] = (float*)malloc(sizeof(float)*3);
	}
	for (i=0; i<numPoints; i++){
		fscanf(fp,"%f %f %f", &f1,&f2,&f3);
		coords[i][0] = f1;
		coords[i][1] = f2;
        coords[i][2] = f3;
	}
	fclose(fp);
	
	// SCALE TRACK
	for (i=0; i<numPoints;i++){
		coords[i][0] *= 60;
		coords[i][1] *= 60;
		coords[i][2] *= 60;
	}
	init();
	glutMainLoop();
	return 0;
}

GLuint createTrack(float **points, int numPoints, float inc, float poleInc, float tieInc){
	float i = 0.0;
	float a = 0.1; //scalar for track
	GLuint glid =  glGenLists(1);
	glNewList(glid, GL_COMPILE);

	//DRAW POLES
	glColor3f(1.0f,1.0f,0.0f);
    for (i=3; i<=numPoints; i=i+poleInc){
		glPushMatrix();
		  calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		  glTranslatef(q[0], 0.0, q[2]);
		  glRotatef(-90.0, 1.0, 0.0, 0.0);
		  GLUquadricObj *quadObj = gluNewQuadric();
		  gluCylinder(quadObj, 0.1, 0.1, q[1], 100, 1);
		  gluDeleteQuadric(quadObj);
		glPopMatrix();
	}

	// DRAW MAIN RAIL LEFT
    glPushMatrix();
    glBegin(GL_QUAD_STRIP);
    glColor3f(1.0f,0.0f,0.0f);
    for (i=3; i<=numPoints; i=i+inc){
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		glNormal3f(-u[0],-u[1],-u[2]);
		glVertex3f(q[0]-a*v[0]-a*u[0], q[1]-a*v[1]-a*u[1], q[2]-a*v[2]-a*u[2]);
		glVertex3f(q[0]+a*v[0]-a*u[0], q[1]+a*v[1]-a*u[1], q[2]+a*v[2]-a*u[2]);
	}
	glEnd();
	glPopMatrix();

	// DRAW MAIN RAIL RIGHT
    glPushMatrix();
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.5f,0.0f,0.5f);
    for (i=3; i<=numPoints; i=i+inc){
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		glNormal3f(u[0],u[1],u[2]);
		glVertex3f(q[0]+a*v[0]+a*u[0], q[1]+a*v[1]+a*u[1], q[2]+a*v[2]+a*u[2]);
		glVertex3f(q[0]-a*v[0]+a*u[0], q[1]-a*v[1]+a*u[1], q[2]-a*v[2]+a*u[2]);
	}
	glEnd();
	glPopMatrix();

	//DRAW MAIN RAIL TOP
    glPushMatrix();
    glBegin(GL_QUAD_STRIP);
    glColor3f(1.0f,0.0f,1.0f);
    for (i=3; i<=numPoints; i=i+inc){
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		glNormal3f(v[0],v[1],v[2]);
		glVertex3f(q[0]+a*v[0]-a*u[0], q[1]+a*v[1]-a*u[1], q[2]+a*v[2]-a*u[2]);
		glVertex3f(q[0]+a*v[0]+a*u[0], q[1]+a*v[1]+a*u[1], q[2]+a*v[2]+a*u[2]);
	}
	glEnd();
	glPopMatrix();

	//DRAW MAIN RAIL BOTTOM
    glPushMatrix();
    glBegin(GL_QUAD_STRIP);
    glColor3f(0.0f,1.0f,1.0f);
    for (i=3; i<=numPoints; i=i+inc){
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		glNormal3f(-v[0],-v[1],-v[2]);
		glVertex3f(q[0]-a*v[0]+a*u[0], q[1]-a*v[1]+a*u[1], q[2]-a*v[2]+a*u[2]);
		glVertex3f(q[0]-a*v[0]-a*u[0], q[1]-a*v[1]-a*u[1], q[2]-a*v[2]-a*u[2]);
	}
	glEnd();
	glPopMatrix();

	// DRAW RIGHT RAIL LEFT
	a = 0.1;
    glBegin(GL_QUAD_STRIP);
	glColor3f(1.0f,0.0f,0.0f);
    for (i=3; i<=numPoints; i=i+inc){
    	float x = 0.5;
    	float y = 0.5;
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		q[0] += x*u[0] + y*v[0];
		q[1] += x*u[1] + y*v[1];
		q[2] += x*u[2] + y*v[2];
		glNormal3f(-u[0],-u[1],-u[2]);
		glVertex3f(q[0]-a*v[0]-a*u[0], q[1]-a*v[1]-a*u[1], q[2]-a*v[2]-a*u[2]);
		glVertex3f(q[0]+a*v[0]-a*u[0], q[1]+a*v[1]-a*u[1], q[2]+a*v[2]-a*u[2]);
	}
	glEnd();
	
	// DRAW RIGHT RAIL RIGHT
    glBegin(GL_QUAD_STRIP);
	glColor3f(1.0f,0.0f,0.0f);
    for (i=3; i<=numPoints; i=i+inc){
    	float x = 0.5;
    	float y = 0.5;
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		q[0] += x*u[0] + y*v[0];
		q[1] += x*u[1] + y*v[1];
		q[2] += x*u[2] + y*v[2];
		glNormal3f(u[0],u[1],u[2]);
		glVertex3f(q[0]+a*v[0]+a*u[0], q[1]+a*v[1]+a*u[1], q[2]+a*v[2]+a*u[2]);
		glVertex3f(q[0]-a*v[0]+a*u[0], q[1]-a*v[1]+a*u[1], q[2]-a*v[2]+a*u[2]);
	}
	glEnd();
	
	// DRAW RIGHT RAIL TOP
    glBegin(GL_QUAD_STRIP);
	glColor3f(1.0f,0.0f,0.0f);
    for (i=3; i<=numPoints; i=i+inc){
    	float x = 0.5;
    	float y = 0.5;
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		q[0] += x*u[0] + y*v[0];
		q[1] += x*u[1] + y*v[1];
		q[2] += x*u[2] + y*v[2];
		glNormal3f(v[0],v[1],v[2]);
		glVertex3f(q[0]+a*v[0]-a*u[0], q[1]+a*v[1]-a*u[1], q[2]+a*v[2]-a*u[2]);
		glVertex3f(q[0]+a*v[0]+a*u[0], q[1]+a*v[1]+a*u[1], q[2]+a*v[2]+a*u[2]);
	}
	glEnd();
	
	// DRAW RIGHT RAIL BOTTOM
    glBegin(GL_QUAD_STRIP);
	glColor3f(1.0f,0.0f,0.0f);
    for (i=3; i<=numPoints; i=i+inc){
    	float x = 0.5;
    	float y = 0.5;
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		q[0] += x*u[0] + y*v[0];
		q[1] += x*u[1] + y*v[1];
		q[2] += x*u[2] + y*v[2];
		glNormal3f(-v[0],-v[1],-v[2]);
		glVertex3f(q[0]-a*v[0]+a*u[0], q[1]-a*v[1]+a*u[1], q[2]-a*v[2]+a*u[2]);
		glVertex3f(q[0]-a*v[0]-a*u[0], q[1]-a*v[1]-a*u[1], q[2]-a*v[2]-a*u[2]);
	}
	glEnd();
	
	// DRAW LEFT RAIL LEFT
	glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_QUAD_STRIP);
    for (i=3; i<=numPoints; i=i+inc){
    	float x = -0.5;
    	float y = 0.5;
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		q[0] += x*u[0] + y*v[0];
		q[1] += x*u[1] + y*v[1];
		q[2] += x*u[2] + y*v[2];
		glNormal3f(-u[0],-u[1],-u[2]);
		glVertex3f(q[0]-a*v[0]-a*u[0], q[1]-a*v[1]-a*u[1], q[2]-a*v[2]-a*u[2]);
		glVertex3f(q[0]+a*v[0]-a*u[0], q[1]+a*v[1]-a*u[1], q[2]+a*v[2]-a*u[2]);
	}
	glEnd();
	
	// DRAW LEFT RAIL RIGHT
	glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_QUAD_STRIP);
    for (i=3; i<=numPoints; i=i+inc){
    	float x = -0.5;
    	float y = 0.5;
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		q[0] += x*u[0] + y*v[0];
		q[1] += x*u[1] + y*v[1];
		q[2] += x*u[2] + y*v[2];
		glNormal3f(u[0],u[1],u[2]);
		glVertex3f(q[0]+a*v[0]+a*u[0], q[1]+a*v[1]+a*u[1], q[2]+a*v[2]+a*u[2]);
		glVertex3f(q[0]-a*v[0]+a*u[0], q[1]-a*v[1]+a*u[1], q[2]-a*v[2]+a*u[2]);
	}
	glEnd();
	
	// DRAW LEFT RAIL TOP
	glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_QUAD_STRIP);
    for (i=3; i<=numPoints; i=i+inc){
    	float x = -0.5;
    	float y = 0.5;
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		q[0] += x*u[0] + y*v[0];
		q[1] += x*u[1] + y*v[1];
		q[2] += x*u[2] + y*v[2];
		glNormal3f(v[0],v[1],v[2]);
		glVertex3f(q[0]+a*v[0]-a*u[0], q[1]+a*v[1]-a*u[1], q[2]+a*v[2]-a*u[2]);
		glVertex3f(q[0]+a*v[0]+a*u[0], q[1]+a*v[1]+a*u[1], q[2]+a*v[2]+a*u[2]);
	}
	glEnd();
	
	// DRAW LEFT RAIL BOTTOM
	glColor3f(1.0f,0.0f,0.0f);
    glBegin(GL_QUAD_STRIP);
    for (i=3; i<=numPoints; i=i+inc){
    	float x = -0.5;
    	float y = 0.5;
		calcDrawTrack(i,q,qp,qpp,n,u,up,v);
		q[0] += x*u[0] + y*v[0];
		q[1] += x*u[1] + y*v[1];
		q[2] += x*u[2] + y*v[2];
		glNormal3f(-v[0],-v[1],-v[2]);
		glVertex3f(q[0]-a*v[0]+a*u[0], q[1]-a*v[1]+a*u[1], q[2]-a*v[2]+a*u[2]);
		glVertex3f(q[0]-a*v[0]-a*u[0], q[1]-a*v[1]-a*u[1], q[2]-a*v[2]-a*u[2]);
	}
	glEnd();

	// DRAW TIES BETWEEN MAIN RAIL AND SIDE RAILS
	int j;
	float q2[3], p1[3], norm[3], len;
	for (j=0; j<2; j++){
		// for left/right
		float x = 0.5, y = 0.5;
		if (j==1){
			x = -x;
		}
    	for (i=3; i<=numPoints; i+=tieInc){
			//q will be the point on one end
			//q2 will be the point on left/right ending point
			//compute the point on the coaster and the vectors
			calcDrawTrack(i,q,qp,qpp,n,u,up,v);
			// compute the point left/right
			q2[0] = q[0] + x*u[0] + y*v[0];
			q2[1] = q[1] + x*u[1] + y*v[1];
			q2[2] = q[2] + x*u[2] + y*v[2];
			// calculate normal vector for the sides
			p1[0] = q [0] - q2[0];
			p1[1] = q [1] - q2[1];
			p1[2] = q [2] - q2[2];
			//cross product of vector from q to q2 with forward vector
			norm[0] = p1[1]*n[2] - p1[2]*n[1];
			norm[1] = p1[2]*n[0] - p1[0]*n[2];
			norm[2] = p1[0]*n[1] - p1[1]*n[0];
			len = sqrt(norm[0]*norm[0]+norm[1]*norm[1]+norm[2]*norm[2]);
			norm[0] = norm[0]/len;
			norm[1] = norm[1]/len;
			norm[2] = norm[2]/len;
			if (j != 1){ //for the left
				norm[0] = -norm[0];
				norm[1] = -norm[1];
				norm[2] = -norm[2];
			}
    		glBegin(GL_QUAD_STRIP);
			glColor3f(0.7f,0.3f,0.0f);
			
			// front bottom 
			glNormal3f(n[0],n[1],n[2]);
			glVertex3f(q [0]-a*n[0]+a*u[0], q [1]-a*n[1]+a*u[1], q [2]-a*n[2]+a*u[2]);
			glVertex3f(q2[0]-a*n[0]+a*u[0], q2[1]-a*n[1]+a*u[1], q2[2]-a*n[2]+a*u[2]);
			
			//front top
			glVertex3f(q [0]-a*n[0]-a*u[0], q [1]-a*n[1]-a*u[1], q [2]-a*n[2]-a*u[2]);
			glVertex3f(q2[0]-a*n[0]-a*u[0], q2[1]-a*n[1]-a*u[1], q2[2]-a*n[2]-a*u[2]);

			//back top
			glNormal3f(norm[0], norm[1], norm[2]);
			glVertex3f(q [0]+a*n[0]-a*u[0], q [1]+a*n[1]-a*u[1], q [2]+a*n[2]-a*u[2]);
			glVertex3f(q2[0]+a*n[0]-a*u[0], q2[1]+a*n[1]-a*u[1], q2[2]+a*n[2]-a*u[2]);

			//back bottom
			glNormal3f(-n[0], -n[1], -n[2]);
			glVertex3f(q [0]+a*n[0]+a*u[0], q [1]+a*n[1]+a*u[1], q [2]+a*n[2]+a*u[2]);
			glVertex3f(q2[0]+a*n[0]+a*u[0], q2[1]+a*n[1]+a*u[1], q2[2]+a*n[2]+a*u[2]);

			//front bottom
			glNormal3f(-norm[0], -norm[1], -norm[2]);
			glVertex3f(q [0]-a*n[0]+a*u[0], q [1]-a*n[1]+a*u[1], q [2]-a*n[2]+a*u[2]);
			glVertex3f(q2[0]-a*n[0]+a*u[0], q2[1]-a*n[1]+a*u[1], q2[2]-a*n[2]+a*u[2]);
			glEnd();
		}
	}
	glEndList();
	return glid;
}

void myDisplay(){
	float r = 100.0; //distance for rotation of camera
	float h = 0.8; //height offset 
	float camera = 40.0; //height of rotating camera
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	
	// ROTATE CAMERA
	if (view%2 == 0){
		gluLookAt(r*cos(theta), camera, -r*sin(theta), 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	}
	
	//RIDE THE COASTER
	else if (view%2 == 1){
		calcDrawTrack(cameraLocation,q,qp,qpp,n,u,up,v);
		gluLookAt(q[0]+h*v[0], q[1]+h*v[1], q[2]+h*v[2], q[0]+qp[0], q[1]+qp[1], q[2]+qp[2], v[0], v[1], v[2]);
	}
	
	// DRAW PLANE
	glDisable(GL_LIGHTING);
	glColor3f(0.0f,1.0f,0.0f);
	glBegin(GL_QUADS);
		glNormal3f(0.0,1.0,0.0);
		glVertex3f(-200.0f, 0.0f, -200.0f);
		glVertex3f(-200.0f, 0.0f,  200.0f);
        glVertex3f( 200.0f, 0.0f,  200.0f);
        glVertex3f( 200.0f, 0.0f, -200.0f);
    glEnd();
	
	// DRAW SKY TOP
	glBegin(GL_QUADS);
		glNormal3f(0.0,-1.0,0.0);
		glColor3f(0.0f,0.0f,1.0f);
        glVertex3f( 220.0f, 170.0f, -220.0f);
        glVertex3f( 220.0f, 170.0f,  220.0f);
		glVertex3f(-220.0f, 170.0f,  220.0f);
		glVertex3f(-220.0f, 170.0f, -220.0f);
    glEnd();
    
	// DRAW CYLINDER SKY
	float k, dk, R;
	dk = 2*M_PI/60.0;
	R = 200.0f;
	glBegin(GL_QUAD_STRIP);
	for (k=0.0; k<=2*M_PI; k+=dk) {
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(cos(k)*R, 170.f, R*sin(k));
			glColor3f(0.0f, 0.6f, 1.0f);
			glVertex3f(cos(k)*R,    0.0, R*sin(k));
	}
	glEnd();
	glEnable(GL_LIGHTING);
	
	// DRAW TRACK 
	glCallList(track);
	glutSwapBuffers();
}

void myTimer(int value){
	float du, v, q[3], qp[3], len;

	// INCREMENT CAMERA POSITION FOR ROTATING CAMERA
	if (view%2 == 0){
		theta += dt*0.10;
	}
	
	// RIDING THE COASTER
	else if (view%2 == 1){
		// calculate the current position of the camera
		qCalc(cameraLocation, coords, q);
		qpCalc(cameraLocation, coords, qp);
		// calculate the camera speed
		v = sqrt(2*(workTotal/mass - 9.81*q[1]));
		len = sqrt(qp[0]*qp[0] + qp[1]*qp[1] + qp[2]*qp[2]);
		// calculate displacement in terms of u
		du = (v*dt) / len;
		// change camera position
		cameraLocation += du;
		if (cameraLocation > numPoints-1){
			cameraLocation = cameraLocation - (numPoints-1) + 3.0;
		}
	}
	glutPostRedisplay();
	glutTimerFunc(16, myTimer, value);
}

void myKey(unsigned char key, int x, int y){
	if (key == 'c'){
		view++;
	}
	else if (key == 'q'){
		free(coords);
		exit(0);
	}
}

void myReshape(int w, int h){
	xMax = w;
	yMax = h;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    gluPerspective (60, xMax/yMax, 0.1, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void init(){
	GLfloat Ka[] = {0.1, 0.1, 0.1, 1.0};
	GLfloat Kd[] = {0.6, 0.7, 0.7, 1.0};
	GLfloat Ks[] = {0.2, 0.2, 0.2, 1.0};
	GLfloat la[] = {0.4, 0.4, 0.4, 1.0};
	GLfloat ld[] = {0.5, 0.6, 0.4, 1.0};
	GLfloat ls[] = {0.5, 0.5, 0.5, 1.0};
	GLfloat pos[] = {1.0, 1.0, 0.0, 1.0};
	cameraLocation = 3.0f;
	workTotal = mass*9.81*(getHeightDiff(coords, numPoints, 0.01f));
	workTotal += workConstant;
	track = createTrack(coords, numPoints, 0.01, 0.5, 0.2);
	
	glEnable(GL_NORMALIZE);
	glEnable(GL_CULL_FACE);
	
	// LIGHTING
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_AMBIENT, Ka);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, Kd);
	glMaterialf(GL_FRONT, GL_SHININESS, 89);
	glMaterialfv(GL_FRONT, GL_SPECULAR, Ks);
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, la);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ld);
	glLightfv(GL_LIGHT0, GL_SPECULAR, ls);
}

float getHeightDiff(float **points, int numPoints, float inc){
	float i, q[3];
	float highest = 0;
	for (i=3; i<=numPoints; i=i+inc){
		qCalc(i, points, q);
		if(i == 3.0 || q[1] > highest)
			highest = q[1];
	}
	return highest;
}

void calcDrawTrack(float step, float *q, float *qp, float *qpp, float *n, float *u, float *up, float *v){
	up[0] = 0;
	up[1] = 1;
	up[2] = 0;
	qCalc(step, coords, q);
	qpCalc(step, coords, qp);
	qppCalc(step, coords, qpp);
	nCalc(qp, n);
	uCalc(u, up, n, qp, qpp);
	vCalc(v, n, u);
}

void nCalc(float *qp, float *n){
	n[0] = -qp[0];
	n[1] = -qp[1];
	n[2] = -qp[2];
	float len = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
	n[0] = n[0]/len;
	n[1] = n[1]/len;
	n[2] = n[2]/len;
}

void rotateVector(float *up, float *n, float theta){
	float c, s;
	float tmp[3];
	c = cos(theta);
	s = sin(theta);
	tmp[0] =(up[0])*((1 - c) * (n[0]) * (n[0]) + c)
          + (up[1])*((1 - c) * (n[0]) * (n[1]) - s * (n[2]))
          + (up[2])*((1 - c) * (n[0]) * (n[2]) + s * (n[1]));
	tmp[1] =(up[0])*((1 - c) * (n[0]) * (n[1]) + s * (n[2]))
          + (up[1])*((1 - c) * (n[1]) * (n[1]) + c)
          + (up[2])*((1 - c) * (n[1]) * (n[2]) - s * (n[0]));
	tmp[2] =(up[0])*((1 - c) * (n[0]) * (n[2]) - s * (n[1]))
          + (up[1])*((1 - c) * (n[1]) * (n[2]) + s * (n[0]))
          + (up[2])*((1 - c) * (n[2]) * (n[2]) + c);
	up[0] = tmp[0];
	up[1] = tmp[1];
	up[2] = tmp[2];
}

void uCalc(float *u, float *up, float *n, float *qp, float *qpp){
	qpp[1] = 0;
	float constant = 0.6;
	float len = sqrt(qp[0]*qp[0]+qp[1]*qp[1]+qp[2]*qp[2]);
	float k=((qp[2]*qpp[0])-(qp[0]*qpp[2]))/((len*len*len)*constant);
	//rotate up around n by angle k
	rotateVector(up,n,k);
	u[0] = up[1]*n[2] - up[2]*n[1];
	u[1] = up[2]*n[0] - up[0]*n[2];
	u[2] = up[0]*n[1] - up[1]*n[0];
	len = sqrt(u[0]*u[0]+u[1]*u[1]+u[2]*u[2]);
	u[0] = u[0]/len;
	u[1] = u[1]/len;
	u[2] = u[2]/len;
}

void vCalc(float *v, float *n, float *u){
	v[0] = n[1]*u[2] - n[2]*u[1];
	v[1] = n[2]*u[0] - n[0]*u[2];
	v[2] = n[0]*u[1] - n[1]*u[0];
}

void qCalc(float u, float **points, float *q){
	float blank[] = {0.0, 0.0, 0.0};
	int segment = (int)u;
	float t = u-segment;
	float *p0 = points[segment-3];
	float *p1 = points[segment-2];
	float *p2 = points[segment-1];
	float *p3 = ((numPoints == segment) ? blank : points[segment-0]);
	float r0 = (1.0/6.0)*t*t*t;
	float r1 = (1.0/6.0)*(-3*t*t*t+3*t*t+3*t+1);
	float r2 = (1.0/6.0)*(3*t*t*t-6*t*t+4);
	float r3 = (1.0/6.0)*(1-t)*(1-t)*(1-t);
	q[0] = r3*p0[0] + r2*p1[0] + r1*p2[0] + r0*p3[0];
	q[1] = r3*p0[1] + r2*p1[1] + r1*p2[1] + r0*p3[1];
	q[2] = r3*p0[2] + r2*p1[2] + r1*p2[2] + r0*p3[2];
}

void qpCalc(float u, float **points, float *qp){
	float blank[] = {0.0, 0.0, 0.0};
	int segment = (int)u;
	float t = u-segment;
	float *p0 = points[segment-3];
	float *p1 = points[segment-2];
	float *p2 = points[segment-1];
	float *p3 = ((numPoints == segment) ? blank : points[segment-0]);
	float r0 = (1.0/2.0)*t*t;
	float r1 = -1.5*t*t+t+0.5;
	float r2 = t*(1.5*t-2.0);
	float r3 = -0.5*(t-1.0)*(t-1.0);
	qp[0] = r3*p0[0] + r2*p1[0] + r1*p2[0] + r0*p3[0];
	qp[1] = r3*p0[1] + r2*p1[1] + r1*p2[1] + r0*p3[1];
	qp[2] = r3*p0[2] + r2*p1[2] + r1*p2[2] + r0*p3[2];
}

void qppCalc(float u, float **points, float *qpp){
	float blank[] = {0.0, 0.0, 0.0};
	int segment = (int)u;
	float t = u-segment;
	float *p0 = points[segment-3];
	float *p1 = points[segment-2];
	float *p2 = points[segment-1];
	float *p3 = ((numPoints == segment) ? blank : points[segment-0]);
	float r0 = t;
	float r1 = 1-3*t;
	float r2 = 3*t-2;
	float r3 = 1-t;
	qpp[0] = r3*p0[0] + r2*p1[0] + r1*p2[0] + r0*p3[0];
	qpp[1] = r3*p0[1] + r2*p1[1] + r1*p2[1] + r0*p3[1];
	qpp[2] = r3*p0[2] + r2*p1[2] + r1*p2[2] + r0*p3[2];
}
