#include "pch.h"
#include "shader.h"
#include "Camera.h"
#include "Model.h"

using namespace glm;


bool userExternal = false;

const float PI = 3.14159265358979323846;

int WIDTH = 3840;
int HEIGHT = 2160;
float lastFrame = 0.0f;
float deltaTime = 0.0f;	


float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;
bool firstMouse = true;

bool flashLight = false;


Camera camera = Camera(vec3(0.0f, 0.0f, -7.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f));

//resize rendering window when window resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	WIDTH = width;
	HEIGHT = height;
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoff, double yoff) {//zoom (change fov)
	camera.ProcessMouseScroll(yoff);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {//turn camera
	if (firstMouse) {//to account for first mouse movement having large offset
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
	//camera movement
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
	//display cursor when space pressed
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

}

unsigned int loadTexture(char const * path) //returns ID for texture stored
{
    unsigned int tID;
    glGenTextures(1, &tID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, tID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return tID;
}

unsigned int loadCubeMap(std::vector<string> faces) {//returns ID for the cubemap stored
	unsigned int texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	int w, h, c;
	for (int i = 0; i < 6; i++) {
		unsigned char *data = stbi_load(faces[i].c_str(), &w, &h, &c, 0);
		std::cout << data;
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else {
			std::cout << "fail texture load" << faces[i] << std::endl;
		}

		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return texID;
}

int main() {

	glfwInit();
	//version 4.6, core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
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

	std::string baPath = (userExternal ? "..\\..\\" : "..\\");
	std::string verPath = baPath + "GraphicsEngine\\shaders\\shader.vert";
	std::string fraPath = baPath + "GraphicsEngine\\shaders\\shader.frag";
	Shader shader(verPath.c_str(), fraPath.c_str());

	float vertices[]{
		//vertex				
		-1.0f, -1.0f, 0.0f,		
		1.0f, 1.0f, 0.0f,		
		-1.0f, 1.0f, 0.0f,		
		-1.0f, -1.0f, 0.0f,		
		1.0f, 1.0f, 0.0f,		
		1.0f, -1.0f, 0.0f
	};

	unsigned int VBO, VAO, texID;

	//vao and vbo
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);




	//textures
	std::vector<string> skyboxTextures
	{
		"right.png",
		"left.png",
		"top.png",
		"bottom.png",
		"front.png",
		"back.png"
	};
	
	texID = loadCubeMap(skyboxTextures);
	

	glBindVertexArray(0);
	shader.use();

	shader.setI("skybox", 0);

	glEnable(GL_DEPTH_TEST);
	/*std::string path = "C:/Users/aravp/Downloads/backpack/backpack.obj";
	Model ourModel(path);*/



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


		//rendering code

		shader.use();

		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
		if (true) {//view moves around black hole
			float r = 7.0f;
			float time = glfwGetTime();

			float timeS = sin(0.1 * time);
			float timeC = cos(0.1 * time);

			camera.Position = vec3(r * timeC, 0.0f, -r * timeS);
			camera.Yaw = -0.1 * time * 180 / PI - 180;
			if (camera.Yaw > 180) {
				camera.Yaw -= 360;
			}
			if (camera.Yaw < -180) {
				camera.Yaw += 360;
			}
		}
		camera.updateCameraVectors();



		shader.setVec3("camPos", camera.Position);
		shader.setVec3("camDir", camera.Front);
		shader.setVec3("camRight", camera.Right);
		shader.setVec3("camUp", camera.Up); 
		shader.setF("WIDTH", WIDTH);
		shader.setF("HEIGHT", HEIGHT);
		shader.setMat4("lookat", glm::lookAt(camera.Position, camera.Position+camera.Front, camera.Up));

		glDrawArrays(GL_TRIANGLES, 0, 6);


		//actually display
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	glfwTerminate();
	return 0;
}