#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"

#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;


// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

glm::vec3 pLightPos;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

GLuint pLightPosLoc;

//shadow mapping - directional light
GLuint shadowMapFBO;
GLuint depthMapTexture;

//shadow mapping - point light
unsigned int depthCubemap;
GLuint depthMapFBO;

const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

// camera
gps::Camera myCamera(
	glm::vec3(-3.74433f, 1.60775f, 1.44585f),
	glm::vec3(-0.943888f, 1.60775f, 1.7225f),
	glm::vec3(0.0f, 1.0f, 0.0f));


GLfloat cameraSpeed = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D sky;
gps::Model3D ground;
gps::Model3D bench;
gps::Model3D lamps;
gps::Model3D bodyCrow;
gps::Model3D wingL;
gps::Model3D wingR;
gps::Model3D raindrop;
GLfloat angle;

//animaton variables
float bodyCrowY = 0.092817f;
float bodyCrowZ = 0.655062f;

float wingLY = 0.083589f;
float wingLZ = 0.647693f;

float wingRY = 0.083822f;
float wingRZ = 0.645177f;

bool wingUp;
float wingAngle = 0;


// shaders
gps::Shader myBasicShader;
gps::Shader depthMapShader;

int changeLight = 0; //true - directional; false - point
int fog = 0;


//wireframe view
bool wireframe = false;
//smooth surfaces
bool smooth = false;

//wind effect
bool wind = false;

//rain effect
bool rain = false;
std::vector<glm::vec3> raindropsInitialPos;
std::vector<glm::vec3> raindropsPos;
float raindropZ;

GLenum glCheckError_(const char* file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
		case GL_INVALID_ENUM:
			error = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error = "INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			error = "INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			error = "STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			error = "STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			error = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {
		if (action == GLFW_PRESS) {
			pressedKeys[key] = true;
		}
		else if (action == GLFW_RELEASE) {
			pressedKeys[key] = false;
		}
	}
}


float xpos0 = 0, ypos0 = 0;
float mouseSpeed = 0.02f;
GLboolean begin = true;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	//TODO
	if (begin) {
		xpos0 = xpos;
		ypos0 = ypos;
		begin = false;
	}
	else {
		double deltaX = xpos - xpos0;
		double deltaY = ypos - ypos0;
		myCamera.rotate(-1 * deltaY * mouseSpeed, deltaX * mouseSpeed);

		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix for teapot
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));

		xpos0 = xpos;
		ypos0 = ypos;
	}
}

void renderScene();

void sceneAnimation() {

	std::vector<glm::vec3> path;
	glm::vec3 targetPos = glm::vec3(8.6625f, 1.81263f, 2.37074f);

	path.push_back(glm::vec3(0.85717f, 4.0657f, -5.00509f));
	float z = -5.00509f;
	while (z < 9.81967f) {
		z += 0.1f;
		path.push_back(glm::vec3(0.85717f, 4.0657f, z));
	}

	path.push_back(glm::vec3(0.85717f, 4.0657f, 9.81967f));
	float x = 0.85717f;
	while (x < 15.7986f) {
		x += 0.1f;
		path.push_back(glm::vec3(x, 4.0657f, 9.81967f));
	}

	path.push_back(glm::vec3(15.7986f, 4.0657f, 9.81967f));
	z = 9.81967f;
	while (z > -3.59243f) {
		z -= 0.1f;
		path.push_back(glm::vec3(15.7986f, 4.0657f, z));
	}

	path.push_back(glm::vec3(15.7986f, 4.0657f, -3.59243f));
	x = 15.7986f;
	while (x > 0.85717f) {
		x -= 0.1f;
		path.push_back(glm::vec3(x, 4.0657f, -3.59243f));
	}


	while (path.size())	{
		myCamera = gps::Camera(path.back(),
			targetPos,
			glm::vec3(0.0f, 1.0f, 0.0f));

		path.pop_back();

		renderScene();
		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

	}
}

void initRain() {
	for (int i = 0; i < 3000; i++) {
		float initialX = (rand() % 14476 + 1874) / 1000.0f;
		float initialY = ((rand() % 18304) - 11712) / 1000.0f;
		float initialZ = (rand() % 8081) / 1000.0f;
		raindropsInitialPos.push_back(glm::vec3(initialX, initialZ, -initialY));
		raindropsPos.push_back(glm::vec3(initialX, initialZ, -initialY));
	}
}

void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
		//update view matrix
		view = myCamera.getViewMatrix();
		myBasicShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		// compute normal matrix 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_T]) {
		sceneAnimation();
	}

	if (pressedKeys[GLFW_KEY_M]) {
		wireframe = !wireframe;
		if (wireframe) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}

	if (pressedKeys[GLFW_KEY_N]) {
		smooth = !smooth;
		if (smooth) {
			glfwWindowHint(GLFW_SAMPLES, 4);
			glEnable(GL_MULTISAMPLE);
		}
		else {
			glDisable(GL_MULTISAMPLE);
		}
	}

	if (pressedKeys[GLFW_KEY_L]) {
		if (changeLight == 1) {
			changeLight = 0;
		}
		else {
			changeLight = 1;
		}
		myBasicShader.useShaderProgram();
		glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "changeLight"), changeLight);
	}

	if (pressedKeys[GLFW_KEY_F]) {
		if (fog == 1) {
			fog = 0;
		}
		else {
			fog = 1;
		}
		myBasicShader.useShaderProgram();
		glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "fog"), fog);
	}

	if (pressedKeys[GLFW_KEY_C]) {
		bodyCrowY += 0.01f;
		bodyCrowZ += 0.01f;
		wingLY += 0.01f;
		wingLZ += 0.01f;
		wingRY += 0.01f;
		wingRZ += 0.01f;

		if (wingUp) {
			wingAngle += 0.1f;
		}
		else {
			wingAngle -= 0.1f;
		}

		if (wingAngle >= 0.8) {
			wingUp = !wingUp;
		}
		
		if (wingAngle <= -0.8) {
			wingUp = !wingUp;
		}
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		rain = !rain;

		if (rain) {
			initRain();
		}
	}

	if (pressedKeys[GLFW_KEY_X]) {
		wind = !wind;
	}
}


void initOpenGLWindow() {
	myWindow.Create(1024, 768, "OpenGL Project Core");
}

void setWindowCallbacks() {
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
	glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
	glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
	glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
	glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
	sky.LoadModel("models/sky/sky.obj");
	ground.LoadModel("models/gate+ground/ground.obj");
	lamps.LoadModel("models/street-lamp/lamp.obj");
	bench.LoadModel("models/bench/bench.obj");
	bodyCrow.LoadModel("models/bodyCrow/body.obj");
	wingL.LoadModel("models/wingL/wingL.obj");
	wingR.LoadModel("models/wingR/wingR.obj");
	raindrop.LoadModel("models/raindrop/raindrop.obj");
}

void initShaders() {
	myBasicShader.loadShader(
		"shaders/basic.vert",
		"shaders/basic.frag");
	depthMapShader.loadShader(
		"shaders/shadow.vert",
		"shaders/shadow.frag");

}

void initUniforms() {
	myBasicShader.useShaderProgram();

	// create model matrix 
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
	modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

	// get view matrix for current camera
	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
	// send view matrix to shader
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// compute normal matrix 
	normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

	// create projection matrix
	projection = glm::perspective(glm::radians(45.0f),
		(float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
		0.1f, 500.0f);
	projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
	// send projection matrix to shader
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 7.0f, 1.0f);
	lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
	// send light dir to shader
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
	// send light color to shader
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}

void initFBO() {
	glGenFramebuffers(1, &shadowMapFBO);

	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
		0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initCubeMap() {
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
	for (unsigned int i = 0; i < 6; ++i)
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
			SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderGround(gps::Shader shader, bool depthPass) {
	// select active shader program
	shader.useShaderProgram();

	//send teapot model matrix data to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	if (depthPass == false) {
		//send teapot normal matrix data to shader
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	ground.Draw(shader);
}

void renderSky(gps::Shader shader) {
	// select active shader program
	shader.useShaderProgram();

	//send teapot model matrix data to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	//send teapot normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	sky.Draw(shader);
}

void renderBench(gps::Shader shader, bool depthPass) {
	// select active shader program
	shader.useShaderProgram();

	//position
	glm::mat4 modelBench = glm::mat4(1.0f);
	modelBench = glm::translate(modelBench, glm::vec3(4.31311f, -0.000201f, 1.25905f));

	//send teapot model matrix data to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelBench));

	if (!depthPass) {
		//send teapot normal matrix data to shader
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	bench.Draw(shader);
}

void renderLamp(gps::Shader shader, bool depthPass) {
	// select active shader program
	shader.useShaderProgram();

	//position
	glm::mat4 modelLamp = glm::mat4(1.0f);
	modelLamp = glm::translate(modelLamp, glm::vec3(3.7833f, -0.019674f, 3.02676f));

	//send teapot model matrix data to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelLamp));

	if (!depthPass) {
		//send teapot normal matrix data to shader
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	lamps.Draw(shader);
}

void renderBodyCrow(gps::Shader shader, bool depthPass) {
	// select active shader program
	shader.useShaderProgram();

	//position
	glm::mat4 modelBodyCrow = glm::mat4(1.0f);
	modelBodyCrow = glm::translate(modelBodyCrow, glm::vec3(5.9248f, bodyCrowY, bodyCrowZ));



	//send teapot model matrix data to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelBodyCrow));

	if (!depthPass) {
		//send teapot normal matrix data to shader
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	bodyCrow.Draw(shader);
}

void renderWingL(gps::Shader shader, bool depthPass) {
	// select active shader program
	shader.useShaderProgram();

	//position
	glm::mat4 modelWingL = glm::mat4(1.0f);

	modelWingL = glm::translate(modelWingL, glm::vec3(5.94813f, wingLY, wingLZ));
	modelWingL = glm::rotate(modelWingL, wingAngle, glm::vec3(0.0f, 0.0f, 1.0f));

	//modelWingL = glm::translate(modelWingL, glm::vec3(-5.94813f, -wingLY, -wingLZ));
	//modelWingL = glm::translate(modelWingL, glm::vec3(5.94813f, wingLY, wingLZ));

	//send teapot model matrix data to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWingL));

	if (!depthPass) {
		//send teapot normal matrix data to shader
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	wingL.Draw(shader);
}

void renderWingR(gps::Shader shader, bool depthPass) {
	// select active shader program
	shader.useShaderProgram();

	//position
	glm::mat4 modelWingR = glm::mat4(1.0f);

	modelWingR = glm::translate(modelWingR, glm::vec3(5.89672f, wingRY, wingRZ));
	modelWingR = glm::rotate(modelWingR, -wingAngle, glm::vec3(0.0f, 0.0f, 1.0f));

	//send teapot model matrix data to shader
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelWingR));

	if (!depthPass) {
		//send teapot normal matrix data to shader
		glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}

	wingR.Draw(shader);
}

bool checkCollision(glm::vec3 raindropPos) {
	bool collisionRoofR = false;
	bool collisionRoofL = false;
	bool collisionWallR = false;
	bool collisionWallL = false;

	glm::vec3 A = glm::vec3(8.45154f, 3.58871f, 2.47549f);  //top-front vertex of roof
	glm::vec3 B = glm::vec3(12.8966f, 3.62955f, 2.55956f);  //top-back vertex of roof
	glm::vec3 C = glm::vec3(8.40777f, 1.63252f, 3.92667f);  //top-front vertex of right wall
	glm::vec3 D = glm::vec3(12.8452f, 1.63252f, 4.00412f);  //top-back vertex of right wall
	glm::vec3 E = glm::vec3(8.47413f, 1.63252f, 1.04179f);  //top-front vertex of left wall
	glm::vec3 F = glm::vec3(12.9299f, 1.59839f, 1.10053f);  //top-back vertex of left wall
	glm::vec3 G = glm::vec3(8.45149f, 0.007231f, 3.904f);   //bottom-front vertex of right wall
	glm::vec3 H = glm::vec3(12.8534f, 0.007231f, 3.99226f); //bottom-back vertex of right wall
	glm::vec3 I = glm::vec3(8.46879f, 0.007231f, 1.07433f); //bottom-front vertex of left-wall
	glm::vec3 J = glm::vec3(12.9299f, 0.007231f, 1.11824f); //bottom-back vertex of left-wall

	
	glm::vec3 normalRoofR = glm::cross(A - C, D - C);

	if (raindropPos.x > 8.40777f and raindropPos.x < 12.8966f and
		raindropPos.y > 1.63252f and raindropPos.y < 3.62955f and
		raindropPos.z > 2.47549f and raindropPos.z < 4.00412f and
		glm::dot(normalRoofR, (raindropPos - A)) < 0) {
		collisionRoofR = true;
	}

	glm::vec3 normalRoofL = glm::cross(A - E, F - E);

	if (raindropPos.x > 8.45154f and raindropPos.x < 12.9299f and
		raindropPos.y > 1.59839f and raindropPos.y < 3.62955f and
		raindropPos.z > 1.04179f and raindropPos.z < 2.55956f and
		glm::dot(normalRoofL, (raindropPos - A)) < 0) {
		collisionRoofL = true;
	}

	glm::vec3 normalWallR = glm::cross(C - G, H - G);

	if (raindropPos.x > 8.40777f and raindropPos.x < 12.8534f and
		raindropPos.y > 0.007231f and raindropPos.y < 1.63252f and
		raindropPos.z > 3.904f and raindropPos.z < 4.00412f and
		glm::dot(normalWallR, (raindropPos - C)) < 0) {
		collisionWallR = true;
	}

	glm::vec3 normalWallL = glm::cross(E - I, J - I);

	if (raindropPos.x > 8.46879f and raindropPos.x < 12.9299f and
		raindropPos.y > 0.007231f and raindropPos.y < 1.63252f and
		raindropPos.z > 1.04179f and raindropPos.z < 1.11824f and
		glm::dot(normalWallL, (raindropPos - E)) < 0) {
		collisionWallL = true;
	}

	return (collisionRoofR or collisionRoofL or collisionWallR or collisionRoofL);
}

void renderRain(gps::Shader shader, bool depthPass) {
	for (int i = 0; i < 3000; i++) {
		// select active shader program
		shader.useShaderProgram();

		//position
		glm::mat4 modelRaindrop = glm::mat4(1.0f);
		glm::vec3 raindropPos = raindropsPos.at(i);

		modelRaindrop = glm::translate(modelRaindrop, raindropPos);

		raindropsPos.at(i).y -= 0.05;

		if (wind) {
			modelRaindrop = glm::rotate(modelRaindrop, 0.1f, glm::vec3(1.0f, 0.0f, 0.0f));
			raindropsPos.at(i).z -= 0.02;
		}

		if (raindropsPos.at(i).y < 0.0f or checkCollision(raindropsPos.at(i)) == true) {
			raindropsPos.at(i) = raindropsInitialPos.at(i);
			raindropsPos.at(i).y = 8.081f;
		}

		//send teapot model matrix data to shader
		glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelRaindrop));

		if (!depthPass) {
			//send teapot normal matrix data to shader
			glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
		}

		raindrop.Draw(shader);
	}

	
}


glm::mat4 computeLightSpaceTrMatrix() {
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(12.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 5.0f, far_plane = 20.0f;
	glm::mat4 lightProjection = glm::ortho(-7.0f, 7.0f, -7.0f, 7.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
	
	return lightSpaceTrMatrix;
}

void renderScene() {

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	//render the ground
	renderGround(depthMapShader, true);
	//render the lamp
	renderLamp(depthMapShader, true);
	//render the bench
	renderBench(depthMapShader, true);
	//render the body of the crow
	renderBodyCrow(depthMapShader, true);
	//render the wings
	renderWingL(depthMapShader, true);
	renderWingR(depthMapShader, true);
	//render rain
	if (rain) {
		renderRain(depthMapShader, true);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	

	//render with shadow mapping

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	myBasicShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);

	glViewport(0, 0, (float)myWindow.getWindowDimensions().width, (float)myWindow.getWindowDimensions().height);
	myBasicShader.useShaderProgram();

	//render the scene
	//render the ground
	renderGround(myBasicShader, false);
	//render the sky
	renderSky(myBasicShader);
	//render the lamp
	renderLamp(myBasicShader, false);
	//render the bench
	renderBench(myBasicShader, false);
	//render the body of the crow
	renderBodyCrow(myBasicShader, false);
	//render the wings
	renderWingL(myBasicShader, false);
	renderWingR(myBasicShader, false);
	//render rain
	if (rain) {
		renderRain(myBasicShader, false);
	}

	pLightPos = glm::vec3(3.77206f, 0.789307f, 2.86863f);
	pLightPosLoc = glGetUniformLocation(myBasicShader.shaderProgram, "pLightPosition");
	glUniform3fv(pLightPosLoc, 1, glm::value_ptr(pLightPos));

}

void cleanup() {
	glDeleteTextures(1, &depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	myWindow.Delete();
	//cleanup code for your own data
}

int main(int argc, const char* argv[]) {

	try {
		initOpenGLWindow();
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	
	initOpenGLState();
	initFBO();
	initModels();
	initShaders();
	initUniforms();
	setWindowCallbacks();

	glCheckError();
	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
		processMovement();
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

	return EXIT_SUCCESS;
}
