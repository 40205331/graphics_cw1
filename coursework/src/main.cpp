#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;


effect eff;
effect skybox_eff;
effect point_eff;
effect direction_eff;
mesh skybox_mesh;
free_camera cam;
target_camera cam2;
map<string, mesh> meshes;
map<string, texture> textures;
bool cams = true;
point_light p_light;
static float light_off;
static float light_on;
directional_light d_light;



cubemap cube_map;

double cursor_x;
double cursor_y;

bool initialise() {
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	return true;
}

bool load_content() {
	//skybox:
	skybox_mesh = mesh(geometry_builder::create_box(vec3(10.0f, 10.0f, 10.0f)));
	skybox_mesh.get_transform().scale = vec3(100.0f, 100.0f, 100.0f);
	array<string, 6> filenames = { "textures/hell_ft.tga", "textures/hell_bk.tga", "textures/hell_up.tga",
		"textures/hell_dn.tga", "textures/hell_rt.tga", "textures/hell_lf.tga" };

	cube_map = cubemap(filenames);

	skybox_eff.add_shader("shaders/skybox.vert", GL_VERTEX_SHADER);
	skybox_eff.add_shader("shaders/skybox.frag", GL_FRAGMENT_SHADER);
	// Build Effect
	skybox_eff.build();

	// Load direction light
	d_light.set_ambient_intensity(vec4(0.1f, 0.1f, 0.1f, 1.0f));
	d_light.set_light_colour(vec4(0.1f, 0.2f, 0.2f, 1.0f));
	d_light.set_direction(normalize(vec3(1.0f, 1.0f, -1.0f)));

	//direction_eff.add_shader("shaders/direction.frag", GL_FRAGMENT_SHADER);
	//direction_eff.build();

	// Load point light
	p_light.set_position(vec3(20.0f, 10.0f, 0.0f));
	p_light.set_light_colour(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	p_light.set_range(200.0f);

	point_eff.add_shader("shaders/point.frag", GL_FRAGMENT_SHADER);
	point_eff.add_shader("shaders/point.vert", GL_VERTEX_SHADER);
	point_eff.build();

	// Target camera properties
	cam2.set_position(vec3(0.0f, 10.0f, 0.0f));
	cam2.set_target(vec3(10.0f, 10.0f, 10.0f));
	cam2.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);



	//Models:
	meshes["plane"] = mesh(geometry_builder::create_plane(50.0, 50.0));
	meshes["plane"].get_transform().position = vec3(0.0f, 0.0f, 0.0f);
	meshes["alduin"] = mesh(geometry("models/alduin.obj"));
	meshes["alduin"].get_transform().scale = vec3(0.05f, 0.05f, 0.05f);
	meshes["alduin"].get_transform().translate(vec3(0.0f, 0.0f, 0.0f));
	//Temp
	

	textures["plane"] = texture("textures/check_1.png");
	textures["alduin"] = texture("textures/alduin.jpg");
	// Load in shaders
	//eff.add_shader("shaders/simple_texture.frag", GL_FRAGMENT_SHADER);
	//eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);
	// Build effect
	//eff.build();

	// Set camera properties
	cam.set_position(vec3(0.0f, 10.0f, 10.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	return true;
}


bool update(float delta_time) {
	// The ratio of pixels to rotation - remember the FOV
	static double ratio_width = quarter_pi<float>() / static_cast<float>(renderer::get_screen_width());
	static double ratio_height =
		(quarter_pi<float>() *
		(static_cast<float>(renderer::get_screen_height()) / static_cast<float>(renderer::get_screen_width()))) /
		static_cast<float>(renderer::get_screen_height());

	double current_x;
	double current_y;

	// Get the current cursor position
	glfwGetCursorPos(renderer::get_window(), &current_x, &current_y);
	// Calculate delta of cursor positions from last frame
	double delta_x = current_x - cursor_x;
	double delta_y = cursor_y - current_y;

	delta_x = delta_x * ratio_width;
	delta_y = delta_y * ratio_height;

	cam.rotate(delta_x, delta_y);
	// Use keyboard to move the camera - WSAD
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_W)) {
		cam.move(vec3(0.0f, 0.0f, 0.5f));
		cams = true;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_A)) {
		cam.move(vec3(-0.5f, 0.0f, 0.0f));
		cams = true;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_S)) {
		cam.move(vec3(0.0f, 0.0f, -0.5));
		cams = true;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_D)) {
		cam.move(vec3(0.5f, 0.0f, 0.0f));
		cams = true;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_K)) {
		cam2.set_position(vec3(0.0f, 50.0f, 0.0f));
		cams = false;
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_L)) {
		light_off = 0.0f;
			p_light.set_range(light_off);
	}
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_O)) {
		light_on = 100.0f;
			p_light.set_range(light_on);
	}
	// Update the camera
	cam.update(delta_time);
	cam2.update(delta_time);
	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	// Update cursor pos
	cursor_x = current_x;
	cursor_y = current_y;

	return true;  
}

bool render() {
	// Render Skybox
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	renderer::bind(skybox_eff);

	mat4 M = skybox_mesh.get_transform().get_transform_matrix();
	auto V = cam.get_view();
	auto P = cam.get_projection();
	auto MVP = P * V * M;

	glUniformMatrix4fv(skybox_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

	renderer::bind(cube_map, 0);
	glUniform1i(skybox_eff.get_uniform_location("cubemap"), 0);

	renderer::render(skybox_mesh);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST); 
	glDepthMask(GL_TRUE);

	//Render Objects
	for (auto &e : meshes) {
		auto m = e.second;
		renderer::bind(point_eff);

		//M = mat4(1.0f);
		M = m.get_transform().get_transform_matrix();
		if (cams == true)
		{
			V = cam.get_view();
			P = cam.get_projection();
		}
		else
		{
			V = cam2.get_view();
			P = cam2.get_projection();
		}
		MVP = P * V * M;
		// set MVP uniform
		glUniformMatrix4fv(point_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
		// set light uniform
		glUniformMatrix4fv(point_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(M));
		glUniformMatrix3fv(point_eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
		renderer::bind(m.get_material(), "mat");
		renderer::bind(p_light, "point");
		renderer::bind(d_light, "directional");
		renderer::bind(textures[e.first], 0);
		glUniform1i(point_eff.get_uniform_location("tex"), 0);
		glUniform3fv(point_eff.get_uniform_location("position"), 1, value_ptr(cam.get_position()));
		// render the mesh
		renderer::render(m);
		}

	return true;
}

void main() {
  // Create application
  app application("Graphics Coursework");
  // Set load content, update and render methods
  application.set_load_content(load_content);
  application.set_update(update);
  application.set_render(render);
  application.set_initialise(initialise);
  // Run application
  application.run();
}