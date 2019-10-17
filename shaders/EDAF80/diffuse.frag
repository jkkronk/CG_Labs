#version 410

uniform vec3 light_position;
uniform int has_diffuse_texture;
uniform sampler2D diffuse_texture;
uniform vec3 ambient;
in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 diffuse_color = vec3(1.0);
	diffuse_color = texture(diffuse_texture, fs_in.texcoord).xyz;
	
	frag_color = vec4(ambient + diffuse_color * clamp(dot(normalize(fs_in.normal), L), 0.0, 1.0),1.0);

}
