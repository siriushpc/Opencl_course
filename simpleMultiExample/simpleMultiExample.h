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


#ifndef SIMPLEMULTIEXAMPLE_H_
#define SIMPLEMULTIEXAMPLE_H_



#include <CL/cl.h>
#include <string.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <fstream>


/*** GLOBALS ***/

/*
 * Input data is stored here.
 */
cl_float *A;
cl_float *B;

/*
 * Output data is stored here.
 */
cl_float *C;

/*
 * Multiplier is stored in this variable 
 */
cl_uint multiplier;

/* widthA, heightA, widthB, heightB */
cl_uint widthA;
cl_uint widthB;
cl_uint heightA;
cl_uint heightB;

/* The memory buffer that is used as input/output for OpenCL kernel */
cl_mem   ABuffer;
cl_mem	 BBuffer;
cl_mem	 CBuffer;

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


#endif  /* #ifndef SIMPLEMULTIEXAMPLE_H_ */
