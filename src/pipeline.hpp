#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.hpp>
#include "windows.h"

#include "utils.hpp"
#include "desc_sets.hpp"

#include <map>

//struct layout_f;
class pipeline_t;

struct mesh{
	std::vector<polygon> polygons;
};


struct scene_t{
	std::vector<mesh> objects;

	void add_object(const mesh &obj);
	uint32_t get_vertex_count() const;
};

struct indeced_mash_vk{
	buffer_t vertex_buffer;
	buffer_t index_buffer;

	uint32_t vertex_count;
	uint32_t index_count;

	std::map<std::string, image_t>	textures;

	std::vector<material_t> materials;

	std::vector<material_range_t> no_texture_ranges;
	std::vector<material_range_t> texture_ranges;
	std::vector<material_range_t> transparency_ranges;

	void cmd_draw(vk::Device device, const pipeline_t *pipeline_ptr,
		const vk::CommandBuffer &cmd_buffer, const vk::PipelineLayout &pipeline_layout,
		const std::vector<vk::DescriptorSet> &desc_sets,
		const std::vector<material_range_t> &ranges) const;

	inline void cmd_no_tex_draw(vk::Device device, const pipeline_t *pipeline_ptr,
		const vk::CommandBuffer &cmd_buffer, const vk::PipelineLayout &pipeline_layout,
		const std::vector<vk::DescriptorSet> &desc_sets) const{
			cmd_draw(device, pipeline_ptr, cmd_buffer, pipeline_layout,
				desc_sets, no_texture_ranges);
		}

	inline void cmd_tex_draw(vk::Device device, const pipeline_t *pipeline_ptr,
		const vk::CommandBuffer &cmd_buffer, const vk::PipelineLayout &pipeline_layout,
		const std::vector<vk::DescriptorSet> &desc_sets) const{
			cmd_draw(device, pipeline_ptr, cmd_buffer, pipeline_layout,
				desc_sets, texture_ranges);
		}

	inline void cmd_trans_draw(vk::Device device, const pipeline_t *pipeline_ptr,
		const vk::CommandBuffer &cmd_buffer, const vk::PipelineLayout &pipeline_layout,
		const std::vector<vk::DescriptorSet> &desc_sets) const{
			cmd_draw(device, pipeline_ptr, cmd_buffer, pipeline_layout,
				desc_sets, transparency_ranges);
		}
};

class pipeline_t{
public:
	void update_camera(const vk::Device &device, camera cam);
	void cmd_fill_render_pass(vk::Device device, const vk::CommandBuffer &cmd_buffer,
		const vk::Framebuffer &frame, vk::Rect2D area) const;

	void load_scene(const vk::Device &device,
		const vk::PhysicalDevice &physical_device,
		const vk::CommandBuffer &cmd_buffer, const vk::Queue &queue,
		const indeced_mash &mash, vk::Format format);

	void init_depth_buffer(const vk::Device &device,
		const vk::PhysicalDevice &physical_device, vk::Extent2D window_size);

	void init_graphic_pipeline(const vk::Device &device,
		const vk::PhysicalDevice &physical_device);

	std::vector<vk::Framebuffer> create_framebuffers(
		const vk::Device &device,
		const vk::PhysicalDevice &physical_device,
		const std::vector<swapchain_buffers_type> &buffers,
		vk::Extent2D window_size,
		const vk::Format &format);

private:
	static constexpr vk::SampleCountFlagBits num_samples
		= vk::SampleCountFlagBits::e1;

	struct {
		vk::Format format;

		vk::Image image;
		vk::DeviceMemory mem;
		vk::ImageView view;

		vk::DescriptorImageInfo info;
	} depth;

	enum class SHADER_TYPE{
		VERT = 0,
		COLOR_FRAG = 1,
		TEXTURE_FRAG = 2,
		TRANC_FRAG = 3,
		SIZE = 4
	};
	std::array<vk::PipelineShaderStageCreateInfo, static_cast<int>(SHADER_TYPE::SIZE)>
		shader_stages;

//	std::vector<vk::DescriptorSetLayout> desc_set_layout;	// init_pipeline_layouts, to del
//	std::vector<vk::DescriptorSet> desc_sets;	//	bindDescriptorSets, to del

	descriptor_t vertex_descriptor;
	mutable descriptor_t fragment_descriptor;

	vk::DescriptorSetLayout texture_layout;
//	std::vector<vk::DescriptorSetLayout> texture_layouts;

	std::vector<vk::PushConstantRange> const_range;

	vk::PipelineLayout pipeline_layout;	// to del ???

	vk::DescriptorPool desc_pool;

	vk::PipelineCache pipeline_cache;

	enum class PIPELINE_TYPE{
		COLOR = 0,
		TEXTURE = 1,
		TRANSPARENCY = 2,
		SIZE = 3
	};
	vk::Pipeline pipeline[static_cast<int>(PIPELINE_TYPE::SIZE)];

	vk::RenderPass render_pass;

	indeced_mash_vk scene_buffer;
	vk::VertexInputBindingDescription vi_binding;
	std::array<vk::VertexInputAttributeDescription, 4> vi_attribs;

	buffer_t mvp_buffer;

	std::vector<vk::DescriptorPoolSize> add_descriptor_set_layout(
		const vk::Device &device, const std::vector<layout_f> &layouts);
	void init_const_range();

	void init_pipeline_layouts(const vk::Device &device);
	void init_descriptor_pool(const vk::Device &device,
		const std::vector<vk::DescriptorPoolSize> &type_count);

	void init_descriptor_sets(const vk::Device &device,
		const std::vector<layout_f> &layouts);

	void update_descriptor_sets(const vk::Device &device,
		const std::vector<layout_f> &layouts);

	void init_render_pass(const vk::Device &device, const vk::Format &format);

	void describing_vertex_data();

	void init_pipeline(const vk::Device &device);

//	void destroy_vertex_buffer(const vk::Device &device);
};

/*struct layout_f{
	vk::DescriptorSetLayoutBinding descriptor_set_binding;
	const vk::DescriptorImageInfo* pImageInfo_ = nullptr;
	const vk::DescriptorBufferInfo* pBufferInfo_ = nullptr;
	const vk::BufferView* pTexelBufferView_ = nullptr;
};*/

#endif
