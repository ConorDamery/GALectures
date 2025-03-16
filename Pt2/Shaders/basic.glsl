#ifdef VERT
precision highp float;
layout (location = 0) in vec3 Position;
layout (location = 1) in vec4 Color;

uniform mat3x2 Proj;

out vec4 Frag_Color;

// proj: [right, left], [top, bottom], [near, far]
vec4 project(const mat3x2 proj, const vec3 p)
{
    const float rl = 1.0 / (proj[0].x - proj[0].y);
    const float tb = 1.0 / (proj[1].x - proj[1].y);
    const float nf = 1.0 / (proj[2].x - proj[2].y);

    const float sx = 2.0 * proj[2].x * rl;
    const float sy = 2.0 * proj[2].x * tb;
    const float ox = (proj[0].x + proj[0].y) * rl;
    const float oy = (proj[1].x + proj[1].y) * tb;
    const float fa = (proj[2].x + proj[2].y) * nf;
    const float fb = 2.0 * proj[2].x * proj[2].y * nf;

	return vec4(sx * p.x + ox * p.z, sy * p.y + oy * p.z, fa - fb * p.z, p.z);
}

void main()
{
	Frag_Color = Color;
	gl_Position = project(Proj, Position);
}
#endif

#ifdef FRAG
precision mediump float;
layout (location = 0) out vec4 Out_Color;

in vec4 Frag_Color;

void main()
{
	Out_Color = Frag_Color;
}
#endif