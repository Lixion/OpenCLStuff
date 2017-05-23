#define from(row,col) from[DIRECTIONS*(row)+(col)]
#define to(row, col) to[DIRECTIONS*(row)+(col)]
#define omega(row,col) omega[DIRECTIONS*(row)+(col)]
#define dist(row,col) dist[DIRECTIONS*(row)+(col)]

__kernel void update(__global float* from, __global float* to, __global int* dist, __constant float* omega){
	int k, n, outptr;
	int j = get_global_id(0);

	float new_destiny;

	for(k = 0; k < DIRECTIONS; k++){
		new_destiny = 0.0;
		for(n = 0; n < DIRECTIONS; n++){
			new_destiny += omega(k,n)*from(j,n);
		}
		outptr = dist(j,k);
		to(outptr,k) += (from(j,k) + new_destiny);
	}
	for( k = 0; K < DIRECTIONS; k++){
		from(j,k) = 0.0;
	}
}