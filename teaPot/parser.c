#include <stdio.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define VPASSES 24
#define JITTER 0.1

typedef struct{
	char *map_KD;
	char *map_N;
	}material;
typedef struct{
	float x,y,z;
	}point;
typedef struct{
	float a, b;
	}texcoord;
typedef struct{
	int   fcount;
	int   (***faces);
	float (**normal);
	float (**vertex);
	float (**tcoord);
	float (**tangent);
	float (**bitangent);
	int iTan;
	int iBin;
	int p;
	}usemtl;

usemtl use;
material mat;

double genrand()
{
	return(((double)(random()+1))/2147483649);
}

point cross(point u, point v)
{
	point w;
	w.x = u.y*v.z - u.z*v.y;
	w.y = -(u.x*v.z - u.z*v.x);
	w.z = u.x*v.y - u.y*v.x;
	return(w);
}

point unit_length(point u)
{
	double length;
	point v;
	length = sqrt(u.x*u.x + u.y*u.y + u.z*u.z);
	v.x = u.x/length;
	v.y = u.y/length;
	v.z = u.z/length;
	return(v);
}

char * read_shader_program(char *filename)
{
	FILE *fp;
	char *content = NULL;
	int fd, count;
	fd = open(filename, O_RDONLY);
	count = lseek(fd, 0, SEEK_END);
	close(fd);
	content = (char *)calloc(1, (count + 1));
	fp = fopen(filename, "r");
	count = fread(content, sizeof(char), count, fp);
	content[count] = '\0';
	fclose(fp);
	return content;
}

unsigned int set_shaders()
{
	char *vs, *fs;
	GLuint v, f, p;
	
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	vs = read_shader_program("shader.vert");
	fs = read_shader_program("shader.frag");
	glShaderSource(v, 1, (const char **)&vs, NULL);
	glShaderSource(f, 1, (const char **)&fs, NULL);
	free(vs);
	free(fs);
	glCompileShader(v);
	glCompileShader(f);
	p = glCreateProgram();
	glAttachShader(p,f);
	glAttachShader(p,v);
	glLinkProgram(p);
	glUseProgram(p);
	return(p);
}

void load_objfile(char *filename) {
   int vcount, ncount, tcount, fcount, cvindex, cnindex, xcount, ycount, j;
   int ctindex, cyindex, cxindex;
   double x,y,z;
   FILE* mptr;
   FILE* fptr;
   char buf[512];
   char* parse;
   float mat_ambient[4];
   float mat_diffuse[4];
   float mat_shininess[4];
   float mat_specular[1];

   fptr = fopen(filename, "r");
   do {
      fgets(buf,512,fptr);
      } while (buf[0] != 'm');
   parse = strtok(buf, " ");
   parse = strtok(NULL, "\r\n");
   mptr = fopen(parse, "r");
   if(!mptr) fprintf(stderr, "No material library: %s\n\n", parse);

   while(fgets(buf,512,mptr)>0) {
      if(buf[0] != '#') {
         parse = strtok(buf, " ");
         if(strcmp(parse, "newmtl")==0) {
         continue;
         }
      }
   }
   rewind(mptr);

while(fgets(buf,512,mptr)>0) {
      if(buf[0] == '#') continue;
	 if(buf[0] == 'n') {
	    parse = strtok(buf, " ");
	    parse = strtok(NULL, "\r\n");
         }
         if(buf[0] == 'K') {
	    switch(buf[1]) {
	       case 'a': sscanf(buf, "Ka %lf %lf %lf", &x, &y, &z);
			 mat_ambient[0] = x;
			 mat_ambient[1] = y;
			 mat_ambient[2] = z;
			 mat_ambient[3] = 1.0;
			 break;
	       case 'd': sscanf(buf, "Kd %lf %lf %lf", &x, &y, &z);
			 mat_diffuse[0] = x;
			 mat_diffuse[1] = y;
			 mat_diffuse[2] = z;
			 mat_diffuse[3] = 1.0;
			 break;
	       case 's': sscanf(buf, "Ks %lf %lf %lf", &x, &y, &z);
			 mat_specular[0] = x;
	 		 mat_specular[1] = y;
	 		 mat_specular[2] = z;
	 		 mat_specular[3] = 1.0;
			 break;
            }
         }
         if(buf[0] == 'N') {
           sscanf(buf, "Ns %lf", &x);
	   mat_shininess[0] = x;
         }
         if(buf[0] == 'm') 
	 {
	    switch(buf[4]) 
	    {
	        case 'K': mat.map_KD = malloc(sizeof(char) * strlen(parse));
			  strcpy(mat.map_KD, parse);
			  mat.map_KD[strlen(mat.map_KD) - 1] = '\0';
			  break;
	        case 'n': parse = strtok(buf, " ");
			  parse = strtok(NULL, "\n");
			  mat.map_N = malloc(sizeof(char) * strlen(parse));
			  strcpy(mat.map_N, parse);
			  break;
           }
         }
   }

   fclose(mptr);

     
   glMaterialfv(GL_FRONT,GL_AMBIENT, mat_ambient);
   glMaterialfv(GL_FRONT,GL_DIFFUSE, mat_diffuse);
   glMaterialfv(GL_FRONT,GL_SPECULAR, mat_specular);
   glMaterialfv(GL_FRONT,GL_SHININESS, mat_shininess);

   vcount = ncount = tcount = fcount = xcount = ycount = 0;
   while(fgets(buf, 512, fptr) > 0) {
      if(buf[0] != '#') {
         if(buf[0] == 'v') {
            switch(buf[1]) {
               case ' ': vcount++;
                      break;
               case 'n': ncount++;
                      break;
               case 't': tcount++;
                      break;
               case 'x': xcount++;
		      break;
	       case 'y': ycount++;
		      break;
            }
         }
         if(buf[0]=='f') fcount++;
      }
   }
   int i;
   use.tangent = malloc((xcount + 1) * sizeof(float *));
   for(i = 0; i < xcount + 1; i++)
	use.tangent[i] = malloc(3 * sizeof(float));

   use.bitangent = malloc((ycount + 1) * sizeof(float *));
   for(i = 0; i < ycount + 1; i++)
	use.bitangent[i] = malloc(3 * sizeof(float));

   use.tcoord = malloc((tcount + 1) * sizeof(float *));
   for(i = 0; i < tcount + 1; i++)
	use.tcoord[i] = malloc(2 * sizeof(float));

   use.vertex = malloc((vcount + 1) * sizeof(float *));
   for(i = 0; i < vcount + 1; i++)
	use.vertex[i] = malloc(3 * sizeof(float));

   use.normal = malloc((ncount + 1) * sizeof(float *));
   for(i = 0; i < ncount + 1; i++)
	use.normal[i] = malloc(3 * sizeof(float));

   use.faces = malloc(fcount * sizeof(int **));
   for(i = 0; i < fcount; i++)
	use.faces[i] = malloc(4 * sizeof(int*));
   for(i = 0; i < fcount; i++)
	for(j = 0; j < 4; j++)
		use.faces[i][j] = malloc(3 *sizeof(int));
	
   int look[3];

   rewind(fptr);
   use.fcount = fcount;

   cvindex = cnindex = ctindex = cxindex = cyindex = 1;
   fcount = 0;
   while(fgets(buf,512,fptr)>0) {
      if(buf[0] != '#') {
         switch(buf[0]) {
            case 'v': switch(buf[1]) {
                      case ' ': sscanf(buf,"v %lf %lf %lf", &x, &y, &z);
				use.vertex[cvindex][0] = x;
				use.vertex[cvindex][1] = y;
				use.vertex[cvindex][2] = z;
				cvindex++;
                                break;
                      case 't': sscanf(buf,"vt %lf %lf", &x, &y);
				use.tcoord[ctindex][0] = x;
				use.tcoord[ctindex][1] = y;
				ctindex++;
                                break;
                      case 'n': sscanf(buf,"vn %lf %lf %lf", &x, &y, &z);
				use.normal[cnindex][0] = x;
				use.normal[cnindex][1] = y;
				use.normal[cnindex][2] = z;
				cnindex++;
                                break;
                      case 'x': sscanf(buf,"vn %lf %lf %lf", &x, &y, &z);
				use.tangent[cxindex][0] = x;
				use.tangent[cxindex][1] = y;
				use.tangent[cxindex][2] = z;
				cxindex++;
                                break;
                      case 'y': sscanf(buf,"vn %lf %lf %lf", &x, &y, &z);
				use.bitangent[cyindex][0] = x;
				use.bitangent[cyindex][1] = y;
				use.bitangent[cyindex][2] = z;
				cyindex++;
                                break;
		      }
                      break;
            case 'f': parse = strtok(buf, " ");
		      for(j = 0; j < 4; j++){
                         parse = strtok(NULL, " ");
                         sscanf(parse, "%d/%d/%d", &look[0], &look[1], &look[2]);
			 use.faces[fcount][j][0] = look[0];
			 use.faces[fcount][j][1] = look[1];
			 use.faces[fcount][j][2] = look[2];
		      }
		      fcount++;
                      break;
            case 'u': parse = strtok(buf," ");
                      parse = strtok(NULL, "\r\n");
                      break;
	}
      }
   }
   
   fclose(fptr);
}

void drawFloor() {
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0, 0);
	glVertex3f(-20.0f / 2, 0, 20.0f / 2);
	glTexCoord2f(0, 1);
	glVertex3f(-20.0f / 2, 0, -20.0f / 2);
	glTexCoord2f(1, 1);
	glVertex3f(20.0f / 2, 0, -20.0f / 2);
	glTexCoord2f(1, 0);
	glVertex3f(20.0f / 2, 0, 20.0f / 2);
	glEnd();
}

void load_texture(char *filename, int i)
{
	FILE *fptr;
	char buf[512], *parse;
	int im_size, im_width, im_height;
	unsigned char *texture_bytes;

	fptr = fopen(filename, "r");
	fgets(buf, 512, fptr);
	do{
		fgets(buf, 512, fptr);
	}while(buf[0] == '#');

	parse = strtok(buf, " \t");
	im_width = atoi(parse);
	parse = strtok(buf, " \n");
	im_height = atoi(parse);

	fgets(buf,512,fptr);
	parse = strtok(buf," \n");

	im_size = im_width * im_height;
	texture_bytes = (unsigned char*)calloc(3,im_size);
	fread(texture_bytes,3,im_size,fptr);
	fclose(fptr);

	glBindTexture(GL_TEXTURE_2D, i);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,im_width, im_height, 0, GL_RGB,
		GL_UNSIGNED_BYTE, texture_bytes);
	free(texture_bytes);
}

void draw_teapot(int scale){
	int i, j, v, t, n;	
	glBegin(GL_QUADS);
	for(i = 0; i < use.fcount; i++)
		for(j = 0; j < 4; j++){
			v = use.faces[i][j][0];
			t = use.faces[i][j][1];
			n = use.faces[i][j][2];
			glNormal3fv(use.normal[n]);
			glTexCoord2fv(use.tcoord[t]);
			glVertexAttrib3fv(use.iTan, use.tangent[v]);
			glVertexAttrib3fv(use.iBin, use.bitangent[v]);
			glVertex3f(scale * use.vertex[v][0], scale * use.vertex[v][1],
				    scale * use.vertex[v][2]);
		}
	glEnd();
}

void initRendering() {
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glClearColor(0.0,0.0,0.0,0.0);
	glClearAccum(0.0,0.0,0.0,0.0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void handleResize(int w, int h) {
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (float)w / (float)h, 1.0, 100.0);
}

void setup_viewvolume()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1024/768, 1.0, 100.0);
}

void do_lights(){
	GLfloat lightColor[] = {0.6f, 0.6f, 0.6f, 1.0f};
	GLfloat lightPos[] = {6.0f, 10.0f, 1.0f, -5.0f};
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
}

void drawWorld() {
       GLUquadricObj *quadObj = gluNewQuadric();
       gluQuadricTexture(quadObj,GL_TRUE);
       gluQuadricOrientation(quadObj,GLU_INSIDE);
       glActiveTexture(GL_TEXTURE2);
       glBindTexture(GL_TEXTURE_2D, 3);
       glEnable(GL_TEXTURE_2D);
       gluSphere(quadObj,40.0,64,64);
       glDisable(GL_TEXTURE_2D);
}

void drawScene() {
	int loc1;
	loc1 = glGetUniformLocation(use.p, "Floor");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glTranslatef(0.0f, -4.0f, -40.0f);
	glRotatef(30, 1, 0, 0);
	
	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	glTranslatef(0, 4.0f, 0);
	glUniform1i(loc1, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 2);
	draw_teapot(8.0);
	glPopMatrix();
	
	glEnable(GL_STENCIL_TEST);
	glColorMask(0, 0, 0, 0); 
	glDisable(GL_DEPTH_TEST); 
	glStencilFunc(GL_ALWAYS, 1, 1); 
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glUniform1i(loc1, 1);
	drawFloor();
	
	glColorMask(1, 1, 1, 1);
	glEnable(GL_DEPTH_TEST); 
	glStencilFunc(GL_EQUAL, 1, 1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); 
	
	glPushMatrix();
	glScalef(1, -1, 1);
	glRotatef(90, 0, 1, 0);
	glTranslatef(0, 4.0f, 0);
	glUniform1i(loc1, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 2);
	draw_teapot(8.0);
	glPopMatrix();
	
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glColor4f(0.8,0.8,0.8,0.3);
	glUniform1i(loc1, 1);
	drawFloor();
	glDisable(GL_BLEND);
	glUniform1i(loc1, 2);
	drawWorld();
	glutSwapBuffers();
}

void set_uniform(int p){
	GLint loc1, loc2, loc3;
	loc1 = glGetUniformLocation(p, "mytexture");
	loc2 = glGetUniformLocation(p, "mynormalmap");
	loc3 = glGetUniformLocation(p, "backdrop");
 	use.iTan = glGetAttribLocation(p, "Tangent");
 	use.iBin = glGetAttribLocation(p, "Binormal");
	glUseProgram(p);
	glUniform1i(loc1, 0);
	glUniform1i(loc2, 1);
	glUniform1i(loc3, 2);
}

void jitter_view()
{
	point eye, view, up, vdir, utemp, vtemp;
	eye.x = 0.0; eye.y = 0.0; eye.z = 0.0;
	view.x = JITTER*genrand();	
	view.y = JITTER*genrand();	
	view.z = JITTER*genrand();
	up.x = 0.0; up.y = 1.0; up.z = 0.0;
	vdir.x = view.x - eye.x;	
	vdir.y = view.y - eye.y;	
	vdir.z = view.z - eye.z;
	vtemp = cross(vdir,up);
	utemp = cross(vtemp,vdir);
	up = unit_length(utemp);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye.x, eye.y, eye.z, view.x, view.y, view.z, up.x, up.y, up.z);
}

void go()
{
	int view_pass;
	glClear(GL_ACCUM_BUFFER_BIT);
	for(view_pass = 0; view_pass < VPASSES; view_pass++)
	{
		jitter_view();
		drawScene();
		glFlush();
		glAccum(GL_ACCUM,1.0/(float)(VPASSES));
	}
	glAccum(GL_RETURN,1.0);
}

int main(int argc, char **argv){
	srandom(123456789);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE| GLUT_RGB| GLUT_DEPTH| GLUT_STENCIL| GLUT_ACCUM);
	glutInitWindowSize(1024, 768);
	glutCreateWindow("teapot");
	load_objfile(argv[1]);
	load_texture(mat.map_KD, 1);
	load_texture(mat.map_N,  2);
	load_texture(argv[2],    3);
	initRendering();
	do_lights();
	jitter_view();
	use.p = set_shaders();
	set_uniform(use.p);
	glutDisplayFunc(go);
	glutReshapeFunc(handleResize);
	glutMainLoop();
	return 0;
}
