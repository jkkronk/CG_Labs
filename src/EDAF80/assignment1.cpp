#include "config.hpp"
#include "parametric_shapes.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/Misc.h"
#include "core/node.hpp"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>
#include <external/imgui_impl_glfw_gl3.h>
#include "CelestialBody.hpp"
#include <stack>

#include <cstdlib>


int main()
{
	//
	// Set up the framework
	//
	Bonobo framework;

	//
	// Set up the camera
	//
	InputHandler input_handler;
	FPSCameraf camera(0.5f * glm::half_pi<float>(),
	                  static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	                  0.01f, 1000.0f);
	camera.mWorld.SetTranslate(glm::vec3(0.0f, 0.0f, 6.0f));
	camera.mMouseSensitivity = 0.003f;
	camera.mMovementSpeed = 0.25f * 12.0f;

	//
	// Create the window
	//
	WindowManager& window_manager = framework.GetWindowManager();
	WindowManager::WindowDatum window_datum{ input_handler, camera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};
	GLFWwindow* window = window_manager.CreateGLFWWindow("EDAF80: Assignment 1", window_datum, config::msaa_rate);
	if (window == nullptr) {
		LogError("Failed to get a window: exiting.");

		return EXIT_FAILURE;
	}

	//
	// Load the sphere geometry
	//
	std::vector<bonobo::mesh_data> const objects = bonobo::loadObjects("sphere.obj");
	if (objects.empty()) {
		LogError("Failed to load the sphere geometry: exiting.");

		return EXIT_FAILURE;
	}
	bonobo::mesh_data const& sphere = objects.front();


	//
	// Create the shader program
	//
	ShaderProgramManager program_manager;
	GLuint celestial_body_shader = 0u;
	program_manager.CreateAndRegisterProgram("Celestial Body",
	                                         { { ShaderType::vertex, "EDAF80/default.vert" },
	                                           { ShaderType::fragment, "EDAF80/default.frag" } },
	                                         celestial_body_shader);
	if (celestial_body_shader == 0u) {
		LogError("Failed to generate the “Celestial Body” shader program: exiting.");

		Log::View::Destroy();
		Log::Destroy();

		return EXIT_FAILURE;
	}
	GLuint celestial_ring_shader = 0u;
	program_manager.CreateAndRegisterProgram("Celestial Ring",
	                                         { { ShaderType::vertex, "EDAF80/celestial_ring.vert" },
	                                           { ShaderType::fragment, "EDAF80/celestial_ring.frag" } },
	                                         celestial_ring_shader);
	if (celestial_ring_shader == 0u) {
		LogError("Failed to generate the “Celestial Ring” shader program: exiting.");

		Log::View::Destroy();
		Log::Destroy();

		return EXIT_FAILURE;
	}


	//
	// Set up the sun node and other related attributes
	//

	Node sun_translate_node;

	Node sun_node;
	sun_node.set_geometry(sphere);
	sun_node.set_program(&celestial_body_shader, [](GLuint /*program*/){});
	TRSTransformf& sun_transform_reference = sun_node.get_transform();
	GLuint const sun_texture = bonobo::loadTexture2D("sunmap.png");
	sun_node.add_texture("diffuse_texture", sun_texture, GL_TEXTURE_2D);
	float const sun_spin_speed = glm::two_pi<float>() / 6.0f; // Full rotation in six seconds
	
	//CelestialBody sun_node = CelestialBody(sphere, &celestial_body_shader, sun_texture);
	//sun_node.set_scale(glm::vec3(1, 0.2, 1));
	//sun_node.set_spinning(glm::vec3(1, 0, 0), glm::pi<float>(), 0);

	Node earth_rotate_node;

	Node earth_translate_node;
	earth_translate_node.get_transform().SetTranslate(glm::vec3(2, 0, 0));

	Node earth_node;
	earth_node.set_geometry(sphere);
	earth_node.set_program(&celestial_body_shader, [](GLuint /*program*/) {});
	GLuint const earth_texture = bonobo::loadTexture2D("earthmap.png");
	earth_node.add_texture("diffuse_texture", earth_texture, GL_TEXTURE_2D);
	earth_node.get_transform().SetScale(glm::vec3(0.25, 0.25, 0.25));

	Node moon_node;
	moon_node.set_geometry(sphere);
	moon_node.set_program(&celestial_body_shader, [](GLuint /*program*/) {});
	GLuint const moon_texture = bonobo::loadTexture2D("moonmap.png");
	moon_node.add_texture("diffuse_texture", moon_texture, GL_TEXTURE_2D);
	moon_node.get_transform().SetTranslate(glm::vec3(3.5, 0, 0));
	moon_node.get_transform().SetScale(glm::vec3(0.25, 0.25, 0.25));


	Node solar_system_node;
	earth_rotate_node.add_child(&earth_translate_node);
	earth_translate_node.add_child(&earth_node);
	earth_translate_node.add_child(&moon_node);
	sun_translate_node.add_child(&earth_rotate_node);
	sun_translate_node.add_child(&sun_node);
	solar_system_node.add_child(&sun_translate_node);

	


	//
	// TODO: Create nodes for the remaining of the solar system
	//


	// Retrieve the actual framebuffer size: for HiDPI monitors, you might
	// end up with a framebuffer larger than what you actually asked for.
	// For example, if you ask for a 1920x1080 framebuffer, you might get a
	// 3840x2160 one instead.
	int framebuffer_width, framebuffer_height;
	glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

	glViewport(0, 0, framebuffer_width, framebuffer_height);
	glClearDepthf(1.0f);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_DEPTH_TEST);


	size_t fpsSamples = 0;
	double lastTime = GetTimeSeconds();
	double fpsNextTick = lastTime + 1.0;


	bool show_logs = true;
	bool show_gui = true;
	bool printed = false;
	sun_translate_node.get_transform().SetTranslate(glm::vec3(2, 0, 0));
	while (!glfwWindowShouldClose(window)) {
		//
		// Compute timings information
		//
		double const nowTime = GetTimeSeconds();
		double const delta_time = nowTime - lastTime;
		lastTime = nowTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1.0;
			fpsSamples = 0;
		}
		++fpsSamples;


		//
		// Process inputs
		//
		glfwPollEvents();

		ImGuiIO const& io = ImGui::GetIO();
		input_handler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);
		input_handler.Advance();
		camera.Update(delta_time, input_handler);

		if (input_handler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (input_handler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;


		//
		// Start a new frame for Dear ImGui
		//
		ImGui_ImplGlfwGL3_NewFrame();


		//
		// Clear the screen
		//
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		//
		// Update the transforms
		//
		sun_transform_reference.RotateY(sun_spin_speed * delta_time);


		//
		// Traverse the scene graph and render all nodes
		//
		std::stack<Node const*> node_stack({ &solar_system_node });
		std::stack<glm::mat4> matrix_stack({ glm::mat4(1.0f) });
		// TODO: Replace this explicit rendering of the Sun with a
		// traversal of the scene graph and rendering of all its nodes.
		while (!node_stack.empty()) {
			const Node* n = node_stack.top();
			glm::mat4 world_transform = matrix_stack.top();
			glm::mat4 transform = world_transform * n->get_transform().GetMatrix();
			if (!printed) {
				std::cout << transform << std::endl;
			}
			node_stack.pop();
			matrix_stack.pop();
			n->render(camera.GetWorldToClipMatrix(), world_transform);
			for (size_t i = 0; i < n->get_children_nb(); ++i) {
				node_stack.push(n->get_child(i));
				matrix_stack.push(transform);
			}
		}
		printed = true;
		//sun_node.render(camera.GetWorldToClipMatrix());
		
		//
		// Display Dear ImGui windows
		//
		if (show_logs)
			Log::View::Render();
		if (show_gui)
			ImGui::Render();


		//
		// Queue the computed frame for display on screen
		//
		glfwSwapBuffers(window);
	}

	glDeleteTextures(1, &sun_texture);
	glDeleteTextures(1, &earth_texture);
	glDeleteTextures(1, &moon_texture);

	return EXIT_SUCCESS;
}
