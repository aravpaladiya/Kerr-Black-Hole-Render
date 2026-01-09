#include "pch.h"
#include <vector>
#include "shader.h"
#include "Camera.h"

using namespace glm;
using namespace std;

const float PI = 3.14159265358979323846f;

int WIDTH = 1920;
int HEIGHT = 1040;
float lastFrame = 0.0f;
float deltaTime = 0.03f;

float lastX = WIDTH / 2;
float lastY = HEIGHT / 2;
bool firstMouse = true;
bool f11Held = false;

float spin_a = 0.0f; //kerr spin parameter [0, 0.998]
bool spinChanged = false;
int debugTileLevel = 0; //0 = off, 1 = color code by tile level
int forcePerPixel = 0;  //0 = adaptive tiling, 1 = force per pixel
bool tHeld = false;
bool fHeld = false;

Camera camera = Camera(vec3(0.0f, 3.0f, -25.0f), vec3(0.0f, 1.0f, 0.0f), glm::normalize(vec3(0.0f, -0.1f, 1.0f)));

//skip compute dispatch when camera and spin unchanged
glm::vec3 prevCamPos = glm::vec3(std::numeric_limits<float>::max());
glm::vec3 prevCamDir = glm::vec3(std::numeric_limits<float>::max());
float prevSpin = -1.0f;

//isco for prograde kerr orbit, M=1
float computeISCO(float a) {
	float a2 = a * a;
	float z1 = 1.0f + cbrt(1.0f - a2) * (cbrt(1.0f + a) + cbrt(1.0f - a));
	float z2 = sqrt(3.0f * a2 + z1 * z1);
	return 3.0f + z2 - sqrt((3.0f - z1) * (3.0f + z1 + 2.0f * z2));
}

//cartesian (y = spin axis) to boyer-lindquist
void cartesianToBL(vec3 pos, float a, float& r_out, float& theta_out, float& phi_out) {
	float x = pos.x, y = pos.y, z = pos.z;
	float a2 = a * a;
	float w2 = x * x + y * y + z * z - a2;
	float r2 = (w2 + sqrt(w2 * w2 + 4.0f * a2 * y * y)) / 2.0f;
	r_out = sqrt(r2);
	r_out = glm::max(r_out, 0.001f);
	theta_out = acos(glm::clamp(y / r_out, -0.999f, 0.999f));
	phi_out = atan2(z, x);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	WIDTH = width;
	HEIGHT = height;
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
	float xoff = xpos - lastX;
	lastX = xpos;
	float yoff = lastY - ypos;
	lastY = ypos;
	camera.ProcessMouseMovement(xoff, yoff);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

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
	if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_RELEASE)
		f11Held = false;

	//spin control, Q to decrease, E to increase
	static bool qHeld = false, eHeld = false;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !qHeld) {
		qHeld = true;
		spin_a = glm::max(spin_a - 0.05f, -0.998f);
		spinChanged = true;
		std::cout << "spin a = " << spin_a << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) qHeld = false;

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !eHeld) {
		eHeld = true;
		spin_a = glm::min(spin_a + 0.05f, 0.998f);
		spinChanged = true;
		std::cout << "spin a = " << spin_a << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) eHeld = false;

	//debug tile level toggle on T
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !tHeld) {
		tHeld = true;
		debugTileLevel = 1 - debugTileLevel;
		spinChanged = true; //force recompute
		std::cout << "debug tile level: " << (debugTileLevel ? "ON" : "OFF") << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) tHeld = false;

	//force per pixel toggle on F, bypasses the coarse grid scheme
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fHeld) {
		fHeld = true;
		forcePerPixel = 1 - forcePerPixel;
		spinChanged = true;
		std::cout << "force per-pixel: " << (forcePerPixel ? "ON" : "OFF") << std::endl;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) fHeld = false;

	//display cursor when R pressed
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

unsigned int createCompute(const char* p) {
	std::string computeCode;
	std::ifstream cShaderFile;
	cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		cShaderFile.open(p);
		std::stringstream cShaderStream;
		cShaderStream << cShaderFile.rdbuf();
		cShaderFile.close();
		computeCode = cShaderStream.str();
	} catch (std::ifstream::failure& e) {
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
	}
	const char* cShaderCode = computeCode.c_str();

	unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(compute, 1, &cShaderCode, NULL);
	glCompileShader(compute);
	int success;
	char infoLog[512];
	glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(compute, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::COMPUTE::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int ID = glCreateProgram();
	glAttachShader(ID, compute);
	glLinkProgram(ID);
	glGetProgramiv(ID, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(ID, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(compute);
	return ID;
}

unsigned int loadTexture(char const* path) {
	unsigned int tID;
	glGenTextures(1, &tID);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format = GL_RGB;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3) format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, tID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
	} else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return tID;
}

unsigned int loadCubeMap(vector<string> faces) {
	unsigned int texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

	int w, h, c;
	for (int i = 0; i < 6; i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &w, &h, &c, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		} else {
			std::cout << "fail texture load " << faces[i] << std::endl;
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Kerr Black Hole", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW Window" << std::endl;
		glfwTerminate();
		return -1;
	}
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

	//shaders
	Shader shader("shaders\\shader.vert", "shaders\\kerr.frag");
	unsigned int tileProgram = createCompute("shaders\\kerr_tile.comp");
	unsigned int compProgram = createCompute("shaders\\kerr.comp");

	//quad vertices
	float vertices[]{
		-1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	//ssbo for per pixel results, 4 vec4s = 64 bytes each
	int numPixels = WIDTH * HEIGHT;
	size_t pixelResultSize = 4 * 16; //4 vec4s = 64 bytes
	unsigned int pixelSSBO;
	glGenBuffers(1, &pixelSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, pixelSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numPixels * pixelResultSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pixelSSBO);

	//ssbo for coarse grid, 4 vec4s = 64 bytes per point
	int coarseK = 8;
	int coarseW = WIDTH / coarseK + 1;
	int coarseH = HEIGHT / coarseK + 1;
	int numCoarse = coarseW * coarseH;
	size_t coarsePointSize = 4 * 16; //4 vec4s, sky_hit + disk0 + disk1 + disk2
	unsigned int coarseSSBO;
	glGenBuffers(1, &coarseSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, coarseSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, numCoarse * coarsePointSize, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, coarseSSBO);

	//textures for skybox
	std::string skysubpath = "res/eso_cubemap/";
	std::vector<std::string> skyboxTextures{
		skysubpath + "right.png",
		skysubpath + "left.png",
		skysubpath + "top.png",
		skysubpath + "bottom.png",
		skysubpath + "front.png",
		skysubpath + "back.png"
	};
	unsigned int skyboxID = loadCubeMap(skyboxTextures);

	//blackbody colormap
	unsigned int colID = loadTexture("res/blackbody.png");

	glBindVertexArray(0);
	shader.use();
	shader.setI("skybox", 0);

	glEnable(GL_DEPTH_TEST);

	//fps counter
	float lastTime = (float)(int)glfwGetTime();
	int numFrames = 0;

	while (!glfwWindowShouldClose(window)) {
		numFrames++;
		float currentTime = (float)(int)glfwGetTime();
		if (currentTime > lastTime) {
			std::cout << numFrames << " fps" << std::endl;
			numFrames = 0;
			lastTime = currentTime;
		}

		processInput(window);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//reallocate ssbos on resize
		int newNumPixels = WIDTH * HEIGHT;
		int newCoarseW = WIDTH / coarseK + 1;
		int newCoarseH = HEIGHT / coarseK + 1;
		int newNumCoarse = newCoarseW * newCoarseH;
		if (newNumPixels != numPixels) {
			numPixels = newNumPixels;
			coarseW = newCoarseW; coarseH = newCoarseH; numCoarse = newNumCoarse;
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, pixelSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, numPixels * pixelResultSize, nullptr, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, coarseSSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, numCoarse * coarsePointSize, nullptr, GL_DYNAMIC_DRAW);
			prevCamPos = glm::vec3(std::numeric_limits<float>::max());
		}

		//compute dispatch only if camera or spin changed
		bool cameraChanged = (camera.Position != prevCamPos || camera.Front != prevCamDir);
		if (cameraChanged || spinChanged) {
			camera.updateCameraVectors();

			//camera position to boyer-lindquist
			float r_obs, theta_obs, phi_obs;
			cartesianToBL(camera.Position, spin_a, r_obs, theta_obs, phi_obs);

			float isco = computeISCO(spin_a);

			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, pixelSSBO);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, coarseSSBO);

			//pass 1, coarse grid integration, one thread per coarse point
			if (forcePerPixel == 0) {
				glUseProgram(tileProgram);
				glUniform1f(glGetUniformLocation(tileProgram, "spin_a"), spin_a);
				glUniform1f(glGetUniformLocation(tileProgram, "r_obs"), r_obs);
				glUniform1f(glGetUniformLocation(tileProgram, "theta_obs"), theta_obs);
				glUniform1f(glGetUniformLocation(tileProgram, "phi_obs"), phi_obs);
				glUniform1f(glGetUniformLocation(tileProgram, "fov"), camera.Zoom);
				glUniform1i(glGetUniformLocation(tileProgram, "WIDTH"), WIDTH);
				glUniform1i(glGetUniformLocation(tileProgram, "HEIGHT"), HEIGHT);
				glUniform3fv(glGetUniformLocation(tileProgram, "camDir"), 1, value_ptr(camera.Front));
				glUniform3fv(glGetUniformLocation(tileProgram, "camRight"), 1, value_ptr(camera.Right));
				glUniform3fv(glGetUniformLocation(tileProgram, "camUp"), 1, value_ptr(camera.Up));
				glUniform1i(glGetUniformLocation(tileProgram, "COARSE_W"), coarseW);
				glUniform1i(glGetUniformLocation(tileProgram, "COARSE_H"), coarseH);
				glUniform1f(glGetUniformLocation(tileProgram, "diskIn"), isco);
				glUniform1f(glGetUniformLocation(tileProgram, "diskOut"), 20.0f);
				glDispatchCompute((numCoarse + 255) / 256, 1, 1);
				glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			}

			//pass 2, per pixel interpolate or fallback
			glUseProgram(compProgram);
			glUniform1f(glGetUniformLocation(compProgram, "spin_a"), spin_a);
			glUniform1f(glGetUniformLocation(compProgram, "r_obs"), r_obs);
			glUniform1f(glGetUniformLocation(compProgram, "theta_obs"), theta_obs);
			glUniform1f(glGetUniformLocation(compProgram, "phi_obs"), phi_obs);
			glUniform1f(glGetUniformLocation(compProgram, "fov"), camera.Zoom);
			glUniform1i(glGetUniformLocation(compProgram, "WIDTH"), WIDTH);
			glUniform1i(glGetUniformLocation(compProgram, "HEIGHT"), HEIGHT);
			glUniform3fv(glGetUniformLocation(compProgram, "camDir"), 1, value_ptr(camera.Front));
			glUniform3fv(glGetUniformLocation(compProgram, "camRight"), 1, value_ptr(camera.Right));
			glUniform3fv(glGetUniformLocation(compProgram, "camUp"), 1, value_ptr(camera.Up));
			glUniform1i(glGetUniformLocation(compProgram, "debugTileLevel"), debugTileLevel);
			glUniform1i(glGetUniformLocation(compProgram, "forcePerPixel"), forcePerPixel);
			glUniform1i(glGetUniformLocation(compProgram, "COARSE_W"), coarseW);
			glUniform1i(glGetUniformLocation(compProgram, "COARSE_H"), coarseH);
			glUniform1f(glGetUniformLocation(compProgram, "diskIn"), isco);
			glUniform1f(glGetUniformLocation(compProgram, "diskOut"), 20.0f);
			glDispatchCompute((WIDTH + 15) / 16, (HEIGHT + 15) / 16, 1);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

			prevCamPos = camera.Position;
			prevCamDir = camera.Front;
			prevSpin = spin_a;
			spinChanged = false;
		}

		//rendering
		shader.use();
		glBindVertexArray(VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxID);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, colID);

		float isco = computeISCO(spin_a);

		shader.setI("WIDTH", WIDTH);
		shader.setI("HEIGHT", HEIGHT);
		shader.setF("time", (float)glfwGetTime());
		shader.setF("spin_a", spin_a);
		shader.setF("tempconst", 35000.0f);
		shader.setF("brightconst", 1.4f);
		shader.setF("diskIn", isco);
		shader.setF("diskOut", 20.0f);
		shader.setI("skybox", 0);
		shader.setI("colormap", 1);

		camera.updateCameraVectors();
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &pixelSSBO);
	glDeleteBuffers(1, &coarseSSBO);
	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}
