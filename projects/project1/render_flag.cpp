#include "render_flag.h"

RenderFlag::RenderFlag(const Options& options): Application(options) {
	// create star shader
	const char* vsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec2 aPosition;\n"
		"void main() {\n"
		"	gl_Position = vec4(aPosition, 0.0f, 1.0f);\n"
		"}\n";

	const char* fsCode =
		"#version 330 core\n"
		"out vec4 fragColor;\n"
		"void main() {\n"
		"	fragColor = vec4(1.0f, 0.870f, 0.0f, 1.0f);\n"
		"}\n";

	_starShader.reset(new GLSLProgram);
	_starShader->attachVertexShader(vsCode);
	_starShader->attachFragmentShader(fsCode);
	_starShader->link();

	// TODO: create 5 stars
	// hint: aspect_of_the_window = _windowWidth / _windowHeight
	// write your code here
	// ---------------------------------------------------------------
	// _stars[i].reset(new Star(ndc_position, rotation_in_radians, size_of_star, aspect_of_the_window));

	// 300*200
	// (100, 20), (120, 40), (120, 70), (100, 90) r=10 (50, 50) r=30
	// arctan(5.0/3)+M_PI_2 arctan(7)+M_PI_2 0 -arctan(1.25)

	// _stars[0].reset(new Star({0, 0}, 0, 0.3, 1.5));
	_stars[0].reset(new Star({-2/3.0, 0.5}, 0, 0.3, 1.5));
	_stars[1].reset(new Star({-1/3.0, 0.8}, atan(5.0/3)+M_PI, 0.1, 1.5));
	_stars[2].reset(new Star({-0.2, 0.6}, atan(7)+M_PI, 0.1, 1.5));
	_stars[3].reset(new Star({-0.2, 0.3}, 0, 0.1, 1.5));
	_stars[4].reset(new Star({-1/3.0, 0.1}, -atan(1.25), 0.1, 1.5));

	// ---------------------------------------------------------------
}

void RenderFlag::handleInput() {
	if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		glfwSetWindowShouldClose(_window, true);
		return ;
	}
}

void RenderFlag::renderFrame() {
	showFpsInWindowTitle();

	// we use background as the flag
	glClearColor(0.87f, 0.161f, 0.063f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	_starShader->use();
	for (int i = 0; i < 5; ++i) {
		if (_stars[i] != nullptr) {
			_stars[i]->draw();
			// std::cout << "ok\n";
		}
	}
}