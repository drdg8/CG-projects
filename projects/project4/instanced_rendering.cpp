#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "instanced_rendering.h"

const std::string planetRelPath = "obj/sphere.obj";
const std::string asternoidRelPath = "obj/rock.obj";

InstancedRendering::InstancedRendering(const Options& options): Application(options) {
	/* init imgui */
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init();

	/* import models */
	_planet.reset(new Model(getAssetFullPath(planetRelPath)));
	_planet->transform.scale = glm::vec3(10.0f, 10.0f, 10.0f);

	_asternoid.reset(new Model(getAssetFullPath(asternoidRelPath)));

	// init camera
	_camera.reset(new PerspectiveCamera(
		glm::radians(45.0f),
		1.0f * _windowWidth / _windowHeight,
		0.1f, 10000.0f));

	_camera->transform.position = glm::vec3(0.0f, 25.0f, 100.0f);
	_camera->transform.rotation = glm::angleAxis(-glm::radians(20.0f), _camera->transform.getRight());

	/* shader issues */
	initShaders();

	constexpr float radius = 50.0f;
	constexpr float offset = 10.0f;
	for (int i = 0; i < _amount; ++i) {
		glm::mat4 model(1.0f);
		// translate
		float angle = (float)i / (float)_amount * 360.0f;
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float x = sin(angle) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.2f;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(angle) * radius + displacement;
		model = glm::translate(model, glm::vec3(x, y, z));

		// scale
		float scale = (rand() % 20) / 100.0f + 0.05f;
		model = glm::scale(model, glm::vec3(scale));

		// rotate
		float rotAngle = 1.0f * (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		_modelMatrices.push_back(model);
	}

	// TODO: configure the instanced buffer and transfer the matrix data to GPU
	// write your code here
	// ---------------------------------------------------------
	// glXXX(_instanceBuffer); ... 

    glGenBuffers(1, &_instanceBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _instanceBuffer);
    glBufferData(GL_ARRAY_BUFFER, _amount * sizeof(glm::mat4), &_modelMatrices[0], GL_STATIC_DRAW);

    // for (unsigned int i = 0; i < _amount; i++)
    // {
	unsigned int VAO = _asternoid->getVao();
	glBindVertexArray(VAO);
	// set attribute pointers for matrix (4 times vec4)
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
    // }

	// ---------------------------------------------------------
}

InstancedRendering::~InstancedRendering() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void InstancedRendering::initShaders() {
	const char* planetVsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"
		"void main() {\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";

	const char* planetFsCode =
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	FragColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);\n"
		"}\n";

	_planetShader.reset(new GLSLProgram);
	_planetShader->attachVertexShader(planetVsCode);
	_planetShader->attachFragmentShader(planetFsCode);
	_planetShader->link();

	const char* asternoidVsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"
		"void main() {\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";

	const char* asternoidInstancedVsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 3) in mat4 aInstanceMatrix;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"void main() {\n"
		"	gl_Position = projection * view * aInstanceMatrix * vec4(aPosition, 1.0f);\n"
		"}\n";

	const char* asternoidFsCode =
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main() {\n"
		"	FragColor = vec4(0.8f, 0.8f, 0.8f, 1.0f);\n"
		"}\n";

	_asternoidShader.reset(new GLSLProgram);
	_asternoidShader->attachVertexShader(asternoidVsCode); 
	_asternoidShader->attachFragmentShader(asternoidFsCode);
	_asternoidShader->link();

	_asternoidInstancedShader.reset(new GLSLProgram);
	_asternoidInstancedShader->attachVertexShader(asternoidInstancedVsCode);
	_asternoidInstancedShader->attachFragmentShader(asternoidFsCode);
	_asternoidInstancedShader->link();
}

void InstancedRendering::handleInput() {
	if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		glfwSetWindowShouldClose(_window, true);
		return;
	}

	if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
		_camera->transform.position += _camera->transform.getFront() * _cameraMoveSpeed * _deltaTime;
	}

	if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
		_camera->transform.position -= _camera->transform.getRight() * _cameraMoveSpeed * _deltaTime;
	}

	if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
		_camera->transform.position -= _camera->transform.getFront() * _cameraMoveSpeed * _deltaTime;
	}

	if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
		_camera->transform.position += _camera->transform.getRight() * _cameraMoveSpeed * _deltaTime;
	}
}

void InstancedRendering::renderFrame() {
	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// get camera properties
	glm::mat4 projection = _camera->getProjectionMatrix();
	glm::mat4 view = _camera->getViewMatrix();

	// draw planet
	_planetShader->use();
	_planetShader->setUniformMat4("model", _planet->transform.getLocalMatrix());
	_planetShader->setUniformMat4("view", view);
	_planetShader->setUniformMat4("projection", projection);
	_planet->draw();

	// draw asternoids
	switch (_renderMode) {
	case RenderMode::Ordinary:
		_asternoidShader->use();
		_asternoidShader->setUniformMat4("view", view);
		_asternoidShader->setUniformMat4("projection", projection);
		for (int i = 0; i < _amount; ++i) {
			_asternoidShader->setUniformMat4("model", _modelMatrices[i]);
			_asternoid->draw();
		}
		break;
	case RenderMode::Instanced:
		_asternoidInstancedShader->use();
		_asternoidInstancedShader->setUniformMat4("view", view);
		_asternoidInstancedShader->setUniformMat4("projection", projection);

		// TODO: draw the asternoids by the instance rendering method
		// write your code here
		// ---------------------------------------------------------
		// ...
		// glDraw...(...);

		// _asternoidInstancedShader->setUniformMat4("model", _modelMatrices[0]);
		// for (unsigned int i = 0; i < _amount; i++)
  //       {
		// 	_asternoidInstancedShader->setUniformMat4("model", _modelMatrices[i]);
  //       }
		glBindVertexArray(_asternoid->getVao());
		glDrawElementsInstanced(GL_TRIANGLES, static_cast<unsigned int>(3*_asternoid->getFaceCount()), GL_UNSIGNED_INT, 0, _amount);
		glBindVertexArray(0);
		// ---------------------------------------------------------
		break;
	}

	// draw ui
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	const auto flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings;

	if (!ImGui::Begin("Control Panel", nullptr, flags)) {
		ImGui::End();
	} else {
		ImGui::Text("render method");
		ImGui::Separator();
		ImGui::RadioButton("ordinary rendering", (int*)&_renderMode, (int)(RenderMode::Ordinary));
		ImGui::RadioButton("instanced rendering", (int*)&_renderMode, (int)(RenderMode::Instanced));
		ImGui::Checkbox("wireframe", &_wireframe);
		ImGui::NewLine();

		std::string fpsInfo = "avg fps: " + std::to_string(_fpsIndicator.getAverageFrameRate());
		ImGui::Text("%s", fpsInfo.c_str());
		ImGui::PlotLines("", _fpsIndicator.getDataPtr(), _fpsIndicator.getSize(), 0,
			nullptr, 0.0f, std::numeric_limits<float>::max(), ImVec2(240.0f, 50.0f));

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}