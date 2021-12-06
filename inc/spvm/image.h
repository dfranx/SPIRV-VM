#ifndef __SPIRV_VM_IMAGE_H__
#define __SPIRV_VM_IMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct spvm_state;
struct spvm_image;
struct spvm_result;
struct spvm_image_info;

typedef struct spvm_vec4f {
	float data[4];
} spvm_vec4f;

typedef struct spvm_image {
	// Size values of the first (base) mip level accessible to the shader.
	// If the full image has size 1024x1024 but the imageView/texture bound
	// to the descriptor has firstLevel = 2, expects 256x256 here.
	unsigned width;
	unsigned height;
	unsigned depth;
	unsigned layers;
	unsigned levels;
	// TODO: support for multisampling
} spvm_image;

// Sampler definition, close to VkSamplerCreateInfo
typedef enum spvm_sampler_filter {
	spvm_sampler_filter_nearest,
	spvm_sampler_filter_linear,
} spvm_sampler_filter;

typedef enum spvm_sampler_address_mode {
	spvm_sampler_address_mode_repeat,
	spvm_sampler_address_mode_mirrored_repeat,
	spvm_sampler_address_mode_clamp_to_edge,
	spvm_sampler_address_mode_clamp_to_border,
} spvm_sampler_address_mode;

typedef enum spvm_sampler_compare_op {
	spvm_sampler_compare_op_never,
	spvm_sampler_compare_op_less,
	spvm_sampler_compare_op_equal,
	spvm_sampler_compare_op_less_or_equal,
	spvm_sampler_compare_op_greater,
	spvm_sampler_compare_op_not_equal,
	spvm_sampler_compare_op_greater_or_equal,
	spvm_sampler_compare_op_always,
} spvm_sampler_compare_op;

typedef struct spvm_sampler_desc {
	spvm_sampler_filter filter_min;
	spvm_sampler_filter filter_mag;
	spvm_sampler_filter mipmap_mode;
	spvm_sampler_address_mode address_mode_u;
	spvm_sampler_address_mode address_mode_v;
	spvm_sampler_address_mode address_mode_w;
	spvm_vec4f border_color;
	float mip_bias;
	spvm_sampler_compare_op compare_op;
	float min_lod;
	float max_lod;
	// TODO: no support for anisotropy yet
	// TODO: no support for unnormalized coordinates yet
} spvm_sampler_desc;

typedef struct spvm_sampler {
	spvm_sampler_desc desc;
} spvm_sampler;

spvm_vec4f spvm_image_read(struct spvm_state*, spvm_image*,
	int x, int y, int z, int layer, int level);
void spvm_image_write(struct spvm_state*, spvm_image*,
	int x, int y, int z, int layer, int level, const spvm_vec4f* data);
spvm_vec4f spvm_sampled_image_sample(struct spvm_state*, spvm_image*, spvm_sampler*,
	float x, float y, float z, float layer, float level);

// Applies the addressing mode from the sampler but always just
// reads a single texel.
spvm_vec4f spvm_fetch_texel(struct spvm_state* state,
	spvm_image* img, const spvm_sampler_desc* desc, int x, int y, int z, int layer, int level);

// spvm_image implementation
// Functions will interpret user_data as float* with tight layout
// and directly read/write it.
typedef struct spvm_image_data {
	struct spvm_image base;
	spvm_vec4f* data;
} spvm_image_data;

void spvm_image_create(struct spvm_image_data* dst, float* data, int width, int height, int depth);
void spvm_image_create_ext(struct spvm_image_data* dst, float* data,
	int width, int height, int depth, int layers, int levels);
spvm_vec4f spvm_image_read_impl(struct spvm_state*, struct spvm_image*,
	int x, int y, int z, int layer, int level);
void spvm_image_write_impl(struct spvm_state*, struct spvm_image*,
	int x, int y, int z, int layer, int level, const spvm_vec4f* data);

struct spvm_sampled_image_lod_query {
	float lambda_prime; // the level without any clamping/rounding
	float dl; // the selected level
};

// Expects the coords to be loaded into the derivative buffers of the given state.
struct spvm_sampled_image_lod_query spvm_sampled_image_query_lod(
	struct spvm_state*, spvm_image*, const struct spvm_image_info*, spvm_sampler*,
	unsigned coord_id, float shader_lod_bias, float shader_lod_min);

// For cubemaps, ddx and ddy must already correspond to the selected face.
struct spvm_sampled_image_lod_query spvm_sampled_image_query_lod_from_grad(
	struct spvm_state*, spvm_image*, const struct spvm_image_info*, spvm_sampler*,
	float* ddx/*[3]*/, float* ddy/*[3]*/, float shader_lod_bias, float shader_lod_min);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SPIRV_VM_IMAGE_H__
