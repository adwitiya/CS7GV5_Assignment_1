
#/*********************************************************************/
//   source.cpp -- Implementation of OpenGL using C++				   *
//		 Author:  Adwitiya Chakraborty                                 *
//                                                                     *
//      Purpose: Evaluate OpenGL shading techniques using C++		   *
//                                                                     *
// GitHub Repo : https://github.com/adwitiya                 		   *
//		 Email : chakraad@tcd.ie									   *
//  Build Date : 27.01.2018											   *
#/*********************************************************************/
//Some Windows Headers (For Time, IO, etc.)
//#include <windows.h>
//#include <mmsystem.h>

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>

#include "maths_funcs.h" //Anton's math class
#include "mesh.h"
#include "model.h"
#include "shader_m.h"
#include "filesystem.h"

#include <string> 
#include <fstream>
#include <iostream>
#include <sstream>


//typedef double DWORD;


// Macro for indexing vertex buffer
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;
GLuint shaderProgramID;

unsigned int teapot_vao = 0;
int width = 800.0;
int height = 600.0;
GLuint loc1;
GLuint loc2;

Model *ourModel;
Model *ourPropeller;
Model *ourCity;

// degree of rotation
GLfloat rotatingdeg = 0.0f;
GLfloat propellerdeg = 0.0f;
// general scaling factor
float scaleFactor=1.0f;


// x and y origin of mouse movement
int xOrigin = -1, yOrigin = -1;
// distance from xOrigin and yOrigin
float deltaAngleX = 0.0f, deltaAngleY = 0.0f;
// angle of rotation for the camera direction
float angleX=0.0, angleY=0.0;
// components of camera position
float camx=0.0f,camy=0.0f,camz=0.0f;
// actual camera position
vec3 cameraPosition= vec3(camx,camy,camz);
// quaternion representing negated initial camera orientation
versor quaternion = quat_from_axis_deg(-0.0f, 0.0f, 1.0f, 0.0f);
// convert the quaternion to a rotation matrix
mat4 Rotate=rotate_y_deg( quat_to_mat4( quaternion ), 180);
// camera view matrix
mat4 cameraview = Rotate;
// vectors storing the x, y and z camera-relative axis
vec4 fwd( 0.0f, 0.0f, -1.0f, 0.0f );
vec4 rgt( 1.0f, 0.0f, 0.0f, 0.0f );
vec4 up( 0.0f, 1.0f, 0.0f, 0.0f );
bool TPV=true;
float shot=0.0f;
bool inputs[10];
int shadertype=0;

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS

std::string readShaderSource(const std::string& fileName){
	std::ifstream file(fileName.c_str()); 
	if(file.fail()) {
		cout << "error loading shader called " << fileName;
		exit (1); 
	} 
	
	std::stringstream stream;
	stream << file.rdbuf();
	file.close();

	return stream.str();
}


// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrComponents;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void generate_skyBox() {

	Shader skyboxShader("../shaders/skybox.vs", "../shaders/skybox.fs");
	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
	};

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// load textures
	// -------------
	vector<std::string> faces
	{
		FileSystem::getPath("skybox/midnight/right.jpg"),
		FileSystem::getPath("skybox/midnight/left.jpg"),
		FileSystem::getPath("skybox/midnight/top.jpg"),
		FileSystem::getPath("skybox/midnight/bottom.jpg"),
		FileSystem::getPath("skybox/midnight/front.jpg"),
		FileSystem::getPath("skybox/midnight/back.jpg"),
	};
	unsigned int cubemapTexture = loadCubemap(faces);

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// draw skybox as last
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	skyboxShader.use();
	glm::mat4 view;
	glm::mat4 projection;
	skyboxShader.setMat4("view", view);
	skyboxShader.setMat4("projection", projection);
	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);

}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) {
	// create a shader object
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
	std::string outShader = readShaderSource(pShaderText);
	const char* pShaderSource = outShader.c_str();

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
    glCompileShader(ShaderObj);
    GLint success;
	// check for shader related errors using glGetShaderiv
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
	// Attach the compiled shader object to the program object
    glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders() {
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
    shaderProgramID = glCreateProgram();
    if (shaderProgramID == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

	// Create two shader objects, one for the vertex, and one for the fragment shader
        AddShader(shaderProgramID, "../Shaders/simpleVertexShader.txt", GL_VERTEX_SHADER);
        AddShader(shaderProgramID, "../Shaders/simpleFragmentShader.txt", GL_FRAGMENT_SHADER);


    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

	// After compiling all shader objects and attaching them to the program, we can finally link it
    glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
    glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
    glUseProgram(shaderProgramID);

	return shaderProgramID;
	
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS


#pragma endregion VBO_FUNCTIONS


void setLighting(){
	//set up lighting
	// fixed point light properties
	vec3 light_position_world  = vec3 (0.0, 100.0, 0.0);
    vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
    vec3 Ld = vec3 (1.0, 1.0, 1.0); // dull white diffuse light colour
    vec3 La = vec3 (0.5, 0.5, 0.5); // grey ambient colour
	// surface reflectance
	vec3 Ks = vec3 (0.7, 0.7, 0.7); // fully reflect specular light
	vec3 Kd = vec3 (0.7, 0.7, 0.7); //  diffuse surface reflectance
	vec3 Ka = vec3 (0.7, 0.7, 0.7); // fully reflect ambient light

	int light_pos_loc = glGetUniformLocation (shaderProgramID, "light_position_world");
	int Ls_loc = glGetUniformLocation (shaderProgramID, "Ls");
	int Ld_loc = glGetUniformLocation (shaderProgramID, "Ld");
	int La_loc = glGetUniformLocation (shaderProgramID, "La");
	int Ks_loc = glGetUniformLocation (shaderProgramID, "Ks");
	int Kd_loc = glGetUniformLocation (shaderProgramID, "Kd");
	int Ka_loc = glGetUniformLocation (shaderProgramID, "Ka");

	glUniform3fv(light_pos_loc, 1, light_position_world.v);
	glUniform3fv(Ls_loc, 1, Ls.v);
	glUniform3fv(Ld_loc, 1, Ld.v);
	glUniform3fv(La_loc, 1, La.v);
	glUniform3fv(Ks_loc, 1, Ks.v);
	glUniform3fv(Kd_loc, 1, Kd.v);
	glUniform3fv(Ka_loc, 1, Ka.v);
}


void display(){

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor (0.5f, 0.5f, 0.5f, 1.0f);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram (shaderProgramID);

	//Declare your uniform variables that will be used in your shader
	int matrix_location = glGetUniformLocation (shaderProgramID, "model");
	int view_mat_location = glGetUniformLocation (shaderProgramID, "view");
	int proj_mat_location = glGetUniformLocation (shaderProgramID, "proj");


	mat4 view ;
	mat4 persp_proj ;
	mat4 Translate = translate( identity_mat4(), cameraPosition );

    if (TPV == true)
        view = look_at(cameraPosition + vec3(fwd) * -30.0 + vec3(up) * -1, cameraPosition, vec3(up));
    else
        view = look_at(cameraPosition + vec3(fwd) * -5.0 + vec3(up) * 1.0, cameraPosition, vec3(up));
    persp_proj = perspective(30.0, (float)width / (float)height, 0.1, 5000.0);

    mat4 cityPosition = scale(translate(identity_mat4(),vec3(0.0, -500, 0.0)) , vec3(0.5,0.5,0.5));
    glViewport(0, 0, width, height);
    glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
    glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
    glUniformMatrix4fv(matrix_location, 1, GL_FALSE, cityPosition.m);
	ourCity->Draw(shaderProgramID);


	mat4 cityPos_2 = translate(identity_mat4(), vec3(0.0, 0.0, 1000.0));
	mat4 globalcityPos = cityPos_2 * cityPosition;
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalcityPos.m);
	ourCity->Draw(shaderProgramID);

	mat4 cityPos_3 = translate(identity_mat4(), vec3(0.0, 0.0, 1000.0));
	mat4 globalcityPos_1 = cityPos_3 * globalcityPos;
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, globalcityPos_1.m);
	ourCity->Draw(shaderProgramID);


	mat4 local1 = Translate * Rotate;
   // Draw the plane
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, local1.m);
	ourModel->Draw(shaderProgramID);

	mat4 local_prop = identity_mat4();
	local_prop = translate(local_prop, vec3(0.0f, 1.15f, 0.0f));
	local_prop = scale(local_prop,vec3(100.0f,100.0f,100.0f));

	mat4 global_prop = local1 * local_prop;
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, persp_proj.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, view.m);
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, global_prop.m);
	ourPropeller->Draw(shaderProgramID);
	generate_skyBox();
    glutSwapBuffers();
}



void quaternionUpdate (int sign, vec3 dir){
    float angle = 0.5f;
    versor newQuaternion= quat_from_axis_deg (angle*sign, dir.v[0], dir.v[1], dir.v[2]);
    quaternion = newQuaternion * quaternion;

    // update axes to suit new orientation
    Rotate = rotate_y_deg(quat_to_mat4(quaternion), 180);
    fwd = Rotate * vec4( 0.0, 0.0, 1.0, 0.0 );
    rgt = Rotate * vec4( 1.0, 0.0, 0.0, 0.0 );
    up = Rotate * vec4( 0.0, 1.0, 0.0, 0.0 );
}

void inputHandler(){
    if (inputs[0]==true) {
        quaternionUpdate (+1, rgt);
    }
    if (inputs[1]==true) {
        quaternionUpdate (-1, rgt);
    }
    if (inputs[2]==true) {
        quaternionUpdate (-1, fwd);
    }
    if (inputs[3]==true) {
        quaternionUpdate (+1, fwd);
    }
    if (inputs[4]==true) {
        quaternionUpdate (+1, up);
    }
    if (inputs[5]==true) {
        quaternionUpdate (-1, up);
    }
    if (inputs[6]==true) {
        camz=1.0f;
        cameraPosition = cameraPosition + vec3( fwd ) * camz;
        print(cameraview);
        glutPostRedisplay();
    }
    if (inputs[7]==true) {
        camz=-1.0f;
        cameraPosition = cameraPosition + vec3( fwd ) * camz;
        print(cameraview);
        glutPostRedisplay();
    }
    if (inputs[8]==true && !shot) {
        shot=0.01;
        glutPostRedisplay();

    }
}



void updateScene() {
    if (rotatingdeg>0) rotatingdeg+=1.0f;
    if (rotatingdeg==360) rotatingdeg=0;
    if (shot > 0) shot+= 1.0f;
    if (shot > 30.0) shot= 0.0f;
    propellerdeg += 30.0f;
    inputHandler();
    // Draw the next frame
    glutPostRedisplay();
}

void init()
{
	// Set up the shaders
	shaderProgramID = CompileShaders();
	setLighting();

    // load models
    // N.B. does not work with relative path
    ourModel = new Model("../model/piper_pa18.obj");
    ourPropeller = new Model("../model/prop/Propeller from airboat.obj");
	// City Model
    ourCity = new Model("../model/The City.obj");

    for (int i=0; i<9; i++){
        inputs[i]=false;
    }
}

// Placeholder code for the keypress
void keypress(unsigned char key, int x, int y) {
    float angle;
	if (key == 'a' | key == 'A') {
        inputs[4]=true;
        glutPostRedisplay();
	}
	if (key == 'd' || key == 'D') {
        inputs[5]=true;
        glutPostRedisplay();
	}
    if (key == 'w' || key == 'W') {
        inputs[6]=true;
        glutPostRedisplay();
    }
    if (key == 's' || key == 'S') {
        inputs[7]=true;
        glutPostRedisplay();
    }
    if (key == 'f' || key == 'F') {;
        if (TPV==true) TPV=false;
        else TPV = true;
        glutPostRedisplay();

    }
    if (key == '-') {
        if (rotatingdeg==0) rotatingdeg=1.0f;
        glutPostRedisplay();

    }
    if (key == 'e' | key == 'E') {
        inputs[8]=true;
        glutPostRedisplay();
    }
    if (key == 'c') {
        shadertype+=1;
        if (shadertype ==3) shadertype=0;
        CompileShaders();
        setLighting();
        glutPostRedisplay();
    }


}


void keySpecial(int keyspecial, int x, int y) {

	switch (keyspecial)
	{
	case GLUT_KEY_UP:
		inputs[0] = true;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		inputs[1] = true;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		inputs[2] = true;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		inputs[3] = true;
		glutPostRedisplay();
		break;
	}
}


void keyRelease(unsigned char key, int x, int y) {
    if (key == 'a'|| key == 'A') {
        inputs[4]=false;
        glutPostRedisplay();
    }
    if (key == 'd' || key == 'D') {
        inputs[5]=false;
        glutPostRedisplay();
    }
    if (key == 'w' || key == 'W') {
        inputs[6]=false;
        glutPostRedisplay();
    }
    if (key == 's' || key == 'S') {
        inputs[7]=false;
        glutPostRedisplay();
    }
    if (key == 'e' || key == 'E') {
        inputs[8]=false;
        glutPostRedisplay();

    }
}

void keySpecialUp(int key, int x, int y) {
	switch (key)
	{
	case GLUT_KEY_UP:
		inputs[0] = false;
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		inputs[1] = false;
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		inputs[2] = false;
		glutPostRedisplay();
		break;
	case GLUT_KEY_LEFT:
		inputs[3] = false;
		glutPostRedisplay();
		break;
	}

}
void mouseButton(int button, int state, int x, int y) {

	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			angleX += deltaAngleX;
			angleY += deltaAngleY;
			xOrigin = -1;
			yOrigin = -1;
		}
		else  { // when the button is pressed
			xOrigin = x;
			yOrigin = y;
		}
	}
}



void mouseMove(int x, int y) {
	// this will only be true when the left button is down
	if (xOrigin >= 0) {

		deltaAngleX = (x - xOrigin) * 0.1f;
		versor xQuaternion= quat_from_axis_deg (deltaAngleX, up.v[0], up.v[1], up.v[2]);
		quaternion = xQuaternion * quaternion;
		// update axes to suit new orientation
		Rotate = rotate_y_deg(quat_to_mat4(quaternion), 180);
		fwd = Rotate * vec4( 0.0, 0.0, 1.0, 0.0 );
		rgt = Rotate * vec4( 1.0, 0.0, 0.0, 0.0 );
		up = Rotate * vec4( 0.0, 1.0, 0.0, 0.0 );


		deltaAngleY = (y - yOrigin) * 0.1f;
		versor zQuaternion= quat_from_axis_deg (deltaAngleY, rgt.v[0], rgt.v[1], rgt.v[2]);
		quaternion = zQuaternion * quaternion;

		// update axes to suit new orientation
		Rotate = rotate_y_deg(quat_to_mat4(quaternion), 180);
		fwd = Rotate * vec4( 0.0, 0.0, 1.0, 0.0 );
		rgt = Rotate * vec4( 1.0, 0.0, 0.0, 0.0 );
		up = Rotate * vec4( 0.0, 1.0, 0.0, 0.0 );


		xOrigin = x;
		yOrigin = y;

	}
}




int main(int argc, char** argv){

	// Set up the window
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(width, height);
    glutCreateWindow("CS7GV5_ASSIGNMENT_1");
	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutIdleFunc(updateScene);
    glutKeyboardFunc(keypress);
	glutSpecialFunc(keySpecial);
    glutKeyboardUpFunc(keyRelease);
	glutSpecialUpFunc(keySpecialUp);
	glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);

	 // A call to glewInit() must be done after glut is initialized!
	glewExperimental = GL_TRUE; //for non-lab machines, this line gives better modern GL support
    GLenum res = glewInit();
	// Check for any errors
    if (res != GLEW_OK) {
      fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
      return 1;
    }
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
    return 0;
}













