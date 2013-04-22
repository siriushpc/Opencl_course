/*
 * Application convolucionPrueba
 * Copyright (C) kiwi 2013 <kiwi@kiwi>
 * 
convolucionPrueba is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * convolucionPrueba is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "convolucionPrueba.h"

int imageToGrayFloat(cl_uchar4* imageData, float* imageDataFloat, int tamano){
	for (int i = 0; i < tamano ; i++){
		imageDataFloat[i] = (float)(imageData[i].x * 0.3) + (float)(imageData[i].y * 0.59) + (float)(imageData[i].z * 0.11);
	}

	return 0;

}


unsigned int roundUp(unsigned int value, unsigned int multiple) {
	
	// Determine how far past the nearest multiple the value is
	unsigned int remainder = value % multiple;
	// Add the difference to make the value a multiple
	if(remainder != 0) {
		value += (multiple-remainder);
	}
	return value;
}


/*
 * \brief Host Initialization 
 *        Allocate and initialize memory 
 *        on the host. Print input array. 
 */
int
initializeHost(void)
{
    
	inputBitmap.load("/opt/AMDAPP/samples/opencl/myprojects/app/convolucionPrueba/imagen.bmp");

	if(!inputBitmap.isLoaded())
    {
        printf("Error cargando la imagen\n");
        return SDK_FAILURE;
    }

	/////////////////////////////////////////////////////////////////
	// Obtain width and height of the image
	/////////////////////////////////////////////////////////////////
	height = inputBitmap.getHeight();
	width = inputBitmap.getWidth();
	dataSize = height * width * sizeof(float);
	deviceWidth = width;
	deviceHeight = height;
	deviceDataSize = height * deviceWidth * sizeof(float);
	filterWidth = 7;
	filterRadius = filterWidth/2;
	paddingPixels = (cl_uint)(filterWidth/2)*2;


	/////////////////////////////////////////////////////////////////
	// Allocate memory for input & output image data
	/////////////////////////////////////////////////////////////////
    inputImageData  = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));
	if(inputImageData == NULL)
    {
        printf("Error: Failed to allocate inputImageData memory on host\n");
        return 1; 
    }

	outputImageData = (cl_uchar4*)malloc(width * height * sizeof(cl_uchar4));
	if(outputImageData == NULL)
    {
        printf("Error: Failed to allocate outputImageData memory on host\n");
        return 1; 
    }

	/////////////////////////////////////////////////////////////////
	// Copy pixel data into inputImageData
	////////////////////////////////////////////////////////////////
	pixelData = inputBitmap.getPixels();
    memcpy(inputImageData, pixelData, width * height * pixelSize);

	/////////////////////////////////////////////////////////////////
    // Tipo de dato Float para convertir la imagen 
	//////////////////////////////////////////////////////////////////
	inputImageDataFloat = (cl_float*)malloc(dataSize);
	if(inputImageDataFloat == NULL)
    {
        printf("Error: Failed to allocate inputImageDataFloat memory on host\n");
        return 1; 
    }

	imageToGrayFloat(inputImageData,inputImageDataFloat,width*height);

	outputImageDataFloat = (cl_float*)malloc(dataSize);
	if(outputImageDataFloat == NULL)
    {
        printf("Error: Failed to allocate outputImageDataFloat memory on host\n");
        return 1; 
    }
	
	
	/////////////////////////////////////////////////////////////////
	// Initialize the Image data to NULL
	/////////////////////////////////////////////////////////////////
    memset(outputImageData, 0, width * height * pixelSize);

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
                                      CL_DEVICE_TYPE_GPU, 
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
    inputImageBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR,
                      height*width*sizeof(cl_float),
                      inputImageDataFloat, 
                      &status);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: clCreateBuffer (inputImageBuffer)\n");
        return 1;
    }

    outputImageBuffer = clCreateBuffer(
                       context, 
                       CL_MEM_READ_WRITE ,
                       deviceDataSize,
                       NULL, 
                       &status);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: clCreateBuffer (outputImageBuffer)\n");
        return 1;
    }

	filterImageBuffer = clCreateBuffer(
                      context, 
                      CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
                      filterWidth*filterWidth*sizeof(cl_float),
                      filter, 
                      &status);
	if(status != CL_SUCCESS) 
    { 
        printf("Error: clCreateBuffer (filterImageBuffer)\n");
        return 1;
    }

    /////////////////////////////////////////////////////////////////
    // Load CL file, build CL program object, create CL kernel object
    /////////////////////////////////////////////////////////////////
    const char * filename  = "/opt/AMDAPP/samples/opencl/myprojects/app/convolucionPrueba/convolucionPrueba_kernels.cl";
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

	// If there are build errors, print them to the screen
	if (status != CL_SUCCESS) {
		printf("Program failed to build.\n");
		cl_build_status buildStatus;
		for (unsigned int i = 0; i < 1; i++) {
			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_STATUS,
					sizeof(cl_build_status), &buildStatus, NULL);
			if (buildStatus == CL_SUCCESS) {
				continue;
			}

			char *buildLog;
			size_t buildLogSize;
			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG, 0,
					NULL, &buildLogSize);
			buildLog = (char*) malloc(buildLogSize);
			if (buildLog == NULL) {
				perror("malloc");
				exit(-1);
			}

			clGetProgramBuildInfo(program, devices[i], CL_PROGRAM_BUILD_LOG,
					buildLogSize, buildLog, NULL);

			buildLog[buildLogSize - 1] = '\0';
			printf("Device %u Build Log:\n%s\n", i, buildLog);
			free(buildLog);
		}
		exit(0);
	} else {
		printf("No build errors\n");
	}

    /*if(status != CL_SUCCESS) 
    { 
        printf("Error: Building Program (clBuildProgram)\n");
        return 1; 
    }*/

    /* get a kernel object handle for a kernel with the given name */
    kernel = clCreateKernel(program, "convolution", &status);
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

	cl_uint totalWorkItemsX = roundUp(width-paddingPixels,16);
	cl_uint totalWorkItemsY = roundUp(height-paddingPixels,16);
	
    size_t globalThreads[2] = {totalWorkItemsX,totalWorkItemsY};
    size_t localThreads[2] = {16,16};

	cl_uint localWidth = 16 + paddingPixels;
	cl_uint localHeight = 16 + paddingPixels;

	size_t localMemSize = (localWidth * localHeight * sizeof(float));

	
    /*** Set appropriate arguments to the kernel ***/
    /* the inputImageBuffer array to the kernel */
    status = clSetKernelArg(
                    kernel, 
                    0, 
                    sizeof(cl_mem), 
                    (void *)&inputImageBuffer);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (inputImageBuffer)\n");
        return 1;
    }

    /* the outputImageBuffer array to the kernel */
    status = clSetKernelArg(
                    kernel, 
                    1, 
                    sizeof(cl_mem), 
                    (void *)&outputImageBuffer);
    if(status != CL_SUCCESS) 
    { 
        printf("Error: Setting kernel argument. (outputImageBuffer)\n");
        return 1;
    }

    /*filterImageBuffer*/
    status = clSetKernelArg(
                    kernel, 
                    2, 
                    sizeof(cl_mem), 
                    (void *)&filterImageBuffer);
    if(status != CL_SUCCESS) 
    { 
        printf( "Error: Setting kernel argument. (filterImageBuffer)\n");
        return 1;
    }

	/*height*/
    status = clSetKernelArg(
                    kernel, 
                    3, 
                    sizeof(cl_uint), 
                    (void *)&height);
    if(status != CL_SUCCESS) 
    { 
        printf( "Error: Setting kernel argument. (height)\n");
        return 1;
    }

	/*width*/
    status = clSetKernelArg(
                    kernel, 
                    4, 
                    sizeof(cl_uint), 
                    (void *)&width);
    if(status != CL_SUCCESS) 
    { 
        printf( "Error: Setting kernel argument. (width)\n");
        return 1;
    }

	/*filterWidth*/
    status = clSetKernelArg(
                    kernel, 
                    5, 
                    sizeof(cl_uint), 
                    (void *)&filterWidth);
    if(status != CL_SUCCESS) 
    { 
        printf( "Error: Setting kernel argument. (filterWidth)\n");
        return 1;
    }

	/*localMemSize*/
    status = clSetKernelArg(
                    kernel, 
                    6, 
                    localMemSize, 
                    NULL);
    if(status != CL_SUCCESS) 
    { 
        printf( "Error: Setting kernel argument. (localMemSize)\n");
        return 1;
    }

	/*localHeight*/
    status = clSetKernelArg(
                    kernel, 
                    7, 
                    sizeof(cl_uint), 
                    (void *)&localHeight);
    if(status != CL_SUCCESS) 
    { 
        printf( "Error: Setting kernel argument. (localHeight)\n");
        return 1;
    }

	/*localWidth*/
    status = clSetKernelArg(
                    kernel, 
                    8, 
                    sizeof(cl_uint), 
                    (void *)&localWidth);
    if(status != CL_SUCCESS) 
    { 
        printf( "Error: Setting kernel argument. (localWidth)\n");
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
                outputImageBuffer,
                CL_TRUE,
                0,
                deviceDataSize,
                (void *)outputImageDataFloat,
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

	for(int i=0;i<width*height;i++){
		outputImageData[i].x = (cl_uchar)outputImageDataFloat[i];
		outputImageData[i].y = (cl_uchar)outputImageDataFloat[i];
		outputImageData[i].z = (cl_uchar)outputImageDataFloat[i];
		outputImageData[i].w = 0;
	}
	
	// copy output image data back to original pixel data 
    memcpy(pixelData, outputImageData, width * height * pixelSize);

    // write the output bmp file 
    if(!inputBitmap.write("salida.bmp"));
    {
        std::cout << "Failed to write output image!" << std::endl;
        return SDK_FAILURE;
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
    status = clReleaseMemObject(inputImageBuffer);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseMemObject (inputBuffer)\n");
        return 1; 
    }
    status = clReleaseMemObject(outputImageBuffer);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseMemObject (outputBuffer)\n");
        return 1; 
    }

	status = clReleaseMemObject(filterImageBuffer);
    if(status != CL_SUCCESS)
    {
        printf("Error: In clReleaseMemObject (filterImageBuffer)\n");
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
    if(inputImageData != NULL)
    {
        free(inputImageData);
        inputImageData = NULL;
    }
    if(outputImageData != NULL)
    {
        free(outputImageData);
        outputImageData = NULL;
    }
    if(devices != NULL)
    {
        free(devices);
        devices = NULL;
    }
}

int convolucionSecuencial(){

	int rows = height;
	int cols = width;
	int halfFilterWidth = filterWidth/2;
	float sum;
	
	// Iterate over the rows of the source image
	for(int i = halfFilterWidth; i < rows - halfFilterWidth; i++) {
	// Iterate over the columns of the source image
		for(int j = halfFilterWidth; j < cols - halfFilterWidth; j++) {
			sum = 0; // Reset sum for new source pixel
			// Apply the filter to the neighborhood
			for(int k = - halfFilterWidth; k <= halfFilterWidth; k++) {
				for(int l = - halfFilterWidth; l <= halfFilterWidth; l++) {
					sum += inputImageDataFloat[(j+l)*rows+(i+k)]*filter[(l+ halfFilterWidth)*filterWidth+(k+halfFilterWidth)];
				}
			}
			//outputImage[i][j] = sum;
			outputImageDataFloat[j*rows+i] = sum;	
		}
	}
	for(int i=0;i<width*height;i++){
		outputImageData[i].x = (cl_uchar)outputImageDataFloat[i];
		outputImageData[i].y = (cl_uchar)outputImageDataFloat[i];
		outputImageData[i].z = (cl_uchar)outputImageDataFloat[i];
		outputImageData[i].w = 0;
	}
	
	// copy output image data back to original pixel data 
    memcpy(pixelData, outputImageData, width * height * pixelSize);

    // write the output bmp file 
    if(!inputBitmap.write("salidaSecuencial.bmp"));
    {
        std::cout << "Failed to write output image!" << std::endl;
        return SDK_FAILURE;
    }
}

int 
main(int argc, char * argv[])
{
	clock_t start, end;
	double cpu_time_used;
	
    // Initialize Host application 
    if(initializeHost() == 1)
        return 1;

	char a = argv[1][0];
	
	if(a == 'c'){
		start = clock();
		convolucionSecuencial();
		end = clock();
		cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
		printf("\nTime elapsed: %.20f\n", cpu_time_used);
		printf("Convolucion Secuencial \n");
		return 0;
	}
		
	
    // Initialize OpenCL resources
    if(initializeCL() == 1)
        return 1;

    // Run the CL program
	start = clock();
	int test = runCLKernels();
	end = clock();
	cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
	printf("\nTime elapsed: %.20f\n", cpu_time_used);
	printf("Convolucion Paralela \n");
    if(test == 1){
        return 1;
	}
	
    // Releases OpenCL resources 
    if(cleanupCL() == 1)
        return 1;

    // Release host resources
    cleanupHost();

    return 0;
}