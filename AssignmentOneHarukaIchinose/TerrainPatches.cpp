//  ========================================================================
//  COSC363: Computer Graphics (2021);  University of Canterbury.
//
//  FILE NAME: TerrainPatches.cpp
//  See Lab-11.pdf for details.
//
//	The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  Requires files  TerrainPatches.vert, TerrainPatches.frag
//                  TerrainPatches.cont, TerrainPatches.eval
//  ========================================================================
#define  GLM_FORCE_RADIANS
#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h"
using namespace std;

//Globals
GLuint vaoID;
GLuint mvpMatrixLoc, eyePosLoc, mvMatrixLoc, norMatrixLoc, lgtLoc, waterLevelLoc, snowLevelLoc;
float CDR = 3.14159265/180.0;     //Conversion from degrees to rad (required in GLM 0.9.6)
float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];       //Element array for 81 quad patches
glm::mat4 projView;
glm::vec3 eye;
float angle = 0, look_x = 0, look_z = -70, ex = 0, ez = 30;
float waterLevel = 1, snowLevel = 8;
bool lineMode = false;


//Generate vertex and element data for the terrain floor
void generateData()
{
	int indx, start;
	//verts array
	for(int i = 0; i < 10; i++)   //100 vertices on a 10x10 grid
	{
		for(int j = 0; j < 10; j++)
		{
			indx = 10*i + j;
			verts[3*indx] = 10*i - 45;		//x
			verts[3*indx+1] = 0;			//y
			verts[3*indx+2] = -10*j;		//z
		}
	}

	//elems array
	for(int i = 0; i < 9; i++)
	{
		for(int j = 0; j < 9; j++)
		{
			indx = 9*i +j;
			start = 10*i + j;
			elems[4*indx] = start;
			elems[4*indx+1] = start+10;
			elems[4*indx+2] = start+11;
			elems[4*indx+3] = start+1;			
		}
	}
}

//Loads terrain texture
void loadTextures(const char* terrainFileName)
{
	// Terrain height map
	GLuint texIDs[5];
    glGenTextures(5, texIDs);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texIDs[0]);
	loadTGA(terrainFileName);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Water texture
   
    glActiveTexture(GL_TEXTURE1);  //Texture unit 0
    glBindTexture(GL_TEXTURE_2D, texIDs[1]);
	loadTGA("water.tga");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Grass texture
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texIDs[2]);
	loadTGA("grass.tga");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Rock texture
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texIDs[3]);
	loadTGA("rock.tga");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Snow texture
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texIDs[4]);
	loadTGA("snow.tga");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}


//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
	ifstream shaderFile(filename.c_str());
	if(!shaderFile.good()) cout << "Error opening shader file." << endl;
	stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	string shaderStr = shaderData.str();
	const char* shaderTxt = shaderStr.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderTxt, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		const char *strShaderType = NULL;
		cerr <<  "Compile failure in shader: " << strInfoLog << endl;
		delete[] strInfoLog;
	}
	return shader;
}

//Initialise the shader program, create and load buffer data
void initialise()
{
	glm::mat4 proj, view;
	//--------Load terrain height map-----------
	loadTextures("Terrain_hm_02.tga");

	//--------Load shaders----------------------
	GLuint shaderv = loadShader(GL_VERTEX_SHADER, "TerrainPatches.vert");
	GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "TerrainPatches.frag");
	GLuint shaderc = loadShader(GL_TESS_CONTROL_SHADER, "TerrainPatches.cont");
	GLuint shadere = loadShader(GL_TESS_EVALUATION_SHADER, "TerrainPatches.eval");
	GLuint shaderg = loadShader(GL_GEOMETRY_SHADER, "TerrainPatches.geom");

	//--------Attach shaders---------------------
	GLuint program = glCreateProgram();
	glAttachShader(program, shaderv);
	glAttachShader(program, shaderf);
	glAttachShader(program, shaderc);
	glAttachShader(program, shadere);
	glAttachShader(program, shaderg);

	glLinkProgram(program);

	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}

	glUseProgram(program);
	eye = glm::vec3(ex, 20.0, ez);

	mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
	eyePosLoc = glGetUniformLocation(program, "eyePos");
	waterLevelLoc = glGetUniformLocation(program, "waterLevel");
	snowLevelLoc = glGetUniformLocation(program, "snowLevel");
	norMatrixLoc = glGetUniformLocation(program, "norMatrix");
	lgtLoc = glGetUniformLocation(program, "lightPos");
	GLuint texLoc = glGetUniformLocation(program, "heightMap");
	glUniform1i(texLoc, 0);
	texLoc = glGetUniformLocation(program, "water");
	glUniform1i(texLoc, 1);
	texLoc = glGetUniformLocation(program, "grass");
	glUniform1i(texLoc, 2);
	texLoc = glGetUniformLocation(program, "rock");
	glUniform1i(texLoc, 3);
	texLoc = glGetUniformLocation(program, "snow");
	glUniform1i(texLoc, 4);

//--------Compute matrices----------------------
	proj = glm::perspective(30.0f*CDR, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
	view = glm::lookAt(glm::vec3(0.0, 20.0, 30.0), glm::vec3(0.0, 0.0, -70.0), glm::vec3(0.0, 1.0, 0.0));
	glm::vec4 light = glm::vec4(20.0, 10.0, 30.0, 1.0);
	glm::vec4 lightEye = view * light;
	glUniform4fv(lgtLoc, 1, &lightEye[0]);

//---------Load buffer data-----------------------
	generateData();

	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

void special(int key, int x, int y)
{
	if (key == GLUT_KEY_UP) {
		ex += sin(angle);
		ez -= cos(angle);
	}
	else if (key == GLUT_KEY_DOWN) {
		ex -= sin(angle);
		ez += cos(angle);
	}
	else if (key == GLUT_KEY_LEFT) angle -= 0.1;  //Change direction
	else if (key == GLUT_KEY_RIGHT) angle += 0.1;

	look_x = ex + 100 * sin(angle);
	look_z = ez - 100 * cos(angle);

	glutPostRedisplay();
}

void NormalKeyHandler(unsigned char key, int x, int y)
{
	if (key == '1') {
		loadTextures("Terrain_hm_02.tga");	
	}
	else if (key == '2') {
		loadTextures("MtCook.tga");
	}
	else if (key == 'q') {
		waterLevel += 0.1;
	}
	else if (key == 'a') {
		waterLevel -= 0.1;
	}
	else if (key == 'w') {
		snowLevel += 0.1;
	}
	else if (key == 's') {
		snowLevel -= 0.1;
	}
	else if (!lineMode && key == ' ') {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		lineMode = true;
	}
	else if (lineMode && key == ' ') {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		lineMode = false;
	}
	glutPostRedisplay();
}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
	glm::mat4 proj, view;
	glm::vec3 eyePos = glm::vec3(ex, 20, ez);
	proj = glm::perspective(30.0f * CDR, 1.25f, 20.0f, 500.0f);  
	view = glm::lookAt(eyePos, glm::vec3(look_x, 0.0, look_z), glm::vec3(0.0, 1.0, 0.0));
	projView = proj * view;  //Product matrix

	//glm::vec4 lightEye = view * light;     //Light position in eye coordinates
	
	glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &projView[0][0]);
	glUniform3fv(eyePosLoc, 1, &eyePos[0]);
	//glUniform4fv(lgtLoc, 1, &lightEye[0]);
	glUniform1f(waterLevelLoc, waterLevel);
	glUniform1f(snowLevelLoc, snowLevel);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vaoID);
	glDrawElements(GL_PATCHES, 81*4, GL_UNSIGNED_SHORT, NULL);
	glFlush();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Terrain");
	glutInitContextVersion (4, 2);
	glutInitContextProfile ( GLUT_CORE_PROFILE );

	if(glewInit() == GLEW_OK)
	{
		cout << "GLEW initialization successful! " << endl;
		cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
	}
	else
	{
		cerr << "Unable to initialize GLEW  ...exiting." << endl;
		exit(EXIT_FAILURE);
	}

	initialise();
	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutKeyboardFunc(NormalKeyHandler);
	glutMainLoop();
}

