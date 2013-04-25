#include <unistd.h>
#include <GL/glut.h>
#include <pthread.h>

#define HEIGHT 600
#define WIDTH  600
#define PADDING 2

int boxNum = 0;

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
			if(boxNum==i*x+j){
				fillBox(j*(w+PADDING)+offsetx, i*(h+PADDING)+offsety, j*(w+PADDING)+w+offsetx, i*(h+PADDING)+h+offsety, 1.0, 0.0, 0.0);
			}else{
				fillBox(j*(w+PADDING)+offsetx, i*(h+PADDING)+offsety, j*(w+PADDING)+w+offsetx, i*(h+PADDING)+h+offsety, 1.0, 1.0, 1.0);
			}
		}
	}
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	drawContainBox();
	drawBoxes(10, PADDING*2, PADDING*2);

	glutSwapBuffers();
	glutPostRedisplay();
}

int counter = 0;
void *updateFromServer(void *arg){
	//TODO: connect to server
	while(1){
		//TODO: read int from server
		sleep(1);
		counter++;
		if(counter==100){
			counter=0;
		}
		boxNum = counter;
		usleep(100000);
		boxNum = -1;
	}
}

int main(int argc, char **argv)
{
	pthread_t updateThread;
	pthread_create(&updateThread, 0, updateFromServer, 0);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutCreateWindow("single triangle");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMainLoop();
	return 0;             /* ANSI C requires main to return int. */
}

