/*
 * Application simpleMultiExample
 * Copyright (C) john 2013 <john@vabe-UX32VD>
 * 
simpleMultiExample is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * simpleMultiExample is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "simpleMultiExample.h"


void printMatrix(int height,int width,float *vector){
	for(int i = 0; i < height ; i++){
		for(int j = 0; j < width ; j++)
			printf("%.2f ",vector[i*width+j]);
		printf("\n");
	}
		

}

/*
 * \brief Host Initialization 
 *        Allocate and initialize memory 
 *        on the host. Print input array. 
 */
int
initializeHost(void)
{

	A = NULL;
	B = NULL;
	C = NULL;

	widthA = 32;
	widthB = 32;
	heightA = 32;
	heightB = 32;
	
    /////////////////////////////////////////////////////////////////
    // Allocate and initialize memory used by host 
    /////////////////////////////////////////////////////////////////
    A = (cl_float *) malloc(sizeof(cl_float)*widthA*heightA);
    if(A == NULL)
    {
        printf("Error: Failed to allocate input memory A on host\n");
        return 1; 
    }
	
	B = (cl_float *) malloc(sizeof(cl_float)*widthB*heightB);
    if(B == NULL)
    {
        printf("Error: Failed to allocate input memory B on host\n");
        return 1; 
    }

    C = (cl_float *) malloc(sizeof(cl_float)*widthB*heightA);
    if(C == NULL)
    {
        printf("Error: Failed to allocate output memory C on host\n");
        return 1; 
    }

	for(int i = 0; i < heightA ; i++)
		for(int j = 0; j < widthA ; j++)
			A[i*widthA+j] = 2.5;
	
	for(int i = 0; i < heightB ; i++)
		for(int j = 0; j < widthB ; j++)
			B[i*widthB+j] = 3.5;
    

    return 0;
}

/*
 * Converts the contents of a file into a string
 */
int
convertToString(const char *filename, std::string& s)
{
    size_t size;
    char*  str;

    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        str = new char[size+1];
        if(!str)
        {
            f.close();
            return NULL;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
    
        s = str;
        delete[] str;
        return 0;
    }
    printf("Error: Failed to open file %s\n", filename);
    return 1;
}

/*
 * \brief OpenCL related initialization 
 *        Create Context, Device list, Command Queue
 *        Create OpenCL memory buffer objects
 *        Load CL file, compile, link CL source 
 *		  Build program and kernel objects
 */
int
initializeCL(void)
{
    cl_int status = 0;
    size_t deviceListSize;

    /*
     * Have a look at the available platforms and pick either
     * the AMD one if available or a reasonable default.
     */

    cl_uint numPlatforms;
    cl_platform_id platform = NULL;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(status != CL_SUCCESS)
    {
        printf("Error: Getting Platforms. (clGetPlatformsIDs)\n");
        return 1;
    }
    
    if(numPlatforms > 0)
    {
        cl_platform_id* platforms = (cl_platform_id *)malloc(numPlatforms*sizeof(cl_platform_id));
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        if(status != CL_SUCCESS)
        {
            printf("Error: Getting Platform Ids. (clGetPlatformsIDs)\n");
            return 1;
        }
        for(unsigned int i=0; i < numPlatforms; ++i)
        {
            char pbuff[100];
            status = clGetPlatformInfo(
                        platforms[i],
                        CL_PLATFORM_VENDOR,
                        sizeof(pbuff),
                        pbuff,
                        NULL);
            if(status != CL_SUCCESS)
            {
                printf("Error: Getting Platform Info. (clGetPlatformInfo)\n");
                return 1;
            }
            platform = platforms[i];
            if(!strcmp(pbuff, "Advanced Micro Devices, Inc."))
            {
                break;
            }
        }
        delete platforms;
    }

    if(NULL == platform)
    {
        std::cout << "NULL platform found so Exiting Application." << std::endl;
        return 1;
    }

    /*
     * If we could find our platform, use it. Otherwise use just available platform.
     */

    cl_context_properties cps[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };


    /////////////////////////////////////////////////////////////////
    // Create an OpenCL context
    /////////////////////////////////////////////////////////////////
    context = clCreateContextFromType(cps, 
                                      CL_DEVICE_TYPE_CPU, 
                                      NULL, 
                                      NULL, 
                                      &status);
    if(status != CL_SUCCESS) 
    {  
        printf("Error: Creating Context. (clCreateContextFromType)\n");
        return 1; 
    }

    /* First, get the size of device list data */
    status = clGetContextInfo(context, 
                              CL_CONTEXT_DEVICES, 
                              0, 
                              NULL, 
                              &deviceListSize);
    if(status != CL_SUCCESS) 
    {  
        printf(
            "Error: Getting Context Info \
            (device list size, clGetContextInfo)\n");
        return 1;
    }

    /////////////////////////////////////////////////////////////////
    // Detect OpenCL devices
    /////////////////////////////////////////////////////////////////
    devices = (cl_device_id *)malloc(deviceListSize);
    if(devices == 0)
    {
        printf("Error: No devices found.\n");
        return 1;
    }

    /* Now, get the device list data */
    status = clGetContextInfo(
                 context, 
                 CL_CONTEXT_DEVICES, 
                 deviceListSize, 
                 devices, 
                 NULL);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Getting Context Info \
            (device list, clGetContextInfo)\n");
        return 1;
    }

    /////////////////////////////////////////////////////////////////
    // Create an OpenCL command queue
    /////////////////////////////////////////////////////////////////
    commandQueue = clCreateCommandQueue(
                       context, 
                       devices[0], 
                       0, 
                       &status);
    if(status != CL_SUCCESS) 
    { 
        printf("Creating Command Queue. (clCreateCommandQueue)\n");
        return 1;
    }

    /////////////////////////////////////////////////////////////////
    // Create OpenCL memory buffers
    /////////////////////////////////////////////////////////////////
    ABuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * widthA * heightA,
                      A, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: clCreateBuffer (ABuffer)\n");
        return 1;
    }

    BBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                      sizeof(cl_float) * widthB * heightB,
                      B, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: clCreateBuffer (BBuffer)\n");
        return 1;
    }


	CBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_WRITE,
                      sizeof(cl_float) * widthB * heightA,
                      NULL, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: clCreateBuffer (CBuffer)\n");
        return 1;
    }


    /////////////////////////////////////////////////////////////////
    // Load CL file, build CL program object, create CL kernel object
    /////////////////////////////////////////////////////////////////
    const char * filename  = "/opt/AMDAPP/samples/opencl/myprojects/app/simpleMultiExample/simpleMultiExample_kernels.cl";
    std::string  sourceStr;
    status = convertToString(filename, sourceStr);
    if(status != CL_SUCCESS)
        return 1;

    const char * source    = sourceStr.c_str();
    size_t sourceSize[]    = { strlen(source) };

    program = clCreateProgramWithSource(
                  context, 
                  1, 
                  &source,
                  sourceSize,
                  &status);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Loading Binary into cl_program \
               (clCreateProgramWithBinary)\n");
      return 1;
    }

    /* create a cl program executable for all the devices specified */
    status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Building Program (clBuildProgram)\n");
        return 1; 
    }

    /* get a kernel object handle for a kernel with the given name */
    kernel = clCreateKernel(program, "simplemultiexampleKernel", &status);
    if(status != CL_SUCCESS) 
    {  
        printf("Error: Creating Kernel from program. (clCreateKernel)\n");
        return 1;
    }

    return 0;
}


/*
 * \brief Run OpenCL program 
 *		  
 *        Bind host variables to kernel arguments 
 *		  Run the CL kernel
 */
int 
runCLKernels(void)
{
    cl_int   status;
    cl_event events[2];
    size_t globalThreads[2] = {heightA,widthB};
    size_t localThreads[2] = {16,16};
    
    /*** Set appropriate arguments to the kernel ***/
    /* the output array to the kernel */
    status = clSetKernelArg(kernel, 
                    0, 
                    sizeof(cl_mem), 
                    (void *)&CBuffer);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (CBuffer)\n");
        return 1;
    }

    status = clSetKernelArg(kernel, 
                    1, 
                    sizeof(cl_uint), 
                   (void *)&widthA);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (widthA)\n");
        return 1;
    }

	status = clSetKernelArg(kernel, 
                    2, 
                    sizeof(cl_uint), 
                   (void *)&heightA);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (heightA)\n");
        return 1;
    }

	status = clSetKernelArg(kernel, 
                    3, 
                    sizeof(cl_uint), 
                   (void *)&widthB);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (widthB)\n");
        return 1;
    }

	status = clSetKernelArg(kernel, 
                    4, 
                    sizeof(cl_uint), 
                   (void *)&heightB);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (heightB)\n");
        return 1;
    }

	status = clSetKernelArg(kernel, 
                    5, 
                    sizeof(cl_mem), 
                   (void *)&ABuffer);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (ABuffer)\n");
        return 1;
    }

	
	status = clSetKernelArg(kernel, 
                    6, 
                    sizeof(cl_mem), 
                   (void *)&BBuffer);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (BBuffer)\n");
        return 1;
    }

	
    /* 
     * Enqueue a kernel run call.
     */
    status = clEnqueueNDRangeKernel(
                 commandQueue,
                 kernel,
                 2,
                 NULL,
                 globalThreads,
                 localThreads,
                 0,
                 NULL,
                 &events[0]);
    if(status != CL_SUCCESS) 
    { 
        printf(
            "Error: Enqueueing kernel onto command queue. \
            (clEnqueueNDRangeKernel)\n");
        return 1;
    }


    /* wait for the kernel call to finish execution */
    status = clWaitForEvents(1, &events[0]);
    if(status != CL_SUCCESS) 
    { 
        printf(
            "Error: Waiting for kernel run to finish. \
            (clWaitForEvents)\n");
        return 1;
    }

    status = clReleaseEvent(events[0]);
    if(status != CL_SUCCESS)
    {
        printf("Error: clReleaseEvent. (events[0])\n");
        return 1;
    }

    /* Enqueue readBuffer*/
    status = clEnqueueReadBuffer(
                commandQueue,
                CBuffer,
                CL_TRUE,
                0,
                widthB * heightA * sizeof(cl_float),
                (void*)C,
                0,
                NULL,
                &events[1]);
    
    if(status != CL_SUCCESS) 
    {
        printf( 
            "Error: clEnqueueReadBuffer failed. \
             (clEnqueueReadBuffer)\n");
        return 1;
    }
    
    /* Wait for the read buffer to finish execution */
    status = clWaitForEvents(1, &events[1]);
    if(status != CL_SUCCESS) 
    { 
        printf(
            "Error: Waiting for read buffer call to finish. \
            (clWaitForEvents)\n");
        return 1;
    }
    
    status = clReleaseEvent(events[1]);
    if(status != CL_SUCCESS)
    {
        printf("Error: clReleaseEvent. (events[1])\n");
        return 1;
    }

    return 0;
}


/*
 * \brief Release OpenCL resources (Context, Memory etc.) 
 */
int 
cleanupCL(void)
{
    cl_int status;

    status = clReleaseKernel(kernel);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseKernel \n");
        return 1; 
    }
    status = clReleaseProgram(program);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseProgram\n");
        return 1; 
    }
    status = clReleaseMemObject(ABuffer);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseMemObject (ABuffer)\n");
        return 1; 
    }
    status = clReleaseMemObject(BBuffer);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseMemObject (BBuffer)\n");
        return 1; 
    }
	status = clReleaseMemObject(CBuffer);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseMemObject (CBuffer)\n");
        return 1; 
    }
    status = clReleaseCommandQueue(commandQueue);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseCommandQueue\n");
        return 1;
    }
    status = clReleaseContext(context);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseContext\n");
        return 1;
    }

    return 0;
}


/* 
 * \brief Releases program's resources 
 */
void
cleanupHost(void)
{
    if(A != NULL)
    {
        free(A);
        A = NULL;
    }
    if(B != NULL)
    {
        free(B);
        B = NULL;
    }
	if(C != NULL)
    {
        free(C);
        C = NULL;
    }
    if(devices != NULL)
    {
        free(devices);
        devices = NULL;
    }
}


int 
main(int argc, char * argv[])
{
    // Initialize Host application 
    if(initializeHost() == 1)
        return 1;
	
    // Initialize OpenCL resources
    if(initializeCL() == 1)
        return 1;

    // Run the CL program
    if(runCLKernels() == 1)
        return 1;

    // Releases OpenCL resources 
    if(cleanupCL() == 1)
        return 1;

	printMatrix(heightA,widthB,C);
	
	
    // Release host resources
    cleanupHost();

    return 0;
}
