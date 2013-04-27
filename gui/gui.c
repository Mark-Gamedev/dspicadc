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
			if(maxIndex==i*x+j){
				fillBox(j*(w+PADDING)+offsetx, i*(h+PADDING)+offsety, j*(w+PADDING)+w+offsetx, i*(h+PADDING)+h+offsety, 1.0, 0.0, 0.0);
			}else{
				fillBox(j*(w+PADDING)+offsetx, i*(h+PADDING)+offsety,
						j*(w+PADDING)+w+offsetx, i*(h+PADDING)+h+offsety,
						(float)locationCorr[i*x+j], (float)locationCorr[i*x+j], (float)locationCorr[i*x+j]);
			}
		}
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	drawContainBox();
	fillBox(PADDING, PADDING, WIDTH+PADDING, HEIGHT+PADDING, 1.0, 1.0, 1.0);
	drawBoxes(10, PADDING*2, PADDING*2);

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

int main(int argc, char **argv)
{
	if(argc < 2){
		exit(0);
	}

	signal(SIGINT, interruptHandler);

	pthread_t updateThread;
	pthread_create(&updateThread, 0, updateFromServer, argv[1]);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("single triangle");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;             /* ANSI C requires main to return int. */
}

