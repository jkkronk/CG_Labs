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
	glm::vec3 get_direction();
	void update(InputHandler);
	//void set_position();
private:
	Node baseNode;
	float velocity = 0;
	glm::vec3 direction = glm::vec3(0.0,0.0,-1.0);
	float angleY = 0;
	float angleZ = 0;

};
