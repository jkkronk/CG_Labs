#include "airplane.hpp"
#include "../core/helpers.hpp"
#include "core/Misc.h"
#include <math.h>
Airplane::Airplane(GLuint* shader){
	std::vector<bonobo::mesh_data> const objects = bonobo::loadObjects("airplane.obj");
	if (objects.empty()) {
		printf("Failed to load the sphere geometry: exiting.\n");
	}
	auto airplane_texture = bonobo::loadTexture2D("airplane.png");
	bonobo::mesh_data const& airplane = objects.front();
	baseNode.set_geometry(airplane);
	baseNode.set_program(shader,  [](GLuint /*program*/) {});
	baseNode.add_texture("diffuse_texture", airplane_texture, GL_TEXTURE_2D);
	//baseNode.get_transform().RotateY(glm::pi<float>());
	for(size_t i = 1 ; i < objects.size(); i++){
		printf("%d\n", i);
	}
	
	printf("INIT\n");

}
void Airplane::render(glm::mat4 worldToClipMatrix){
	baseNode.render(worldToClipMatrix);
}
glm::vec3 Airplane::get_position() {
	return baseNode.get_transform().GetTranslation();
}
glm::vec3 Airplane::get_direction() {
	return direction;
}
void Airplane::update(InputHandler inputHandler) {
	if (inputHandler.GetKeycodeState(GLFW_KEY_SPACE) & PRESSED) {
		if (abs(velocity) < 2) {
			velocity += 0.01;
		}
	}
	else {
		velocity -= 0.02;
		if (velocity < 0) {
			velocity = 0;
		}
	}
	if (inputHandler.GetKeycodeState(GLFW_KEY_W) & PRESSED) {
		angleY -= 0.005;
		if (angleY < -0.5) {
			angleY = -0.5;
		}
		direction.z = -glm::cos(glm::pi<double>() * angleY);
		direction.y = glm::sin(glm::pi<double>() * angleY);
	}
	if (inputHandler.GetKeycodeState(GLFW_KEY_S) & PRESSED) {
		angleY += 0.005;
		if (angleY > 0.5) {
			angleY = 0.5;
		}
		direction.z = -glm::cos(glm::pi<double>() * angleY);
		direction.y = glm::sin(glm::pi<double>() * angleY);
	}
	if (inputHandler.GetKeycodeState(GLFW_KEY_A) & PRESSED) {
		angleZ += 0.005;
		if (angleZ > 0.5) {
			angleZ = 0.5;
		}
		direction.z = -glm::cos(glm::pi<double>() * angleY);
		direction.y = glm::sin(glm::pi<double>() * angleY);
	}
	baseNode.get_transform().SetRotateX(glm::pi<double>() * angleY);
	baseNode.get_transform().RotateY(glm::pi<double>() * angleZ);

	baseNode.get_transform().Translate(velocity*direction);
}
