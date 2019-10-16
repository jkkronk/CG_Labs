#include "assignment5.hpp"
#include "interpolation.hpp"
#include "parametric_shapes.hpp"
#include "airplane.hpp"
#include "config.hpp"
#include "core/Bonobo.h"
#include "core/FPSCamera.h"
#include "core/helpers.hpp"
#include "core/Misc.h"
#include "core/ShaderProgramManager.hpp"

#include <imgui.h>
#include <external/imgui_impl_glfw_gl3.h>
#include <tinyfiledialogs.h>
#include <array>
#include <stdexcept>
#include "../core/node.hpp"
#include <string>
#include <stack>
#include <iostream>
#include <ctime>
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>





edaf80::Assignment5::Assignment5(WindowManager& windowManager) :
	mCamera(0.5f* glm::half_pi<float>(),
		static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
		0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0 };

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 5", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}
}

void
edaf80::Assignment5::run()
{
	// Set up the camera
	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 0.025;

	auto camera_position = mCamera.mWorld.GetTranslation();


	// Create the shader programs
	ShaderProgramManager program_manager;
	GLuint fallback_shader = 0u;
	program_manager.CreateAndRegisterProgram("Fallback",
		{ { ShaderType::vertex, "EDAF80/fallback.vert" },
		  { ShaderType::fragment, "EDAF80/fallback.frag" } },
		fallback_shader);
	if (fallback_shader == 0u) {
		LogError("Failed to load fallback shader");
		return;
	}
	GLuint default_shader = 0u;
	program_manager.CreateAndRegisterProgram("default",
		{ { ShaderType::vertex, "EDAF80/default.vert" },
		  { ShaderType::fragment, "EDAF80/default.frag" } },
		default_shader);
	if (default_shader == 0u) {
		LogError("Failed to load default shader");
		return;
	}
	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//
	GLuint water_shader = 0u;
	program_manager.CreateAndRegisterProgram("Water",
		{ { ShaderType::vertex, "EDAF80/water.vert" },
		  { ShaderType::fragment, "EDAF80/water.frag" } },
		water_shader);
	if (water_shader == 0u) {
		LogError("Failed to load water shader");
		return;
	}
	GLuint skybox_shader = 0u;
	program_manager.CreateAndRegisterProgram("Skybox",
		{ { ShaderType::vertex, "EDAF80/cube_map.vert" },
		  { ShaderType::fragment, "EDAF80/cube_map.frag" } },
		skybox_shader);
	if (skybox_shader == 0u) {
		LogError("Failed to load cube map");
		return;
	}
	GLuint phong_shader = 0u;
	program_manager.CreateAndRegisterProgram("Phong",
		{ { ShaderType::vertex, "EDAF80/phong.vert" },
		  { ShaderType::fragment, "EDAF80/phong.frag" } },
		phong_shader);
	if (phong_shader == 0u)
		LogError("Failed to load phong shader");

	//
	// Todo: Load your geometry
	//

	// Water
	glm::vec3 deep_color = glm::vec3(0.0f, 0.0f, 0.3f);
	glm::vec3 shallow_color = glm::vec3(0.0f, 0.5f, 0.5f);

	float amplitudes[4] = { 1.0,0.5,0.0,0.0 };
	float frequencies[4] = { 0.2,0.4,0.0,0.0 };
	float phases[4] = { 0.5,1.3,0.0,0.0 };
	float sharpness[4] = { 2.0, 2.0,0.0,0.0 };
	glm::vec2 direction[4] = { glm::vec2(-1.0,0.0),glm::vec2(-0.7,0.7),glm::vec2(0.0),glm::vec2(0.0) };
	int nbr_of_waves = 2;

	float time = 0.0f;
	auto const water_set_uniforms = [&amplitudes, &frequencies, &sharpness, &phases, &direction,
		&time, &nbr_of_waves, &deep_color, &shallow_color, &camera_position](GLuint program) {
		glUniform1fv(glGetUniformLocation(program, "amplitudes"), 4, amplitudes);
		glUniform1fv(glGetUniformLocation(program, "frequencies"), 4, frequencies);
		glUniform1fv(glGetUniformLocation(program, "sharpness"), 4, sharpness);
		glUniform1fv(glGetUniformLocation(program, "phases"), 4, phases);
		glUniform2fv(glGetUniformLocation(program, "directions"), 4, glm::value_ptr(direction[0]));
		glUniform1f(glGetUniformLocation(program, "time"), time);
		glUniform1i(glGetUniformLocation(program, "nbr_of_waves"), nbr_of_waves);
		glUniform3fv(glGetUniformLocation(program, "deep_color"), 1, glm::value_ptr(deep_color));
		glUniform3fv(glGetUniformLocation(program, "shallow_color"), 1, glm::value_ptr(shallow_color));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
	};

	auto skybox_id = bonobo::loadTextureCubeMap("sea_sky/posx.jpg", "sea_sky/negx.jpg",
		"sea_sky/posy.jpg", "sea_sky/negy.jpg",
		"sea_sky/negz.jpg", "sea_sky/posz.jpg", true);
	auto water_normal_id = bonobo::loadTexture2D("waves.png");
	auto sphere_sky = parametric_shapes::createSphere(400, 400, 400);
	auto plane = parametric_shapes::createQuadXZ(800, 800);
	auto water = Node();
	water.set_geometry(plane);
	water.set_program(&water_shader, water_set_uniforms);
	water.get_transform().SetTranslate(glm::vec3(-400.0f, -10.0f, -400.0f));
	water.add_texture("skybox", skybox_id, GL_TEXTURE_CUBE_MAP);
	water.add_texture("normal_map", water_normal_id, GL_TEXTURE_2D);

	// Sky
	auto skybox = Node();
	skybox.set_geometry(sphere_sky);
	skybox.set_program(&skybox_shader, [](GLuint /*program*/) {});
	skybox.add_texture("skybox", skybox_id, GL_TEXTURE_CUBE_MAP);

	
	Airplane airplane = Airplane(&default_shader);


	//Path
	float catmull_rom_tension = 0.8f;
	
	glm::vec3 interpts[7] = {
								glm::vec3(0.0f,0.0f,0.0f),
								glm::vec3(0.0f,0.0f,0.0f),
								glm::vec3(0.0f,0.0f,0.0f),
								glm::vec3(0.0f,0.0f,0.0f),
								glm::vec3(0.0f,0.0f,0.0f),
								glm::vec3(0.0f,0.0f,0.0f),
								glm::vec3(0.0f,0.0f,0.0f)
	};
	
	int x_next = 0.0f;
	int y_next = 0.0f;
	int z_next = -40.0f;

	for (int i = 0; i < (sizeof(interpts) / sizeof(glm::vec3)); i++) {
		interpts[i] = glm::vec3(x_next, y_next, z_next);

		x_next += -75 + (std::rand() % (150 + 75 + 1));
		y_next += -25 + (std::rand() % (50 + 25 + 1));
		z_next += -75 + (std::rand() % (150 + 75 + 1));

		if (y_next < 5) {
			y_next = 5.0f;
		}
	}
	
	// TORUS
	int const inner_radii = 6.0f;
	auto light_position = glm::vec3(-2.0f, inner_radii+2, inner_radii);
	auto const set_uniforms = [&light_position](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
	};
	
	auto ambient = glm::vec3(0.95f, 0.1f, 0.1f);
	auto diffuse = glm::vec3(1.0f, 0.2f, 0.2f);
	auto specular = glm::vec3(1.0f, 1.0f, 1.0f);
	auto shininess = 0.5f;
	
	auto const phong_set_uniforms = [&light_position, &camera_position, &ambient, &diffuse, &specular, &shininess](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(program, "ambient"), 1, glm::value_ptr(ambient));
		glUniform3fv(glGetUniformLocation(program, "diffuse"), 1, glm::value_ptr(diffuse));
		glUniform3fv(glGetUniformLocation(program, "specular"), 1, glm::value_ptr(specular));
		glUniform1f(glGetUniformLocation(program, "shininess"), shininess);
	};

	auto shape_torus = parametric_shapes::createTorus(20.0f, 20.0f, 2.0f, 0.5f);

	int const torus_per_part = 3;
	int const nbr_torus = torus_per_part * (sizeof(interpts) / sizeof(glm::vec3)-2);
	std::vector<Node> torus_rings(nbr_torus);

	for (int i = 0; i < nbr_torus; i++) {
		torus_rings[i].set_geometry(shape_torus);
		torus_rings[i].set_program(&phong_shader, phong_set_uniforms); 
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		torus_rings[i].get_transform().RotateY(r*3); // start w different rotations
	}


	// set path for torus
	float x = 0.0f;
	for (int i = 0; i < nbr_torus; i++) { 
		int idx = static_cast<int>(x);
		int size = sizeof(interpts) / sizeof(glm::vec3);
		glm::vec3 translation = interpolation::evalCatmullRom(interpts[idx], interpts[(idx) % size], interpts[(idx + 1) % size], interpts[(idx + 2) % size], catmull_rom_tension, x - idx);
		x += (float)1/ (float)torus_per_part;
		torus_rings[i].get_transform().SetTranslate(translation);
	}

	// GREEN TORUS
	int next_node = 0;
	int done_node = 0;

	auto ambient_green = glm::vec3(0.1f, 0.95f, 0.1f);
	auto diffuse_green = glm::vec3(0.2f, 1.0f, 0.2f);
	auto const phong_set_uniforms_green = [&light_position, &camera_position, &ambient_green, &diffuse_green, &specular, &shininess](GLuint program) {
		glUniform3fv(glGetUniformLocation(program, "light_position"), 1, glm::value_ptr(light_position));
		glUniform3fv(glGetUniformLocation(program, "camera_position"), 1, glm::value_ptr(camera_position));
		glUniform3fv(glGetUniformLocation(program, "ambient"), 1, glm::value_ptr(ambient_green));
		glUniform3fv(glGetUniformLocation(program, "diffuse"), 1, glm::value_ptr(diffuse_green));
		glUniform3fv(glGetUniformLocation(program, "specular"), 1, glm::value_ptr(specular));
		glUniform1f(glGetUniformLocation(program, "shininess"), shininess);
	};

	torus_rings[0].set_program(&phong_shader, phong_set_uniforms_green);

	// MISC
	int score = 0;
	int plane_radii = 2.0f;

	glEnable(GL_DEPTH_TEST);

	// Enable face culling to improve performance:
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	//glCullFace(GL_BACK);

	f64 ddeltatime;
	size_t fpsSamples = 0;
	double nowTime, lastTime = GetTimeMilliseconds();
	double fpsNextTick = lastTime + 1000.0;

	bool show_logs = true;
	bool show_gui = true;
	bool shader_reload_failed = false;

	while (!glfwWindowShouldClose(window)) {
		nowTime = GetTimeMilliseconds();
		ddeltatime = nowTime - lastTime;
		if (nowTime > fpsNextTick) {
			fpsNextTick += 1000.0;
			fpsSamples = 0;
		}
		fpsSamples++;
		time += ddeltatime / 1000.0f;
		auto& io = ImGui::GetIO();
		inputHandler.SetUICapture(io.WantCaptureMouse, io.WantCaptureKeyboard);

		glfwPollEvents();
		inputHandler.Advance();
		mCamera.Update(ddeltatime, inputHandler);
		camera_position = mCamera.mWorld.GetTranslation();

		if (inputHandler.GetKeycodeState(GLFW_KEY_F3) & JUST_RELEASED)
			show_logs = !show_logs;
		if (inputHandler.GetKeycodeState(GLFW_KEY_F2) & JUST_RELEASED)
			show_gui = !show_gui;
		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
					"An error occurred while reloading shader programs; see the logs for details.\n"
					"Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
					"error");
		}

		ImGui_ImplGlfwGL3_NewFrame();

		// ROTATE torus
		for (int i = 0; i < nbr_torus; i++) {
			torus_rings[i].get_transform().RotateY(0.01f); // maybe rotate so every torus allways is directed towards plane?
		}

		// If plane is in goal torus --> make next torus green and set as goal
		glm::vec3 distance_vec = torus_rings[next_node].get_transform().GetTranslation() - camera_position;
		float distance = sqrt(dot(distance_vec, distance_vec));

		if (distance < plane_radii + inner_radii) {
			torus_rings[next_node].set_program(&phong_shader, phong_set_uniforms);

			next_node += 1;
			score += 1;
			std::cout << score ;  // print score

			if (next_node > nbr_torus - 1) {
				next_node = 0;
			}
			std::cout << next_node;
			torus_rings[next_node].set_program(&phong_shader, phong_set_uniforms_green);
			
		}

		//
		// Todo: If you need to handle inputs, you can do it here
		//

		int framebuffer_width, framebuffer_height;
		glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
		glViewport(0, 0, framebuffer_width, framebuffer_height);
		glClearDepthf(1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		if (!shader_reload_failed) {
			water.render(mCamera.GetWorldToClipMatrix());
			skybox.render(mCamera.GetWorldToClipMatrix());
			airplane.render(mCamera.GetWorldToClipMatrix());
			for (int i = 0; i < nbr_torus; i++) {
				//torus_rings[i].render(mCamera.GetWorldToClipMatrix());
			}

		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


		//
		// Todo: If you want a custom ImGUI window, you can set it up
		//       here
		//



		if (show_logs)
			Log::View::Render();
		if (show_gui)
			ImGui::Render();

		glfwSwapBuffers(window);
		lastTime = nowTime;
	}
}

int main()
{
	Bonobo framework;

	try {
		edaf80::Assignment5 assignment5(framework.GetWindowManager());
		assignment5.run();
	}
	catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}

