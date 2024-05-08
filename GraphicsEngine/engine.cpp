#include "pch.h"
#include "shader.h"
#include "Camera.h"
#include "Model.h"

using namespace glm;


bool userExternal = false;

int WIDTH = 800;
int HEIGHT = 600;
float lastFrame = 0.0f;
float deltaTime = 0.0f;	


float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;
bool firstMouse = true;

bool flashLight = false;


Camera camera = Camera(vec3(0.0f, 0.0f, 3.0f), vec3(0.0f, 1.0f, 0.0f));
vec3 lightPos(1.2f, 1.0f, 2.0f);

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
	if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
		flashLight = true;
	} else {
		flashLight = false;
	}

}

unsigned int loadTexture(char const * path)
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

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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


	shader.use();

	glEnable(GL_DEPTH_TEST);
	std::string path = "C:/Users/aravp/Downloads/backpack/backpack.obj";
	Model ourModel(path);



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


		int w, h;
		glfwGetWindowSize(window, &w, &h);

		mat4 view = camera.GetViewMatrix();

		mat4 projection = perspective(radians(camera.Zoom), 
		(float)w / (float)h, 0.1f, 100.0f);





		shader.setVec3("objColor", 1.0f, 0.5f, 0.0f);
		shader.setVec3("eye", camera.Position);
		shader.setB("flashlight", flashLight);

        shader.setVec3("pointLight.position", 0.0f, 0.0f, 0.0f);
        shader.setVec3("pointLight.ambient", 0.1f, 0.1f, 0.1f);
        shader.setVec3("pointLight.diffuse", 0.5f, 0.5f, 0.5f);
        shader.setVec3("pointLight.specular", 1.0f, 1.0f, 1.0f);
        shader.setF("pointLight.constant", 1.0f);
        shader.setF("pointLight.linear", 0.09f);
        shader.setF("pointLight.quadratic", 0.032f);

		// spotLight
		shader.setVec3("spotlight.position", camera.Position);
		shader.setVec3("spotlight.direction", camera.Front);
		shader.setVec3("spotlight.ambient", 0.0f, 0.0f, 0.0f);
		shader.setVec3("spotlight.diffuse", 1.0f, 1.0f, 1.0f);
		shader.setVec3("spotlight.specular", 1.0f, 1.0f, 1.0f);
		shader.setF("spotlight.constant", 1.0f);
		shader.setF("spotlight.linear", 0.09f);
		shader.setF("spotlight.quadratic", 0.032f);
		shader.setF("spotlight.cutoffIn", cos(radians(12.5f)));
		shader.setF("spotlight.cutoffOut", cos(radians(15.0f)));

        

		shader.setMat4("view", view);
		shader.setMat4("projection", projection);
		mat4 model = glm::translate(mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	

		shader.setMat4("model", model);


		ourModel.draw(shader);
		

		/*lightShader.use();		

		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);
		
		mat4 model = mat4(1.0f);
		model = translate(model, vec3(0.0f, 0.0f, 4.0f));
		
		lightShader.setMat4("model", model);
		lightShader.setVec3("color", 1.0f, 1.0f, 1.0f);

		*/



		//actually display
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}