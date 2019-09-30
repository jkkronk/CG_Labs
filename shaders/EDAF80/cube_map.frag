#version 410

in VS_OUT {
	vec3 normal;
} fs_in;

out vec4 frag_color;
uniform samplerCube cube_map;
void main()
{
	frag_color = texture(cube_map, fs_in.normal);
}
