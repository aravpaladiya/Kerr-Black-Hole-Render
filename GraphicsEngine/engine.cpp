#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader.h"
#include <windows.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Camera.h"

using namespace glm;

boolean userExternal = false;

int WIDTH = 800;
int HEIGHT = 600;

float lastFrame = 0.0f;
float deltaTime = 0.0f;

float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;
boolean firstMouse = true;


Camera camera = Camera(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 1.0f, 0.0f));
vec3 lightPos = vec3(0.0f, 0.0f, -0.0f);

//resize rendering window when window resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoff, double yoff) {
	camera.ProcessMouseScroll(yoff);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoff = xpos-lastX;
	lastX = xpos;
	float yoff = lastY - ypos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoff, yoff);
}

void processInput(GLFWwindow* window) {
	//close on esc press

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}

}



int main() {

	glfwInit();
	//version 3.3, core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//init glfw
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Learn", NULL, NULL);

	if (window == NULL) {
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
	//init glad
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, WIDTH, HEIGHT);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	std::string baPath = (userExternal ? "..\\..\\" : "..\\");
	std::string verPath = baPath + "GraphicsEngine\\shaders\\shader.vs";
	std::string fraPath = baPath + "GraphicsEngine\\shaders\\shader.fs";
	std::string lVerPa = baPath + "GraphicsEngine\\shaders\\lightShader.vs";
	std::string lFraPa = baPath + "GraphicsEngine\\shaders\\lightShader.fs";
	Shader shader(verPath.c_str(), fraPath.c_str());
	Shader lightShader(lVerPa.c_str(), lFraPa.c_str());


	float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
	float vsf = 1.0f;
	for (int i = 0; i < 216; i++) {
		vertices[i]*=vsf;
		i++;
		vertices[i]*=vsf;
		i++;
		vertices[i]*=vsf;
		i+=3;
	}
	vec3 cubePositions[] = {
		vec3(0.0f, 0.0f, 0.0f),
		vec3(2.0f, 5.0f, -15.0f),
		vec3(-1.5f, -2.2f, -2.5f),
		vec3(-3.8f, -2.0f, -12.3f),
		vec3(2.4f, -0.4f, -3.5f),
		vec3(-1.7f, 3.0f, -7.5f),
		vec3(1.3f, -2.0f, -2.5f),
		vec3(1.5f, 2.0f, -2.5f),
		vec3(1.5f, 0.2f, -1.5f),
		vec3(-1.3f, 1.0f, -1.5f)
	};


	//vao

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(1);

	
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);

	glBindVertexArray(lightVAO);

	unsigned int lightVBO;
	glGenBuffers(1, &lightVBO);

	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	glBindVertexArray(0);





	//textures



	// int w, h, nrChannels;
	// unsigned char* containerImgData = stbi_load("container.jpg", &w, &h, &nrChannels, 0);

	// unsigned int texture1;
	// glGenTextures(1, &texture1);

	// glBindTexture(GL_TEXTURE_2D, texture1);



	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, containerImgData);

	

	// stbi_image_free(containerImgData);

	// shader.setI("texture1", 0);
	glEnable(GL_DEPTH_TEST);




	/************************


		   RENDER LOOP


	*************************/
	// int fts = 200 ; 
	// float frameTimes[fts] = {0.0f};
	// for (int i = 0; i < fts; i++) {
	// 	frameTimes[fts] = 0;
	// }

	while (!glfwWindowShouldClose(window)) {

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		// float sum = 0;

		// for (int i = 0; i < fts-1; i++) {
		// 	frameTimes[i] = frameTimes[i+1];
		// 	sum+=frameTimes[i];
		// }
		// frameTimes[fts] = 1/deltaTime;
		// sum+=frameTimes[fts];
		// sum/=fts;
		
		// std::cout << sum << std::endl;
		//std::cout << 1/deltaTime << std::endl;
		processInput(window);

		//clear
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, texture1);



		//rendering code

		shader.use();


		int w, h;
		glfwGetWindowSize(window, &w, &h);

		mat4 view = camera.GetViewMatrix();

		mat4 projection = glm::perspective(glm::radians(camera.Zoom), 
		(float)w / (float)h, 0.1f, 100.0f);



		glBindVertexArray(VAO);

		mat4 cubeModel = mat4(1.0f);
		// cubeModel = translate(cubeModel, vec3(3.0f*cos(glfwGetTime()/3), 2.0f*sin(glfwGetTime()/3), 3.0f*sin(glfwGetTime()/3)));
		cubeModel = translate(cubeModel, vec3(1.5f, 1.0f, -3.0f));

		shader.setVec3("objColor", 1.0f, 0.5f, 0.0f);
		shader.setVec3("lightPos", lightPos);
		shader.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
		shader.setVec3("eye", camera.Position);

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		shader.setMat4("model", cubeModel);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		lightShader.use();		
		glBindVertexArray(lightVAO);

		mat4 lightModel = mat4(1.0f);
		lightModel = translate(lightModel, lightPos);

		
		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);
		lightShader.setMat4("model", lightModel);

		glDrawArrays(GL_TRIANGLES, 0, 36);




		//actually display
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}