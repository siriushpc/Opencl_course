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


#ifndef CONVOLUCIONPRUEBA_H_
#define CONVOLUCIONPRUEBA_H_



#include <CL/cl.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>
#include <SDKBitMap.hpp>
#include <time.h>


/*** GLOBALS ***/

/*
 * Input data is stored here.
 */
streamsdk::SDKBitMap inputBitmap;
cl_uchar4* inputImageData;
streamsdk::uchar4* pixelData;
cl_uint pixelSize = sizeof(streamsdk::uchar4);
float * inputImageDataFloat;


/*
 * Output data is stored here.
 */
float* outputImageDataFloat;
cl_uchar4* outputImageData;


/* Height and width of our Image */
cl_uint width;
cl_uint height;
cl_uint dataSize;
cl_uint deviceWidth;
cl_uint deviceHeight;
cl_uint deviceDataSize;

/* Kernel of our convolution */

 cl_float filter[49] = {-10/10,-5/10,-2/10,-1/10,-2/10,-5/10,-10/10,
  	  -5/10,0,3/10,4/10,3/10,0,-5/10,
  	  -2/10,3/10,6/10,7/10,6/10,3/10,-2/10,
  	  -1/10,4/10,7/10,8/10,7/10,4/10,-1/10,
  	  -2/10,3/10,6/10,7/10,6/10,3/10,-2/10,
  	  -5/10,0/10,3/10,4/10,3/10,0/10,-5/10,
  	  -10/10,-5/10,-2/10,-1/10,-2/10,-5/10,-10/10};

/*cl_float filter[49] = 
	{0,		0,		0,		0,		0,		0.0145, 0,
	 0,		0,		0,		0,		0.0376, 0.1283,	0.0145,
	 0,		0,		0,		0.0376, 0.1283, 0.0376,	0,
	 0,		0,		0.0376, 0.1283, 0.0376, 0,		0,
	 0,		0.0376, 0.1283, 0.0376, 0,		0,		0,
	 0.0145,0.1283, 0.0376, 0,		0,		0,		0,
	 0,		0.0145, 0,		0,		0,		0,		0
	};*/
/*cl_float filter[9] = 
	{-1,2,-1,
	 -1,2,-1
	 -1,2,-1};*/


cl_uint filterWidth;
cl_uint filterRadius;
cl_uint paddingPixels;


/* The memory buffer that is used as input/output for OpenCL kernel */
cl_mem   inputImageBuffer;
cl_mem	 outputImageBuffer;
cl_mem	 filterImageBuffer;

cl_context          context;
cl_device_id        *devices;
cl_command_queue    commandQueue;

cl_program program;

/* This program uses only one kernel and this serves as a handle to it */
cl_kernel  kernel;


/*** FUNCTION DECLARATIONS ***/
/*
 * OpenCL related initialisations are done here.
 * Context, Device list, Command Queue are set up.
 * Calls are made to set up OpenCL memory buffers that this program uses
 * and to load the programs into memory and get kernel handles.
 */
int initializeCL(void);

/*
 *
 */
int convertToString(const char * filename, std::string& str);

/*
 * This is called once the OpenCL context, memory etc. are set up,
 * the program is loaded into memory and the kernel handles are ready.
 * 
 * It sets the values for kernels' arguments and enqueues calls to the kernels
 * on to the command queue and waits till the calls have finished execution.
 *
 * It also gets kernel start and end time if profiling is enabled.
 */
int runCLKernels(void);

/* Releases OpenCL resources (Context, Memory etc.) */
int cleanupCL(void);

/* Releases program's resources */
void cleanupHost(void);

/*
 * Prints no more than 256 elements of the given array.
 * Prints full array if length is less than 256.
 *
 * Prints Array name followed by elements.
 */
void print1DArray(
         const std::string arrayName, 
         const unsigned int * arrayData, 
         const unsigned int length);

/*redondeo*/
unsigned int roundUp(unsigned int value, unsigned int multiple);

/* Conversion de la imagen a  gris y float */
int imageToGrayFloat(cl_uchar4* imageData, float* imageDataFloat, int tamano);

#endif  /* #ifndef SIMPLEIMAGEROTATION_H_ */