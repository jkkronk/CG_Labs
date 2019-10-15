#include "assignment4.hpp"
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
#include "parametric_shapes.hpp"
#include "../core/node.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <string>

edaf80::Assignment4::Assignment4(WindowManager& windowManager) :
	mCamera(0.5f * glm::half_pi<float>(),
	        static_cast<float>(config::resolution_x) / static_cast<float>(config::resolution_y),
	        0.01f, 1000.0f),
	inputHandler(), mWindowManager(windowManager), window(nullptr)
{
	WindowManager::WindowDatum window_datum{ inputHandler, mCamera, config::resolution_x, config::resolution_y, 0, 0, 0, 0};

	window = mWindowManager.CreateGLFWWindow("EDAF80: Assignment 4", window_datum, config::msaa_rate);
	if (window == nullptr) {
		throw std::runtime_error("Failed to get a window: aborting!");
	}
}

void
edaf80::Assignment4::run()
{
	glm::vec3 deep_color = glm::vec3(0.0f,0.0f,0.3f);
	glm::vec3 shallow_color = glm::vec3(0.0f,0.5f,0.5f);
	
	float amplitudes[4] = {1.0,0.5,0.0,0.0};
	float frequencies[4] = {0.2,0.4,0.0,0.0};
	float phases[4] = {0.5,1.3,0.0,0.0};
	float sharpness[4] = {2.0, 2.0,0.0,0.0};
	glm::vec2 direction[4] = {glm::vec2(-1.0,0.0),glm::vec2(-0.7,0.7),glm::vec2(0.0),glm::vec2(0.0)};
	int nbr_of_waves = 2;
	// Set up the camera
	//mCamera.mWorld.SetTranslate(glm::vec3(0.0f, 10.0f, 0.0f));
	//mCamera.mWorld.SetRotateX(-glm::pi<float>()/2);

	mCamera.mMouseSensitivity = 0.003f;
	mCamera.mMovementSpeed = 0.025;

	auto camera_position = mCamera.mWorld.GetTranslation();

	

	// Create the shader programs
	ShaderProgramManager program_manager;
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

	//
	// Todo: Insert the creation of other shader programs.
	//       (Check how it was done in assignment 3.)
	//

	//
	// Todo: Load your geometry
	//

	float time = 0.0f;
	auto const water_set_uniforms = [&amplitudes,&frequencies,&sharpness,&phases,&direction,
		&time,&nbr_of_waves,&deep_color,&shallow_color,&camera_position](GLuint program){
		glUniform1fv(glGetUniformLocation(program, "amplitudes"), 4, amplitudes);
		glUniform1fv(glGetUniformLocation(program, "frequencies"), 4,frequencies);
		glUniform1fv(glGetUniformLocation(program, "sharpness"), 4, sharpness);
		glUniform1fv(glGetUniformLocation(program, "phases"), 4, phases);
		glUniform2fv(glGetUniformLocation(program, "directions"), 4, glm::value_ptr(direction[0]));
		glUniform1f(glGetUniformLocation(program, "time"), time);
		glUniform1i(glGetUniformLocation(program, "nbr_of_waves"), nbr_of_waves);
		glUniform3fv(glGetUniformLocation(program, "deep_color"),1, glm::value_ptr(deep_color));
		glUniform3fv(glGetUniformLocation(program, "shallow_color"),1, glm::value_ptr(shallow_color));
		glUniform3fv(glGetUniformLocation(program, "camera_position"),1, glm::value_ptr(camera_position));
	};

	auto skybox_id = bonobo::loadTextureCubeMap("cloudyhills/posx.png", "cloudyhills/negx.png",
		"cloudyhills/posy.png", "cloudyhills/negy.png",
		"cloudyhills/posz.png", "cloudyhills/negz.png", true);
	auto water_normal_id = bonobo::loadTexture2D("waves.png");
	auto sphere = parametric_shapes::createSphere(25,25,25);
	auto plane  = parametric_shapes::createQuadXZ(50,50);
	auto water  = Node();
	water.set_geometry(plane);
	water.set_program(&water_shader, water_set_uniforms);
	water.get_transform().SetTranslate(glm::vec3(-25.0f,0.0f,-25.0f));
	water.add_texture("skybox", skybox_id, GL_TEXTURE_CUBE_MAP);
	water.add_texture("normal_map", water_normal_id, GL_TEXTURE_2D);

	auto skybox  = Node();
	skybox.set_geometry(sphere);
	skybox.set_program(&skybox_shader,  [](GLuint /*program*/){});
	skybox.add_texture("skybox", skybox_id, GL_TEXTURE_CUBE_MAP);

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
		time += ddeltatime/1000.0f;
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
		}

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		bool opened = ImGui::Begin("Scene Control", &opened, ImVec2(300, 100), -1.0f, 0);
		ImGui::ColorEdit3("Deep", glm::value_ptr(deep_color));
		ImGui::ColorEdit3("Shallow", glm::value_ptr(shallow_color));
		ImGui::Separator();
		float* p = amplitudes;
		for(size_t i = 0; i < nbr_of_waves; ++i){

			ImGui::SliderFloat((std::string("Amplitude ") + std::to_string(i+1)).c_str(), &amplitudes[i], 0.0f, 10.0f);
			ImGui::SliderFloat((std::string("Frequency ") + std::to_string(i+1)).c_str(), &frequencies[i], 0.0f, 10.0f);
			ImGui::SliderFloat((std::string("Phase ") + std::to_string(i+1)).c_str(), &phases[i], 0.0f, 10.0f);
			ImGui::SliderFloat((std::string("Sharpness ") + std::to_string(i+1)).c_str(), &sharpness[i], 0.0f, 10.0f);
			ImGui::SliderFloat2((std::string("Direction ") + std::to_string(i+1)).c_str(), glm::value_ptr(direction[i]), -1.0, 1.0);
			ImGui::Separator();

		}
		if(nbr_of_waves < 4){
			if (ImGui::Button("Add wave")){
				frequencies[nbr_of_waves] = 0.0f;
				sharpness[nbr_of_waves] = 0.0f;
				amplitudes[nbr_of_waves] = 0.0f;
				phases[nbr_of_waves] = 0.0f;
				direction[nbr_of_waves] = glm::vec2(0.0);
				nbr_of_waves++;

			}
		}
		if(nbr_of_waves > 0){
			if(ImGui::Button("Remove wave")){
					nbr_of_waves--;
					//nbr_of_waves = nbr_of_waves < 0 ? 0 : nbr_of_waves; 
			}
		}

		if (inputHandler.GetKeycodeState(GLFW_KEY_R) & JUST_PRESSED) {
			shader_reload_failed = !program_manager.ReloadAllPrograms();
			if (shader_reload_failed)
				tinyfd_notifyPopup("Shader Program Reload Error",
				                   "An error occurred while reloading shader programs; see the logs for details.\n"
				                   "Rendering is suspended until the issue is solved. Once fixed, just reload the shaders again.",
				                   "error");
		}
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
		edaf80::Assignment4 assignment4(framework.GetWindowManager());
		assignment4.run();
	} catch (std::runtime_error const& e) {
		LogError(e.what());
	}
}
