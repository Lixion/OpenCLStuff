__kernel void dwt(__global float *a, unsigned int span){
	unsigned int i = get_local_id(0);
	unsigned int j = get_local_id(0);
	unsigned int step;
	__local float sarray[BS];
	float number;

	sarray[i] = a[(j*BS+i)*span];
	barrier(CLK_LOCAL_MEM_FENCE);

	for(step=BS; step>2; step/=2){
		if(i<BS/step){
			number = sarray[(i*step+step/2]);
			sarray[(i*step+step/2)] = sarray[i*step] - number;
			sarray[i*step] += number;
		}
		barrier(CLK_LOCAL_MEM_FENCE);
	}
	a[(j*BS+i)*span] = sarray[i];
	barrier(CLK_GLOBAL_MEM_FENCE);
}
	
