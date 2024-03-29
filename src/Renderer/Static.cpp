#include "Static.h"

std::string StaticShaders::LineGLSL() {
	return R"(
//Basic Line Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

uniform mat4 u_ViewProjection;

out vec4 v_Color;

void main() {
	v_Color = a_Color;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec4 v_Color;

void main() {
	color = v_Color;
}
		)";
}

std::string StaticShaders::CircleGLSL() {
    return R"(
//Basic Circle Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_WorldPosition;
layout(location = 1) in vec3 a_LocalPosition;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in float a_Thickness;
layout(location = 4) in float a_Fade;

uniform mat4 u_ViewProjection;

out vec3 v_LocalPosition;
out vec4 v_Color;
out float v_Thickness;
out float v_Fade;

void main() {
	v_LocalPosition = a_LocalPosition;
	v_Color = a_Color;
	v_Thickness = a_Thickness;
	v_Fade = a_Fade;
	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

in vec3 v_LocalPosition;
in vec4 v_Color;
in float v_Thickness;
in float v_Fade;

void main() {
	float distance = 1.0 - length(v_LocalPosition);
	float circle = smoothstep(0.0, v_Fade, distance);
	circle *= smoothstep(v_Thickness + v_Fade, v_Thickness, distance);
	if (circle == 0.0)
		discard;

	color = v_Color;
	color.a *= circle;
}
	)";
}

std::string StaticShaders::SphereGLSL() {
	return R"(
//Basic Sphere Shader

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in float a_Radius;

uniform mat4 u_ViewProjection;

void main() {
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
	gl_PointSize = 2500.0 * a_Radius / gl_Position.w;

}

#type fragment
#version 450 core

layout(location = 0) out vec4 color;

void main() {
	float x = gl_PointCoord.x * 2.0 - 1.0;
	float y = gl_PointCoord.y * 2.0 - 1.0;
	float z = sqrt(1.0 - (pow(x, 2.0) + pow(y, 2.0)));
	vec3 pos = vec3(x, y, z);
	if (dot(pos.xy, pos.xy) > 1.0) discard;
	color = vec4(normalize(pos) * 0.5 + 0.5, 1.0);
}
	)";
}
