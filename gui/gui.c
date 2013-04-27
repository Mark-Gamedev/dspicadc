#include <unistd.h>
#include <stdio.h>
#include <GL/glut.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#define HEIGHT 700
#define WIDTH  700
#define PADDING 2

#define PORT 3300
#define CORR_THRESHOLD 0.4

int boxNum = 0;
#define LOCATION 100
double locationCorr[LOCATION];
int maxIndex;
int win = 0;
int mole = -1;

int exitProgram = 0;
void interruptHandler(int signum){
	exitProgram = 1;
}

int maxIndexOfLocationCorr(){
	int index = 0;
	double max = locationCorr[index];
	int i=1;
	for(;i<LOCATION;i++){
		if(locationCorr[i] > max){
			index = i;
			max = locationCorr[i];
		}
	}
	if(max < CORR_THRESHOLD){
		return maxIndex;
	}else{
		return index;
	}
}

void error(char *m){
	perror(m);
	exit(0);
}

void reshape(int w, int h)
{
	/* Because Gil specified "screen coordinates" (presumably with an
	   upper-left origin), this short bit of code sets up the coordinate
	   system to correspond to actual window coodrinates.  This code
	   wouldn't be required if you chose a (more typical in 3D) abstract
	   coordinate system. */

	glViewport(0, 0, w, h);       /* Establish viewing area to cover entire window. */
	glMatrixMode(GL_PROJECTION);  /* Start modifying the projection matrix. */
	glLoadIdentity();             /* Reset project matrix. */
	glOrtho(0, w, 0, h, -1, 1);   /* Map abstract coords directly to window coords. */
	glScalef(1, -1, 1);           /* Invert Y axis so increasing Y goes down. */
	glTranslatef(0, -h, 0);       /* Shift origin up to upper-left corner. */
}


void drawBox(int x0, int y0, int x1, int y1){
	glBegin(GL_LINES);
	glColor3f(1.0, 1.0, 1.0);  /* blue */

	glVertex2i(x0, y0);
	glVertex2i(x1, y0);
	glVertex2i(x1, y0);
	glVertex2i(x1, y1);
	glVertex2i(x1, y1);
	glVertex2i(x0, y1);
	glVertex2i(x0, y1);
	glVertex2i(x0, y0);

	glEnd();
}

void fillBox(int x0, int y0, int x1, int y1, float r, float g, float b){
	glBegin(GL_TRIANGLES);
	glColor3f(r, g, b);  /* blue */

	glVertex2i(x0, y0);
	glVertex2i(x1, y0);
	glVertex2i(x0, y1);

	glVertex2i(x0, y1);
	glVertex2i(x1, y1);
	glVertex2i(x1, y0);

	glEnd();
}

void drawContainBox(){
	drawBox(PADDING, PADDING, WIDTH+PADDING, HEIGHT+PADDING);
}

void drawBoxes(int x, int offsetx, int offsety){
	int i=0;
	int w,h;

	w = (WIDTH - (x+1)*PADDING)/x;
	h = (HEIGHT - (x+1)*PADDING)/x;
	for(;i<x;i++){
		int j=0;
		for(;j<x;j++){
			int currentIndex = i*x+j;
			fillBox(j*(w+PADDING)+offsetx, i*(h+PADDING)+offsety,
					j*(w+PADDING)+w+offsetx, i*(h+PADDING)+h+offsety,
					(float)locationCorr[i*x+j], (float)locationCorr[i*x+j], (float)locationCorr[i*x+j]);
			if(maxIndex==currentIndex){
				fillBox(j*(w+PADDING)+offsetx, i*(h+PADDING)+offsety, j*(w+PADDING)+w+offsetx, i*(h+PADDING)+h+offsety, 1.0, 0.0, 0.0);
			}
			if(mole==currentIndex){
				fillBox(j*(w+PADDING)+offsetx, i*(h+PADDING)+offsety, j*(w+PADDING)+w+offsetx, i*(h+PADDING)+h+offsety, 0.0, 1.0, 0.0);
			}
			if(mole == currentIndex && maxIndex == currentIndex){
				fillBox(j*(w+PADDING)+offsetx, i*(h+PADDING)+offsety, j*(w+PADDING)+w+offsetx, i*(h+PADDING)+h+offsety, 1.0, 1.0, 0.0);
			}
		}
	}
}

// Here are the fonts: 
void *glutFonts[7] = { 
	GLUT_BITMAP_9_BY_15, 
	GLUT_BITMAP_8_BY_13, 
	GLUT_BITMAP_TIMES_ROMAN_10, 
	GLUT_BITMAP_TIMES_ROMAN_24, 
	GLUT_BITMAP_HELVETICA_10, 
	GLUT_BITMAP_HELVETICA_12, 
	GLUT_BITMAP_HELVETICA_18 
}; 

// Here is the function 
void glutPrint(float x, float y, void *font, char* text, float r, float g, float b, float a) 
{ 
	if(!text || !strlen(text)) return; 
	int blending = 0; 
	if(glIsEnabled(GL_BLEND)) blending = 1; 
	glEnable(GL_BLEND); 
	glColor4f(r,g,b,a); 
	glRasterPos2f(x,y); 
	while (*text) { 
		glutBitmapCharacter(font, *text); 
		text++; 
	} 
	if(!blending) glDisable(GL_BLEND); 
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	drawContainBox();
	fillBox(PADDING, PADDING, WIDTH+PADDING, HEIGHT+PADDING, 1.0, 1.0, 1.0);
	drawBoxes(10, PADDING*2, PADDING*2);
	if(win){
		glutPrint(200.0f, 100.0f, glutFonts[3], "Good Job!!", 1.0f, 1.0f, 0.0f, 0.7f);
	}

	glutSwapBuffers();
	glutPostRedisplay();
}

int sockfd;
void tcpClient(char *hostname){
	struct sockaddr_in serveraddr;
	struct hostent *server;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
		error("ERROR opening socket");

	server = gethostbyname(hostname);
	if(!server){
		exit(0);
	}

	bzero((char*)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(PORT);

	if(connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) <0)
		error("ERROR connecting");
}

void *updateFromServer(void *arg){
	tcpClient(arg);
	while(!exitProgram){
		read(sockfd, locationCorr, sizeof(locationCorr));
		maxIndex = maxIndexOfLocationCorr();
	}
	close(sockfd);
	exit(0);
}

void *moleFunc(void *arg){
	while(1){
		if(mole==maxIndex){
			win = 1;
			sleep(2);
			win = 0;
		}
		mole = rand()%100;
		while(mole==maxIndex){
			mole = rand()%100;
		}
		sleep(1);
	}
}

int main(int argc, char **argv)
{
	if(argc < 2){
		exit(0);
	}

	signal(SIGINT, interruptHandler);

	pthread_t updateThread;
	pthread_create(&updateThread, 0, updateFromServer, argv[1]);
	pthread_t moleThread;
	pthread_create(&moleThread, 0, moleFunc, 0);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("single triangle");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;             /* ANSI C requires main to return int. */
}

