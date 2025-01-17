//
//  main.cpp
//  OpenGL Advances Lighting
//
//  Created by CGIS on 28/11/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#if defined (__APPLE__)
    #define GLFW_INCLUDE_GLCOREARB
    #define GL_SILENCE_DEPRECATION
#else
    #define GLEW_STATIC
    #include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/matrix_inverse.hpp>
#include <gtc/type_ptr.hpp>

#include "Shader.hpp"
#include "Model3D.hpp"
#include "Camera.hpp" 
#include "SkyBox.hpp"

#include <iostream>

int glWindowWidth = 800;
int glWindowHeight = 600;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL; 
float deltaTime = 0.0f;
float lastFrame = 0.0f;
///glm::vec3 fogColor = glm::vec3(0.5f, 0.5f, 0.5f);
///float fogStart = 50.0f;  
//float fogEnd = 1000.0f;    

const unsigned int SHADOW_WIDTH = 8192;
const unsigned int SHADOW_HEIGHT = 8192;
bool enableFog = false;
bool enableSnow = false;
std::vector<glm::vec3> snowdrops;
GLuint snowVAO, snowVBO;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat4 lightRotation;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

gps::Camera myCamera(
	glm::vec3(0.0f, 0.0f, 3.0f),
	glm::vec3(0.0f, 0.0f, -10.0f),
	glm::vec3(0.0f, 1.0f, 0.0f));
float cameraSpeed = 0.1f;

bool pressedKeys[1024];
float angleY = 0.0f;
GLfloat lightAngle;

// models
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D teapot; 
///Planes
gps::Model3D grassField;
gps::Model3D water;
gps::Model3D woodenCrate;
//animals
gps::Model3D cows;
gps::Model3D horses;
//plants
gps::Model3D wheatField;
gps::Model3D tree;

//buildings
gps::Model3D medievalHouse;
gps::Model3D hayCart;
gps::Model3D scarecrow;
gps::Model3D cartWheels;
gps::Model3D wheel1;
gps::Model3D wheel2;
gps::Model3D houses2;
gps::Model3D house3;
gps::Model3D towers;
gps::Model3D blacksmith;
gps::Model3D fences;
gps::Model3D walls;
gps::Model3D windmill;
gps::Model3D windmillWings;
gps::Model3D gate;
gps::Model3D gateDoor;
gps::Model3D catapult;
gps::Model3D boat;
gps::Model3D fairyTower;

gps::Model3D nanosuit;
gps::Model3D ground;  

gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader screenQuadShader;
gps::Shader depthMapShader;
gps::Shader snowShader;


gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

bool showDepthMap;

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);

	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
		showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

}

void processMovement()
{
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed); 
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed); 
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed); 
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed); 
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
	 
	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 1.0f;
	}
	 
	if (pressedKeys[GLFW_KEY_LEFT]) {
		myCamera.rotate(0.0f, 1.0f);   
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_RIGHT]) {
		myCamera.rotate(0.0f, -1.0f);   
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
	// Handle camera rotation
	if (pressedKeys[GLFW_KEY_UP]) {
		myCamera.rotate(1.0f, 0.0f);  
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_DOWN]) {
		myCamera.rotate(-1.0f, 0.0f);  
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}
	if (pressedKeys[GLFW_KEY_SPACE]) {
		myCamera.move(gps::MOVE_UP, cameraSpeed); 
		view = myCamera.getViewMatrix();
		myCustomShader.useShaderProgram();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view)); 
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (pressedKeys[GLFW_KEY_X]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	if (pressedKeys[GLFW_KEY_V]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); 
	}
	 
	static bool keyPressed = false;
	if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_PRESS) {
		if (!keyPressed) {
			enableSnow = !enableSnow; // Toggle snow
			keyPressed = true;
		}
	}
	if (glfwGetKey(glWindow, GLFW_KEY_R) == GLFW_RELEASE) {
		keyPressed = false;
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    
    //window scaling for HiDPI displays
    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

    //for sRBG framebuffer
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    //for antialising
    glfwWindowHint(GLFW_SAMPLES, 4);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(glWindow);

	glfwSwapInterval(1);

#if not defined (__APPLE__)
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();
#endif

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise

	glEnable(GL_FRAMEBUFFER_SRGB);
}

void initObjects() { 
	tree.LoadModel("models/Tree/tree.obj");
	horses.LoadModel("models/animals/horse/horses.obj"); 
	grassField.LoadModel("models/grass_field/grass.obj");
	woodenCrate.LoadModel("models/Wooden_Crate/Wooden_Crate.obj");
	//animals
	cows.LoadModel("models/animals/cow/cows.obj");
	///buildings
	medievalHouse.LoadModel("models/buildings/medieval_house/Medieval_House.obj");
	hayCart.LoadModel("models/buildings/medieval_house/cart/Cart.obj");
	//cartWheels.LoadModel("models/buildings/medieval_house/cart/cartWheels.obj");
	scarecrow.LoadModel("models/scarecrow/scarecrow.obj");
	wheel1.LoadModel("models/buildings/medieval_house/cart/wheel1.obj");
	wheel2.LoadModel("models/buildings/medieval_house/cart/wheel2.obj");
	water.LoadModel("models/water/water.obj");
	houses2.LoadModel("models/buildings/house2/houses2.obj");
	house3.LoadModel("models/buildings/house3/house3.obj");
	towers.LoadModel("models/buildings/tower/towers.obj");
	blacksmith.LoadModel("models/buildings/black_smith/black_smith.obj");
	fences.LoadModel("models/buildings/fence/fences.obj");
	walls.LoadModel("models/buildings/wall/walls.obj");
	windmill.LoadModel("models/buildings/windmill/windmill.obj");
	windmillWings.LoadModel("models/buildings/windmill/windmillWings.obj");
	wheatField.LoadModel("models/plants/wheat/wheatField.obj");
	gate.LoadModel("models/buildings/gate/gate.obj");
	gateDoor.LoadModel("models/buildings/gate/gateDoor.obj");

	catapult.LoadModel("models/catapult/catapult.obj");
	boat.LoadModel("models/ship/boat.obj");
	fairyTower.LoadModel("models/buildings/castle/fairy_tower.obj");
	lightCube.LoadModel("models/cube.obj");
}

void initShaders() {
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	myCustomShader.useShaderProgram();
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	lightShader.useShaderProgram();
	screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
	screenQuadShader.useShaderProgram();
	depthMapShader.loadShader("shaders/depthShader.vert", "shaders/depthShader.frag");
	depthMapShader.useShaderProgram();
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag"); 
	skyboxShader.useShaderProgram();
	snowShader.loadShader("shaders/snowShader.vert", "shaders/snowShader.frag");
	snowShader.useShaderProgram();
}

void initSkybox() {
	std::vector<const GLchar*> faces;
	/*faces.push_back("skybox/right.tga");
	faces.push_back("skybox/left.tga");
	faces.push_back("skybox/top.tga");
	faces.push_back("skybox/bottom.tga");
	faces.push_back("skybox/back.tga");
	faces.push_back("skybox/front.tga");*/

	faces.push_back("skybox/sunset/sunset_rt.tga");
	faces.push_back("skybox/sunset/sunset_lf.tga");
	faces.push_back("skybox/sunset/sunset_up.tga");
	faces.push_back("skybox/sunset/sunset_dn.tga");
	faces.push_back("skybox/sunset/sunset_bk.tga");
	faces.push_back("skybox/sunset/sunset_ft.tga");


	mySkyBox.Load(faces);
}

void initSnow() {
	// random positions 
	for (int i = 0; i < 1000; i++) {
		snowdrops.push_back(glm::vec3(
			static_cast<float>(rand() % 100 - 50), 
			static_cast<float>(rand() % 100),    
			static_cast<float>(rand() % 100 - 50) 
		));
	}
	glGenVertexArrays(1, &snowVAO);
	glGenBuffers(1, &snowVBO);
	glBindVertexArray(snowVAO);
	glBindBuffer(GL_ARRAY_BUFFER, snowVBO);
	glBufferData(GL_ARRAY_BUFFER, snowdrops.size() * sizeof(glm::vec3), &snowdrops[0], GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void updateSnow(float deltaTime) {
	for (auto& drop : snowdrops) {
		drop.y -= 5.0f * deltaTime; // snow falling speed
		if (drop.y < 0.0f) {
			drop.y = static_cast<float>(rand() % 100); //get to top
		} 

	} 
	glBindBuffer(GL_ARRAY_BUFFER, snowVBO);
	glBufferData(GL_ARRAY_BUFFER, snowdrops.size() * sizeof(glm::vec3), &snowdrops[0], GL_DYNAMIC_DRAW);
}



void initUniforms() {
	myCustomShader.useShaderProgram();

	model = glm::mat4(1.0f);
	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	view = myCamera.getViewMatrix();
	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	
	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	//set the light direction (direction towards the light)
	lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");	
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

	lightShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glEnable(GL_PROGRAM_POINT_SIZE);
}

void initFBO() {
	//TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);
	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	//  Return the light-space transformation matrix
	glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const GLfloat near_plane = 0.1f, far_plane = 60.0f;
	glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
	glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;


	return lightSpaceTrMatrix;
}


glm::mat4 doorModel = glm::mat4(1.0f);
glm::mat4 cartWheel1Model = glm::mat4(1.0f);
glm::mat4 cartWheel2Model = glm::mat4(1.0f);
glm::mat4 cartModel = glm::mat4(1.0f);
glm::mat4 windmillWingsModel = glm::mat4(1.0f);
bool wingsRotating = false;
bool wheelsRotating = false;
bool cartMoving = false;
float distance = 0.0f;

void drawObjects(gps::Shader shader, bool depthPass) {
		
	shader.useShaderProgram();
	
	model = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// do not send the normal matrix if we are rendering in the depth map
	if (!depthPass) {
		normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	}  
	woodenCrate.Draw(shader);
	//animals
	cows.Draw(shader);
	horses.Draw(shader);
	//buildings
	medievalHouse.Draw(shader);
	//hayCart.Draw(shader);
	houses2.Draw(shader);
	towers.Draw(shader);
	blacksmith.Draw(shader);
	house3.Draw(shader);
	fences.Draw(shader);
	walls.Draw(shader);
	windmill.Draw(shader);
	//windmillWings.Draw(shader);
	wheatField.Draw(shader);
	gate.Draw(shader);
	catapult.Draw(shader);
	boat.Draw(shader);
	fairyTower.Draw(shader);
	grassField.Draw(shader);	
	scarecrow.Draw(shader);

	tree.Draw(shader);
	water.Draw(shader);

	GLint cartModelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	glUniformMatrix4fv(cartModelLoc, 1, GL_FALSE, glm::value_ptr(cartModel));
	if (pressedKeys[GLFW_KEY_H]) {
		cartMoving = !cartMoving;
		wheelsRotating = !wheelsRotating;
	}
	if (cartMoving)
	{
		cartModel = glm::translate(cartModel, glm::vec3(0.004f, 0.0f, 0.008f));
	}
	hayCart.Draw(shader);


	GLint cartWheel1ModelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	glUniformMatrix4fv(cartWheel1ModelLoc, 1, GL_FALSE, glm::value_ptr(cartWheel1Model));

	if (wheelsRotating) {
		cartWheel1Model = glm::translate(cartWheel1Model, glm::vec3(-9.02f, -0.22f, -2.21f));
		cartWheel1Model = glm::rotate(cartWheel1Model, glm::radians(1.0f), glm::vec3(0.2f, 0.0f, -0.1f));
		cartWheel1Model = glm::translate(cartWheel1Model, glm::vec3(9.02f, 0.22f, 2.21f));

		glm::mat4 translationM(1.0f);
		translationM = glm::translate(translationM, glm::vec3(0.004f, 0.0f, 0.008f));

		cartWheel1Model = translationM * cartWheel1Model;


	}
	wheel1.Draw(shader);

	GLint cartWheel2ModelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	glUniformMatrix4fv(cartWheel2ModelLoc, 1, GL_FALSE, glm::value_ptr(cartWheel2Model));

	if (wheelsRotating) {
		glm::mat4 translationM(1.0f);
		translationM = glm::translate(translationM, glm::vec3(0.004f, 0.0f, 0.008f));

		cartWheel2Model = glm::translate(cartWheel2Model, glm::vec3(-9.97f, -0.22f, -1.76f + distance));
		cartWheel2Model = glm::rotate(cartWheel2Model, glm::radians(1.0f), glm::vec3(0.2f, 0.0f, -0.1f));
		cartWheel2Model = glm::translate(cartWheel2Model, glm::vec3(9.97f, 0.22f, 1.76f - distance));

		cartWheel2Model = translationM * cartWheel2Model;

	}
	wheel2.Draw(shader);


	GLint windmillWingsModelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	glUniformMatrix4fv(windmillWingsModelLoc, 1, GL_FALSE, glm::value_ptr(windmillWingsModel));

	if (pressedKeys[GLFW_KEY_P]) {
		wingsRotating = !wingsRotating;
	}
	if (wingsRotating) {
		windmillWingsModel = glm::translate(windmillWingsModel, glm::vec3(104.33f, 9.52f, -35.05f));
		windmillWingsModel = glm::rotate(windmillWingsModel, glm::radians(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		windmillWingsModel = glm::translate(windmillWingsModel, glm::vec3(-104.33f, -9.52f, 35.05f));
	}
	windmillWings.Draw(shader);


	GLint doorModelLoc = glGetUniformLocation(shader.shaderProgram, "model");
	glUniformMatrix4fv(doorModelLoc, 1, GL_FALSE, glm::value_ptr(doorModel));
	if (pressedKeys[GLFW_KEY_C]) {
		doorModel = glm::translate(doorModel, glm::vec3(1.35f, 0.61f, -35.620f));
		doorModel = glm::rotate(doorModel, glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		doorModel = glm::translate(doorModel, glm::vec3(-1.35f, -0.61f, 35.620f));
	}
	if (pressedKeys[GLFW_KEY_O]) {
		doorModel = glm::translate(doorModel, glm::vec3(1.35f, 0.61f, -35.620f));
		doorModel = glm::rotate(doorModel, glm::radians(-1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		doorModel = glm::translate(doorModel, glm::vec3(-1.35f, -0.61f, 35.620f));
	}
	gateDoor.Draw(shader);

}

glm::vec3 getCameraPosition(const glm::mat4& viewMatrix) {
	glm::mat4 inverseView = glm::inverse(viewMatrix);
	return glm::vec3(inverseView[3][0], inverseView[3][1], inverseView[3][2]);
}



// Set the uniform value in the shader

void renderScene() {
	// depth maps creation pass
	//TODO - Send the light-space transformation matrix to the depth map creation shader and
	//		 render the scene in the depth map

	depthMapShader.useShaderProgram();
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "enableFog"), enableFog);///////////////
 

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	drawObjects(depthMapShader, true);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	 

	// render depth map on screen - toggled with the M key

	if (showDepthMap) {
		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT);

		screenQuadShader.useShaderProgram();

		//bind the depth map
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

		glDisable(GL_DEPTH_TEST);
		screenQuad.Draw(screenQuadShader);
		glEnable(GL_DEPTH_TEST);
	}
	else {

		// final scene rendering pass (with shadows)

		glViewport(0, 0, retina_width, retina_height);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		myCustomShader.useShaderProgram();

		view = myCamera.getViewMatrix();
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
				
		lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(glm::inverseTranspose(glm::mat3(view * lightRotation)) * lightDir));


		//bind the shadow map
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, depthMapTexture);
		glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);


		glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
			1,
			GL_FALSE,
			glm::value_ptr(computeLightSpaceTrMatrix()));

		drawObjects(myCustomShader, false);

		//draw a white cube around the light

		lightShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

		model = lightRotation;
		model = glm::translate(model, 1.0f * lightDir);
		model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
		glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

		//lightCube.Draw(lightShader);
	}

	if (enableSnow) {
		snowShader.useShaderProgram();

		glUniformMatrix4fv(glGetUniformLocation(snowShader.shaderProgram, "view"),
			1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(snowShader.shaderProgram, "projection"),
			1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(snowVAO);
		glDrawArrays(GL_POINTS, 0, snowdrops.size());
		glBindVertexArray(0);
	}
	mySkyBox.Draw(skyboxShader, view, projection);
}


void cleanup() {
	glDeleteTextures(1,& depthMapTexture);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &shadowMapFBO);
	glfwDestroyWindow(glWindow);
	//close GL context and any other GLFW resources
	glfwTerminate();
}

int main(int argc, const char * argv[]) {

	if (!initOpenGLWindow()) {
		glfwTerminate();
		return 1;
	}

	initOpenGLState();
	initObjects(); 
	initSkybox();
	initSnow();
	initShaders();
	initUniforms();
	initFBO();

	glCheckError();
	 glEnable(GL_PROGRAM_POINT_SIZE);
	while (!glfwWindowShouldClose(glWindow)) {
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processMovement();
		updateSnow(deltaTime);  
		renderScene();		

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	cleanup();

	return 0;
}
