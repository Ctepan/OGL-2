#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

const GLuint winWidth = 1000;
const GLuint winHeight = 700;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

float alpha = 0;

int keys[1024];

//Функция для считывания шейдера из файла
GLuint readShaderFile(const string path, GLenum type);
//Функция для создания шейдерной программы
GLuint createShProgtamm(const vector<GLuint> &progs);
//Функции для обработки нажатий клавиш
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void key_action();
//Функция для инициализации начальных данных моделей 1,2
void gravitationShowInit(int numOfParts, GLuint &vaoHandle, GLuint &numElements);
//Функции для отрисовки модели
void drawGravitationShow(GLuint compProg, GLuint rendProg, GLuint VAO, int numOfParts, GLuint numElements, glm::mat4 projection);

int main()
{
	glfwInit();//Иницализация окна
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow *window = glfwCreateWindow(winWidth, winHeight, "Program",
		NULL, NULL);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	glEnable(GL_DEPTH_TEST);

	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);


	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("GL Vendor : %s\n", vendor);
	printf("GL Renderer : %s\n", renderer);
	printf("GL Version (string) : %s\n", version);
	printf("GL Version (integer) : %d.%d\n", major, minor);
	printf("GLSL Version : %s\n", glslVersion);


	glfwSetKeyCallback(window, key_callback);

	GLuint vertShader;
	GLuint fragShader;
	GLuint computeShader;
	vector<GLuint> renShaders;
	vector<GLuint> compShaders;
	vector<GLuint> compNShaders;
	GLuint programHandle;
	GLuint compProg;
	GLuint compNProg;
	GLuint computeNShader;

	vertShader = readShaderFile("vshader1.vert", GL_VERTEX_SHADER);
	fragShader = readShaderFile("fshader1.frag", GL_FRAGMENT_SHADER);
	computeShader = readShaderFile("cshader.txt", GL_COMPUTE_SHADER);
	renShaders.push_back(vertShader);
	renShaders.push_back(fragShader);
	compShaders.push_back(computeShader);
	programHandle = createShProgtamm(renShaders);
	compProg = createShProgtamm(compShaders);
	
	
	GLuint vaoHandle;
	int numOfParts = 490000;
	GLuint numElements = 0;
	gravitationShowInit(numOfParts, vaoHandle, numElements);


	int readBuf = 0;
	GLfloat dt = 0.0005;
	glm::mat4 projection;
	projection = glm::perspective(60.0f, (GLfloat)width / (GLfloat)height, 0.1f, 500.0f);
	//Главный цикл отрисовки
	while (!glfwWindowShouldClose(window))
	{
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		std::cout << 1/deltaTime << std::endl;
		glfwPollEvents();
		key_action();
		glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		drawGravitationShow(compProg, programHandle, vaoHandle, numOfParts, numElements, projection);
		glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

void drawGravitationShow(GLuint compProg, GLuint rendProg, GLuint VAO, int numOfParts, GLuint numElements, glm::mat4 projection)
{
	glUseProgram(compProg);
	
	glUniform1f(glGetUniformLocation(compProg, "Gravity1"), 10000.0f);
	glUniform1f(glGetUniformLocation(compProg, "Gravity2"), 10000.0f);
	glUniform3f(glGetUniformLocation(compProg, "BlackHolePos1"), 100.0f*cos(glfwGetTime()), 100.0f*sin(glfwGetTime()), -100.0f);
	glUniform3f(glGetUniformLocation(compProg, "BlackHolePos2"), 0.0f, 0.0f, -100.0f);
	//glUniform1f(glGetUniformLocation(compProg, "DeltaT"), dt);
	glDispatchCompute(glm::max(1, numOfParts / 1000), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glUseProgram(rendProg);//======================================================================
	glUniformMatrix4fv(glGetUniformLocation(rendProg, "proj"), 1, GL_FALSE, &projection[0][0]);
	glm::mat4 model;
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	//model = glm::rotate(model, alpha, glm::vec3(0.0, 1.0, 0.0));
	glUniformMatrix4fv(glGetUniformLocation(rendProg, "model"), 1, GL_FALSE, &model[0][0]);
	glm::mat4 view;
	glUniformMatrix4fv(glGetUniformLocation(rendProg, "view"), 1, GL_FALSE, &view[0][0]);
	//==================================================================================================

	glBindVertexArray(VAO);
	//glDrawArraysInstanced(GL_TRIANGLES, 0, 3, numOfParts);
	glDrawElementsInstanced(GL_TRIANGLE_STRIP, numElements, GL_UNSIGNED_INT, 0, numOfParts);
	glBindVertexArray(0);
}

//void drawClothShow(GLuint &compProg, GLuint &compNProg, GLuint &rendProg, GLuint &VAO, glm::ivec3 &nParticles, glm::mat4 &projection, int &readBuf, clothBuf &clBuf, GLuint &numElements)
//{
//	
//	glUseProgram(compProg);
//	float dx = 4.0f / (nParticles.x - 1);
//	float dy = 3.0f / (nParticles.y - 1);
//	glUniform1f(glGetUniformLocation(compProg, "RestLengthHoriz"), dx);
//	glUniform1f(glGetUniformLocation(compProg, "RestLengthVert"), dy);
//	glUniform1f(glGetUniformLocation(compProg, "RestLengthDiag"), sqrtf(dx * dx + dy * dy));
//	for (int i = 0; i < 2000; i++)
//	{
//		glDispatchCompute(nParticles.x / 10, nParticles.y / 10, 1);
//		int a;
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//
//		readBuf = 1 - readBuf;
//		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, clBuf.posBufs[readBuf]);
//		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, clBuf.posBufs[1 - readBuf]);
//		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, clBuf.velBufs[readBuf]);
//		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, clBuf.velBufs[1 - readBuf]);
//	}
//
//	glUseProgram(compNProg);
//	glDispatchCompute(nParticles.x / 10, nParticles.y / 10, 1);
//	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
//
//	glUseProgram(rendProg);//======================================================================
//	glUniformMatrix4fv(glGetUniformLocation(rendProg, "proj"), 1, GL_FALSE, &projection[0][0]);
//	glm::mat4 model;
//	model = glm::translate(model, glm::vec3(-0.0f, 0.0f, 0.0f));
//	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
//	glUniformMatrix4fv(glGetUniformLocation(rendProg, "model"), 1, GL_FALSE, &model[0][0]);
//	glm::mat4 view;
//	view = glm::translate(view, glm::vec3(-0.05f, -0.05f, -0.5f));
//	view = glm::rotate(view, alpha, glm::vec3(0.0, 1.0, 0.0));
//	glUniformMatrix4fv(glGetUniformLocation(rendProg, "view"), 1, GL_FALSE, &view[0][0]);
//	glm::mat3 MVN;
//	MVN = glm::mat3(glm::transpose(glm::inverse(view * model)));
//	glUniformMatrix3fv(glGetUniformLocation(rendProg, "MVN"), 1, GL_FALSE, &MVN[0][0]);
//	//==================================================================================================
//
//	glBindVertexArray(VAO);
//
//	glDrawElements(GL_TRIANGLE_STRIP, numElements, GL_UNSIGNED_INT, 0);
//	glBindVertexArray(0);
//}


GLuint readShaderFile(const string path, GLenum type)
{
	std::ifstream inFile(path);
	std::stringstream code;
	code.str("");
	code << inFile.rdbuf();
	inFile.close();
	string codeStr(code.str());
	const GLchar* codeArray = codeStr.c_str();
	GLuint Shader = glCreateShader(type);
	if (0 == Shader)
	{
		fprintf(stderr, "Error creating Shader.\n");
		exit(EXIT_FAILURE);
	}
	glShaderSource(Shader, 1, &codeArray, NULL);
	glCompileShader(Shader);
	GLint result;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &result);
	if (GL_FALSE == result)
	{
		fprintf(stderr, "Shader compilation failed!\n");
		GLint logLen;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			char * log = new char[logLen];
			GLsizei written;
			glGetShaderInfoLog(Shader, logLen, &written, log);
			fprintf(stderr, "Shader log:\n%s", log);
			delete[] log;
		}
	}
	return Shader;
}
GLuint createShProgtamm(const vector<GLuint> &progs)
{
	GLuint programHandle = glCreateProgram();
	if (0 == programHandle)
	{
		fprintf(stderr, "Error creating program object.\n");
		exit(1);
	}
	for each(GLuint i in progs)
	{
		glAttachShader(programHandle, i);
	}
	glLinkProgram(programHandle);
	GLint status;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
	if (GL_FALSE == status) {
		fprintf(stderr, "Failed to link shader program!\n");
		GLint logLen;
		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH,
			&logLen);
		if (logLen > 0)
		{
			char * log = new char[logLen];
			GLsizei written;
			glGetProgramInfoLog(programHandle, logLen, &written, log);
			fprintf(stderr, "Program log: \n%s", log);
			delete[] log;
		}
	}
	else
	{
		return programHandle;
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);


	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}
void key_action()
{
	if (keys[GLFW_KEY_LEFT])
		alpha += 1;
	if (keys[GLFW_KEY_RIGHT])
		alpha -= 1;
}

void gravitationShowInit(int numOfParts, GLuint &vaoHandle, GLuint &numElements)
{
	///////////////

	int initSize = sqrt(4 * numOfParts);
	std::vector<GLfloat> initPos = std::vector<GLfloat>(4 * numOfParts);
	for (int i = 0; i < initPos.size() / initSize; i++)
	{
		for (int j = 0; j < initPos.size() / initSize; j += 4)
		{
			initPos[i * initSize + j] = i*0.01f + 10;
			initPos[i * initSize + j + 1] = j*0.01f + 10;
			initPos[i * initSize + j + 2] = -100;
			initPos[i * initSize + j + 3] = 1;

		}
	}
	GLuint bufSize = numOfParts * 4 * sizeof(GLfloat);
	GLuint posBuf;
	////
	glGenBuffers(1, &posBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, posBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initPos[0], GL_DYNAMIC_DRAW);

	GLuint lastPosBuf;
	glGenBuffers(1, &lastPosBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lastPosBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initPos[0], GL_DYNAMIC_DRAW);

	std::vector<GLfloat> initSpeed = std::vector<GLfloat>(4 * numOfParts);
	for (int i = 0; i < initSpeed.size() / initSize; i++)
	{
		for (int j = 0; j < initSpeed.size() / initSize; j += 4)
		{
			initSpeed[i * initSize + j] = 110;
			initSpeed[i * initSize + j + 1] = 110;
			initSpeed[i * initSize + j + 2] = 110;
			initSpeed[i * initSize + j + 3] = 1;

		}
	}
	bufSize = numOfParts * 4 * sizeof(GLfloat);
	GLuint speedBuf;
	glGenBuffers(1, &speedBuf);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, speedBuf);
	glBufferData(GL_SHADER_STORAGE_BUFFER, bufSize, &initSpeed[0], GL_DYNAMIC_DRAW);


	/////Шары
	float particleVertexes2[] = {
		-1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f };
	int xPoints = 12;
	int yPoints = 12;
	std::vector<GLfloat> particleVertexes = std::vector<GLfloat>(xPoints*yPoints*3);
	GLfloat rad = 10.0f;
	for (int i = 0; i < yPoints; i++)
	{
		for (int j = 0; j < xPoints; j++)
		{
			particleVertexes[i * xPoints *3 + j*3] = rad*glm::sin(i*3.14f/ (yPoints/2))*glm::cos(j*3.14f / (xPoints / 2));
			particleVertexes[i * xPoints *3 + j*3 + 1] = rad*glm::sin(i*3.14f / (yPoints / 2))*glm::sin(j*3.14f / (xPoints / 2));
			particleVertexes[i * xPoints *3 + j*3 + 2] = rad*glm::cos(i*3.14f / (yPoints / 2));
		}
	}
	std::vector<GLuint> el;
	for (int row = 0; row < yPoints -  1; row++)
	{
		for (int col = 0; col < xPoints; col++)
		{
			el.push_back((row + 1) * xPoints + (col));
			el.push_back((row)* xPoints + (col));
		}
		row++;
		if (row < yPoints - 1)
		{
			for (int col = xPoints - 1; col >= 0; col--)
			{
				el.push_back((row)* xPoints + (col));
				el.push_back((row + 1) * xPoints + (col));
			}
		}
	}
	numElements = GLuint(el.size());
	GLuint particleBuffer;
	glGenBuffers(1, &particleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);
	glBufferData(GL_ARRAY_BUFFER, particleVertexes.size() * sizeof(GLfloat), &particleVertexes[0], GL_STATIC_DRAW);
	GLuint elParticleBuffer;
	glGenBuffers(1, &elParticleBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, elParticleBuffer);
	glBufferData(GL_ARRAY_BUFFER, el.size() * sizeof(GLuint), &el[0], GL_STATIC_DRAW);
	/////

	glGenVertexArrays(1, &vaoHandle);
	glBindVertexArray(vaoHandle);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);

	glBindVertexBuffer(0, posBuf, 0, 4 * sizeof(GLfloat));
	glBindVertexBuffer(1, particleBuffer, 0, 3 * sizeof(GLfloat));
	

	glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat));
	glVertexAttribFormat(2, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat));

	glVertexAttribDivisor(0, 1);

	glVertexAttribBinding(0, 0);
	glVertexAttribBinding(2, 1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elParticleBuffer);
	glBindVertexArray(0);
}
//void clothShowInit(glm::ivec3 nParticles, GLuint &numElements, GLuint &clothVao, clothBuf &clBuf)
//{
//
//	glm::vec3 clothSize(4.0f, 3.0f, 0);
//
//	std::vector<GLfloat> initPos;
//	std::vector<GLfloat> initVel(nParticles.x * nParticles.y * 4, 0.0f);
//	float dx = clothSize.x / (nParticles.x - 1);
//	float dy = clothSize.y / (nParticles.y - 1);
//	for (int i = 0; i < nParticles.y; i++)
//	{
//		for (int j = 0; j < nParticles.x; j++)
//		{
//			initPos.push_back(dx * j);
//			initPos.push_back(dy * i);
//			initPos.push_back(0.0f);
//			initPos.push_back(1.0f);
//		}
//	}
//	std::vector<GLuint> el;
//	for (int row = 0; row < nParticles.y - 1; row++)
//	{
//		for (int col = 0; col < nParticles.x; col++)
//		{
//			el.push_back((row + 1) * nParticles.x + (col));
//			el.push_back((row)* nParticles.x + (col));
//		}
//		row++;
//		if (row < nParticles.y - 1)
//		{
//			for (int col = nParticles.x - 1; col >= 0; col--)
//			{
//				el.push_back((row)* nParticles.x + (col));
//				el.push_back((row + 1) * nParticles.x + (col));
//			}
//		}
//	}
//
//	GLuint bufs[7];
//	GLuint posBufs[2], velBufs[2], normBuf, elBuf;
//	glGenBuffers(7, bufs);
//	clBuf.posBufs[0] = bufs[0];
//	clBuf.posBufs[1] = bufs[1];
//	clBuf.velBufs[0] = bufs[2];
//	clBuf.velBufs[1] = bufs[3];
//	clBuf.normBuf = bufs[4];
//	elBuf = bufs[5];
//	GLuint parts = nParticles.x * nParticles.y;
//
//	//positions
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, clBuf.posBufs[0]);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), &initPos[0], GL_DYNAMIC_DRAW);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, clBuf.posBufs[1]);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
//
//	// Velocities
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, clBuf.velBufs[0]);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), &initVel[0], GL_DYNAMIC_COPY);
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, clBuf.velBufs[1]);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);
//
//	// Normal buffer
//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, clBuf.normBuf);
//	glBufferData(GL_SHADER_STORAGE_BUFFER, parts * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_COPY);
//
//	// Element indicies
//	glBindBuffer(GL_ARRAY_BUFFER, elBuf);
//	glBufferData(GL_ARRAY_BUFFER, el.size() * sizeof(GLuint), &el[0], GL_DYNAMIC_COPY);
//
//	numElements = GLuint(el.size());
//
//	glGenVertexArrays(1, &clothVao);
//	glBindVertexArray(clothVao);
//	glEnableVertexAttribArray(0);
//	glEnableVertexAttribArray(1);
//
//	glBindVertexBuffer(0, clBuf.posBufs[0], 0, 4 * sizeof(GLfloat));
//	glBindVertexBuffer(1, clBuf.normBuf, 0, 4 * sizeof(GLfloat));
//
//	glVertexAttribFormat(0, 4, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat));
//	glVertexAttribFormat(1, 4, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat));
//
//	glVertexAttribBinding(0, 0);
//	glVertexAttribBinding(1, 1);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elBuf);
//	glBindVertexArray(0);
//}