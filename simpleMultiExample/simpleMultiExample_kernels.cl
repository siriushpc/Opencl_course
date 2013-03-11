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

// widthA = heightB for valid matrix multiplication
__kernel void simplemultiexampleKernel(__global float* outputC, int widthA, int heightA,
							int widthB, int heightB, __global float* inputA,
							__global float* inputB) {
	//Get global position in Y direction
	int row = get_global_id(1);
	//Get global position in X direction
	int col = get_global_id(0);
	float sum = 0.0f;
	//Calculate result of one element of Matrix C
	for (int i = 0; i < widthA; i++) {
		sum += inputA[row*widthA+i] * inputB[i*widthB+col];
	}
	outputC[row*widthB+col] = sum;
}
