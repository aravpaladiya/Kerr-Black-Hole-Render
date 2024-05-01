#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader_h.h"
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
	std::string verPath = baPath + "GraphicsEngine\\shader.vs";
	std::string fraPath = baPath + "GraphicsEngine\\shader.fs";
	Shader shader(verPath.c_str(), fraPath.c_str());



	float vertices[] = {
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		0.5f,  -0.5f, -0.5f, 1.0f, 0.0f,
		0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
		0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
		-0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,  0.0f, 0.0f,
		0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f, 0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,  0.0f, 0.0f,
		-0.5f, 0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, 0.5f,  -0.5f, 1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, -0.5f, 0.5f,  0.0f, 0.0f,
		-0.5f, 0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
		0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f,  -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f,  -0.5f, 0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		0.5f,  -0.5f, -0.5f, 1.0f, 1.0f,
		0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
		0.5f,  -0.5f, 0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f, 0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
		-0.5f, 0.5f,  -0.5f, 0.0f, 1.0f,
		0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, 0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, 0.5f,  -0.5f, 0.0f, 1.0f
	};

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

	unsigned int indices[] = {
		0, 1, 3,
		1, 2, 3
	};

	//vao

	unsigned int VBO;
	glGenBuffers(1, &VBO);

	/*unsigned int EBO;
	glGenBuffers(1, &EBO);*/



	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	/*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);*/

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(1);


	glBindVertexArray(0);

	//textures



	int w, h, nrChannels;
	unsigned char* containerImgData = stbi_load("container.jpg", &w, &h, &nrChannels, 0);

	unsigned int texture1;
	glGenTextures(1, &texture1);

	glBindTexture(GL_TEXTURE_2D, texture1);



	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, containerImgData);

	

	stbi_image_free(containerImgData);

	shader.use();
	shader.setI("texture1", 0);
	glEnable(GL_DEPTH_TEST);




	/************************


		   RENDER LOOP


	*************************/



	while (!glfwWindowShouldClose(window)) {

		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		//clear
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);


		glBindVertexArray(VAO);

		//rendering code



		int w, h;
		glfwGetWindowSize(window, &w, &h);

		mat4 view = camera.GetViewMatrix();

		mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)w / (float)h, 0.1f, 100.0f);


		shader.setMat4("view", view);
		shader.setMat4("projection", projection);

		for (int i = 0; i < 10; i++) {
			mat4 model = mat4(1.0f);
			model = glm::translate(model, cubePositions[i]);




			shader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);

		}







		//actually display
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}