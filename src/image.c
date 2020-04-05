#include <spvm/image.h>
#include <string.h>

void spvm_image_create(spvm_image_t img, float* data, int width, int height, int depth)
{
	img->data = (float*)calloc(width * height * depth * 4, sizeof(float));
	memcpy(img->data, data, sizeof(float) * width * height * depth * 4);

	img->width = width;
	img->height = height;
	img->depth = depth;
}
float* spvm_image_sample(spvm_image_t img, float s, float t, float u)
{
	int x = (img->width - 1) * s;
	int y = (img->height - 1) * t;
	int z = (img->depth - 1) * u;

	return img->data + (z * img->height * img->width + y * img->width + x) * 4;
}