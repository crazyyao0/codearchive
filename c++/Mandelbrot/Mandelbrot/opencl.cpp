#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include <cl/cl.h>
#include "opencl.h"


unsigned int HSVToARGB(double H, double S, double V)
{
  double R, G, B;
  while (H < 0) { H += 360; };
  while (H >= 360) { H -= 360; };
  if (V <= 0) { R = G = B = 0; }
  else if (S <= 0){ R = G = B = V;}
  else
  {
    double hf = H / 60.0;
    int i = (int)(hf);
    double f = hf - i;
    double pv = V * (1 - S);
    double qv = V * (1 - S * f);
    double tv = V * (1 - S * (1 - f));
    switch (i)
    {
      case 0:
        R = V; G = tv; B = pv; break;
      case 1:
        R = qv; G = V; B = pv; break;
      case 2:
        R = pv; G = V; B = tv; break;
      case 3:
		R = pv; G = qv; B = V; break;
      case 4:
        R = tv; G = pv; B = V; break;
      case 5:
        R = V; G = pv; B = qv; break;
      case 6:
        R = V; G = tv; B = pv; break;
      case -1:
        R = V; G = pv; B = qv; break;
      default:
        R = G = B = V;
        break;
    }
  }
  return 0xFF000000 + ((int)(R * 255.0)<<16) + ((int)(G * 255.0)<<8) + (int)(B * 255.0);
}


void print_platform_info(cl_platform_id i)
{
	char buf[1024];
	unsigned int num;

	clGetPlatformInfo(i, CL_PLATFORM_NAME, 1024, buf, &num);
	printf("CL_PLATFORM_NAME: %s\n", buf);

	clGetPlatformInfo(i, CL_PLATFORM_VENDOR, 1024, buf, &num);
	printf("CL_PLATFORM_VENDOR: %s\n", buf);

	clGetPlatformInfo(i, CL_PLATFORM_VERSION, 1024, buf, &num);
	printf("CL_PLATFORM_VERSION: %s\n", buf);

	clGetPlatformInfo(i, CL_PLATFORM_PROFILE, 1024, buf, &num);
	printf("CL_PLATFORM_PROFILE: %s\n", buf);

	clGetPlatformInfo(i, CL_PLATFORM_EXTENSIONS, 1024, buf, &num);
	printf("CL_PLATFORM_EXTENSIONS: %s\n", buf);

	clGetDeviceIDs(i, CL_DEVICE_TYPE_GPU, 0, 0, &num);

	if(num == 0)
		return;

	std::vector<cl_device_id> did(num);
	clGetDeviceIDs(i, CL_DEVICE_TYPE_GPU, num, &did[0], &num);

	for(std::vector<cl_device_id>::iterator it = did.begin(); it!= did.end(); ++it)
	{
		clGetDeviceInfo(*it, CL_DEVICE_NAME, 1024, buf, &num);
		printf("Device %d: CL_DEVICE_NAME: %s\n",*it, buf);

		clGetDeviceInfo(*it, CL_DEVICE_EXTENSIONS, 1024, buf, &num);
		printf("Device %d: CL_DEVICE_EXTENSIONS: %s\n",*it, buf);

		clGetDeviceInfo(*it, CL_DEVICE_MAX_COMPUTE_UNITS, 1024, buf, &num);
		printf("Device %d: CL_DEVICE_MAX_COMPUTE_UNITS: %d\n",*it, *(unsigned int*)buf);

		clGetDeviceInfo(*it, CL_DEVICE_MAX_CLOCK_FREQUENCY, 1024, buf, &num);
		printf("Device %d: CL_DEVICE_MAX_CLOCK_FREQUENCY: %d\n",*it, *(unsigned int*)buf);

		clGetDeviceInfo(*it, CL_DEVICE_GLOBAL_MEM_SIZE, 1024, buf, &num);
		printf("Device %d: CL_DEVICE_GLOBAL_MEM_SIZE: %d\n",*it, *(unsigned long long*)buf);

		clGetDeviceInfo(*it, CL_DEVICE_LOCAL_MEM_SIZE, 1024, buf, &num);
		printf("Device %d: CL_DEVICE_LOCAL_MEM_SIZE: %d\n",*it, *(unsigned long long*)buf);
	}
}

void print_build_log(cl_program program, cl_device_id device)
{
	char* build_log;  
	size_t log_size;  
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);  
	build_log = new char[log_size+1];  
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);  
	build_log[log_size] = '\0';  
	printf("%s\n", build_log);
	delete[] build_log;
}

cl_device_id getCl_device_id(int type)
{
    cl_uint numPlatforms = 0;
	cl_platform_id platforms[10];

    cl_int status = clGetPlatformIDs(10, platforms, &numPlatforms);
    if (status != CL_SUCCESS || numPlatforms == 0)
        return NULL;

	for(size_t i=0;i<numPlatforms;i++)
	{
		print_platform_info(platforms[i]);
	}

	for(size_t i=0;i<numPlatforms;i++)
	{
		cl_device_id devices[10];
	    cl_uint numDevices = 0;
		status = clGetDeviceIDs(platforms[i], type, 10, devices, &numDevices);
	    if (status != CL_SUCCESS || numDevices == 0)
		    continue;
		return devices[0];
	}
	return NULL;
}

MandelbrotGPU::MandelbrotGPU()
{
	int err;
	size_t num;
	char source[8192];
	const char* sourcep = source;

	device = getCl_device_id(CL_DEVICE_TYPE_GPU);
	err = clGetDeviceInfo(device, CL_DEVICE_DOUBLE_FP_CONFIG, 8, &fp64, &num);

	context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
	commandQueue = clCreateCommandQueue(context, device, 0, NULL);
	output = new int[512*512];

	outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY , 512*512*sizeof(int), NULL, NULL);
	for(int i=0;i<1024;i++)
		colors[i] = HSVToARGB(i, 1.0, 1.0);
	colors[1023] = 0xFF000000;
	colorTable = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 4096, colors, &err);

	FILE* fp = fopen("mandelbrot32.cl", "r");
	size_t source_len = fread(source, 1, 8192, fp);
	fclose(fp);
	cl_program program = clCreateProgramWithSource(context, 1, &sourcep, &source_len, &err);
	err=clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	kernel32 = clCreateKernel(program, "mandelbrot", NULL);
	err = clSetKernelArg(kernel32, 0, sizeof(cl_mem), (void *)&outputBuffer);
	err = clSetKernelArg(kernel32, 1, sizeof(cl_mem), (void *)&colorTable);

	if(fp64)
	{
		fp = fopen("mandelbrot64.cl", "r");
		source_len = fread(source, 1, 8192, fp);
		fclose(fp);
		cl_program program = clCreateProgramWithSource(context, 1, &sourcep, &source_len, &err);
		err=clBuildProgram(program, 1, &device, NULL, NULL, NULL);
		kernel64 = clCreateKernel(program, "mandelbrot", NULL);
		err = clSetKernelArg(kernel64, 0, sizeof(cl_mem), (void *)&outputBuffer);
		err = clSetKernelArg(kernel64, 1, sizeof(cl_mem), (void *)&colorTable);
	}
}

MandelbrotGPU::~MandelbrotGPU()
{
	delete[] output;
}

void* MandelbrotGPU::Render(double cx, double cy, double r)
{
	if(fp64 == 0)
		return Render32((float)cx, (float)cy, (float)r);

	int err;
	size_t global_work_size = 512*512;
	cl_event event;

	err = clSetKernelArg(kernel64, 2, sizeof(double), (void *)&cx);
	err = clSetKernelArg(kernel64, 3, sizeof(double), (void *)&cy);
	err = clSetKernelArg(kernel64, 4, sizeof(double), (void *)&r);
		
	err = clEnqueueNDRangeKernel(commandQueue, kernel64, 1, NULL, &global_work_size, NULL, 0, NULL, &event);
	err = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, 512*512*sizeof(int), output, 1, &event, NULL);
	err = clReleaseEvent(event);

	return output;
}

void* MandelbrotGPU::Render32(float cx, float cy, float r)
{
	int err;
	size_t global_work_size = 512*512;
	cl_event event;

	err = clSetKernelArg(kernel32, 2, sizeof(float), (void *)&cx);
	err = clSetKernelArg(kernel32, 3, sizeof(float), (void *)&cy);
	err = clSetKernelArg(kernel32, 4, sizeof(float), (void *)&r);
		
	err = clEnqueueNDRangeKernel(commandQueue, kernel32, 1, NULL, &global_work_size, NULL, 0, NULL, &event);
	err = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0, 512*512*sizeof(int), output, 1, &event, NULL);
	err = clReleaseEvent(event);

	return output;
}

void MandelbrotGPU::DeviceName(char* name)
{
	char buf[1024];
	unsigned int num;
	clGetDeviceInfo(device, CL_DEVICE_NAME, 1024, buf, &num);
	sprintf(name, "GPU: %s", buf);
}

MandelbrotCPU::MandelbrotCPU()
{
	output = new int[512*512];
	for(int i=0;i<1024;i++)
		colors[i] = HSVToARGB(i, 1.0, 1.0);
	colors[1023] = 0xFF000000;
}

void* MandelbrotCPU::Render(double cx, double cy, double r)
{
	for(int dy=0;dy<512;dy++)
	for(int dx=0;dx<512;dx++)
	{
		double u = cx - r + 2 * r * dx / 512;
		double v = cy + r - 2 * r * dy / 512;

		double x=0.0;
		double y=0.0;
		int i;
		for(i=0; i<1023; i++)
		{
			if(x*x + y*y > 4.0)
				break;
			double a = x*x - y*y + u;
			double b = 2*x*y + v;
			x = a;
			y = b;
		}
		output[dy*512+dx] = colors[i];
	}

	return output;
}

void* MandelbrotCPU::Render32(float cx, float cy, float r)
{
	for(int dy=0;dy<512;dy++)
	for(int dx=0;dx<512;dx++)
	{
		float u = cx - r + 2 * r * dx / 512;
		float v = cy + r - 2 * r * dy / 512;

		float x=0.0;
		float y=0.0;
		int i;
		for(i=0; i<1023; i++)
		{
			if(x*x + y*y > 4.0)
				break;
			float a = x*x - y*y + u;
			float b = 2*x*y + v;
			x = a;
			y = b;
		}
		output[dy*512+dx] = colors[i];
	}

	return output;
}

MandelbrotCPU::~MandelbrotCPU()
{
	delete[] output;
}

void MandelbrotCPU::DeviceName(char* name)
{
	cl_device_id device = getCl_device_id(CL_DEVICE_TYPE_CPU);
	char buf[1024];
	unsigned int num;
	clGetDeviceInfo(device, CL_DEVICE_NAME, 1024, buf, &num);
	sprintf(name, "CPU: %s", buf);
}