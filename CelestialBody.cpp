#include "CelestialBody.hpp"
#include "../core/helpers.hpp"

CelestialBody::CelestialBody(bonobo::mesh_data const& shape, GLuint const* program, GLuint diffuse_texture_id)
{
	_node.set_geometry(shape);
	_node.set_program(program, [](GLuint) {});
	_node.add_texture("diffuse_texture", diffuse_texture_id, GL_TEXTURE_2D);
	_spinning_speed = 0;
	_spin_angle = 0;
	_spinning_axis = glm::vec3(0, 1, 0);
	_scale = glm::vec3(1);
}

void CelestialBody::render(float ellapsed_time, glm::mat4 const& view_projection, glm::mat4 const& parent_transform)
{
	_spin_angle += _spinning_speed * ellapsed_time;
	glm::mat4 scale_transform = glm::scale(glm::mat4(1.0f), _scale);
	glm::mat4 rot_transform = glm::rotate(glm::mat4(1.f), _spin_angle, _spinning_axis);

	glm::mat4 transform = scale_transform * rot_transform;

	_node.render(view_projection, transform);

}

void CelestialBody::set_scale(glm::vec3 const& scale) {
	_scale = scale;
}

void CelestialBody::set_spinning(glm::vec3 const& spinning_axis, float spinning_speed, float initial_spin_angle) {
	_spinning_axis = spinning_axis;
	_spin_angle = initial_spin_angle;
	_spinning_speed = spinning_speed;

}

