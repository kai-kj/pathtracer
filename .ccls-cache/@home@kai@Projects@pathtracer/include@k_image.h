#ifndef K_IMAGE_H
#define K_IMAGE_H

typedef struct Image {
	int width;
	int height;
	unsigned char *data;
} Image;

void write_image(
	Image *image,
	char *fileName
);

void destroy_image(
	Image *image
);

#ifdef K_IMAGE_IMPLEMENTATION

#include <string.h>

#include "k_util.h"
#include "stb_image_write.h"

void write_image(Image *image, char *fileName) {
	char *format = get_file_ext(fileName);

	if(strcmp(format, "png") == 0) {
		stbi_write_png(fileName, image->width, image->height, 3, image->data, 0);

	} else if(strcmp(format, "bmp") == 0) {
		stbi_write_bmp(fileName, image->width, image->height, 3, image->data);

	} else if(strcmp(format, "jpg") == 0 || strcmp(format, "jpeg") == 0) {
		stbi_write_jpg(fileName, image->width, image->height, 3, image->data, 95);

	} else if(strcmp(format, "tga") == 0) {
		stbi_write_tga(fileName, image->width, image->height, 3, image->data);

	}
}

void destroy_image(Image *image) {
	free(image->data);
	free(image);
}

#endif

#endif
