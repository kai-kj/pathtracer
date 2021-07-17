#ifndef K_IMAGE_H
#define K_IMAGE_H

//---- definition ------------------------------------------------------------//

typedef struct k_Image {
	unsigned int width;
	unsigned int height;
	unsigned char *data;
} k_Image;

k_Image *k_create_image(
	const unsigned int width,
	const unsigned int height
);

int k_set_pixel_color(
	const k_Image *image,
	const unsigned int x, const unsigned int y,
	const unsigned char r, const unsigned char g, const unsigned char b
);

int k_write_image(
	const k_Image *image,
	const char *fileName
);

int k_destroy_image(
	k_Image *image
);

//---- implementation --------------------------------------------------------//

#ifdef K_IMAGE_IMPLEMENTATION

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <string.h>
#include "stb_image_write.h"

static char *_get_file_ext(const char *filename) {
	char *dot = strrchr(filename, '.');
	if(!dot || dot == filename) return "";
	return dot + 1;
}

k_Image *k_create_image(const unsigned int width, const unsigned int height) {
	k_Image *image = malloc(sizeof(k_Image));

	image->width = width;
	image->height = height;
	image->data = malloc(
		sizeof(unsigned char) * 3 * image->width * image->height
	);

	return image;
}

int k_set_pixel_color(
	const k_Image *image, const unsigned int x, const unsigned int y,
	const unsigned char r, const unsigned char g, const unsigned char b
) {
	if(x > image->width || y > image->height) {
		return -1;
	}

	int idx = x + y * image->width;

	image->data[idx * 3 + 0] = r;
	image->data[idx * 3 + 1] = g;
	image->data[idx * 3 + 2] = b;

	return 0;
}

int k_write_image(const k_Image *image, const char *fileName) {
	char *format = _get_file_ext(fileName);

	if(strcmp(format, "png") == 0) {
		stbi_write_png(
			fileName, image->width, image->height, 3, image->data, 0
		);

	} else if(strcmp(format, "bmp") == 0) {
		stbi_write_bmp(
			fileName, image->width, image->height, 3, image->data
		);

	} else if(strcmp(format, "jpg") == 0 || strcmp(format, "jpeg") == 0) {
		stbi_write_jpg(
			fileName, image->width, image->height, 3, image->data, 95
		);

	} else {
		return -1;
	}

	return 0;
}

int k_destroy_image(k_Image *image) {
	if(image != NULL) {
		free(image->data);
		free(image);
		image = NULL;
		return 0;
	
	} else {
		return -1;
	}
}

#endif

#endif
