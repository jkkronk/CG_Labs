#pragma once
#include "../core/node.hpp"
#include <glm/glm.hpp>
#include "core/InputHandler.h"

class Airplane{
public:
	Airplane(GLuint* shader);
	~Airplane() = default;
	void render(glm::mat4);
	glm::vec3 get_position();
	void update(InputHandler);
	//void set_position();
private:
	Node baseNode;
	glm::vec3 velocity = glm::vec3(0.0,0.0,0.0);
};
