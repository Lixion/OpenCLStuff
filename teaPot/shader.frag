varying vec3 ec_vnormal, ec_vposition, ec_tangent, ec_binormal;

uniform sampler2D mytexture;
uniform sampler2D mynormalmap;
uniform sampler2D backdrop;
uniform int Floor;

const float PI = 3.14159265;

void main()
{
	if(!Floor){
		mat3 tform;
		vec3 P, N, L, V, H, B, T, tcolor, ncolor, mapN;
		vec4 specular = gl_FrontMaterial.specular;
		float shininess = gl_FrontMaterial.shininess;
		tcolor = vec3(texture2D(mytexture, gl_TexCoord[0].st));	
		ncolor = vec3(texture2D(mynormalmap, gl_TexCoord[0].st));
		vec4 diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
		P = ec_vposition;
		N = normalize(ec_vnormal);
		L = normalize(gl_LightSource[0].position - P);
		V = normalize(-P);
		H = normalize(L+V);
		T = normalize(ec_tangent);
		B = normalize(ec_binormal);
		tform = mat3(T, B, N);
		mapN =  normalize((2.0 * ncolor) - vec3(1.0,1.0,1.0));
		N = normalize(tform * mapN);

		diffuse = vec4(tcolor,1.0);
		diffuse  *= max(dot(N,L), 0.0);
		specular *= pow(max(dot(H,N),0.0),shininess);
		specular *= ((shininess + 2) / (8 * PI));
		gl_FragColor = diffuse + specular;
	}
	else{
		if(Floor == 1){
			gl_FragColor = vec4(0.2,0.6,0.6,0.2);
		}
		else{
			vec3 tcolor = vec3(texture2D(backdrop, gl_TexCoord[0].st));	
			gl_FragColor = vec4(tcolor, 1.0);
		}
	}
}
