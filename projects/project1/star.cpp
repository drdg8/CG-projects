#include <cmath>
#include <iostream>
#include "star.h"

Star::Star(const glm::vec2& position, float rotation, float radius, float aspect)
	: _position(position), _rotation(rotation), _radius(radius) {
	// TODO: assemble the vertex data of the star
	// write your code here
	// -------------------------------------
	// assign the radiu is the outer radiu and the width long
	// 18 = pi/10 
	// 钝角顶点半径
	float rv = _radius*(cos(M_PI/10)*tan(M_PI/5) - sin(M_PI/10));
	for (int i = 0; i < 3; ++i) {
		_vertices.push_back({_position.x + _radius*sin(_rotation + i*2*M_PI/5)/aspect, _position.y + _radius*cos(_rotation + i*2*M_PI/5)});
		_vertices.push_back({_position.x + rv*sin(M_PI*3/5 + _rotation + i*2*M_PI/5)/aspect, _position.y + rv*cos(M_PI*3/5 + _rotation + i*2*M_PI/5)});
		_vertices.push_back({_position.x + _radius*sin(2*M_PI*3/5 + _rotation + i*2*M_PI/5)/aspect, _position.y + _radius*cos(2*M_PI*3/5 + _rotation + i*2*M_PI/5)});
	}
	for (int i = 0; i < 9; i++)
		std::cout << _vertices[i].x << " " << _vertices[i].y << "\n";

	// -------------------------------------

	glGenVertexArrays(1, &_vao);
	glGenBuffers(1, &_vbo);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

Star::Star(Star&& rhs) noexcept
	: _position(rhs._position), _rotation(rhs._rotation), _radius(rhs._radius),
	_vao(rhs._vao), _vbo(rhs._vbo) {
	rhs._vao = 0;
	rhs._vbo = 0;
}

Star::~Star() {
	if (_vbo) {
		glDeleteVertexArrays(1, &_vbo);
		_vbo = 0;
	}

	if (_vao) {
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
}

void Star::draw() const {
	// std::cout << _position.x << " " << _position.y << " "
	// 			<< _rotation << " "
	// 			<< _radius << "\n";
	glBindVertexArray(_vao);
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(_vertices.size()));
}