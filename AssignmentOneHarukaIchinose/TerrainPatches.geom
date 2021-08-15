#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;
uniform float waterLevel;
uniform float snowLevel;

in ES_OUT
{
    vec2 tcoord;
    float height;
} es_in[];

out vec4 lighting;
out vec2 geomTcord;
out vec4 texWeight;

vec3 GetNormal()
{
    vec3 a = vec3(gl_in[0].gl_Position.xyz) - vec3(gl_in[2].gl_Position.xyz);
    vec3 b = vec3(gl_in[1].gl_Position.xyz) - vec3(gl_in[2].gl_Position.xyz);
    return normalize(cross(a, b));
}


void main()
{
	vec4 white = vec4(1.0);
	vec4 grey = vec4(0.2);

	for (int i = 0; i < 3; i++) {
		vec4 posnEye = mvMatrix * gl_in[i].gl_Position;
		vec4 normalEye = vec4(GetNormal(), 0);
		vec4 lgtVec = normalize(lightPos - posnEye);

		vec4 ambOut = white;
		float diffTerm = max(dot(lgtVec, normalEye), 0);
		vec4 diffOut = white * diffTerm;

		geomTcord = es_in[i].tcoord;
		lighting = diffOut;

		float t = (-posnEye.z + 4.0) / (-20.0 + 4.0);     // Interpolation parameter for fog
		//lighting = (1-t)*lighting + t*white;
//		lighting = vec4(posnEye.z, 0, 0, 1);

		float height = es_in[i].height;
		float blendingRatio;
		float grassThreshold = 2;
		float grassRockThreshold = 6.0;
		float rockThreshold = 8;

		if (height < waterLevel) {
			texWeight = vec4(1, 0, 0, 0);
		}
		else if (height > waterLevel && height <= grassThreshold) {
			texWeight = vec4(0, 1, 0, 0);
		}
		else if (height > grassThreshold && height <= grassRockThreshold) {
			blendingRatio = (height - grassThreshold)/(grassRockThreshold - grassThreshold);
			texWeight = vec4(0, 1 - blendingRatio, blendingRatio, 0);
		}
		else if (height > grassRockThreshold && height < snowLevel) {
			blendingRatio = (height - grassRockThreshold) / (snowLevel - grassRockThreshold);
			texWeight = vec4(0, 0, 1 - blendingRatio, blendingRatio);
		}

		if (height > snowLevel && height > waterLevel) {
			texWeight = vec4(0, 0, 0, 1);
		}

		gl_Position = gl_in[i].gl_Position;

		EmitVertex();
	}

	EndPrimitive();
}