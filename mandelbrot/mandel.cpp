#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <oclUtils.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include "mypart.h"

#define DATA_SIZE (WIDTH*DEPTH*4*sizeof(float))
#define TIME 0.0075

GLuint OGL_PBO = 1;

float diff[] = {1.0, 1.0, 1.0, 1.0};
cl_mem oclpbo;
cl_kernel mykernel;
cl_program myprogram;
cl_command_queue mycommandq;
cl_context mycontext;
int left_click, Button;
int mousePos[2] = {0, 0};
size_t worksize[2] = {WIDTH,DEPTH};
size_t lws[2] = {8,8};
float eye[] = {2.0,2.0,2.0};
float view[] = {0.0,0.0,0.0};
float up[] = {0.0,1.0,0.0};

void do_kernel(){
	void *ptr;
	cl_event wlist[1];
	const size_t start[3] = {0, 0, 0};
	const size_t scope[3] = {WIDTH, DEPTH, 1};
	float time = 0.0;
	time += TIME;
	clSetKernelArg(mykernel,1,sizeof(float),(void*)&time);
	clEnqueueNDRangeKernel(mycommandq, mykernel, 2, NULL, worksize, lws, 0, 0, &wlist[0]);
	clWaitForEvents(1, wlist);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, OGL_PBO);
	ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_ONLY);
	clEnqueueReadImage(mycommandq, oclpbo, CL_TRUE, start, scope, 0, 0, ptr, 0, NULL, NULL);
	glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
}

void mydisplayfunc(){
	do_kernel(); 
	glClear(GL_COLOR_BUFFER_BIT);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, OGL_PBO);
	glDrawPixels(WIDTH, DEPTH, GL_RGBA, GL_FLOAT, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	glutSwapBuffers();
	glutPostRedisplay();
}

void setup_the_viewvol(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void InitGL(int argc,char** argv){
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(WIDTH,DEPTH);
	glutInitWindowPosition(100,50);
	glutCreateWindow("Madel Set");
	setup_the_viewvol();
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.1,0.2,0.35,0.0);
	glewInit();
	return;
}

void InitCL(float zoom, float shift){
	cl_platform_id myplatform;
	cl_device_id *mydevice;
	cl_image_format myformat;
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
	mycommandq = clCreateCommandQueue(mycontext,mydevice[0],0,&err);

	header = oclLoadProgSource("mypart.h", "", &program_length);
	oclsource = oclLoadProgSource("mandel.cl",header,&program_length);

	myprogram = clCreateProgramWithSource(mycontext,1,(const char **)&oclsource,&program_length,&err);
	err = clBuildProgram(myprogram,0,NULL,NULL,NULL,NULL);
	mykernel = clCreateKernel(myprogram,"wave",&err);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER,OGL_PBO);
	glBufferData(GL_PIXEL_UNPACK_BUFFER,DATA_SIZE, NULL,GL_DYNAMIC_DRAW);
	myformat.image_channel_order = CL_RGBA;
	myformat.image_channel_data_type = CL_FLOAT;
	oclpbo = clCreateImage2D(mycontext, CL_MEM_WRITE_ONLY, &myformat, WIDTH, DEPTH, 0, NULL, &err);
  	size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(myprogram, mydevice[0], CL_PROGRAM_BUILD_LOG,
                                          sizeof(buffer), buffer, &len);

        fprintf(stderr, "%s\n", buffer);
	clSetKernelArg(mykernel,0,sizeof(cl_mem),(void*)&oclpbo);
}

void cleanup(){
	clReleaseKernel(mykernel);
	clReleaseProgram(myprogram);
	clReleaseCommandQueue(mycommandq);
	glBindBuffer(GL_ARRAY_BUFFER,OGL_PBO);
	glDeleteBuffers(1,&OGL_PBO);
	clReleaseMemObject(oclpbo);
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
	InitCL(2.0, 0.0);
	glutDisplayFunc(mydisplayfunc);
	glutKeyboardFunc(getout);
	glutMainLoop();
}
