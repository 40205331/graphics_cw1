#include <glm\glm.hpp>
#include <graphics_framework.h>

using namespace std;
using namespace graphics_framework;
using namespace glm;


effect eff;
effect skybox_eff;
effect point_eff;
effect mask_eff;
effect grey_eff;
mesh skybox_mesh;
free_camera cam;
target_camera cam2;
map<string, mesh> meshes;
map<string, texture> textures;
bool cams = true;
bool rise;
point_light p_light;
static float light_off;
static float light_on;
frame_buffer frame;
texture tex;
texture mask;
geometry screen_quad;

map < string, int > listofeffects;

cubemap cube_map;
      
double cursor_x;
double cursor_y;

bool initialise() {
	glfwSetInputMode(renderer::get_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwGetCursorPos(renderer::get_window(), &cursor_x, &cursor_y);
	return true;
}

bool load_content() {
	frame = frame_buffer(renderer::get_screen_width(), renderer::get_screen_height());
	// create screen quad
	vector<vec3> positions{ vec3(-1.0f, -1.0f, 0.0f), vec3(1.0f, -1.0f, 0.0f), vec3(-1.0f, 1.0f, 0.0f), vec3(1.0f, 1.0f, 0.0f) };
	vector<vec2> tex_coords{ vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(0.0f, 1.0f), vec2(1.0f, 1.0f) };
	screen_quad.add_buffer(positions, BUFFER_INDEXES::POSITION_BUFFER);
	screen_quad.add_buffer(tex_coords, BUFFER_INDEXES::TEXTURE_COORDS_0);
	screen_quad.set_type(GL_TRIANGLE_STRIP);

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

	// greyscale shaders
	grey_eff.add_shader("shaders/simple_texture.vert", GL_VERTEX_SHADER);
	grey_eff.add_shader("shaders/greyscale.frag", GL_FRAGMENT_SHADER);
	grey_eff.build();

	// masking shaders
	mask_eff.add_shader("shaders/pshader.vert", GL_VERTEX_SHADER);
	mask_eff.add_shader("shaders/mask.frag", GL_FRAGMENT_SHADER);
	mask_eff.build();

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
	meshes["plane"] = mesh(geometry_builder::create_plane(100.0, 100.0));
	meshes["plane"].get_transform().position = vec3(0.0f, 0.0f, 0.0f);
	meshes["alduin"] = mesh(geometry("models/alduin.obj"));
	meshes["alduin"].get_transform().scale = vec3(0.05f, 0.05f, 0.05f);
	meshes["alduin"].get_transform().translate(vec3(0.0f, 0.0f, 0.0f));
	meshes["pyramid"] = mesh(geometry("models/pyramid.obj"));
	meshes["pyramid"].get_transform().scale = vec3(0.5f, 0.5f, 0.5f);
	meshes["pyramid"].get_transform().translate(vec3(-100.0f, 0.0f, 0.0f));
	meshes["eye"] = mesh(geometry("models/eye.obj"));
	meshes["eye"].get_transform().scale = vec3(0.25f, 0.25f, 0.25f);
	meshes["eye"].get_transform().translate(vec3(-100.0f, 90.0f, 0.0f));
	meshes["eye"].get_transform().rotate(vec3(0.0f, pi<float>(), 0.0f));

	//Textures:
	textures["plane"] = texture("textures/lava.jpg");
	textures["alduin"] = texture("textures/alduin.jpg");
	textures["pyramid"] = texture("textures/pyramid.jpg");
	textures["eye"] = texture("textures/eye-texture.jpg");
	mask = texture("textures/mask.jpg");

	// Set camera properties
	cam.set_position(vec3(0.0f, 10.0f, 10.0f));
	cam.set_target(vec3(0.0f, 0.0f, 0.0f));
	cam.set_projection(quarter_pi<float>(), renderer::get_screen_aspect(), 0.1f, 1000.0f);
	return true;

	// bools for model movement
	rise = true;
}

void maskeffect()
{
	renderer::set_render_target();
	//bind the masking effect
	renderer::bind(mask_eff);
	// MVP is identity matrix
	auto MVP = mat4(1.0f);
	// set MVP matrix uniform
	glUniformMatrix4fv(mask_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// bind texture to frame buffer
	renderer::bind(frame.get_frame(), 0);
	//bind mask to alpha map
	glUniform1i(mask_eff.get_uniform_location("tex"), 0);
	renderer::bind(mask, 1);
	glUniform1i(mask_eff.get_uniform_location("alpha_map"), 1);
	// render screen quad
	renderer::render(screen_quad);
}

void redscale()
{
	renderer::set_render_target(frame);
	renderer::set_render_target();
	//Bind tex effect
	renderer::bind(grey_eff);
	// MVP is now the identity matrix
	auto MVP = mat4(1.0f);
	// Set MVP matrix uniform
	glUniformMatrix4fv(grey_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
	// Bind texture from frame buffer
	renderer::bind(frame.get_frame(), 1);
	// Set the tex uniform
	glUniform1i(grey_eff.get_uniform_location("tex"), 1);
	// render the screen
	renderer::render(screen_quad);
}


bool update(float delta_time) {
	cout << 1.0 / delta_time << endl;
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
	// set W to move forward
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_W)) {
		cam.move(vec3(0.0f, 0.0f, 0.5f));
		cams = true;
	}
	// set A to move backwards
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_A)) {
		cam.move(vec3(-0.5f, 0.0f, 0.0f));
		cams = true;
	}
	// set S to move left
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_S)) {
		cam.move(vec3(0.0f, 0.0f, -0.5));
		cams = true;
	}
	// set D to move right
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_D)) {
		cam.move(vec3(0.5f, 0.0f, 0.0f));
		cams = true;
	}

	// switch to target camera
	// set K to turn on target camera
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_K)) {
		cam2.set_position(vec3(0.0f, 50.0f, 0.0f));
		cams = false;
	}

	// activate lights 
	// set L to turn spot light off
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_L)) {
		light_off = 0.0f;
			p_light.set_range(light_off);
	}
	// set O to turn spot light on
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_O)) {
		light_on = 100.0f;
			p_light.set_range(light_on);
	}

	// activate wireframe
	// set F toturn wireframe on 
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_F)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	//set J turn wire frame off
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_J)) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	// activate greyscale
	// set G to turn greyscale off
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_G)) {
		listofeffects["grey_eff"] = 0;
	}
	// set T to turn wireframe on
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_T)) {
		listofeffects["grey_eff"] = 1;
	}

	// activate masking effect
	// set M to turn greyscale off
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_M)) {
		listofeffects["mask_eff"] = 0;
	}
	// set N to turn masking on
	if (glfwGetKey(renderer::get_window(), GLFW_KEY_N)) {
		listofeffects["mask_eff"] = 1;
	}

	// make the dragon model take off
	if ((rise) && (meshes["alduin"].get_transform().position.y <= 10.0))
	{
		meshes["alduin"].get_transform().position.y += 0.1;
	}
	if ((!rise) && (meshes["alduin"].get_transform().position.y >= 0.0))
	{
		meshes["alduin"].get_transform().position.y -= 0.1;
	}
	if (meshes["alduin"].get_transform().position.y >= 10.0)
	{
		rise = false;
		meshes["alduin"].get_transform().position.y -= 0.1;
	}
	if (meshes["alduin"].get_transform().position.y <= 0.0)
	{
		rise = true;
		meshes["alduin"].get_transform().position.y += 0.1;
	}

	meshes["eye"].get_transform().rotate(vec3(0.0f, half_pi<float>()/50.0f, 0.0f));
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
	// clear frame
	// set render target to frame buffer
	if (listofeffects["mask_eff"] == 1)
	{
		renderer::set_render_target(frame);
		//clear frame
		renderer::clear();
	}
	

	// Render Skybox
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	renderer::bind(skybox_eff);

	mat4 MB = skybox_mesh.get_transform().get_transform_matrix();
	auto VB = cam.get_view();
	auto PB = cam.get_projection();
	
	if (cams == true)
	{
		VB = cam.get_view();
		PB = cam.get_projection();
	}
	else
	{
		VB = cam2.get_view();
		PB = cam2.get_projection();
	}
	auto MBVBPB = PB * VB * MB;
	glUniformMatrix4fv(skybox_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MBVBPB));

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
		MB = m.get_transform().get_transform_matrix();
		
		MBVBPB = PB * VB * MB;
		// set MVP uniform
		glUniformMatrix4fv(point_eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MBVBPB));
		// set light uniform
		glUniformMatrix4fv(point_eff.get_uniform_location("M"), 1, GL_FALSE, value_ptr(MB));
		glUniformMatrix3fv(point_eff.get_uniform_location("N"), 1, GL_FALSE, value_ptr(m.get_transform().get_normal_matrix()));
		renderer::bind(m.get_material(), "mat");
		renderer::bind(p_light, "point");
		
		renderer::bind(textures[e.first], 0);
		glUniform1i(point_eff.get_uniform_location("tex"), 0);
		glUniform3fv(point_eff.get_uniform_location("position"), 1, value_ptr(cam.get_position()));
		// render the mesh
		renderer::render(m);
		}
	if (listofeffects["grey_eff"] == 1)
	{
		redscale();
	}
	if (listofeffects["mask_eff"] == 1)
	{
		maskeffect();
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