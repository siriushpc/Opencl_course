/*
 * Application simpleImageRotation
 * Copyright (C) Yensy H. Gómez Villegas, John H. Osorio Ríos 2013 <john@sirius.utp.edu.co>
 * 
simpleImageRotation is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * simpleImageRotation is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

__kernel void simpleimagerotationKernel(__global uchar4* outputImage,
                             __global  uchar4* inputImage,
                             const     unsigned int width, 
							 const     unsigned int height)
{
    const int ix = get_global_id(0);
	const int iy = get_global_id(1);

	outputImage[iy*width+ix].x = inputImage[iy*width+ix].x * 0.3 + inputImage[iy*width+ix].y * 0.59 + inputImage[iy*width+ix].z * 0.11;
	outputImage[iy*width+ix].y = inputImage[iy*width+ix].x * 0.3 + inputImage[iy*width+ix].y * 0.59 + inputImage[iy*width+ix].z * 0.11;;
	outputImage[iy*width+ix].z = 0;//inputImage[iy*width+ix].x * 0.3 + inputImage[iy*width+ix].y * 0.59 + inputImage[iy*width+ix].z * 0.11;;


}
