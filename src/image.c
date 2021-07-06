#include <spvm/image.h>
#include <spvm/value.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

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
	int x = roundf((img->width - 1) * CLAMP(s, 0.0f, 1.0f)); // nearest neighbor (?)
	int y = roundf((img->height - 1) * CLAMP(t, 0.0f, 1.0f));
	int z = (img->depth - 1) * CLAMP(fmodf(u, 1.0f), 0.0f, 1.0f);

	return img->data + (z * img->height * img->width + y * img->width + x) * 4;
}
float* spvm_image_read(spvm_image_t img, int x, int y, int z)
{
	return img->data + (z * img->height * img->width + y * img->width + x) * 4;
}
void spvm_image_write(spvm_image_t img, int x, int y, int z, float* rgba)
{
	if (x < img->width && y < img->height && z < img->depth)
		memcpy(img->data + (z * img->height * img->width + y * img->width + x) * 4, rgba, sizeof(float) * 4);
}