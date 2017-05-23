
#include <iostream>
using namespace std;
#include <oclUtils.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include "mypart.h"

#define DATA_SIZE (4 * TOTAL * sizeof(float))

GLuint OGL_VBO = 1;
cl_mem oclvbo, ocl_velo;

cl_kernel mykernel;
cl_program myprogram;
cl_command_queue mycommandq;
cl_context mycontext;

float mousePos[2] = {0, 0};
int right_click = 0, left_click = 0, Button, start;
float diff[4] = {0.0, 0.0, 0.0, 0.0};
size_t worksize[1] = {TOTAL};
size_t lws[1] = {64};
float eye[] = {0.0, 0.0, 5.0};
float view[] = {0.0, 0.0, 0.0};
float up[] = {0.0, 1.0, 0.0};

void do_kernel(){
	clSetKernelArg(mykernel,2,sizeof(float),(void*)&mousePos[0]);
	clSetKernelArg(mykernel,3,sizeof(float),(void*)&mousePos[1]);
	clSetKernelArg(mykernel,4,sizeof(int),(void*)&left_click);
	clSetKernelArg(mykernel,5,sizeof(int),(void*)&right_click);
	clSetKernelArg(mykernel,6,sizeof(int),(void*)&start);
	if(start){
		clEnqueueNDRangeKernel(mycommandq, mykernel, 1, NULL, 
					worksize, lws, 0, 0, NULL);
		start = 0;
	}
	else{
		clEnqueueNDRangeKernel(mycommandq, mykernel, 1, NULL, 
					worksize, lws, 0, 0, NULL);
	}
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
	change_color();
	glFinish(); 
	clEnqueueAcquireGLObjects(mycommandq, 1, &oclvbo, 0, 0, 0);
	do_kernel(); 
	clEnqueueReleaseGLObjects(mycommandq, 1, &oclvbo, 0, 0, 0);
	clFinish(mycommandq);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, OGL_VBO); 
	glVertexPointer(4, GL_FLOAT, 0, 0); 
	glEnableClientState(GL_VERTEX_ARRAY); 
	glDrawArrays(GL_POINTS, 0, TOTAL); 
	glDisableClientState(GL_VERTEX_ARRAY);
	glutSwapBuffers();
	glutPostRedisplay(); 
}

void setup_the_viewvol(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(65.0, wWIDTH/wHEIGHT, 0.1, 40.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye[0],eye[1],eye[2],view[0],view[1],view[2],up[0],up[1],up[2]);
}

void do_lights(){
	float light_ambient[]	= {0.0, 0.0, 0.0, 0.0};
	float light_diffuse[] 	= {1.0, 1.0, 1.0, 0.0};
	float light_specular[] 	= {1.0, 1.0, 1.0, 0.0};
	float light_position[] 	= {0.0, 0.0, 1.0, 1.0};
	float light_direction[]	= {-1.5, -2.0, -2.0, 1.0};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_ambient);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.0);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.4);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void do_material(){
	float mat_ambient[]		= {0.0, 0.0, 0.0, 1.0};
	float mat_specular[]	= {1.5, 1.5, 1.5, 1.0};
	float mat_shininess[] 	= {2.0};
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess); 
}

void InitGL(int argc, char** argv){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(wWIDTH, wHEIGHT);
	glutInitWindowPosition(100, 50);
	glutCreateWindow("Particles!");
	setup_the_viewvol();
	do_lights();
	do_material();
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.2, 0.2, 0.4, 0.0);
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

	oclGetPlatformID(&myplatform);
	clGetDeviceIDs(myplatform, CL_DEVICE_TYPE_GPU, 0, NULL, &gpudevcount);
	mydevice = new cl_device_id[gpudevcount];
	clGetDeviceIDs(myplatform, CL_DEVICE_TYPE_GPU, gpudevcount, mydevice, NULL);
	cl_context_properties props[] = {
		CL_GL_CONTEXT_KHR, (cl_context_properties)glXGetCurrentContext(), 
		CL_GLX_DISPLAY_KHR, (cl_context_properties)glXGetCurrentDisplay(), 
		CL_CONTEXT_PLATFORM, (cl_context_properties)myplatform, 
		0};
	mycontext = clCreateContext(props, 1, &mydevice[0], NULL, NULL, &err);
	mycommandq = clCreateCommandQueue(mycontext, mydevice[0], 0, &err);
	header = oclLoadProgSource("mypart.h", "", &program_length);
	oclsource = oclLoadProgSource("vv.cl", header, &program_length);
	myprogram = clCreateProgramWithSource(mycontext, 1, (const char **)&oclsource, 							&program_length, &err);

	clBuildProgram(myprogram, 0, NULL, NULL, NULL, NULL);
	mykernel = clCreateKernel(myprogram, "Play", &err);
	if (err != CL_SUCCESS) { printf("Unable to create kernel\n");
	switch(err){
		case CL_INVALID_PROGRAM: printf("1\n"); break;
		case CL_INVALID_PROGRAM_EXECUTABLE: printf("2\n"); break;
		case CL_INVALID_KERNEL_NAME: printf("3\n"); break;
		case CL_INVALID_KERNEL_DEFINITION: printf("4\n"); break;
		case CL_INVALID_VALUE: printf("5\n"); break;
		case CL_OUT_OF_HOST_MEMORY: printf("6\n"); break;
		default: printf("No idea\n"); break;
	}
	
  	size_t len;
        char buffer[2048];
        clGetProgramBuildInfo(myprogram, mydevice[0], CL_PROGRAM_BUILD_LOG,
                                          sizeof(buffer), buffer, &len);

        fprintf(stderr, "%s\n", buffer);
}
	glBindBuffer(GL_ARRAY_BUFFER, OGL_VBO);	
	glBufferData(GL_ARRAY_BUFFER, DATA_SIZE, 0, GL_DYNAMIC_DRAW);	

   	cl_float4 *buffer = new cl_float4[TOTAL];

	oclvbo = clCreateFromGLBuffer(mycontext, CL_MEM_READ_WRITE, OGL_VBO, &err);
	clSetKernelArg(mykernel, 0, sizeof(cl_mem), (void*)&oclvbo);
	ocl_velo = clCreateBuffer(mycontext, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 						DATA_SIZE, (void *)buffer, &err);
	clSetKernelArg(mykernel, 1, sizeof(cl_mem), (void*)&ocl_velo);    srand(time(NULL));
}

void cleanup(){
	clReleaseKernel(mykernel);
	clReleaseProgram(myprogram);
	clFlush(mycommandq);
	clReleaseCommandQueue(mycommandq);
	glBindBuffer(GL_ARRAY_BUFFER, OGL_VBO);
	glDeleteBuffers(1, &OGL_VBO);
	clReleaseMemObject(oclvbo);
	clReleaseMemObject(ocl_velo);
	clReleaseContext(mycontext);
	exit(0);
}

void click(int button, int state, int x, int y){
	Button = button;
	if (state == GLUT_DOWN){
		if(Button == GLUT_LEFT_BUTTON){
			left_click = 1;
		}
		if(Button == GLUT_RIGHT_BUTTON)
			right_click = 1;
		/*mousePos[0] = (2.0 * x / wWIDTH - 1.0) * 3.15;
		mousePos[1] = (2.0 * y / wHEIGHT- 1.0) * -3.15;*/
	} 	
	else if (state == GLUT_UP){
		if(Button == GLUT_LEFT_BUTTON)
			left_click = 0;
		if(Button == GLUT_RIGHT_BUTTON)
			right_click = 0;
    	}
   	glutPostRedisplay();
}

void drag(int x, int y){
	if (left_click /*|| right_click*/){
			mousePos[0] = (2.0 * x / wWIDTH - 1.0) * 3.15;
			mousePos[1] = (2.0 * y / wHEIGHT- 1.0) * -3.15;
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

int main(int argc, char **argv){
	start = 1;
	InitGL(argc, argv);
	InitCL();
	glutDisplayFunc(mydisplayfunc);
	glutKeyboardFunc(getout);
	glutMouseFunc(click);
	glutMotionFunc(drag);
	glutMainLoop();
}
