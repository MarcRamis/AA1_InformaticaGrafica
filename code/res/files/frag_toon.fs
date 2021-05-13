#version 330
	
	// Inputs
	in vec4 vert_Normal;
	in vec4 fragPos;
	in vec2 texCoord;
	
	// Outputs
	out vec4 out_Color;
	
	// Uniforms
	uniform vec4 objColor;	// Object color
	uniform vec4 dir_light; // Light pos
	uniform vec4 viewPos;	// Camera pos
	uniform sampler2D ourTexture; // Texture
	
		// Ambient Components
	uniform vec4 ambient_strength;
	uniform vec4 ambient_color;

		// Diffuse Components
	uniform vec4 diffuse_strength;
	uniform vec4 diffuse_color;

		// Specular Components
	//uniform vec4 specular_strength;
	//uniform vec4 specular_color;
	//uniform float shininess;
	
		// Displace Effect
	uniform bool isMatrix;
	uniform float displaceX;
	uniform float displaceY;
	
		// Toon Shader
	uniform bool isToon;
	//uniform float conditional_1;
	//uniform float parameter_1;
	//uniform float conditional_2;
	//uniform float parameter_2;
	//uniform float conditional_12;
	//uniform float parameter_2;
	
	void main() 
	{
		// ----- TOON SHADING
		float u = max(dot(normalize(dir_light),normalize(vert_Normal)),0.0);
		if(u >= 0.5)
		{
			u = 1.0;
		}
		else if(u >= 0.4)
		{
			u = 0.6;
		}
		else if(u >= 0.2)
		{
			u = 0.3;
		}
		else
		{
			u = 0.0;
		}
		// --- Ambient
		vec4 ambientComp = ambient_color * ambient_strength;
		// --- Diffuse	
		vec4 diffuseComp = u * diffuse_strength * diffuse_color;
		// --- Specular
		//vec4 viewDir = viewPos - fragPos;
		//vec4 reflectDir = reflect(normalize(-dir_light), vert_Normal);
		//vec4 specularComp = pow(max(dot(normalize(viewDir), reflectDir),0.0),shininess) * specular_strength * specular_color;

		// --- Texture
		//vec4 textureColor = texture(ourTexture,texCoord);
		out_Color = objColor * (ambientComp + diffuseComp);
	};