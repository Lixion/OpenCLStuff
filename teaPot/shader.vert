varying vec3 ec_vnormal, ec_vposition, ec_tangent, ec_binormal;

attribute vec3 Tangent;
attribute vec3 Binormal;

void main()
{
	ec_vnormal = gl_NormalMatrix*gl_Normal;
	ec_vposition = gl_ModelViewMatrix*gl_Vertex;
	gl_Position = gl_ProjectionMatrix*gl_ModelViewMatrix*gl_Vertex;
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_TexCoord[2] = gl_MultiTexCoord2;
	ec_tangent = gl_NormalMatrix * Tangent;
	ec_binormal = gl_NormalMatrix * Binormal;
}
