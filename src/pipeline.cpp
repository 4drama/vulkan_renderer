#include "pipeline.hpp"

#include <map>
#include <list>

#include <iostream>

void pipeline_t::init_depth_buffer(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, vk::Extent2D window_size){
	this->depth.format = vk::Format::eD16Unorm;
	const vk::Format &format = this->depth.format;

	this->depth.image = create_depth_image(device, format,
		window_size, physical_device.getFormatProperties(format),
		this->num_samples);

	this->depth.mem = allocate_image_memory(device, physical_device,
		this->depth.image, vk::MemoryPropertyFlagBits::eDeviceLocal);

	device.bindImageMemory(this->depth.image, this->depth.mem, 0);

	this->depth.view = create_2d_image_view(device, this->depth.image, format,
		vk::ImageAspectFlagBits::eDepth);
}

namespace{

vk::PipelineShaderStageCreateInfo load_shader_f(
	const vk::Device &device, std::string path, vk::ShaderStageFlagBits stage,
	const vk::SpecializationInfo* specialization_info_ptr = nullptr){

	auto bytecode = load_file<char>(path);
	if(bytecode.size() == 0)
		throw std::runtime_error("Shader file not exist: " + path);

	const vk::ShaderModuleCreateInfo module_create_info =
		vk::ShaderModuleCreateInfo()
			.setCodeSize(bytecode.size())
			.setPCode(reinterpret_cast<uint32_t*>(bytecode.data()));

	return vk::PipelineShaderStageCreateInfo()
		.setStage(stage)
		.setPName(path.c_str())
		.setModule(device.createShaderModule(module_create_info))
		.setPSpecializationInfo(specialization_info_ptr);
}

vk::PipelineCache create_pipeline_cache_f(const vk::Device &device){

	const std::string pipeline_cache_path("./pipeline.cache");
	std::vector<uint8_t> cache_data = load_file<uint8_t>(pipeline_cache_path);

	const vk::PipelineCacheCreateInfo pipeline_cache_create_info =
		vk::PipelineCacheCreateInfo()
			.setInitialDataSize(cache_data.size())
			.setPInitialData(cache_data.data());

	vk::PipelineCache pipeline_cache =
		device.createPipelineCache(pipeline_cache_create_info);
	if(cache_data.size() == 0){
		cache_data = device.getPipelineCacheData(pipeline_cache);
		save_file<uint8_t>(pipeline_cache_path, cache_data);
	}
	return pipeline_cache;
}

}

void pipeline_t::init_graphic_pipeline(const vk::Device &device){
	const std::vector<layout_f> layouts{};
	this->add_descriptor_set_layout(device, layouts);
	/*	VkDescriptorSetLayoutBinding layout_binding = {};
		layout_binding.binding = 0;
		layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layout_binding.descriptorCount = 1;
		layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layout_binding.pImmutableSamplers = NULL; */
	this->init_const_range();

	this->init_pipeline_layouts(device);
	this->init_descriptor_pool(device, layouts);
	this->init_descriptor_sets(device/*, layouts*/);
	this->update_descriptor_sets(device, layouts);

	this->shader_stages[0] = load_shader_f(device,
		"./shaders/vert_shader.spv", vk::ShaderStageFlagBits::eVertex);

	this->shader_stages[1] = load_shader_f(device,
		"./shaders/frag_shader.spv", vk::ShaderStageFlagBits::eFragment);

	this->pipeline_cache = create_pipeline_cache_f(device);
}

std::vector<vk::Framebuffer> pipeline_t::create_framebuffers(const vk::Device &device,
	const std::vector<swapchain_buffers_type> &buffers, vk::Extent2D window_size) const{
	std::array<vk::ImageView, 2> attachments{vk::ImageView{}, this->depth.view};

	vk::FramebufferCreateInfo framebuffer_info = vk::FramebufferCreateInfo()
	//	.setRenderPass()
		.setAttachmentCount(attachments.size())
		.setPAttachments(attachments.data())
		.setWidth(window_size.width)
		.setHeight(window_size.height)
		.setLayers(1);

	std::vector<vk::Framebuffer> framebuffers{};
	for(auto &buf : buffers){
		attachments[0] = buf.view;
		framebuffers.emplace_back(device.createFramebuffer(framebuffer_info));
	}
	return framebuffers;
}

void pipeline_t::add_descriptor_set_layout(const vk::Device &device,
	const std::vector<layout_f> &layouts){

	if(layouts.size() != 0){
		std::vector<vk::DescriptorSetLayoutBinding> layout_bindings;
		for(auto &layout : layouts){
			layout_bindings.emplace_back(layout.descriptor_set_binding);
		}

		const vk::DescriptorSetLayoutCreateInfo desc_set_layout_info
			= vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(layout_bindings.size())
			.setPBindings(layout_bindings.data());

		this->desc_set_layout.emplace_back(
			device.createDescriptorSetLayout(desc_set_layout_info));
	}
}

void pipeline_t::init_const_range(){
	this->const_range = {};
}

void pipeline_t::init_pipeline_layouts(const vk::Device &device){
	const vk::PipelineLayoutCreateInfo pipeline_layout_info =
		vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount(this->desc_set_layout.size())
		.setPSetLayouts(this->desc_set_layout.data())
		.setPushConstantRangeCount(this->const_range.size())
		.setPPushConstantRanges(this->const_range.data());

	this->pipeline_layout = device.createPipelineLayout(pipeline_layout_info);
}

namespace{

uint32_t get_descriptor_sets_count_f(
	const std::vector<vk::DescriptorSetLayoutBinding> &layout_bindings){
	uint32_t counter = 0;
	for(auto &binding : layout_bindings){
		counter += binding.descriptorCount;
	}
	return counter;
}

std::vector<vk::DescriptorPoolSize> get_pool_size_f(
	const std::vector<layout_f> &layouts){

	std::map<vk::DescriptorType, uint32_t> tmp_pool_size{};
	std::list<vk::DescriptorType> keys{};
	for(auto &layout : layouts){
		const vk::DescriptorSetLayoutBinding &binding = layout.descriptor_set_binding;
		tmp_pool_size[binding.descriptorType] += binding.descriptorCount;
		keys.push_front(binding.descriptorType);
	}

	keys.unique();
	std::vector<vk::DescriptorPoolSize> pool_size{};
	for(auto& key : keys){
		pool_size.emplace_back(key, tmp_pool_size[key]);
	}
	return pool_size;
}

}

void pipeline_t::init_descriptor_pool(const vk::Device &device,
	const std::vector<layout_f> &layouts){

	std::vector<vk::DescriptorPoolSize> type_count
		= get_pool_size_f(layouts);

	const vk::DescriptorPoolCreateInfo descriptor_pool_info =
		vk::DescriptorPoolCreateInfo()	// or desc_sey lay. size? get_descriptor_sets_count_f(layout_bindings)
		.setMaxSets(this->desc_set_layout.size())
		.setPoolSizeCount(type_count.size())
		.setPPoolSizes(type_count.data());

	this->desc_pool = device.createDescriptorPool(descriptor_pool_info);
}

void pipeline_t::init_descriptor_sets(const vk::Device &device
/*	, const std::vector<layout_f> &layout*/){
	const vk::DescriptorSetAllocateInfo desc_sets_info =
		vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(this->desc_pool)	 // or desc_sey lay. size? get_descriptor_sets_count_f(layout_bindings)
		.setDescriptorSetCount(this->desc_set_layout.size())
		.setPSetLayouts(this->desc_set_layout.data());
	this->desc_sets = device.allocateDescriptorSets(desc_sets_info);
}


void pipeline_t::update_descriptor_sets(const vk::Device &device,
	const std::vector<layout_f> &layouts){
	const uint32_t size = this->desc_sets.size();
	std::vector<vk::WriteDescriptorSet> writes(size);
	for(uint32_t i = 0 ; i < size; ++i){
		writes[i] = vk::WriteDescriptorSet()
			.setDstSet(this->desc_sets[i])
			.setDstBinding(layouts[i].descriptor_set_binding.binding)
			.setDescriptorCount(layouts[i].descriptor_set_binding.descriptorCount)
			.setDescriptorType(layouts[i].descriptor_set_binding.descriptorType)
			.setPImageInfo(layouts[i].pImageInfo_)
			.setPBufferInfo(layouts[i].pBufferInfo_)
			.setPTexelBufferView(layouts[i].pTexelBufferView_);
	}
	device.updateDescriptorSets(writes, std::vector<vk::CopyDescriptorSet>{});
}
