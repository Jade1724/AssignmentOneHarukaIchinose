#version 330

//in vec4 lighting;
//in vec4 posn;
uniform sampler2D rock;
uniform sampler2D water;
uniform sampler2D grass;
uniform sampler2D snow;

in vec2 geomTcord;
in vec4 lighting;
in vec4 texWeight;

void main() 
{
	vec4 rockColor = texture(rock, geomTcord);  //Get colour from texture 
	vec4 waterColor = texture(water, geomTcord);
	vec4 grassColor = texture(grass, geomTcord);
	vec4 snowColor = texture(snow, geomTcord);

    gl_FragColor = lighting * (waterColor * texWeight[0] + grassColor * texWeight[1] + rockColor * texWeight[2] + snowColor * texWeight[3]);

}
