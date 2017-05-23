#define STEPS_PER_RENDER 25
#define MASS 1.0f
#define DELTA_T (0.002f)
#define FRICTION 0.1f
#define RESTITUTION 0.5f
#define PI 3.1415
#define EPS_DOWN (-0.4f) 
#define V_DRAG (4.0f) 

////BOTTOM, LEFT = -3.15; TOP, RIGHT = 3.15

float4 getforce(float4 pos, float4 vel){ 
	float4 force;
	force.x = pos.x * -0.3;
	force.y = EPS_DOWN - V_DRAG*vel.y - 0.001;
	force.z = 0.0;
	force.w = 1.0f;
	return(force);
}

__constant float radius = CIRCLEr * CIRCLEr;

__kernel void Play(__global float4* p , __global float4* v,
		   float mousex, float mousey, int left, int right, int start){
	unsigned int i = get_global_id(0);

	float4 force;
	float mylength;
	float4 pos  = p[i];
	float4 velo = v[i];
	float4 normal;
	if(start){
			float x = 2*(i - 1000 *(i/1000)) * 3.14/1000 - 3.14;
			float y = 2*((i/1000) *  3.14/1000) - 3.14;
			pos = (float4)(x, y, 0.0, 1.0);
			velo = (float4)(atan(x), asin(y), 0.0, 0.0);
	}
	else if(right){/* PAUSE IMAGE*/}
	else{
		for(int steps=0;steps<STEPS_PER_RENDER;steps++){
			if(left){
				float dx = mousex - pos.x;
				float dy = mousey - pos.y;
				float dist = sqrt((dx*dx+dy*dy));
				if(dist <1) dist = 1;
				velo.y += dy/dist*0.02/dist + 0.0001;
				velo.x += dx/dist*0.02/dist + 0.0001;
			}
			
			force = getforce(pos,velo);
			velo += force*DELTA_T/2.0; 
			pos += velo*DELTA_T; 
			force = getforce(pos,velo); 
			velo += force*DELTA_T/2.0;

			if(pos.x > 2.50){
				pos.x = 2.49;
				velo.x = velo.x - ((1.0 + RESTITUTION)*velo.x);
				mylength = sqrt(velo.y*velo.y);
				if(mylength>0.0){
					velo.y -= FRICTION*velo.y/mylength;
				}
			}
			if(pos.x < -2.50){
				pos.x = -2.49;
				velo.x = velo.x - ((1.0 + RESTITUTION)*velo.x);
				mylength = sqrt(velo.y*velo.y);
				if(mylength>0.0){
					velo.y -= FRICTION*velo.y/mylength;
				velo.y *= 0.25 ;
				velo.x *= 0.25;
				}
			}
			if(pos.y > 2.50){
				pos.y = -2.49;
			}
		 	if(pos.y < -2.50){
				pos.y = -2.49;
				mylength = sqrt(velo.x*velo.x);
				if(mylength>0.0){
					velo.x -= FRICTION*velo.x/mylength;
				}
			}

			normal = (float4)((CIRCLEx - pos.x + 0.01), 
						(CIRCLEy - pos.y + 0.01), 0.0, 1.0); 
			float dist = sqrt(normal.x * normal.x + normal.y * normal.y);
			if(dist < radius){
					normal.x /= dist;
					normal.y /= dist;
					if(pos.x <= 0)
						pos = (float4)(pos.x - 0.1, pos.y - 0.1, pos.z, pos.w);
					else
						pos = (float4)(pos.x + 0.1, pos.y - 0.1, pos.z, pos.w);
					float4 dotP = (dot(pos,normal)) * normal;
					float4 vout = pos - (1.0 + RESTITUTION) *dotP - 
							FRICTION * (pos - dotP)/ sqrt((pos - dotP) * (pos - dotP));
					vout *= 0.8;
					velo.x = vout.x;
					velo.y = vout.y;
			}
			/*if(right){ Ball at pos was eating pixels or forcing them out of bounds*
				normal = (float4)((mousex - pos.x + 0.01), 
							(mousey - pos.y + 0.01), 0.0, 1.0); 
				float dist = sqrt(normal.x * normal.x + normal.y * normal.y);
				if(dist < radius && right){
						normal.x /= dist;
						normal.y /= dist;
						if(pos.x <= 0)
							pos = (float4)(pos.x - 0.1, pos.y - 0.1, pos.z, pos.w);
						else
							pos = (float4)(pos.x + 0.1, pos.y - 0.1, pos.z, pos.w);
						float4 dotP = (dot(pos,normal)) * normal;
						float4 vout = pos - (1.0 + RESTITUTION) *dotP - 
												FRICTION * (pos - dotP)
						 / sqrt((pos - dotP) * (pos - dotP));
						vout *= 0.8;
						velo.x = vout.x;
						velo.y = vout.y;
				}
			}*/
		}
	}
	p[i] = pos;
	v[i] = velo;
	p[i].w = 1.0;
}
