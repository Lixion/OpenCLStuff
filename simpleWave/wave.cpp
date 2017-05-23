
#include <oclUtils.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include "mypart.h"

#define DATA_SIZE (WIDTH*DEPTH*4*sizeof(float))
GLuint OGL_VBO = 1;

float diff[] = {1.0, 1.0, 1.0, 1.0};
cl_mem oclvbo;
cl_kernel mykernel;
cl_program myprogram;
cl_command_queue mycommandqueue;
cl_context mycontext;
int left_click, Button;
int mousePos[2] = {0, 0};
size_t worksize[2] = {WIDTH,DEPTH};
size_t lws[2] = {8,8};

void do_kernel(){
	static float mytime = 0.0;
	mytime += 0.05;
	int err;
	err = clSetKernelArg(mykernel,1,sizeof(float),&mytime);
	err = clEnqueueNDRangeKernel(mycommandqueue,mykernel,2,NULL,worksize,lws,0,0,NULL);
}

void change_color(){
	static int cycle = 0;
	switch(cycle) {							
		case(0):							
			diff[1]  += 4.0/255.0;
			if(diff[1] >= 2.0){ cycle = 1; diff[1] = 2.0; }
			break;
		case(1):
			diff[0]  -= 4.0/255.0;
			if(diff[0] <= 0.0){ cycle = 2; diff[0] = 0.0; }
			break;
		case(2):
			diff[2]  += 4.0/255.0;
			if(diff[2] >= 2.0){ cycle = 3; diff[2] = 2.0; }
			break;
		case(3):
			diff[1]  -= 4.0/255.0;
			if(diff[1] <= 0.0){ cycle = 4; diff[1] = 0.0; }
			break;
		case(4):
			diff[0]  += 4.0/255.0;
			if(diff[0] >= 2.0){ cycle = 5; diff[0] = 2.0; }
			break;
		case(5):
			diff[2]  -= 4.0/255.0;
			if(diff[2] <= 0.0){ cycle = 0; diff[2] = 0.0; }
			break;
    } 
	glMaterialfv(GL_FRONT, GL_DIFFUSE,   diff);
}

void mydisplayfunc(){
	glFinish(); 
	clEnqueueAcquireGLObjects(mycommandqueue,1,&oclvbo,0,0,0);
	do_kernel(); 
	clEnqueueReleaseGLObjects(mycommandqueue,1,&oclvbo,0,0,0);
	clFinish(mycommandqueue);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindBuffer(GL_ARRAY_BUFFER,OGL_VBO);
	change_color();
	glVertexPointer(4,GL_FLOAT,0,0); 
	glEnableClientState(GL_VERTEX_ARRAY); 
	glDrawArrays(GL_POINTS,0,WIDTH*DEPTH);
	glDisableClientState(GL_VERTEX_ARRAY);
	glutSwapBuffers();
	glutPostRedisplay();
}

float eye[] = {2.0,2.0,2.0};
float view[] = {0.0,0.0,0.0};
float up[] = {0.0,1.0,0.0};

void setup_the_viewvol(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,1.0,0.1,20.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye[0],eye[1],eye[2],view[0],view[1],view[2],up[0],up[1],up[2]);
}

void do_lights(){
	float light_ambient[]	= {0.0,0.0,0.0,0.0};
	float light_diffuse[] 	= {1.0,1.0,1.0,0.0};
	float light_specular[] 	= {1.0,1.0,1.0,0.0};
	float light_position[] 	= {1.5,2.0,2.0,1.0};
	float light_direction[]	= {-1.5,-2.0,-2.0,1.0};

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT,light_ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);

	glLightfv(GL_LIGHT0,GL_AMBIENT,light_ambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,light_diffuse);
	glLightfv(GL_LIGHT0,GL_SPECULAR,light_specular);
	glLightf(GL_LIGHT0,GL_SPOT_EXPONENT,1.0);
	glLightf(GL_LIGHT0,GL_SPOT_CUTOFF,180.0);
	glLightf(GL_LIGHT0,GL_CONSTANT_ATTENUATION,0.5);
	glLightf(GL_LIGHT0,GL_LINEAR_ATTENUATION,0.0);
	glLightf(GL_LIGHT0,GL_QUADRATIC_ATTENUATION,0.4);
	glLightfv(GL_LIGHT0,GL_POSITION,light_position);
	glLightfv(GL_LIGHT0,GL_SPOT_DIRECTION,light_direction);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void do_material(){
	float mat_ambient[]	= {0.0,0.0,0.0,1.0};
	float mat_diffuse[]	= {2.8,2.8,0.4,1.0};
	float mat_specular[]	= {1.5,1.5,1.5,1.0};
	float mat_shininess[] 	= {2.0};

	glMaterialfv(GL_FRONT,GL_AMBIENT,mat_ambient);
	glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diffuse);
	glMaterialfv(GL_FRONT,GL_SPECULAR,mat_specular);
	glMaterialfv(GL_FRONT,GL_SHININESS,mat_shininess); 
}

void InitGL(int argc,char** argv){
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
	glutInitWindowSize(512,512);
	glutInitWindowPosition(100,50);
	glutCreateWindow("Do the wave.");
	setup_the_viewvol();
	do_lights();
	do_material();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.1,0.2,0.35,0.0);
	glewInit();
	return;
}

void InitCL(){
	cl_platform_id myplatform;
	cl_device_id *mydevice;

	size_t program_length;
	int err;
	unsigned int gpudevcount;
	char *oclsource;
	const char *header;

	err = oclGetPlatformID(&myplatform);
	err = clGetDeviceIDs(myplatform,CL_DEVICE_TYPE_GPU,0,NULL,&gpudevcount);
	mydevice = new cl_device_id[gpudevcount];
	err = clGetDeviceIDs(myplatform,CL_DEVICE_TYPE_GPU,gpudevcount,mydevice,NULL);

	cl_context_properties props[] = {
		CL_GL_CONTEXT_KHR,(cl_context_properties)glXGetCurrentContext(),
		CL_GLX_DISPLAY_KHR,(cl_context_properties)glXGetCurrentDisplay(),
		CL_CONTEXT_PLATFORM,(cl_context_properties)myplatform,
		0};

	mycontext = clCreateContext(props,1,&mydevice[0],NULL,NULL,&err);
	mycommandqueue = clCreateCommandQueue(mycontext,mydevice[0],0,&err);

	header = oclLoadProgSource("mypart.h", "", &program_length);
	oclsource = oclLoadProgSource("wave.cl",header,&program_length);

	myprogram = clCreateProgramWithSource(mycontext,1,(const char **)&oclsource,&program_length,&err);
	err = clBuildProgram(myprogram,0,NULL,NULL,NULL,NULL);
	mykernel = clCreateKernel(myprogram,"wave",&err);
	glBindBuffer(GL_ARRAY_BUFFER,OGL_VBO);
	glBufferData(GL_ARRAY_BUFFER,DATA_SIZE,0,GL_DYNAMIC_DRAW);	
	oclvbo = clCreateFromGLBuffer(mycontext,CL_MEM_WRITE_ONLY,OGL_VBO,&err);
	clSetKernelArg(mykernel,0,sizeof(cl_mem),(void*)&oclvbo);
}

void cleanup(){
	clReleaseKernel(mykernel);
	clReleaseProgram(myprogram);
	clReleaseCommandQueue(mycommandqueue);
	glBindBuffer(GL_ARRAY_BUFFER,OGL_VBO);
	glDeleteBuffers(1,&OGL_VBO);
	clReleaseMemObject(oclvbo);
	clReleaseContext(mycontext);

	exit(0);
}

void click(int button, int state, int x, int y)
{
	Button = button;
	if (state == GLUT_DOWN){
		if(button == GLUT_LEFT_BUTTON)
			left_click = 1;
	} 	
	else if (state == GLUT_UP){
		if(button == GLUT_LEFT_BUTTON)
			left_click = 0;
    	}

	mousePos[0] = 2.0 * 512 * x / 800 - 512;
 	mousePos[1] = -1*(2.0 * 512 * y / 800 - 512);
   	glutPostRedisplay();
}

void drag(int x, int y)
{
	if (left_click){
		if(Button == GLUT_LEFT_BUTTON){
			mousePos[0] = 2.0 * 512 * x / 800 - 512;
 			mousePos[1] = -1*(2.0 * 512 * y / 800 - 512);
		}
	}
}

void getout(unsigned char key, int x, int y){
	switch(key){
		case 'q':
			cleanup();
		default:
			break;
	}
}

int main(int argc,char **argv){
	InitGL(argc,argv);
	InitCL();
	glutDisplayFunc(mydisplayfunc);
	glutKeyboardFunc(getout);
	glutMainLoop();
}
