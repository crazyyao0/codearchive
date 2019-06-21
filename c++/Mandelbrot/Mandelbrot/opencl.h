#include <cl/cl.h>
#include <Windows.h>

class Mandelbrot 
{
public:
	Mandelbrot()
	{
	}
	virtual ~Mandelbrot()
	{
	}

	virtual void* Render(double cx, double cy, double r) = 0;
	virtual void* Render32(float cx, float cy, float r) = 0;
	virtual void DeviceName(char* name) = 0;
};

class MandelbrotGPU: public Mandelbrot
{
	int* output;
	cl_device_id device;
	cl_context context;
	cl_command_queue commandQueue;
	cl_mem outputBuffer;
	cl_mem colorTable;
	cl_kernel kernel64;
	cl_kernel kernel32;
	unsigned int colors[1024];
	long long fp64;
public:
	MandelbrotGPU();
	~MandelbrotGPU();
	void* Render(double cx, double cy, double r);
	void* Render32(float cx, float cy, float r);
	void DeviceName(char* name);
};

class MandelbrotCPU: public Mandelbrot
{
	int* output;
	unsigned int colors[1024];
public:
	MandelbrotCPU();
	~MandelbrotCPU();
	void* Render(double cx, double cy, double r);
	void* Render32(float cx, float cy, float r);
	void DeviceName(char* name);
};

