//============================================================================//
// k_image.h                                                                //
//============================================================================//

//----------------------------------------------------------------------------//
// interface                                                                  //
//----------------------------------------------------------------------------//

#ifndef K_IMAGE_H
#define K_IMAGE_H

typedef struct Color {
	float r;
	float g;
	float b;
} Color;

typedef struct Image {
	int width;
	int height;
	Color *data;
} Image;

Color color_add(Color a, Color b);
Color color_sub(Color a, Color b);
Color color_mul(Color a, Color b);
Color color_add(Color a, Color b);
Color color_add_scalar(Color a, float b);
Color color_sub_scalar(Color a, float b);
Color color_mul_scalar(Color a, float b);
Color color_div_scalar(Color a, float b);
Image *create_image(unsigned int width, unsigned int height, Color bgColor);
void destroy_image(Image *image);
void gamma_correct_image(Image *image);
void write_image(Image *image, char *fileName);
void write_image_stb(Image *image, char *filename);

//----------------------------------------------------------------------------//
// implementation                                                             //
//----------------------------------------------------------------------------//

#ifdef K_IMAGE_IMPLEMENTATION

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

Color color_add(Color a, Color b) {
	return (Color){a.r + b.r, a.g + b.g, a.b + b.b};
}

Color color_sub(Color a, Color b) {
	return (Color){a.r - b.r, a.g - b.g, a.b - b.b};
}

Color color_mul(Color a, Color b) {
	return (Color){a.r * b.r, a.g * b.g, a.b * b.b};
}

Color color_div(Color a, Color b) {
	return (Color){a.r / b.r, a.g / b.g, a.b / b.b};
}

Color color_add_scalar(Color a, float b) {
	return (Color){a.r + b, a.g + b, a.b + b};
}

Color color_sub_scalar(Color a, float b) {
	return (Color){a.r - b, a.g - b, a.b - b};
}

Color color_mul_scalar(Color a, float b) {
	return (Color){a.r * b, a.g * b, a.b * b};
}

Color color_div_scalar(Color a, float b) {
	return (Color){a.r / b, a.g / b, a.b / b};
}

Image *create_image(unsigned int width, unsigned int height, Color bgColor) {
	Image *image = malloc(sizeof(Image));

	image->width = width;
	image->height = height;

	image->data = malloc(sizeof(Color) * width * height);

	for(unsigned int i = 0; i < width * height; i++) {
		image->data[i] = bgColor;
	}

	return image;
}

void destroy_image(Image *image) {
	free(image);
}

void gamma_correct_image(Image *image) {
	for(int i = 0; i < image->height; i++) {
		for(int j = 0; j < image->width; j++) {
			Color *pixel = &image->data[j + i * image->width];

			pixel->r = sqrt(pixel->r);
			pixel->g = sqrt(pixel->g);
			pixel->b = sqrt(pixel->b);
		}
	}
}

void write_image(Image *image, char *fileName) {
	FILE *fp = fopen(fileName, "wb");

	fprintf(fp, "P3 %d %d 255\n", image->width, image->height);

	for(int i = 0; i < image->height; i++) {
		for(int j = 0; j < image->width; j++) {
			Color *pixel = &image->data[j + i * image->width];

			if(pixel->r < 0)
				pixel->r = 0;
			if(pixel->g < 0)
				pixel->g = 0;
			if(pixel->b < 0)
				pixel->b = 0;
			
			if(pixel->r > 1)
				pixel->r = 1;
			if(pixel->g > 1)
				pixel->g = 1;
			if(pixel->b > 1)
				pixel->b = 1;

			fprintf(fp, "%d %d %d\n", (int)(pixel->r * 255), (int)(pixel->g * 255), (int)(pixel->b * 255));
		}
	}

	fclose(fp);

}

void write_image_stb(Image *image, char *filename) {
	unsigned char *data = malloc(image->width * image->height * 3);

	for(int i = 0; i < image->height; i++) {
		for(int j = 0; j < image->width; j++) {
			int pos = j + i * image->width;

			if(image->data[pos].r < 0)
				image->data[pos].r = 0;
			if(image->data[pos].g < 0)
				image->data[pos].g = 0;
			if(image->data[pos].b < 0)
				image->data[pos].b = 0;
			
			if(image->data[pos].r > 1)
				image->data[pos].r = 1;
			if(image->data[pos].g > 1)
				image->data[pos].g = 1;
			if(image->data[pos].b > 1)
				image->data[pos].b = 1;

			data[pos * 3 + 0] = (int)(image->data[pos].r * 255);
			data[pos * 3 + 1] = (int)(image->data[pos].g * 255);
			data[pos * 3 + 2] = (int)(image->data[pos].b * 255);
		}
	}

	char *format = get_filename_ext(filename);

	if(strcmp(format, "png") == 0) {
		stbi_write_png(filename, image->width, image->height, 3, data, 0);

	} else if(strcmp(format, "bmp") == 0) {
		stbi_write_bmp(filename, image->width, image->height, 3, data);

	} else if(strcmp(format, "jpg") == 0) {
		stbi_write_jpg(filename, image->width, image->height, 3, data, 95);

	}

	free(data);
}

#endif

#endif
