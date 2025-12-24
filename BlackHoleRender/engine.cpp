#include "pch.h"
#include "shader.h"
#include "Camera.h"
#include "Model.h"

using namespace glm;



const float PI = 3.14159265358979323846;

int WIDTH = 1000;
int HEIGHT = 600;
float lastFrame = 0.0f;
float deltaTime = 0.03f;	


float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;
bool firstMouse = true;
bool f11Held = false;
#define PATH_STEPS 300

Camera camera = Camera(vec3(-0.0f, 1.4f, -6.0f), vec3(0.0f, 1.0f, 0.0f), glm::normalize(vec3(0.0f, 0.0f, 1.0f)));

//skip compute dispatch when camera hasn't moved
glm::vec3 prevCamPos = glm::vec3(std::numeric_limits<float>::max());
glm::vec3 prevCamDir = glm::vec3(std::numeric_limits<float>::max());

//resize rendering window when window resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	WIDTH = width;
	HEIGHT = height;//also update width and height variables for shader
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
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.ProcessKeyboard(UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		camera.ProcessKeyboard(DOWN, deltaTime);
	}
	//toggle fullscreen on F11
	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS && !f11Held) {
		f11Held = true;
		GLFWmonitor* monitor = glfwGetWindowMonitor(window);
		if (monitor == nullptr) {
			monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		} else {
			glfwSetWindowMonitor(window, nullptr, 100, 100, 1000, 600, 0);
		}
	}
	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE) {
		f11Held = false;
	}
	//display cursor when space pressed
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

}

unsigned int createCompute(const char* p) {
	std::string computeCode;
	std::ifstream cShaderFile;

	cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{

		cShaderFile.open(p);
		std::stringstream cShaderStream;

		cShaderStream << cShaderFile.rdbuf();

		cShaderFile.close();

		computeCode = cShaderStream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
	}
	const char* cShaderCode = computeCode.c_str();

	unsigned int compute;


	compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &cShaderCode, NULL);
	glCompileShader(compute);
	int success;
	char infoLog[512];

	unsigned int ID = glCreateProgram();
	glAttachShader(ID, compute);
	glLinkProgram(ID);

	glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(compute, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	
	
	glDeleteShader(compute);

	return ID;
}

unsigned int loadTexture(char const * path) //returns ID for texture stored
{
    unsigned int tID;
    glGenTextures(1, &tID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, tID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Simulation", NULL, NULL);

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
	//init shader
	std::string verPath = "shaders\\shader.vert";
	std::string fraPath = "shaders\\shader.frag";
	std::string compPath = "shaders\\ray.comp";
	Shader shader(verPath.c_str(), fraPath.c_str());
	//quad vertices
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

	//vao and vbo for quad on screen
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	//comp shader for rays
	int numRays = 5*std::max(WIDTH, HEIGHT);
	unsigned int raySSBO;
	glGenBuffers(1, &raySSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, raySSBO);
	//each RayPath in std430 is 300 vec3 pos + 300 vec3 dir, 16 bytes each = 9600 bytes
	size_t rayPathSize = PATH_STEPS * 2 * 16; //vec3 padded to 16 bytes in std430
	glBufferData(GL_SHADER_STORAGE_BUFFER, numRays * rayPathSize, nullptr, GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, raySSBO);



	unsigned int compProgram = createCompute(compPath.c_str());
	

	
	//textures for skybox (ESO Milky Way panorama by ESO/S. Brunier, CC BY 4.0)
	std::string skysubpath = "res/eso_cubemap/";
	std::vector<string> skyboxTextures
	{
		skysubpath + "right.png",   //+x
		skysubpath + "left.png",    //-x
		skysubpath + "top.png",     //+y
		skysubpath + "bottom.png",  //-y
		skysubpath + "front.png",   //+z
		skysubpath + "back.png"     //-z
	};
	
	texID = loadCubeMap(skyboxTextures);
	

	glBindVertexArray(0);
	shader.use();

	shader.setI("skybox", 0);

	glEnable(GL_DEPTH_TEST);
	
	//textures
	unsigned int colID = loadTexture("res/blackbody.png");

	/**

		RENDER LOOP

	**/

	float lastTime = int(glfwGetTime());
	int numFrames = 0;
	while (!glfwWindowShouldClose(window)) {
		numFrames++;
		float currentTime = int(glfwGetTime());
		
		if (currentTime > lastTime) {
			std::cout << numFrames << std::endl;
			numFrames = 0;
			lastTime = currentTime;
		}
		//check for key presses and mouse movement
		processInput(window);
 		//clear
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		//update ray count if window resized
		int newNumRays = 5 * std::max(WIDTH, HEIGHT);
		if (newNumRays != numRays) {
			numRays = newNumRays;
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, raySSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, numRays * rayPathSize, nullptr, GL_DYNAMIC_DRAW);
			prevCamPos = glm::vec3(std::numeric_limits<float>::max()); //force recompute
		}

		//precalculate ray paths along equatorial plane (only if camera moved)
		bool cameraChanged = (camera.Position != prevCamPos || camera.Front != prevCamDir);
		if (cameraChanged) {
			glUseProgram(compProgram);

			glUniform1f(glGetUniformLocation(compProgram, "WIDTH"), static_cast<float>(WIDTH));
			glUniform1f(glGetUniformLocation(compProgram, "HEIGHT"), static_cast<float>(HEIGHT));
			glUniform3fv(glGetUniformLocation(compProgram, "camPos"), 1, value_ptr(camera.Position));
			glUniform3fv(glGetUniformLocation(compProgram, "camDir"), 1, value_ptr(camera.Front));
			glUniform3fv(glGetUniformLocation(compProgram, "camRight"), 1, value_ptr(camera.Right));
			glUniform3fv(glGetUniformLocation(compProgram, "camUp"), 1, value_ptr(camera.Up));

			glUniform1i(glGetUniformLocation(compProgram, "TOTALCALLS"), numRays);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, raySSBO);

			glDispatchCompute((numRays + 63) / 64, 1, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			prevCamPos = camera.Position;
			prevCamDir = camera.Front;
		}

		//rendering

		shader.use();

		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, colID);
		float time = glfwGetTime();

		if (false) {//view moves around black hole
			float r = length(camera.Position);
			float speed = 0.1;
			float timeS = sin(speed * time);
			float timeC = cos(speed * time);

			camera.Position = vec3(r * timeC, 0.0f, -r * timeS);
			camera.Yaw = -speed * time * 180 / PI - 180;
			camera.Pitch = 0;
			if (camera.Yaw > 180) {
				camera.Yaw -= 360;
			}
			if (camera.Yaw < -180) {
				camera.Yaw += 360;
			}
		}
		camera.updateCameraVectors();


		//set uniforms
		shader.setVec3("camPos", camera.Position);
		shader.setVec3("camDir", camera.Front);
		shader.setVec3("camRight", camera.Right);
		shader.setVec3("camUp", camera.Up); 
		shader.setF("WIDTH", static_cast<float>(WIDTH));
		shader.setF("HEIGHT", static_cast<float>(HEIGHT));
		shader.setI("TOTALCALLS", numRays);
		shader.setI("colormap", 1);
		shader.setF("tempconst", 35000.0f);
		shader.setF("brightconst", 1.4f);
		shader.setF("time", time);

		glDrawArrays(GL_TRIANGLES, 0, 6);


		//update buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		
	}
	//cleanup
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	glfwTerminate();
	return 0;
}