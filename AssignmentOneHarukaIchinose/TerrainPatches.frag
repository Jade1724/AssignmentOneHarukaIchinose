#version 330

//in vec4 lighting;
//in vec4 posn;
uniform sampler2D rock;

in vec2 geomTcord;
in vec4 lighting;

void main() 
{
	vec4 texColor1 = texture(rock, geomTcord);  //Get colour from texture  

    gl_FragColor = texColor1 * lighting;
}
