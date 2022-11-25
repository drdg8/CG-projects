#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

#include "../base/vertex.h"
#include "transformation.h"
#include "tiny_obj_loader.h"

const std::string modelRelPath = "obj/bunny.obj";

Transformation::Transformation(const Options& options): Application(options) {
	// use tiny_obj_loader to load mesh data
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;

	std::string modelPath = getAssetFullPath(modelRelPath);
	std::string::size_type index = modelPath.find_last_of("/");
	std::string mtlBaseDir = modelPath.substr(0, index + 1);

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, modelPath.c_str(), mtlBaseDir.c_str())) {
		throw std::runtime_error("load " + modelPath + " failure: " + err);
	}

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			vertex.position.x = attrib.vertices[3 * index.vertex_index + 0];
			vertex.position.y = attrib.vertices[3 * index.vertex_index + 1];
			vertex.position.z = attrib.vertices[3 * index.vertex_index + 2];

			if (index.normal_index >= 0) {
				vertex.normal.x = attrib.normals[3 * index.normal_index + 0];
				vertex.normal.y = attrib.normals[3 * index.normal_index + 1];
				vertex.normal.z = attrib.normals[3 * index.normal_index + 2];
			}

			if (index.texcoord_index >= 0) {
				vertex.texCoord.x = attrib.texcoords[2 * index.texcoord_index + 0];
				vertex.texCoord.y = attrib.texcoords[2 * index.texcoord_index + 1];
			}

			// check if the vertex appeared before to reduce redundant data
			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
	// in this experiment we will not process with any material...
	// no more code for material preprocess

	// pass the data to the bunny
	for (int i = 0; i < 3; ++i) {
		_bunnies.push_back(Bunny(vertices, indices));
	}

	initShader();
}

//!wave to get a square wave with small = s, big = b, gia = k
float wave(float k, float s, float b){
	float time = (float)glfwGetTime();
	float mid = (s + b) / 2;
	float amp = (b - s) / 2;		// amplitude
	float wave = abs(fmod(k * time, 2 * 2 * amp) - 2 * amp) + mid - amp;
	return wave;
}

void Transformation::handleInput() {
	if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		glfwSetWindowShouldClose(_window, true);
		return ;
	}
	
	// update bunnies position / rotation / scale here
	const glm::vec3 velocity = { 0.0f, 2.0f, 0.0f };
	const float angulerVelocity = 1.0f;
	const float scaleRate = 0.2f;
	
	// TODO: update transformation attributes
	// write your code here
	// --------------------------------------------------

	float time = (float)glfwGetTime();
	// std::cout << time << std::endl;
	// std::cout << i - 1 << std::endl;		// size_t
	_positions[0].y = 5.0f * wave(0.5, -1, 1);
	_rotateAngles[1] = time;
	_scales[2] = wave(0.5, 0.5, 1.5) * glm::vec3(1.0f);

	// --------------------------------------------------
}

void Transformation::renderFrame() {
	showFpsInWindowTitle();

	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	// projection matrix transform the homogenous coordinates 
	// from view space to projection space, depending on following parameters:
	// @field of view
	constexpr float fovy = glm::radians(60.0f);
	// @aspect of the window
	const float aspect = 1.0f * _windowWidth / _windowHeight;
	// @near plane for clipping
	constexpr float znear = 0.1f;
	// @far plane for clipping
	constexpr float zfar = 100.0f;

	const glm::mat4 projection = glm::perspective(fovy, aspect, znear, zfar);

	// view matrix transform the homogenous coordinates 
	// from world space to view space (view of camera), depending on following parameters:
	// @camera position
	const glm::vec3 eye = { 0.0f, 0.0f, 15.0f };
	// @target, positon and target defines the direction the camera looking at
	const glm::vec3 target = { 0.0f, 0.0f, 0.0f };
	// @up, up vector of the camera
	const glm::vec3 up = { 0.0f, 1.0f, 0.0f };

	const glm::mat4 view = glm::lookAt(eye, target, up);

	_shader->use();
	for (std::size_t i = 0; i < _bunnies.size(); ++i) {
		_shader->setUniformMat4("projection", projection);
		_shader->setUniformMat4("view", view);
		// model matrix transform the homogenous coodinates from 
		// model space (raw vertex data of the model) to world space, depending on following parametes:
		// TODO: calculate the translation, rotation, scale matrices
		// change your code here
		// -----------------------------------------------

		// x_init = vec4(1.0f, 0, 0, 0);
		// y_init = vec4(0, 1.0f, 0, 0);
		// z_init = vec4(0, 0, 1.0f, 0);
		// w_init = vec4(0, 0, 0, 1.0f);
		// x_tran = vec4(1.0f, 0, 0, 0.5f * (i - 1));
		glm::mat4 mat4_init = glm::mat4(1.0f);

		// @translation

		// std::cout << _positions[i][1] << std::endl;
		// it is by column
		glm::mat4 translation = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			(float)_positions[i][0], (float)_positions[i][1], (float)_positions[i][2], 1.0f
		);

		// @rotation
		glm::mat4 rotation = mat4_init;
		// if (i == 1){
		// 	rotation = glm::rotate(mat4_init, time, glm::vec3(0.0f, 1.0f, 0.0f));
		// }
		rotation = glm::rotate(rotation, _rotateAngles[i], _rotateAxis[i]);

		// @scale
		glm::mat4 scale = mat4_init;
		// if (i == 2){
		// 	scale = glm::scale(mat4_init, glm::vec3(0.5f * wave(0.5, 1, 3)));
		// }
		scale = glm::scale(scale, _scales[i]);

		// ------------------------------------------------

		glm::mat4 model = translation * rotation * scale;
		_shader->setUniformMat4("model", model);

		_bunnies[i].draw();
	}
}

void Transformation::initShader() {
	const char* vsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"out vec3 worldPosition;\n"
		"out vec3 normal;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"void main() {\n"
		"	normal = mat3(transpose(inverse(model))) * aNormal;\n"
		"	worldPosition = vec3(model * vec4(aPosition, 1.0f));\n"
		"	gl_Position = projection * view * vec4(worldPosition, 1.0f);\n"
		"}\n";

	const char* fsCode =
		"#version 330 core\n"
		"in vec3 worldPosition;\n"
		"in vec3 normal;\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"vec3 lightPosition = vec3(100.0f, 100.0f, 100.0f);\n"
		"// ambient color\n"
		"float ka = 0.1f;\n"
		"vec3 objectColor = vec3(1.0f, 1.0f, 1.0f);\n"
		"vec3 ambient = ka * objectColor;\n"
		"// diffuse color\n"
		"float kd = 0.8f;\n"
		"vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);\n"
		"vec3 lightDirection = normalize(lightPosition - worldPosition);\n"
		"vec3 diffuse = kd * lightColor * max(dot(normalize(normal), lightDirection), 0.0f);\n"
		"fragColor = vec4(ambient + diffuse, 1.0f);\n"
		"}\n";

	_shader.reset(new GLSLProgram);
	_shader->attachVertexShader(vsCode);
	_shader->attachFragmentShader(fsCode);
	_shader->link();
}