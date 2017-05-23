
__kernel void mbrot(__write_only image2d_t oimage, float zoom, float shift){
	unsigned int icx = get_global_id(0);
	unsigned int icy = get_global_id(1);
	const float4 darkblue = (float4)(0.0, 0.0, 0.25, 0.0);
	const float4 darkred = (float4)(0.35, 0.05, 0.05, 0.0);
	const float4 white = (float4)(1.0, 1.0, 1.0, 0.0);
	float4 mycolor;
	float cx, cy, x, y, newx, newy, rsq;
	int i;

	cx = (2.0f * zoom * (float)(icx)) / (WIDTH - 1.0f) - zoom + shift;
	cy = (2.0f * zoom * (float)(icy)) / (DEPTH - 1.0f) - zoom;
	rsq = 0.0f;
	x = 0.0f;
	y = 0.0f;
	for(i = 0; i < 500 && rsq < 4.0f; i++){
		newx = x * x - y * y + cx;
		newy = 2.0f * x * y + cy;
		x = newx;
		y = newy;
		rsq = x * x + y * y;
	}
	if(rsq < 4.0) mycolor = darkred;
	else mycolor = mix(darkblue,white, 0.05f * (float)(i%20));
	write_imagef(oimage, (int2)(icx,icy), mycolor);
}
