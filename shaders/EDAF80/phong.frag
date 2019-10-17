#version 410

uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float shininess;


in VS_OUT {
	vec3 vertex;
	vec3 normal;
    vec2 tex_coord;
} fs_in;

out vec4 frag_color;

void main()
{
	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 V = normalize(camera_position - fs_in.vertex);
    vec3 diffuse_color = diffuse;
    

	diffuse_color = diffuse_color * max(dot(normalize(fs_in.normal), L),0.0f);
	vec3 specular = specular * pow( max(dot(reflect(-L,normalize(fs_in.normal)),V),0),shininess) ;

	frag_color = vec4(ambient + diffuse_color + specular, 1.0);// + specular; 
}
