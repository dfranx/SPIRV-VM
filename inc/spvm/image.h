#ifndef __SPIRV_VM_IMAGE_H__
#define __SPIRV_VM_IMAGE_H__

typedef struct spvm_image {
	float* data;

	int width;
	int height;
	int depth;
} spvm_image;
typedef spvm_image* spvm_image_t;

void spvm_image_create(spvm_image_t img, float* data, int width, int height, int depth);
float* spvm_image_sample(spvm_image_t img, float s, float t, float u);

#endif // __SPIRV_VM_IMAGE_H__