#include "pch.h"
#include "shader.h"
#include "Camera.h"

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


	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	vec3 cubePositions[] = {
		vec3( 0.0f,  0.0f,  0.0f),
		vec3( 2.0f,  5.0f, -15.0f),
		vec3(-1.5f, -2.2f, -2.5f),
		vec3(-3.8f, -2.0f, -12.3f),
		vec3( 2.4f, -0.4f, -3.5f),
		vec3(-1.7f,  3.0f, -7.5f),
		vec3( 1.3f, -2.0f, -2.5f),
		vec3( 1.5f,  2.0f, -2.5f),
		vec3( 1.5f,  0.2f, -1.5f),
		vec3(-1.3f,  1.0f, -1.5f)
	};
	vec3 pointLightPositions[] = {
        vec3( 0.7f,  0.2f,  2.0f), vec3(1.0f, 0.0f, 0.0f),
        vec3( 2.3f, -3.3f, -4.0f), vec3(0.0f, 1.0f, 0.0f),
        vec3(-4.0f,  2.0f, -12.0f), vec3(0.0f, 0.0f, 1.0f),
        vec3( 0.0f,  0.0f, -3.0f), vec3(1.0f, 1.0f, 0.0f)
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (3*sizeof(float)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*) (6*sizeof(float)));
	glEnableVertexAttribArray(2);


	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);

	glBindVertexArray(lightVAO);

	unsigned int lightVBO;
	glGenBuffers(1, &lightVBO);

	glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);


	glBindVertexArray(0);





	//textures

	unsigned int diffusemap = loadTexture("container2.png");
	unsigned int specularmap = loadTexture("container2_specular.png");

	// int w, h, nrChannels;
	// unsigned char* diffuseImageData = stbi_load("container2.png", &w, &h, &nrChannels, 0);

	// unsigned int diffusemap;
	// glGenTextures(1, &diffusemap);
	// glBindTexture(GL_TEXTURE_2D, diffusemap);



	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, diffuseImageData);
	
	

	// stbi_image_free(diffuseImageData);
	shader.use();

	shader.setI("material.diffuse", 0);
	shader.setI("material.specular", 1);
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

		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffusemap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularmap);



		//rendering code

		shader.use();


		int w, h;
		glfwGetWindowSize(window, &w, &h);

		mat4 view = camera.GetViewMatrix();

		mat4 projection = perspective(radians(camera.Zoom), 
		(float)w / (float)h, 0.1f, 100.0f);



		glBindVertexArray(VAO);

		// cubeModel = translate(cubeModel, vec3(3.0f*cos(glfwGetTime()/3), 2.0f*sin(glfwGetTime()/3), 3.0f*sin(glfwGetTime()/3)));

		shader.setVec3("objColor", 1.0f, 0.5f, 0.0f);
		shader.setVec3("eye", camera.Position);
		shader.setF("material.shininess", 32.0f);

		shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);	
        shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
        // point light 1
        shader.setVec3("pointLights[0].position", pointLightPositions[0]);
        shader.setVec3("pointLights[0].ambient", pointLightPositions[1]/20.0f);
        shader.setVec3("pointLights[0].diffuse", pointLightPositions[1]);
        shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
        shader.setF("pointLights[0].constant", 1.0f);
        shader.setF("pointLights[0].linear", 0.09f);
        shader.setF("pointLights[0].quadratic", 0.032f);
        // point light 2
        shader.setVec3("pointLights[1].position", pointLightPositions[2]);
        shader.setVec3("pointLights[1].ambient", pointLightPositions[3]/20.0f);
        shader.setVec3("pointLights[1].diffuse", pointLightPositions[3]);
        shader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
        shader.setF("pointLights[1].constant", 1.0f);
        shader.setF("pointLights[1].linear", 0.09f);
        shader.setF("pointLights[1].quadratic", 0.032f);
        // point light 3
        shader.setVec3("pointLights[2].position", pointLightPositions[4]);
        shader.setVec3("pointLights[2].ambient", pointLightPositions[5]/20.0f);
        shader.setVec3("pointLights[2].diffuse", pointLightPositions[5]);
        shader.setVec3("pointLights[2].specular", 1.0f, 1.0f, 1.0f);
        shader.setF("pointLights[2].constant", 1.0f);
        shader.setF("pointLights[2].linear", 0.09f);
        shader.setF("pointLights[2].quadratic", 0.032f);
        // point light 4
        shader.setVec3("pointLights[3].position", pointLightPositions[6]);
        shader.setVec3("pointLights[3].ambient", pointLightPositions[7]/20.0f);
        shader.setVec3("pointLights[3].diffuse", pointLightPositions[7]);
        shader.setVec3("pointLights[3].specular", 1.0f, 1.0f, 1.0f);
        shader.setF("pointLights[3].constant", 1.0f);
        shader.setF("pointLights[3].linear", 0.09f);
        shader.setF("pointLights[3].quadratic", 0.032f);
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

		shader.setB("flashlight", flashLight);

		for (int i = 0; i < 10; i++) {
			mat4 model = mat4(1.0f);
            model = translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, radians(angle), vec3(1.0f, 0.3f, 0.5f));

			shader.setMat4("model", model);

			
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		lightShader.use();		
		glBindVertexArray(lightVAO);

		lightShader.setMat4("view", view);
		lightShader.setMat4("projection", projection);
		
		for (int i = 0; i < 8; i++) {
			mat4 model = mat4(1.0f);
			model = translate(model, pointLightPositions[i]);
			i++;
			lightShader.setMat4("model", model);
			lightShader.setVec3("color", pointLightPositions[i]);
			glDrawArrays(GL_TRIANGLES, 0, 36);

		}
		



		//actually display
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}