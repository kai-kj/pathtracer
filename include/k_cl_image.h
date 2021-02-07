//============================================================================//
// k_cl_image.h                                                                //
//============================================================================//

//----------------------------------------------------------------------------//
// interface                                                                  //
//----------------------------------------------------------------------------//

#ifndef k_cl_image_H
#define k_cl_image_H

#include <CL/cl.h>

typedef struct Color {
	float r;
	float g;
	float b;
} Color;

cl_float3 rgb_to_float3(Color color);
void gamma_correct_image(cl_float3 *image, int pixelCount);
void write_image(cl_float3 *image, int width, int height, char *filename);

//----------------------------------------------------------------------------//
// implementation                                                             //
//----------------------------------------------------------------------------//

#ifdef k_cl_image_IMPLEMENTATION

#include <stdio.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

//---- private functions -----------------------------------------------------//

static char *get_filename_ext(char *filename) {
    char *dot = strrchr(filename, '.');

    if(!dot || dot == filename)
		return "";
	
    return dot + 1;
}

//---- public functions ------------------------------------------------------//

cl_float3 rgb_to_float3(Color color) {
	cl_float3 colorCL;
	colorCL.x = color.r;
	colorCL.y = color.g;
	colorCL.z = color.b;
	return colorCL;
}

void gamma_correct_image(cl_float3 *image, int pixelCount) {
	for(int i = 0; i < pixelCount; i++) {
		image[i].x = sqrt(image[i].x);
		image[i].y = sqrt(image[i].y);
		image[i].z = sqrt(image[i].z);
	}
}

void write_image(cl_float3 *image, int width, int height, char *filename) {
	unsigned char *data = malloc(width * height * 3);

	for(int i = 0; i < width * height; i++) {
		if(image[i].x < 0)
			image[i].x = 0;
		if(image[i].y < 0)
			image[i].y = 0;
		if(image[i].z < 0)
			image[i].z = 0;
		
		if(image[i].x > 1)
			image[i].x = 1;
		if(image[i].y > 1)
			image[i].y = 1;
		if(image[i].z > 1)
			image[i].z = 1;

		data[i * 3 + 0] = (int)(image[i].x * 255);
		data[i * 3 + 1] = (int)(image[i].y * 255);
		data[i * 3 + 2] = (int)(image[i].z * 255);
	}

	char *format = get_filename_ext(filename);

	if(strcmp(format, "png") == 0) {
		stbi_write_png(filename, width, height, 3, data, 0);

	} else if(strcmp(format, "bmp") == 0) {
		stbi_write_bmp(filename, width, height, 3, data);

	} else if(strcmp(format, "jpg") == 0) {
		stbi_write_jpg(filename, width, height, 3, data, 95);

	}

	free(data);
}

#endif

#endif
