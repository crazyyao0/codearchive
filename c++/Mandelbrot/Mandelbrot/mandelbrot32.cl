__kernel void mandelbrot(__global uint* output, __global uint* color, float cx, float cy, float r)
{
	int num = get_global_id(0);
	int dy = num / 512;
	int dx = num % 512;

	float u = cx - r + 2 * r * dx / 512;
	float v = cy + r - 2 * r * dy / 512;

	float x=0.0;
	float y=0.0;
	uint i = 0;
	for(i=0; i<1023; i++)
	{
		if(x*x + y*y > 4.0)
			break;
		float a = x*x - y*y + u;
		float b = 2*x*y + v;
		x = a;
		y = b;
	}
	output[num] = color[i];
};
