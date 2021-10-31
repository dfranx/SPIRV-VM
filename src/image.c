#include <spvm/image.h>
#include <spvm/value.h>
#include <spvm/state.h>
#include <spvm/result.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>

void spvm_image_create_ext(struct spvm_image_data* dst, float* data,
		int width, int height, int depth, int layers, int levels) {
	dst->base.width = width;
	dst->base.height = height;
	dst->base.depth = depth;
	dst->base.layers = layers;
	dst->base.levels = levels;
	dst->data = (spvm_vec4f*) data;
}

void spvm_image_create(struct spvm_image_data* dst, float* data,
		int width, int height, int depth) {
	spvm_image_create_ext(dst, data, width, height, depth, 1, 1);
}

spvm_vec4f spvm_image_read(struct spvm_state* state, spvm_image* image,
	int x, int y, int z, int layer, int level)
{
	assert(state->read_image);
	return state->read_image(state, image, x, y, z, layer, level);
}

void spvm_image_write(struct spvm_state* state, spvm_image* image,
	int x, int y, int z, int layer, int level, const spvm_vec4f* data)
{
	assert(state->write_image);
	state->write_image(state, image, x, y, z, layer, level, data);
}

unsigned spvm_image_texel_id(struct spvm_image* image,
	int x, int y, int z, int layer, int level)
{
	unsigned width = image->width;
	unsigned height = image->height;
	unsigned depth = image->depth;
	unsigned off = 0u;

	for(int l = 0; l < level; ++l)
	{
		width = SPVM_MAX(width >> 1u, 1u);
		height = SPVM_MAX(height >> 1u, 1u);
		depth = SPVM_MAX(depth >> 1u, 1u);
		off += width * height * depth * image->layers;
	}

	unsigned sliceSize = width * height;
	unsigned layerSize = depth * sliceSize;

	off += layer * layerSize;
	off += z * sliceSize;
	off += y * width;
	off += x;

	return off;
}

spvm_vec4f spvm_image_read_impl(struct spvm_state* state, struct spvm_image* image,
	int x, int y, int z, int layer, int level)
{
	spvm_image_data* data = (spvm_image_data*) image;
	unsigned off = spvm_image_texel_id(image, x, y, z, layer, level);
	return data->data[off];
}

void spvm_image_write_impl(struct spvm_state* state, struct spvm_image* image,
	int x, int y, int z, int layer, int level, const spvm_vec4f* newval)
{
	spvm_image_data* data = (spvm_image_data*) image;
	unsigned off = spvm_image_texel_id(image, x, y, z, layer, level);
	data->data[off] = *newval;
}

// For the sampling implementation, see
// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap16.html#textures-texel-filtering
// There are a lot of corner cases we are not handling currently.

int spvm_util_mirror(int n)
{
	return (n >= 0) ? n : -(1 + n);
}

// Returns -1 for border.
int spvm_apply_address_mode(spvm_sampler_address_mode mode, int val, int size)
{
	switch(mode)
	{
		case spvm_sampler_address_mode_repeat:
			return ((val % size) + size) % size;
		case spvm_sampler_address_mode_clamp_to_edge:
			return SPVM_CLAMP(val, 0.f, size - 1);
		case spvm_sampler_address_mode_clamp_to_border:
			return (val < 0 || val >= size) ? -1 : val;
		case spvm_sampler_address_mode_mirrored_repeat: {
			int t = ((val % (2 * size)) + 2 * size) % (2 * size) - size;
			return (size - 1) * spvm_util_mirror(t);
		}
	}

	assert(0);
	return -1;
}

float spvm_frac(float val)
{
	double iptr;
	return modf(val, &iptr);
}

spvm_vec4f spvm_fetch_texel(struct spvm_state* state,
	spvm_image* img, const spvm_sampler_desc* desc, int x, int y, int z, int layer, int level)
{
	layer = SPVM_CLAMP(layer, 0, img->layers - 1);
	level = SPVM_CLAMP(level, 0, img->levels - 1);

	unsigned width = SPVM_MAX(img->width >> level, 1u);
	unsigned height = SPVM_MAX(img->height >> level, 1u);
	unsigned depth = SPVM_MAX(img->depth >> level, 1u);

	x = spvm_apply_address_mode(desc->address_mode_u, x, width);
	y = spvm_apply_address_mode(desc->address_mode_v, y, height);
	z = spvm_apply_address_mode(desc->address_mode_w, z, depth);

	// check for border condition
	if (x < 0 || y < 0 || z < 0) {
		return desc->border_color;
	}

	return spvm_image_read_impl(state, img, x, y, z, layer, level);
}

spvm_vec4f spvm_sampled_image_sample(struct spvm_state* state,
	spvm_image* img, spvm_sampler* sampler,
	float s, float t, float r, float layer, float level)
{
	spvm_sampler_desc* desc = &sampler->desc;

	level = SPVM_CLAMP(level + desc->mip_bias, desc->min_lod, desc->max_lod);
	spvm_sampler_filter filter = (level <= 0.f) ? desc->filter_mag : desc->filter_min;

	int levels[2];
	float level_weights[2];
	unsigned num_level_samples;

	if(desc->mipmap_mode == spvm_sampler_filter_nearest)
	{
		num_level_samples = 1u;
		levels[0] = roundf(level);
		levels[1] = levels[0];
		level_weights[0] = 1.f;
		level_weights[1] = 0.f;
	}
	else
	{
		num_level_samples = 2u;
		levels[0] = floor(level);
		levels[1] = levels[0] + 1;
		level_weights[0] = 1 - (level - levels[0]);
		level_weights[1] = level - levels[0];
	}

	spvm_vec4f res = {0.f};

	for(unsigned l = 0u; l < num_level_samples; ++l)
	{
		unsigned level = SPVM_CLAMP(levels[l], 0, img->levels - 1);
		unsigned width = SPVM_MAX(img->width >> level, 1u);
		unsigned height = SPVM_MAX(img->height >> level, 1u);
		unsigned depth = SPVM_MAX(img->depth >> level, 1u);

		float u = width * s;
		float v = height * t;
		float w = depth * r;

		const float shift = 0.5f;

		if (filter == spvm_sampler_filter_nearest)
		{
			int i = roundf(u - shift);
			int j = roundf(v - shift);
			int k  = roundf(w - shift);
			spvm_vec4f sample = spvm_fetch_texel(state, img, desc,
				i, j, k, roundf(layer), level);

			for(unsigned j = 0u; j < 4; ++j)
				res.data[j] += level_weights[l] * sample.data[j];
		}
		else
		{
			int i0 = floor(u - shift);
			int j0 = floor(v - shift);
			int k0 = floor(w - shift);

			int i1 = i0 + 1;
			int j1 = j0 + 1;
			int k1 = k0 + 1;

			float alpha = spvm_frac(u - shift);
			float beta = spvm_frac(v - shift);
			float gamma = spvm_frac(w - shift);

			for(unsigned s = 0u; s < 8; ++s)
			{
				int i = (s & 1) ? i0 : i1;
				int j = (s & 2) ? j0 : j1;
				int k = (s & 4) ? k0 : k1;

				float lin_weight =
					((s & 1) ? (1 - alpha) : alpha) *
					((s & 2) ? (1 - beta) : beta) *
					((s & 4) ? (1 - gamma) : gamma);

				spvm_vec4f sample = spvm_fetch_texel(state, img, desc,
					i, j, k, roundf(layer), level);

				for(unsigned j = 0u; j < 4; ++j)
					res.data[j] += lin_weight * level_weights[l] * sample.data[j];
			}

		}
	}

	return res;
}

struct spvm_sampled_image_lod_query spvm_sampled_image_query_lod(
		struct spvm_state* state, spvm_image* image, const struct spvm_image_info* imginfo,
		spvm_sampler* sampler, unsigned coord_id, float shader_lod_bias, float shader_lod_min)
{
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap16.html#textures-lod-and-scale-factor
	// TODO: handle proj
	// TODO: handle unnormalized sampling coords
	// TODO: handle cubemaps

	spvm_state_ddx(state, coord_id);
	spvm_state_ddy(state, coord_id);

	return spvm_sampled_image_query_lod_from_grad(state, image, imginfo,
		sampler, state->derivative_buffer_x, state->derivative_buffer_y,
		shader_lod_bias, shader_lod_min);
}

struct spvm_sampled_image_lod_query spvm_sampled_image_query_lod_from_grad(
	struct spvm_state* state, spvm_image* image, const struct spvm_image_info* imginfo,
	spvm_sampler* sampler, float* ddx, float* ddy,
	float shader_lod_bias, float shader_lod_min)
{

	float mx[3] = {0.f};
	float my[3] = {0.f};

	mx[0] = ddx[0] * image->width;
	my[0] = ddy[0] * image->width;

	if(imginfo->dim >= SpvDim2D) {
		mx[1] = ddx[1] * image->height;
		my[1] = ddy[1] * image->height;
	}

	if(imginfo->dim >= SpvDim3D) {
		mx[2] = ddx[2] * image->depth;
		my[2] = ddy[2] * image->depth;
	}

	float ro_x = sqrt(mx[0] * mx[0] + mx[1] * mx[1] + mx[2] * mx[2]);
	float ro_y = sqrt(my[0] * my[0] + my[1] * my[1] + my[2] * my[2]);

	float ro_max = SPVM_MAX(ro_x, ro_y);
	float ro_min = SPVM_MIN(ro_x, ro_y);

	// TODO: handle anisotropy
	const float eta = 1.f;

	float lambda_base = log2(ro_max / eta);

	// NOTE: we don't clamp to any maxSamplerLodBias here
	float lambda_prime = lambda_base + shader_lod_bias + sampler->desc.mip_bias;

	float lod_min = SPVM_MAX(shader_lod_min, sampler->desc.min_lod);
	float lod_max = sampler->desc.max_lod;

	float lambda = SPVM_CLAMP(lambda_prime, lod_min, lod_max);
	float dl = lambda;

	if (sampler->desc.mipmap_mode == spvm_sampler_filter_nearest) {
		dl = round(dl);
	}

	struct spvm_sampled_image_lod_query ret;
	ret.dl = dl;
	ret.lambda_prime = lambda_prime;

	return ret;
}

