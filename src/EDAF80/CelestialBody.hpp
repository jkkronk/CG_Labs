
#include "../core/helpers.hpp"
#include "../core/node.hpp"




namespace edaf80
{
	//! \brief Wrapper class for CelestialBody
	class CelestialBody {
	public:
		//! \brief Default constructor.
		//!
		//! @param shape contains information about the geometry used to model the celestial body
		//! @param program is the shader program used to render the celestial body
		//! @param diffuse_texture_id is the identifier of the diffuse texture used
		CelestialBody(bonobo::mesh_data const& shape, GLuint const* program, GLuint diffuse_texture_id);

		//! \brief Default destructor.
		//!
		//! It will release the bonobo modules initialised by the
		//! constructor, as well as the window.
		~CelestialBody() = default;

		//! \brief Contains the logic of the assignment, along with the
		//! render loop.
		void run();

	private:
		Node  _node;
	};

	//! \brief render
	//!
	//! @param ellapsed_time is the amount of time between two frames
	//! @param view_projection is the matrix for going from world - space to clipspace
	//! @param  parent_transform transforms from the local - space of your parent, to world - space, and defaults to the identity matrix if unspecified.
	bonobo::mesh_data render(float ellapsed_time, glm::mat4 const& view_projection, glm::mat4 const& parent_transform = glm::mat4(1.0f));

	
	
}
