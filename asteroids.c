/******************************************************************************
 Name: Cory Wilkie                Class: CSCI3161                Assignment 1
 Run on Mac OSX 10.6.8 using OpenGL libraries from XCode 3.2.6 64-bit
 Compiled in terminal with: gcc -framework GLUT -framework OpenGL a1.c
 Received help from Devin Horsman for ship movement and drawing objects
 Worked with and shared ideas with Pavlo Skazhenyuk
 ******************************************************************************/

/******************************************************************************
 Standard features include:
 Press the up arrow to accelerate
 Press the down arrow to decelerate
 Press the left arrow to rotate left
 Press the right arrow to rotate right
 Press the spacebar to shoot a photon

 Extra features include:
 Press 'c' for Pokemon mode (turns asteroids to circles)
 Press 'p' to pause the game
 Press 'r' to restart the game
 Press 'q' to quit the game
 A background with stars

 Collisitons are all circle to circle detection between the asteroids 
 and photons, and the asteroids and ship.
 ******************************************************************************/

#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <GLUT/glut.h>

#define RAD2DEG 180.0/M_PI
#define DEG2RAD M_PI/180.0
#define MAX_PHOTONS	    8
#define MAX_ASTEROIDS  32
#define START_ASTEROIDS 8
#define MAX_VERTICES   16
#define MAX_VELOCITY    2
#define MIN_VELOCITY   -2
#define ACCELERATION    2
#define MAX_STARS      70

typedef struct Coords {
	double		x, y;
} Coords;

typedef struct {
	double	x, y, phi, dx, dy;
} Ship;

typedef struct {
	int	active;
	double	x, y, dx, dy;
} Photon;

typedef struct {
	int	active, nVertices;
	double	x, y, phi, dx, dy, dphi, size;
	Coords	coords[MAX_VERTICES];
} Asteroid;

static void	myDisplay(void);
static void	myTimer(int value);
static void	myKey(unsigned char key, int x, int y);
static void	keyPress(int key, int x, int y);
static void	keyRelease(int key, int x, int y);
static void	myReshape(int w, int h);
static void	init(void);
static void	initAsteroid(Asteroid *a, double x, double y, double size);
static void initPhoton(Photon *p);
static void updateAsteroids();
static void updatePhotons();
static void updateShip();
static void photonCollision(Photon *p, Asteroid *a, double size);
static void shipCollision(Ship *s, Asteroid *a);
static void	drawAsteroid(Asteroid *a);
static void drawCircle();
static void	drawPhoton(Photon *p);
static void	drawShip(Ship *s);
static void drawStars();
static double myRandom(double min, double max);

static int	up=0, down=0, left=0, right=0;
static int	xMax, yMax;
static int activePhotons = 0;
static int pause = 0;
static int circles = 0;
static int shipDestroyed = 0;
static int starx[MAX_STARS];
static int stary[MAX_STARS];
static float dt = 0.01667; //60 fps
static float big = 6.0;
static float small = 3.0;
static float circleRadius = 12.0;
static Asteroid	asteroids[MAX_ASTEROIDS];
static Photon	photons[MAX_PHOTONS];
static Ship	ship;

int main(int argc, char *argv[]){
	srand((unsigned int) time(NULL));
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Asteroids");
	glutDisplayFunc(myDisplay);
	glutIgnoreKeyRepeat(1);
	glutKeyboardFunc(myKey);
	glutSpecialFunc(keyPress);
	glutSpecialUpFunc(keyRelease);
	glutReshapeFunc(myReshape);
	glutTimerFunc(16, myTimer, 0);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	myReshape(800,600);
	init();
	glutMainLoop();
	return 0;
}

void myDisplay(){
	int	i;
	glClear(GL_COLOR_BUFFER_BIT);
	glLoadIdentity();
	
	/*draws stars*/
	for (i=0; i<MAX_STARS; i++){
		glPushMatrix();
			glTranslatef(starx[i],stary[i],0);
			glRotatef(1,0,0,1);
			drawStars();
		glPopMatrix();
	}
	
	/*draws ship*/
	if (!shipDestroyed){
		glPushMatrix();
			glTranslatef(ship.x, ship.y, 0);
			glRotatef(ship.phi,0,0,1);
			drawShip(&ship);
		glPopMatrix();
		
		/*draws photons*/
		for (i=0; i<MAX_PHOTONS; i++){
			if (photons[i].active){
				glPushMatrix();
					glTranslatef(photons[i].x, photons[i].y, 0);
					glScalef(2.0,2.0, 0);
					drawPhoton(&photons[i]);
				glPopMatrix();
			}
		}
	}
	
	/*draws asteroids*/
	for (i=0; i<MAX_ASTEROIDS; i++){
		if (asteroids[i].active){
			glPushMatrix();
				glTranslatef(asteroids[i].x, asteroids[i].y, 0);
				glRotatef(asteroids[i].phi,0,0,1);
				
				/*draws big circles if 'c' is true*/
				if (circles%2==1 && asteroids[i].size==big){
					glScalef(asteroids[i].size*2.5, asteroids[i].size*2.5, 1.0);
				}
				
				/*draws small circles if 'c' is true*/
				if (circles%2==1 && asteroids[i].size==small){
					glScalef(asteroids[i].size*3.0, asteroids[i].size*3.0, 1.0);
				}
				drawAsteroid(&asteroids[i]);
			glPopMatrix();
		}
	}
	glutSwapBuffers();
}

void myTimer(int value){
	int i, j;
	if (pause%2==1){
		/*do nothing*/
	}
	else {
		/*update functions*/
		updateShip(dt);
		updatePhotons();
		updateAsteroids(dt);

		/*checks collisions between all active asteroids and active photons*/
		for (i=0; i<MAX_PHOTONS; i++){
			for (j=0; j<MAX_ASTEROIDS; j++){
				if (photons[i].active && asteroids[j].active){
					photonCollision(&photons[i], &asteroids[j], big);
				}
			}
		}
	
		/*checks collisions between all asteroids and ship*/
		for (i=0; i<MAX_ASTEROIDS; i++){
			shipCollision(&ship, &asteroids[i]);
		}
	}
	glutPostRedisplay();
	glutTimerFunc(16, myTimer, value);
}

void myKey(unsigned char key, int x, int y){
	int i;

	/*shoots a photon if spacebar is pressed*/
	if (key == ' '){
		if (!shipDestroyed){
			for(i=0; i<MAX_PHOTONS; i++){
				if (photons[i].active == 0){
					activePhotons++;
					initPhoton(&photons[i]);
					return;
				}
			}
		}
	}
	
	/*switches between asteroids and circles*/
	else if (key == 'c'){
		circles++;
	}
	
	/*restarts the game*/
	else if (key == 'r'){
		init();	
	}
	
	/*pauses the game*/
	else if (key == 'p'){
		pause++;
	}
	
	/*quits the game*/
	else if (key=='q'){
		exit(0);
	}
}

void keyPress(int key, int x, int y){
	switch (key){
		case 100:
			left = 1;
			break;
		case 101:
			up = 1;
			break;
		case 102:
			right = 1;
			break;
		case 103:
			down = 1;
			break;
	}
}

void keyRelease(int key, int x, int y){
	switch (key){
		case 100:
			left = 0;
			break;
		case 101:
			up = 0;
			break;
		case 102:
			right = 0;
			break;
		case 103:
			down = 0;
			break;
	}
}

void myReshape(int w, int h){
	xMax = 100.0*w/h;
	yMax = 100.0;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, xMax, 0.0, yMax, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void init(){
	int i, x, y;
	
	/*set ship variables*/
	ship.x = xMax/2.0;
	ship.y = yMax/2.0;
	ship.phi = 0;
	ship.dx = 0;
	ship.dy = 0;
	shipDestroyed = 0;
	
	/*generate coordinates for stars*/
	for (i=0; i<MAX_STARS; i++){
		x = rand()%xMax;
		y = rand()%yMax;
		starx[i] = x;
		stary[i] = y;
	}
	
	/*set all asteroids to inactive*/
	for (i=0; i<MAX_ASTEROIDS; i++){
		asteroids[i].active=0;
	}
	
	/*set all photons to inactive*/
	for (i=0; i<MAX_PHOTONS; i++){
		photons[i].active=0;
	}
	
	/*initialize the starting asteroids*/
	for (i=0; i<START_ASTEROIDS;){
		x = rand()%xMax;
		y = rand()%yMax;
		if (x < (xMax-50)/2 || x > (xMax+50)/2 && y < (yMax-50)/2 || y > (yMax+50)){
			initAsteroid(&asteroids[i], x, y, big);
			i++;
		}
	}
}

void initAsteroid(Asteroid *a, double x, double y, double size){
	double theta, r;
	int i;
	a->x = x;
	a->y = y;
	a->phi = 0.0;
	a->size = size;
	a->dx = myRandom(-0.8, 0.8)*0.4;
	a->dy = myRandom(-0.8, 0.8)*0.4;
	a->dphi = myRandom(-0.2, 0.2)*10;
	a->nVertices = 6+rand()%(MAX_VERTICES-6);
	for (i=0; i<a->nVertices; i++){
		theta = 2.0*M_PI*i/a->nVertices;
		r = size*myRandom(2.0, 3.0);
		a->coords[i].x = -r*sin(theta);
		a->coords[i].y = r*cos(theta);
	}
	a->active = 1;
}

void initPhoton(Photon *p){
	p->active = 1;
	p->x = ship.x;
	p->y = ship.y;
	p->dx = -sin(ship.phi*DEG2RAD)*MAX_VELOCITY*2;
	p->dy = cos(ship.phi*DEG2RAD)*MAX_VELOCITY*2;
}

void updateAsteroids(){
	int i;
	for(i=0; i<MAX_ASTEROIDS; i++){
	
		/*update active asteroids*/
		if (asteroids[i].active == 1){
			asteroids[i].x += asteroids[i].dx;
			asteroids[i].y += asteroids[i].dy;
			asteroids[i].phi += asteroids[i].dphi;
			asteroids[i].dx = asteroids[i].dx;
			asteroids[i].dy = asteroids[i].dy;
			asteroids[i].dphi = asteroids[i].dphi;
			
			/*edge detection for large asteroids*/
			if (asteroids[i].size==big){
				if (asteroids[i].x>xMax+10){
					asteroids[i].x = 10;
				}
				if (asteroids[i].x<-10){
					asteroids[i].x = xMax+10;
				}
				if (asteroids[i].y>yMax+10){
					asteroids[i].y = -10;
				}
				if (asteroids[i].y<-10){
					asteroids[i].y = yMax+10;
				}
			}
			
			/*edge detection for small asteroids*/
			if (asteroids[i].size==small){
				if (asteroids[i].x>xMax+7){
					asteroids[i].x = 7;
				}
				if (asteroids[i].x<-7){
					asteroids[i].x = xMax+7;
				}
				if (asteroids[i].y>yMax+7){
					asteroids[i].y = -7;
				}
				if (asteroids[i].y<-7){
					asteroids[i].y = yMax+7;
				}
			}
		}
	}
}

void updatePhotons(){
	int i;
	for(i=0; i<MAX_PHOTONS; i++){
	
		/*updates active photons*/
		if (photons[i].active == 1){
			photons[i].x += photons[i].dx;
			photons[i].y += photons[i].dy;
			
			/*deactivates out of bound photons*/
			if (photons[i].x>xMax||photons[i].x<0){
				photons[i].active=0;
				activePhotons--;
			}
			if (photons[i].y>yMax||photons[i].y<0){
				photons[i].active=0;
				activePhotons--;
			}
		}
	}
}

void updateShip(){
	ship.x = ship.x + ship.dx;
	ship.y = ship.y + ship.dy;
	
	/*ship movement*/
	if (left==1&&right==1){
		ship.phi = ship.phi;
	}
	else if (left==1){
		ship.phi = ship.phi+dt*180;
	}
	else if (right==1){
		ship.phi = ship.phi+dt*-180;
	}
	
	/*ship acceleration*/
	if (up==1&&down==1){
		ship.dx = ship.dx;
		ship.dy = ship.dy;
	}
	else if (up==1){
		ship.dx  -= sin(ship.phi*DEG2RAD)*dt*ACCELERATION;
		ship.dy  += cos(ship.phi*DEG2RAD)*dt*ACCELERATION;
	}
	else if (down==1){
		ship.dx  += sin(ship.phi*DEG2RAD)*dt*ACCELERATION;
		ship.dy  -= cos(ship.phi*DEG2RAD)*dt*ACCELERATION;
	}
	if(ship.dx*ship.dx+ship.dy*ship.dy>MAX_VELOCITY*MAX_VELOCITY){
		double temp = sqrt(ship.dx*ship.dx+ship.dy*ship.dy);
		temp = MAX_VELOCITY/temp;
		ship.dx *= temp;
		ship.dy *= temp;
	}
	
	/*edge detection for ship*/
	if (ship.x>xMax+4){
		ship.x = -4;
	}
	if (ship.x<-4){
		ship.x = xMax+4;
	}
	if (ship.y>yMax+4){
		ship.y = -4;
	}
	if (ship.y<-4){
		ship.y = yMax+4;
	}
}

void photonCollision(Photon *p, Asteroid *a, double size){
	int i;

	/*collision of photons and small asteroids*/
	if ((p->x-a->x)*(p->x-a->x)+(p->y-a->y)*(p->y-a->y) <= ((circleRadius-4)*(circleRadius-4)) && a->size==small){
		p->active = 0;
		a->active = 0;
	}
	
	/*collision of photons and large asteroids and creates two small asteroids*/
	if ((p->x-a->x)*(p->x-a->x)+(p->y-a->y)*(p->y-a->y) <= ((circleRadius+3)*(circleRadius+3)) && a->size==big){
		p->active = 0;
		a->active = 0;
		initAsteroid(a, a->x, a->y, small);
		for (i=0; i<MAX_ASTEROIDS; i++){
			if (asteroids[i].active==0){
				initAsteroid(&asteroids[i], a->x, a->y, small);
				break;
			}
		}
	}
}

void shipCollision(Ship *s, Asteroid *a){

	/*collision between big asteroids and ship*/
	if (((s->x-a->x)*(s->x-a->x)+(s->y-a->y)*(s->y-a->y)) <= ((circleRadius+5)*(circleRadius+5)) && a->active==1 && a->size==big){
		shipDestroyed = 1;
	}
	
	/*collision between small asteroids and ship*/
	if (((s->x-a->x)*(s->x-a->x)+(s->y-a->y)*(s->y-a->y)) <= ((circleRadius-3)*(circleRadius-3)) && a->active==1 && a->size==small){
		shipDestroyed = 1;
	}
}

void drawAsteroid(Asteroid *a){
	int i;

	/*draws polygons unless circles are activated*/
	if (circles%2==0){
		glBegin(GL_POLYGON);
			for (i=0;i<a->nVertices;i++){
				glVertex2f(a->coords[i].x, a->coords[i].y);
			}
		glEnd();
	}
	else {
		drawCircle();
	}
}

void drawCircle(){
	int i;

	/*draws boring circles, commented out*/
	/*	
	glBegin(GL_POLYGON);
		for(i=0; i<40; i++){
			glVertex2d(cos(i*M_PI/20.0), sin(i*M_PI/20.0));
		}
	glEnd();
	*/
	
	/*draws Pokeballs*/
	glBegin(GL_POLYGON);
		for(i=0; i<40; i++){
			glVertex2d(cos(i*M_PI/20.0), sin(i*M_PI/20.0));
		}
	glEnd();
	glBegin(GL_POLYGON);
		for(i=0; i<40; i++){
			glVertex2d(cos(i*M_PI/20.0), sin(i*M_PI/20.0));
		}
	glEnd();
	glBegin(GL_POLYGON);
		for(i=0; i<40; i++){
			glVertex2d(cos(i*M_PI/20.0)*.4, sin(i*M_PI/20.0)*.4);
		}
	glEnd();
	glBegin(GL_POLYGON);
		for(i=0; i<40; i++){
			glVertex2d(cos(i*M_PI/20.0)*.2, sin(i*M_PI/20.0)*.2);
		}
	glEnd();
	glBegin(GL_POLYGON);
		for(i=0; i<40; i++){
			glVertex2d(cos(i*M_PI/40.0), sin(i*M_PI/40.0));
		}
	glEnd();
}

void drawPhoton(Photon *p){
	int i;
	
	/*shoots photons*/
	if (circles%2==0){
		glBegin(GL_POLYGON);
			glVertex2f(-0.3, 0.3);
			glVertex2f(-0.3, -0.3);
			glVertex2f(0.3, -0.3);
			glVertex2f(0.3, 0.3);
		glEnd();
	}
	
	/*shoots Pokeballs*/
	else {
		glBegin(GL_POLYGON);
			for(i=0; i<40; i++){
				glVertex2d(cos(i*M_PI/20.0), sin(i*M_PI/20.0));
			}
		glEnd();
		glBegin(GL_POLYGON);
			for(i=0; i<40; i++){
				glVertex2d(cos(i*M_PI/20.0), sin(i*M_PI/20.0));
			}
		glEnd();
		glBegin(GL_POLYGON);
			for(i=0; i<40; i++){
				glVertex2d(cos(i*M_PI/20.0)*.4, sin(i*M_PI/20.0)*.4);
			}
		glEnd();
		glBegin(GL_POLYGON);
			for(i=0; i<40; i++){
				glVertex2d(cos(i*M_PI/20.0)*.2, sin(i*M_PI/20.0)*.2);
			}
		glEnd();
		glBegin(GL_POLYGON);
			for(i=0; i<40; i++){
				glVertex2d(cos(i*M_PI/40.0), sin(i*M_PI/40.0));
			}
		glEnd();
   	 }
}

void drawShip(Ship *s){
	glBegin(GL_POLYGON);
		glVertex2f(0, 4);
		glVertex2f(-2.5, -4);
		glVertex2f(2.5, -4);
	glEnd();
}

void drawStars(){
	glBegin(GL_POLYGON);
		glVertex2f(0,.2);
		glVertex2f(-.2,0);
		glVertex2f(0,-.2);
		glVertex2f(.2,0);
	glEnd();
}

double myRandom(double min, double max){
	double	d;
	/*generates a uniformly distrabuted random number*/
	d = min+(max-min)*(rand()%0x7fff)/32767.0;	
	return d;
}
