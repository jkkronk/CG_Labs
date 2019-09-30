#version 410

out vec4 frag_color;
uniform sampler2D normal_map;
uniform sampler2D diffuse_map;
uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;
uniform mat4 normal_model_to_world;

in VS_OUT {
	vec3 vertex;
	vec3 tangent;
	vec3 normal;
	vec3 binormal;
	vec2 texcoord;
} fs_in;

void main()
{
	mat3 TBN = mat3(normalize(fs_in.tangent),normalize(fs_in.binormal), normalize(fs_in.normal));

	vec3 normal = texture(normal_map, fs_in.texcoord).xyz*2 - 1;
	vec4 normal_world = normal_model_to_world * vec4(TBN * normal, 0.0f);
	normal = normalize(normal_world.xyz);
	
	
	vec3 L = normalize(light_position - fs_in.vertex);
	
	vec3 V = normalize(camera_position - fs_in.vertex);

	vec4 diffuse = texture(diffuse_map, fs_in.texcoord) * max(dot(normal, L),0.0f);
	vec4 specular =  vec4(specular, 1.0) * max(pow(dot(reflect(-L,normal),V),shininess),0);


	frag_color = vec4(ambient, 1.0) + diffuse + specular;
}
