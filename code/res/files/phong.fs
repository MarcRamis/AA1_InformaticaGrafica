	#version 330
	in vec4 vert_Normal;
	in vec4 fragPos;
	out vec4 out_Color;
	uniform vec4 objColor;
	uniform vec4 dir_light; 
	uniform vec4 ambient_strength;
	uniform vec4 ambient_color; 
	uniform vec4 diffuse_strength;
	uniform vec4 diffuse_color; 
	//uniform vec4 specular;
	//uniform vec4 specular_color;
	//uniform vec4 viewPos; 
	//uniform float shininess;
	void main() {
		vec4 ambientComp = ambient_color * ambient_strength; 
		//vec4 diffuseComp = dot(vert_Normal, normalize(dir_light)) * diffuse_strength * diffuse_color;
		//vec4 viewDir = viewPos - fragPos;
		//vec4 reflectDir = reflect(normalize(-dir_light), vert_Normal);
		//vec4 specularComp = pow(max(dot(normalize(viewDir), reflectDir),0.0),shininess) * specular * specular_color;

		out_Color = objColor * (ambientComp);
	};