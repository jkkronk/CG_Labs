#include "interpolation.hpp"
#include <iostream>

glm::vec3 interpolation::evalLERP(glm::vec3 const& p0, glm::vec3 const& p1, float const x)
{
	//! \todo Implement this function
	glm::mat2 t = glm::mat2(1, 0, -1, 1);
	glm::mat2x3 a = glm::mat2x3(p0, p1);
	glm::vec2 b = glm::vec2(1, x);
	return (a * t) * b;
}

glm::vec3
interpolation::evalCatmullRom(glm::vec3 const& p0, glm::vec3 const& p1,
                              glm::vec3 const& p2, glm::vec3 const& p3,
                              float const t, float const x)
{
	glm::mat4 ta = glm::mat4(0,1,0,0,-t,0,t,0,2*t,t-3,3-2*t,-t,-t,2-t,t-2,t);
	glm::mat4x3 a = glm::mat4x3(p0, p1, p2, p3);
	glm::vec4 b = glm::vec4(1, x, x*x,x*x*x);
	return (a * ta)* b;
}
