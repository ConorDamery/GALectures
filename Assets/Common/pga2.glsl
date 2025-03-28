#ifndef PGA2_GUARD
#define PGA2_GUARD

#define motor     vec4
#define point     vec2

const float PI = 3.141592653;

point sw_mp(motor a, point b)
{
	return point(
		2*a.x*a.w + 2*a.y*a.z - 2*a.x*a.y*b.y + b.x * (a.x*a.x - a.y*a.y),
		2*a.y*a.w - 2*a.x*a.z + 2*a.x*a.y*b.x + b.y * (a.x*a.x - a.y*a.y)
	);
}

motor reverse_m(motor a)
{
	return motor(a.x, -a.yzw); 
}

motor gp_mm(motor a, motor b)
{
	return motor(
		a.x*b.x - a.y*b.y,
		a.x*b.y + a.y*b.x,
		a.x*b.z + a.w*b.y + a.z*b.x - a.y*b.w,
		a.x*b.w + a.w*b.x - a.z*b.y + a.y*b.z
	);
}

#endif // PGA2_GUARD