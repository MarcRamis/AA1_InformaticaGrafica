#version 330
	out vec4 out_Color;
	uniform vec4 objColor;
	void main() 
	{
		if(mod(gl_FragCoord.x,5) > 0.5 && mod(gl_FragCoord.y,5) > 0.5)
		{
			discard;
		}
		out_Color = objColor; 
	};