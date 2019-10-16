#include "airplane.hpp"
#include "../core/helpers.hpp"
#include "core/Misc.h"

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