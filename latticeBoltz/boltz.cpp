//2D Lattice_Boltzman
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <oclUtils.h>

cl_command_queue commandQueue;
cl_kernel kernel;
cl_context context;
cl_program program;
cl_mem focl[2], distocl, omega_ocl;

#define SCALE 2
#define WIDTH (256*SCALE)
#define FIRST 0.0
#define LAST 2.0
#define FINAL_TIME (0.125 * ((double) (SCALE * SCALE)))
#define node(ix) (((double)(ix)/(double)(ix)(WIDTH))*(LAST - FIRST) + FIRST)
#define lambda(1.0/128.0) //spacing
#define tau (lambda*lambda/2.0) // time steps
#define SIGMA 0.5
#define DIRECTIONS 2
#define SINK (WIDTH+1)
#define SIZE ((WIDTH+2)*DIRECTIONS*sizeof(float))

float f[2][WIDTH+2][DIRECTIONS] //buffers
int dist[WIDTH+1][DIRECTIONS]; //node to send flow
float omega[DIRECTIONS][DIRECTIONS]; //collision matrix
int ci[DIRECTIONS] = {1, -1};

void geometry(){
	int j, k, tx;
	for(j = 0; j <= WIDTH; j++){
		for(k = 0; k < DIRECTIONS; k++){
			tx = j + ci[k];
			if(tx <= 0 || tx >= WIDTH){
				dist[j][k] = SINK;
			}
			else{
				dist[j][k] = tx;
			}
		}
	}
}

void load_omega(){
	omega[0][0] = SIGMA - 1.0;
	omega[1][1] = SIGMA - 1.0;
	omega[1][0] = 1.0 - SIGMA;
	omega[0][1] = 1.0 - SIGMA;
}

void init_lattice(){
	int j, k;
	for(j = 0; j <= WIDTH/2; j++){
		for(k = 0; k < DIRECTIONS; k++){
			f[0][j][k] = node(j)/2.0;
			f[1][j][k] = 0.0;
		}
	}
	for(j = WIDTH; j <= WIDTH; j++){
		for(k = 0; k < DIRECTIONS; k++){
			f[0][j][k] = node(j)/2.0;
			f[1][j][k] = 0.0;
		}
	}
}

double analytic(double x, double t){
	int k;
	double sign = -1.0;
	double sum = 0.0;
	double coeff;

	for(k = 0; k < 1000; k++){
		sign *= -1.0;
		coeff = 2.0 * ((double)(k) + 1.0) * M_PI;
		sum += 8.0 * sign * exp(-coeff*coeff*t/4.0)*sin(coeff*x/2.0)/(coeff*coeff);
	}
	return(sum);
}

void dump(){
	int j;
	double value, rho, err;
	for(j = 0; j <= WIDTH; j += SCALE){
		value = analytic(node(j), FINAL_TIME/((double)(SCALE * SCALE)));
		rho = (double)(f[0][j][0] + f[0][j][l]);
		err = fabs(rho - value);
		printf("%f %f %f\n", node(j), rho, err );
	}
}

void run_updates(){
	size_t gws[1] = {WIDTH + 1};
	size_t lws[1] = {57};
	int err, from = 0;
	cl_event wait[1];
	double t;

	focl[0]   = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, SIZE, &f[0][0][0], &err);
	focl[1]   = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, SIZE, &f[1][0][0], &err);
	distocl   = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, (WIDTH + 1) * DIRECTIONS *sizeof(int), &dist[0][0], &err);
	omega_ocl = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, DIRECTIONS*DIRECTIONS*sizeof(float), &omega[0][0], &err);

	clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&distocl);
	clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&omega_ocl);

	for(t = 0.0; t < FINAL_TIME; t = t + tau, from = 1 - from){
		clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&focl[from]);
		clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&focl[from - 1]);
		clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, gws, lws, 0, 0, &wait[0]);
		clWaitForEvents(1, wait);
	}
	clEnqueueReadBuffer(commandQueue, focl[from], CL_TRUE, 0, SIZE, f[0][0][0], 0, NULL, NULL);
	return;

}

void setup_ocl(){
	size_t program_length;
	CL_UNSIGNED_INT8 gpuDevCount;
	cl_platform_id platform;
	cl_device_id *device;
	cl_int err;

	char* source;
	err = oclGetPlatformID(&platform);
	cl_context_properties props[] = {
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0
	}
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, NULL, &gpuDevCount);
	device = new cl_device_id[gpuDevCount];
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, gpuDevCount, device, NULL);
	context = clCreateContext(props, 1, device[0], NULL, NULL, &err);
	commandQueue = clCreateCommandQueue(context, device[0], 0, &err);
	source = oclLoadProgSource("1dheat.cl", "#define DIRECTIONS 2\n", &program_length);
	program = clCreateProgramWithSource(context, 1, (const char **)&source, program_length, &err);
	clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	kernel = clCreateKernel(program, "update", &err);
	if(err == CL_SUCCESS){
		fprintf(stderr, "kernel build OK\n", );
	}
	else{
		fprintf(stderr, "kernel build Failed.\n", );

	}
}

void cleanup(){
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(commandQueue);
	clReleaseContext(context);
}

int main(int argc, char **argv){
	setup_ocl();
	geometry();
	init_lattice();
	load_omega();
	run_updates();
	dump();
	cleanup();
	return 0;
}