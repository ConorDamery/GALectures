#ifndef PGA2_GUARD
#define PGA2_GUARD

#define motor     vec4
#define point     vec2

const float PI = 3.141592653;

point sw_mp(motor a, point b)
{
	return point(
		2*a.w*a.x*b.y - 2*a.y*a.x - 2*a.z*a.w - 2*a.w*a.w*b.x + b.x,
		2*a.w*a.x*b.x - 2*a.z*a.x + 2*a.y*a.w - 2*a.w*a.w*b.y + b.y
	);
}

motor reverse_m(motor a)
{
	return motor(a.x, -a.yzw); 
}

motor gp_mm(motor a, motor b)
{
	return motor(
		a.x*b.x - a.w*b.w,
		a.y*b.x + a.w*b.z + a.x*b.y - a.z*b.w,
		a.y*b.w + a.z*b.x + a.x*b.z - a.w*b.y,
		a.w*b.x + a.x*b.w
	);
}

#endif // PGA2_GUARD