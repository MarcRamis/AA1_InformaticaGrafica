#version 330
	
	// Inputs
	in vec4 vert_Normal;
	in vec4 fragPos;
	in vec2 texCoord;
	
	// Outputs
	out vec4 out_Color;
	
	// Uniforms
	uniform vec4 objColor;
	
		// Displace Effect
	uniform bool isMatrix;
	uniform float displaceX;
	uniform float displaceY;
	
	void main() 
	{
		out_Color = objColor;
		
		if(mod(gl_FragCoord.x,displaceX) > 0.5 && mod(gl_FragCoord.y,displaceY) > 0.5)
		{
			discard;
		}
	};