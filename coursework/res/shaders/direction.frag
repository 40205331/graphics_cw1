// Directional light structure
#ifndef DIRECTIONAL_LIGHT
#define DIRECTIONAL_LIGHT
struct directional_light
{
	vec4 ambient_intensity;
	vec4 light_colour;
	vec3 light_dir;
};
#endif

// A material structure
#ifndef MATERIAL
#define MATERIAL
struct material
{
	vec4 emissive;
	vec4 diffuse_reflection;
	vec4 specular_reflection;
	float shininess;
};
#endif

// Calculates the directional light
vec4 calculate_direction(in directional_light light, in material mat, in vec3 normal, in vec3 view_dir, in vec4 tex_colour)
{

	// Calculate ambient component
	vec4 ambient = mat.diffuse_reflection * light.ambient_intensity;

	// Calculate diffuse component
	float kd = max(dot(normal, light.light_dir), 0.0);
  vec4 diffuse = kd * (mat.diffuse_reflection * light.light_colour);

	// Calculate normalized half vector 
	vec3 H = normalize(light.light_dir + view_dir);

	// Calculate specular component
	float ks = pow(max(dot(H, normal), 0.0), mat.shininess);
  vec4 specular = ks * (light.light_colour * mat.specular_reflection);

	// Calculate colour to return
	vec4 colour = ((mat.emissive + ambient + diffuse) * tex_colour) + specular;
	colour.a = 1.0;

	// Return colour
	return colour;

}
