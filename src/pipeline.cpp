#include "pipeline.hpp"

#include <map>
#include <list>

#include <cstddef>
#include <iostream>

void scene_t::add_object(const mesh &obj){
	objects.emplace_back(obj);
};

uint32_t scene_t::get_vertex_count() const{
	uint32_t size = 0;
	for(auto &obj : objects){
		size += obj.polygons.size() * 3;
	}
	return size;
};

void destroy_vertex_buffer(const vk::Device &device){
	// TO DO
}

namespace{

vk::Buffer create_vertex_buffer_f(const vk::Device &device, vk::DeviceSize size){
	const vk::BufferCreateInfo buffer_info = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
/*		.setSharingMode()
		.setQueueFamilyIndexCount()
		.setPQueueFamilyIndices()*/;
	return device.createBuffer(buffer_info);
}

vk::Buffer create_index_buffer_f(const vk::Device &device, vk::DeviceSize size){
	const vk::BufferCreateInfo buffer_info = vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(vk::BufferUsageFlagBits::eIndexBuffer)
/*		.setSharingMode()
		.setQueueFamilyIndexCount()
		.setPQueueFamilyIndices()*/;
	return device.createBuffer(buffer_info);
}

vk::DeviceMemory allocate_buffer_f(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, const vk::MemoryRequirements &mem_req){
	vk::PhysicalDeviceMemoryProperties mem_prop
		= physical_device.getMemoryProperties();

	const vk::MemoryAllocateInfo memory_info = vk::MemoryAllocateInfo()
		.setAllocationSize(mem_req.size)
		.setMemoryTypeIndex(memory_type_from_properties(mem_prop, mem_req,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent));
	return device.allocateMemory(memory_info);
}

vk::DescriptorBufferInfo create_buffer_info_f(
	const vk::Buffer &buf, const vk::MemoryRequirements &mem_req){
	return vk::DescriptorBufferInfo()
		.setBuffer(buf)
		.setOffset(0)
		.setRange(mem_req.size);
}

buffer_t create_vertex_buffer_f(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, vk::DeviceSize size){
	buffer_t vertex_buffer{};
	vertex_buffer.buf = create_vertex_buffer_f(device, size);
	vk::MemoryRequirements mem_req
		= device.getBufferMemoryRequirements(vertex_buffer.buf);
	vertex_buffer.mem = allocate_buffer_f(device, physical_device, mem_req);

	vertex_buffer.info
		= create_buffer_info_f(vertex_buffer.buf, mem_req);
	return vertex_buffer;
}

buffer_t create_index_buffer_f(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, vk::DeviceSize size){
	buffer_t index_buffer{};
	index_buffer.buf = create_index_buffer_f(device, size);
	vk::MemoryRequirements mem_req
		= device.getBufferMemoryRequirements(index_buffer.buf);
	index_buffer.mem = allocate_buffer_f(device, physical_device, mem_req);

	index_buffer.info
		= create_buffer_info_f(index_buffer.buf, mem_req);
	return index_buffer;
}

/*void store_vertex_data_f(const vk::Device &device, const buffer_t &buffer,
	const scene_t &scene){
	void *data_ptr = device.mapMemory(buffer.mem, buffer.info.offset,
		buffer.info.range, vk::MemoryMapFlags());

	std::ptrdiff_t offset = 0;
	const std::size_t polygon_size = sizeof(vertex) * 3;
	for(auto &obj : scene.objects){
		for(auto &poly : obj.polygons){
			memcpy(data_ptr + offset, poly.data.data(), polygon_size);
			offset += polygon_size;
		}
	}
	device.unmapMemory(buffer.mem);
}*/

void store_vertex_data_f(const vk::Device &device, const buffer_t &buffer,
	const std::vector<vertex> &verteces){
	void *data_ptr = device.mapMemory(buffer.mem, buffer.info.offset,
		buffer.info.range, vk::MemoryMapFlags());

	memcpy(data_ptr, verteces.data(), verteces.size() * sizeof(vertex));

	device.unmapMemory(buffer.mem);
}
void store_index_data_f(const vk::Device &device, const buffer_t &buffer,
	const std::vector<uint32_t> &indeces){
	void *data_ptr = device.mapMemory(buffer.mem, buffer.info.offset,
		buffer.info.range, vk::MemoryMapFlags());

	memcpy(data_ptr, indeces.data(), indeces.size() * sizeof(uint32_t));

	device.unmapMemory(buffer.mem);
}

void bind_buffer_f(const vk::Device &device, const buffer_t &buffer){
	device.bindBufferMemory(buffer.buf, buffer.mem, buffer.info.offset);
}

}

void pipeline_t::describing_vertex_data(){
	this->vi_binding = vk::VertexInputBindingDescription()
		.setBinding(0)
		.setStride(sizeof(vertex))
		.setInputRate(vk::VertexInputRate::eVertex);

	this->vi_attribs[0] = vk::VertexInputAttributeDescription()
		.setLocation(0)
		.setBinding(0)
		.setFormat(vk::Format::eR32G32B32A32Sfloat)
		.setOffset(0);
}

void pipeline_t::init_pipeline(const vk::Device &device){
	std::array<vk::DynamicState, VK_DYNAMIC_STATE_RANGE_SIZE> dynamic_state_enables;
	vk::PipelineDynamicStateCreateInfo dynamic_state = vk::PipelineDynamicStateCreateInfo()
		.setFlags(vk::PipelineDynamicStateCreateFlags())
		.setDynamicStateCount(0)
		.setPDynamicStates(dynamic_state_enables.data());

	dynamic_state_enables[dynamic_state.dynamicStateCount++]
		= vk::DynamicState::eViewport;
	dynamic_state_enables[dynamic_state.dynamicStateCount++]
		= vk::DynamicState::eScissor;

	using cc = vk::ColorComponentFlagBits;
	std::array<vk::PipelineColorBlendAttachmentState, 1> att_state{
		vk::PipelineColorBlendAttachmentState()
			.setBlendEnable(false)
			.setSrcColorBlendFactor(vk::BlendFactor::eZero)
			.setDstColorBlendFactor(vk::BlendFactor::eZero)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eZero)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd)
			.setColorWriteMask(cc::eR | cc::eG | cc::eB | cc::eA)
	};

	this->pipeline = device.createGraphicsPipeline(
		this->pipeline_cache,
		vk::GraphicsPipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags())
			.setStageCount(this->shader_stages.size())
			.setPStages(this->shader_stages.data())
			.setPVertexInputState(&vk::PipelineVertexInputStateCreateInfo()
				.setFlags(vk::PipelineVertexInputStateCreateFlags())
				.setVertexBindingDescriptionCount(1)
				.setPVertexBindingDescriptions(&this->vi_binding)
				.setVertexAttributeDescriptionCount(this->vi_attribs.size())
				.setPVertexAttributeDescriptions(this->vi_attribs.data()))
			.setPInputAssemblyState(&vk::PipelineInputAssemblyStateCreateInfo()
				.setFlags(vk::PipelineInputAssemblyStateCreateFlags())
				.setTopology(vk::PrimitiveTopology::eTriangleList)
				.setPrimitiveRestartEnable(false))
			.setPTessellationState(nullptr)
			.setPViewportState(&vk::PipelineViewportStateCreateInfo()
				.setFlags(vk::PipelineViewportStateCreateFlags())
				.setViewportCount(1)
				.setPViewports(nullptr)
				.setScissorCount(1)
				.setPScissors(nullptr))
			.setPRasterizationState(&vk::PipelineRasterizationStateCreateInfo()
				.setFlags(vk::PipelineRasterizationStateCreateFlags())
				.setDepthClampEnable(true)
				.setRasterizerDiscardEnable(false)
				.setPolygonMode(vk::PolygonMode::eFill)
				.setCullMode(vk::CullModeFlagBits::eBack)
				.setFrontFace(vk::FrontFace::eClockwise)
				.setDepthBiasEnable(false)
				.setDepthBiasConstantFactor(0)
				.setDepthBiasClamp(0)
				.setDepthBiasSlopeFactor(0)
				.setLineWidth(0))
			.setPMultisampleState(&vk::PipelineMultisampleStateCreateInfo()
				.setFlags(vk::PipelineMultisampleStateCreateFlags())
				.setRasterizationSamples(vk::SampleCountFlagBits::e1)
				.setSampleShadingEnable(false)
				.setMinSampleShading(0.0)
				.setPSampleMask(nullptr)
				.setAlphaToCoverageEnable(false)
				.setAlphaToOneEnable(false))
			.setPDepthStencilState(&vk::PipelineDepthStencilStateCreateInfo()
				.setFlags(vk::PipelineDepthStencilStateCreateFlags())
				.setDepthTestEnable(true)
				.setDepthWriteEnable(true)
				.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
				.setDepthBoundsTestEnable(false)
				.setStencilTestEnable(false)
				.setFront(vk::StencilOpState()
					.setFailOp(vk::StencilOp::eKeep)
					.setPassOp(vk::StencilOp::eKeep)
					.setDepthFailOp(vk::StencilOp::eKeep)
					.setCompareOp(vk::CompareOp::eAlways)
					.setCompareMask(0)
					.setWriteMask(0)
					.setReference(0))
				.setBack(vk::StencilOpState()
					.setFailOp(vk::StencilOp::eKeep)
					.setPassOp(vk::StencilOp::eKeep)
					.setDepthFailOp(vk::StencilOp::eKeep)
					.setCompareOp(vk::CompareOp::eAlways)
					.setCompareMask(0)
					.setWriteMask(0)
					.setReference(0))
				.setMinDepthBounds(0)
				.setMaxDepthBounds(0))
			.setPColorBlendState(&vk::PipelineColorBlendStateCreateInfo()
				.setFlags(vk::PipelineColorBlendStateCreateFlags())
				.setLogicOpEnable(false)
				.setLogicOp(vk::LogicOp::eNoOp)
				.setAttachmentCount(att_state.size())
				.setPAttachments(att_state.data())
				.setBlendConstants(std::array<float,4>{1.0f, 1.0f, 1.0f, 1.0f}))
			.setPDynamicState(&dynamic_state)
			.setLayout(this->pipeline_layout)
			.setRenderPass(this->render_pass)
			.setSubpass(0)
			.setBasePipelineHandle(vk::Pipeline())
			.setBasePipelineIndex(0)
	);
}

void pipeline_t::cmd_fill_render_pass(const vk::CommandBuffer &cmd_buffer,
	const vk::Framebuffer &frame, vk::Rect2D area) const{
	std::array<vk::ClearValue, 2> clear_values{
		vk::ClearValue().setColor(
			vk::ClearColorValue(std::array<float,4>{0.2f, 0.2f, 0.2f, 0.2f})),
		vk::ClearValue().setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0))
	};
	cmd_buffer.beginRenderPass(
		vk::RenderPassBeginInfo()
			.setRenderPass(this->render_pass)
			.setFramebuffer(frame)
			.setRenderArea(area)
			.setClearValueCount(clear_values.size())
			.setPClearValues(clear_values.data()),
		vk::SubpassContents::eInline
	);

	cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, this->pipeline);
	cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipeline_layout,
		0, this->desc_sets, std::vector<uint32_t>());

	cmd_buffer.bindVertexBuffers(0, std::vector<vk::Buffer>{
		this->scene_buffer.vertex_buffer.buf}, std::vector<vk::DeviceSize>{0});

	cmd_buffer.bindIndexBuffer(this->scene_buffer.index_buffer.buf,
		0, vk::IndexType::eUint32);

	cmd_buffer.setViewport(0, std::vector<vk::Viewport>{
		vk::Viewport()
		//	.setX(0)
		//	.setY(0)
			.setWidth(area.extent.width)
			.setHeight(area.extent.height)
			.setMinDepth((float)0.0f)
			.setMaxDepth((float)1.0f)
	});
	cmd_buffer.setScissor(0, std::vector<vk::Rect2D>{
		vk::Rect2D()
			.setOffset(vk::Offset2D(0, 0))
			.setExtent(vk::Extent2D(area.extent.width, area.extent.height))
	});

//	cmd_buffer.draw(this->vertex_count, 1, 0, 0);

	cmd_buffer.drawIndexed(this->scene_buffer.index_count, 1, 0, 0, 0);

	cmd_buffer.endRenderPass();
}

/*void pipeline_t::load_scene(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, const scene_t &scene){
	destroy_vertex_buffer(device);

	this->vertex_count = scene.get_vertex_count();
	this->vertex_buffer = create_vertex_buffer_f(device,
		physical_device, this->vertex_count * sizeof(polygon));

	store_vertex_data_f(device, this->vertex_buffer, scene);
	bind_buffer_f(device, this->vertex_buffer);
}*/

void pipeline_t::load_scene(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, const indeced_mash &mash){
	this->scene_buffer.vertex_count = mash.verteces.size();

	this->scene_buffer.vertex_buffer = create_vertex_buffer_f(device,
		physical_device, mash.verteces.size() * sizeof(vertex));

	store_vertex_data_f(device, this->scene_buffer.vertex_buffer, mash.verteces);
	bind_buffer_f(device, this->scene_buffer.vertex_buffer);

	this->scene_buffer.index_count = mash.indeces.size();

	this->scene_buffer.index_buffer = create_index_buffer_f(device,
		physical_device, mash.indeces.size() * sizeof(uint32_t));

	store_index_data_f(device, this->scene_buffer.index_buffer, mash.indeces);
	bind_buffer_f(device, this->scene_buffer.index_buffer);
}

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
		.setPName("main")
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

namespace{

buffer_t create_mvp_buffer(const vk::Device &device,
	const vk::PhysicalDevice &physical_device){
	buffer_t buf;
	buf.buf = device.createBuffer(vk::BufferCreateInfo()
		.setSize(get_mvp_buffer_size())
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer));

	vk::MemoryRequirements mem_req = device.getBufferMemoryRequirements(buf.buf);

	buf.mem = device.allocateMemory(vk::MemoryAllocateInfo()
		.setAllocationSize(mem_req.size)
		.setMemoryTypeIndex(memory_type_from_properties(
			physical_device.getMemoryProperties(), mem_req,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent)));
	buf.info = vk::DescriptorBufferInfo()
		.setBuffer(buf.buf)
		.setOffset(0)
		.setRange(VK_WHOLE_SIZE);

	device.bindBufferMemory(buf.buf, buf.mem, buf.info.offset);
	return buf;
}

}

void pipeline_t::init_graphic_pipeline(const vk::Device &device,
	const vk::PhysicalDevice &physical_device){
	this->mvp_buffer = create_mvp_buffer(device, physical_device);
	update_mvp_buffer(camera(), device, this->mvp_buffer);

	const std::vector<layout_f> layouts{
		layout_f{
			vk::DescriptorSetLayoutBinding()
				.setBinding(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1)
				.setStageFlags(vk::ShaderStageFlagBits::eVertex)
				.setPImmutableSamplers(nullptr),
			nullptr,
			&this->mvp_buffer.info,
			nullptr
		}
	};
	this->add_descriptor_set_layout(device, layouts);
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

	this->describing_vertex_data();
	this->init_pipeline(device);
}

void pipeline_t::init_render_pass(const vk::Device &device, const vk::Format &format){
	const std::array<vk::AttachmentDescription, 2> attachments {
		vk::AttachmentDescription()
			.setFormat(format)
			.setSamples(pipeline_t::num_samples)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR),

		vk::AttachmentDescription()
			.setFormat(this->depth.format)
			.setSamples(pipeline_t::num_samples)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
	};

	vk::AttachmentReference color_reference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depth_reference = vk::AttachmentReference()
		.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setInputAttachmentCount(0)
		.setPInputAttachments(nullptr)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_reference)
		.setPResolveAttachments(nullptr)
		.setPDepthStencilAttachment(&depth_reference)
		.setPreserveAttachmentCount(0)
		.setPPreserveAttachments(nullptr);

	const vk::RenderPassCreateInfo render_pass_info
		= vk::RenderPassCreateInfo()
		.setAttachmentCount(attachments.size())
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(0)
		.setPDependencies(nullptr);
	this->render_pass = device.createRenderPass(render_pass_info);
}

std::vector<vk::Framebuffer> pipeline_t::create_framebuffers(const vk::Device &device,
	const vk::PhysicalDevice &physical_device,
	const std::vector<swapchain_buffers_type> &buffers, vk::Extent2D window_size,
	const vk::Format &format){

	this->init_depth_buffer(device, physical_device, window_size);
	this->init_render_pass(device, format);

	std::array<vk::ImageView, 2> attachments{vk::ImageView{}, this->depth.view};

	vk::FramebufferCreateInfo framebuffer_info = vk::FramebufferCreateInfo()
		.setRenderPass(this->render_pass)
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
