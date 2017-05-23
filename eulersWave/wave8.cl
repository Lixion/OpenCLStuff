float dot8(float8 omega, float8 focl){
	return dot(omega.lo, focl.lo) + dot(omega.hi, focl.hi);
}

__kernel void update(__global float8 *focl, __global float8 *tocl, __global int *dist, __global float8 *omega_ocl){
	int k, n, idx;
	int i = get_global_id(0);
	int j = get_global_id(1);
	float8 dir;

	for(k = 0; k < DIRECTIONS; k++){
		dir = (float8)(0.0);
		for(n = 0; n < DIRECTIONS; n++){
			idx = store(i, j, n);
			dir += omega_ocl[k*DIRECTIONS+n] * focl[idx];			
		}
		idx = store(i, j, k);
		tocl[dist(i, j, k)] += (focl[idx] + dir);
	}
	for(k = 0; k < DIRECTIONS; k++){
		idx = store(i, j, k);
		focl[idx] = (float8)(0.0);
	}

}


__kernel void heights(__global float *rbuffer_ocl, __global float8 *focl){
	int k;
	int i = get_global_id(0);
	int j = get_global_id(1);
	int idx = 3 * ((2 * j * WIDTH + 2 * i) - (2 * WIDTH - 1)) + 1;
	int inx = 3 * (2 * j * WIDTH + 2 * i) + 1;
	float8 sum = (float8)(0.0);
	float height = 0.0;

	for(k = 0; k < DIRECTIONS; k++){
		sum = focl[store(j, i, k)];
		height += sum.s0 + sum.s1 + sum.s2 + sum.s3 + sum.s4 + sum.s5
		+ sum.s0 + sum.s7;
	}
	
	rbuffer_ocl[inx] = height;
	if(j > 0){
		rbuffer_ocl[3 * (2 * (j - 1) * WIDTH + 2 * i) + 4] = height;
	}


	
}


__kernel void normals(__global float *rbuffer_ocl, __global float4 *nbuffer_ocl){
	int i = get_global_id(0);
	int j = get_global_id(1);
	float y1, y2, y3, y4;
	int i0, i1, j0, j1;
	float4 n = (float4)(0.0, 1.0, 0.0, 0.0);

	if(j > 0){
		y1 = rbuffer_ocl[3 * (2 * j * WIDTH + 2 * (i + 1)) + 1];
		y2 = rbuffer_ocl[3 * (2 * j * WIDTH + 2 * (i - 1)) + 1];
		y3 = rbuffer_ocl[3 * (2 * (j + 1) * WIDTH + 2 * i) + 1];
		y4 = rbuffer_ocl[3 * (2 * (j - 1) * WIDTH + 2 * i) + 1];
		n = (float4)(y2 - y1, 2 * SCALE/WIDTH, (LENGTH/WIDTH) * (y4 - y3), 0.0);
		n = normalize(n);
	}
	nbuffer_ocl[(j * WIDTH + i)] = n;
	
}


__kernel void colors(__global float* rbuffer_ocl, float4 lightdir, __global float4 *eye_dir, __global float4 *nbuffer_ocl){
	int i = get_global_id(0);
	int j = get_global_id(1);
	int index = 3 * (2 * j * WIDTH + 2 * i) + COLOR_OFF;
	float4 normal = nbuffer_ocl[j * WIDTH + i];
	float4 skycolor = (float4)(0.45,0.45,0.45,1.0),
	darkcolor = (float4)(0.0,0.5,0.8,1.0),
	lightcolor = (float4)(0.2,0.4,0.6,1.0);

	float4 waveNEC = (float4)(dot(eye_dir[0], normal), dot(eye_dir[1], normal),
			dot(-eye_dir[2], normal), 0.0);

	float4 lightness = max(0.0, ((-1.0 * waveNEC) * eye_dir[2]));
	float4 color = dot(waveNEC, lightdir) * (mix(darkcolor,lightcolor,lightness))
		 + 0.1 * skycolor * pow(1 - lightness, 5);

	if(j > 0 && j < WIDTH - 1){
		rbuffer_ocl[index + 0] = color.x;
		rbuffer_ocl[index + 3] = color.x; 
		rbuffer_ocl[index + 1] = color.y; 
		rbuffer_ocl[index + 4] = color.y; 
		rbuffer_ocl[index + 2] = color.z; 
		rbuffer_ocl[index + 5] = color.z; 
	}
}
