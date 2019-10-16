#pragma once
#include "../core/node.hpp"
#include <glm/glm.hpp>

class Airplane{
public:
	Airplane(GLuint* shader);
	~Airplane() = default;
	void render(glm::mat4);
	Node baseNode;
};