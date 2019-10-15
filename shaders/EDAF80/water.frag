#version 410

uniform sampler2D normal_map;
uniform samplerCube skybox;
uniform vec3 deep_color;
uniform vec3 shallow_color;
uniform vec3 camera_position;
uniform float time;
uniform mat4 normal_model_to_world;

in VS_OUT {
	vec3 vertex;
	vec3 normal_water;
	vec3 tangent_water;
	vec3 binormal_water;
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

void main()
{
	mat3 TBN_water = mat3(normalize(fs_in.tangent_water), normalize(fs_in.binormal_water), normalize(fs_in.normal_water));
	vec2 texScale = vec2(8, 4);
	float normalTime = mod(time, 100.0);
	vec2 normalSpeed = vec2(-0.05, 0);
	vec3 normal = vec3(0.0);

	for(int i = 0; i < 3; i++){
		vec2 texcoord;
		if(i == 0){
			texcoord = fs_in.texcoord*texScale + normalTime*normalSpeed;
		}else{
			texcoord = fs_in.texcoord*texScale*pow(2,i) + normalTime*normalSpeed*pow(2,i+1);
		}
		normal += texture(normal_map, texcoord).xyz * 2 - 1;
	}
	normal = normalize(normal);
	normal = (normal_model_to_world * vec4(TBN_water * normal,0)).xyz;
	normal = normalize(normal);
	vec3 V = normalize(camera_position - fs_in.vertex);
	float facing = 1 - max(dot(V,normalize(normal)),0.0);
	vec3 water_color = mix(deep_color, shallow_color, facing);
	vec3 reflection  = texture(skybox, reflect(-V, normal)).xyz;
	float r0 = 0.02037;
	float n1 = 1.33;
	float n2 = 1.0/1.33;
	vec3 refraction  = texture(skybox, refract(-V,normal, n2)).xyz;

	float fastFresnel = r0 + (1-r0)*pow(1-dot(V,normal),5);
	frag_color = vec4(water_color + reflection*fastFresnel + refraction*(1-fastFresnel) ,1.0);
}
