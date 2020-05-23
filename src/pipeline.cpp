#include "pipeline.hpp"

#include <map>
#include <list>
#include <algorithm>

#include <cstddef>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

}

void pipeline_t::describing_vertex_data(){
	this->vi_binding = vk::VertexInputBindingDescription()
		.setBinding(0)
		.setStride(sizeof(vertex))
		.setInputRate(vk::VertexInputRate::eVertex);

	this->vi_attribs[0] = vk::VertexInputAttributeDescription()
		.setLocation(0)
		.setBinding(0)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setOffset(0);

	this->vi_attribs[1] = vk::VertexInputAttributeDescription()
		.setLocation(1)
		.setBinding(0)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setOffset(12);

	this->vi_attribs[2] = vk::VertexInputAttributeDescription()
		.setLocation(2)
		.setBinding(0)
		.setFormat(vk::Format::eR32G32Sfloat)
		.setOffset(24);

	this->vi_attribs[3] = vk::VertexInputAttributeDescription()
		.setLocation(3)
		.setBinding(0)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setOffset(32);
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
	std::vector<vk::PipelineColorBlendAttachmentState> att_state{
		vk::PipelineColorBlendAttachmentState()
			.setBlendEnable(false)
			.setSrcColorBlendFactor(vk::BlendFactor::eZero)
			.setDstColorBlendFactor(vk::BlendFactor::eZero)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eZero)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd)
			.setColorWriteMask(cc::eR | cc::eG | cc::eB | cc::eA),
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

	const vk::PipelineVertexInputStateCreateInfo vertex_info =
		vk::PipelineVertexInputStateCreateInfo()
			.setFlags(vk::PipelineVertexInputStateCreateFlags())
			.setVertexBindingDescriptionCount(1)
			.setPVertexBindingDescriptions(&this->vi_binding)
			.setVertexAttributeDescriptionCount(this->vi_attribs.size())
			.setPVertexAttributeDescriptions(this->vi_attribs.data());
	const vk::PipelineInputAssemblyStateCreateInfo assembly_info =
		vk::PipelineInputAssemblyStateCreateInfo()
			.setFlags(vk::PipelineInputAssemblyStateCreateFlags())
			.setTopology(vk::PrimitiveTopology::eTriangleList)
			.setPrimitiveRestartEnable(false);
	const vk::PipelineViewportStateCreateInfo view_info =
		vk::PipelineViewportStateCreateInfo()
			.setFlags(vk::PipelineViewportStateCreateFlags())
			.setViewportCount(1)
			.setPViewports(nullptr)
			.setScissorCount(1)
			.setPScissors(nullptr);
	const vk::PipelineRasterizationStateCreateInfo rasterization_info =
		vk::PipelineRasterizationStateCreateInfo()
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
			.setLineWidth(0);
	const vk::PipelineMultisampleStateCreateInfo multisample_info =
		vk::PipelineMultisampleStateCreateInfo()
			.setFlags(vk::PipelineMultisampleStateCreateFlags())
			.setRasterizationSamples(vk::SampleCountFlagBits::e1)
			.setSampleShadingEnable(false)
			.setMinSampleShading(0.0)
			.setPSampleMask(nullptr)
			.setAlphaToCoverageEnable(false)
			.setAlphaToOneEnable(false);
	const vk::PipelineDepthStencilStateCreateInfo depth_stencil_info =
		vk::PipelineDepthStencilStateCreateInfo()
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
			.setMaxDepthBounds(0);
	const vk::PipelineColorBlendStateCreateInfo color_blend_info =
		vk::PipelineColorBlendStateCreateInfo()
			.setFlags(vk::PipelineColorBlendStateCreateFlags())
			.setLogicOpEnable(false)
			.setLogicOp(vk::LogicOp::eNoOp)
			.setAttachmentCount(att_state.size())
			.setPAttachments(att_state.data())
			.setBlendConstants(std::array<float,4>{1.0f, 1.0f, 1.0f, 1.0f});

	const std::vector<vk::PipelineShaderStageCreateInfo> color_stages{
		shader_stages[static_cast<int>(SHADER_TYPE::VERT)],
		shader_stages[static_cast<int>(SHADER_TYPE::COLOR_FRAG)]
	};

	this->pipeline[static_cast<int>(PIPELINE_TYPE::COLOR)] = device.createGraphicsPipeline(
		this->pipeline_cache,
		vk::GraphicsPipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags())
			.setStageCount(color_stages.size())
			.setPStages(color_stages.data())
			.setPVertexInputState(&vertex_info)
			.setPInputAssemblyState(&assembly_info)
			.setPTessellationState(nullptr)
			.setPViewportState(&view_info)
			.setPRasterizationState(&rasterization_info)
			.setPMultisampleState(&multisample_info)
			.setPDepthStencilState(&depth_stencil_info)
			.setPColorBlendState(&color_blend_info)
			.setPDynamicState(&dynamic_state)
			.setLayout(this->pipeline_layout)
			.setRenderPass(this->render_pass)
			.setSubpass(0)
			.setBasePipelineHandle(vk::Pipeline())
			.setBasePipelineIndex(0)
	);

	const std::vector<vk::PipelineShaderStageCreateInfo> texture_stages{
		shader_stages[static_cast<int>(SHADER_TYPE::VERT)],
		shader_stages[static_cast<int>(SHADER_TYPE::TEXTURE_FRAG)]
	};

	this->pipeline[static_cast<int>(PIPELINE_TYPE::TEXTURE)] = device.createGraphicsPipeline(
		this->pipeline_cache,
		vk::GraphicsPipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags())
			.setStageCount(texture_stages.size())
			.setPStages(texture_stages.data())
			.setPVertexInputState(&vertex_info)
			.setPInputAssemblyState(&assembly_info)
			.setPTessellationState(nullptr)
			.setPViewportState(&view_info)
			.setPRasterizationState(&rasterization_info)
			.setPMultisampleState(&multisample_info)
			.setPDepthStencilState(&depth_stencil_info)
			.setPColorBlendState(&color_blend_info)
			.setPDynamicState(&dynamic_state)
			.setLayout(this->pipeline_layout)
			.setRenderPass(this->render_pass)
			.setSubpass(1)
			.setBasePipelineHandle(vk::Pipeline())
			.setBasePipelineIndex(0)
	);

	const std::array<vk::PipelineShaderStageCreateInfo, 2> tranc_stages{
		shader_stages[static_cast<int>(SHADER_TYPE::VERT)],
		shader_stages[static_cast<int>(SHADER_TYPE::TRANC_FRAG)]
	};

	att_state = {
		vk::PipelineColorBlendAttachmentState()
			.setBlendEnable(true)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd)
			.setColorWriteMask(cc::eR | cc::eG | cc::eB | cc::eA)
	};

	this->pipeline[static_cast<int>(PIPELINE_TYPE::TRANSPARENCY)] = device.createGraphicsPipeline(
		this->pipeline_cache,
		vk::GraphicsPipelineCreateInfo()
			.setFlags(vk::PipelineCreateFlags())
			.setStageCount(tranc_stages.size())
			.setPStages(tranc_stages.data())
			.setPVertexInputState(&vertex_info)
			.setPInputAssemblyState(&assembly_info)
			.setPTessellationState(nullptr)
			.setPViewportState(&view_info)
			.setPRasterizationState(&rasterization_info)
			.setPMultisampleState(&multisample_info)
			.setPDepthStencilState(&depth_stencil_info)
			.setPColorBlendState(&color_blend_info)
			.setPDynamicState(&dynamic_state)
			.setLayout(this->pipeline_layout)
			.setRenderPass(this->render_pass)
			.setSubpass(2)
			.setBasePipelineHandle(vk::Pipeline())
			.setBasePipelineIndex(0)
	);
}

void pipeline_t::update_camera(const vk::Device &device, camera cam){
	update_mvp_buffer(cam, device, this->mvp_buffer);
}

void indeced_mash_vk::cmd_draw(vk::Device device, const pipeline_t *pipeline_ptr,
	const vk::CommandBuffer &cmd_buffer, const vk::PipelineLayout &pipeline_layout,
	const std::vector<vk::DescriptorSet> &desc_sets,
	const std::vector<material_range_t> &ranges) const{

	cmd_buffer.bindVertexBuffers(0, std::vector<vk::Buffer>{
		this->vertex_buffer.buf}, std::vector<vk::DeviceSize>{0});

	cmd_buffer.bindIndexBuffer(this->index_buffer.buf,
		0, vk::IndexType::eUint32);

	for(auto &m_range : ranges){
		std::vector<vk::DescriptorSet> upd_desc_set = desc_sets;
		const material_t& material = materials[m_range.id];
/*		if(!material.diffuse_texname.empty()){
			upd_desc_set.emplace_back(material.texture_desc);
		} else {
			upd_desc_set.emplace_back(materials[0].texture_desc);
		}*/
		upd_desc_set.emplace_back(material.material_desc);

		cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
			pipeline_layout, 0, upd_desc_set, std::vector<uint32_t>());

		cmd_buffer.drawIndexed(m_range.range, 1, m_range.offset, 0, 0);
	}
}

void pipeline_t::cmd_fill_render_pass(vk::Device device,
	const vk::CommandBuffer &cmd_buffer, const vk::Framebuffer &frame,
	vk::Rect2D area) const{
	std::array<vk::ClearValue, 3> clear_values{
		vk::ClearValue().setColor(
			vk::ClearColorValue(std::array<float,4>{0.2f, 0.2f, 0.2f, 0.2f})),
		vk::ClearValue().setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0)),
		vk::ClearValue().setColor(
			vk::ClearColorValue(std::array<float,4>{0.2f, 0.2f, 0.2f, 0.2f}))
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

	std::vector<vk::DescriptorSet> desc_sets(this->vertex_descriptor.sets); //to del
	desc_sets.insert(desc_sets.cend(),
		this->fragment_descriptor.sets.begin(), this->fragment_descriptor.sets.end());

/*	cmd_buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, this->pipeline_layout,
		0, desc_sets, std::vector<uint32_t>());*/

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

	cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		this->pipeline[static_cast<int>(PIPELINE_TYPE::COLOR)]);

	this->scene_buffer.cmd_no_tex_draw(device, this, cmd_buffer,
		this->pipeline_layout, desc_sets);

	cmd_buffer.nextSubpass(vk::SubpassContents::eInline);

	cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		this->pipeline[static_cast<int>(PIPELINE_TYPE::TEXTURE)]);

	this->scene_buffer.cmd_tex_draw(device, this, cmd_buffer,
		this->pipeline_layout, desc_sets);

	cmd_buffer.nextSubpass(vk::SubpassContents::eInline);

	cmd_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
		this->pipeline[static_cast<int>(PIPELINE_TYPE::TRANSPARENCY)]);

	this->scene_buffer.cmd_trans_draw(device, this, cmd_buffer,
		this->pipeline_layout, desc_sets);

	cmd_buffer.endRenderPass();
}


namespace{

struct host_to_device_f{

	buffer_t host_stage_buffer;
	buffer_t device_buffer;

	vk::BufferMemoryBarrier get_begin_transfer_barrier(vk::AccessFlags srcAccessMask){
		return vk::BufferMemoryBarrier()
			.setSrcAccessMask(srcAccessMask)
			.setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setBuffer(device_buffer.buf)
			.setOffset(0)
			.setSize(VK_WHOLE_SIZE);
	}

	vk::BufferMemoryBarrier get_end_transfer_barrier(vk::AccessFlags dstAccessMask){
		return vk::BufferMemoryBarrier()
			.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
			.setDstAccessMask(dstAccessMask)
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setBuffer(device_buffer.buf)
			.setOffset(0)
			.setSize(VK_WHOLE_SIZE);
	}
};

host_to_device_f create_buffers_f(const vk::Device &device,
	vk::PhysicalDeviceMemoryProperties mem_prop,
	vk::BufferUsageFlags usage_flag, vk::DeviceSize size){

	vk::MemoryPropertyFlags stage_property =
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;

	vk::MemoryPropertyFlags device_property =
		vk::MemoryPropertyFlagBits::eDeviceLocal;

	host_to_device_f buffers{};

	buffers.host_stage_buffer = create_buffer(device, mem_prop,
		usage_flag | vk::BufferUsageFlagBits::eTransferSrc,
		stage_property, size);

	buffers.device_buffer = create_buffer(device, mem_prop,
		usage_flag | vk::BufferUsageFlagBits::eTransferDst,
		device_property, size);

	return buffers;
}

struct stage_vertex_buffer_f{
	host_to_device_f vertex_buffer;
	host_to_device_f index_buffer;
};

stage_vertex_buffer_f create_vertex_stage_buffer_f(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, const indeced_mash &mash){

	const vk::PhysicalDeviceMemoryProperties mem_prop =
		physical_device.getMemoryProperties();

	const uint32_t vertex_count = mash.verteces.size();
	stage_vertex_buffer_f stage_buf{};
	stage_buf.vertex_buffer = create_buffers_f(device, mem_prop,
		vk::BufferUsageFlagBits::eVertexBuffer, vertex_count * sizeof(vertex));

	const uint32_t index_count = mash.indeces.size();
	stage_buf.index_buffer = create_buffers_f(device, mem_prop,
		vk::BufferUsageFlagBits::eIndexBuffer, index_count * sizeof(uint32_t));

	return stage_buf;
}

void update_host_vertex_stage_buffer_f(const vk::Device &device, const indeced_mash &mash,
	stage_vertex_buffer_f *buffer_ptr){

	store_vertex_data_f(device, buffer_ptr->vertex_buffer.host_stage_buffer, mash.verteces);
	store_index_data_f(device, buffer_ptr->index_buffer.host_stage_buffer, mash.indeces);
}

void cmd_to_device_memory_f(std::vector<host_to_device_f*> src_buffers,
	vk::CommandBuffer cmd_buf, vk::PipelineStageFlags final_consuming_stages){

	std::vector<vk::BufferMemoryBarrier> buffer_memory_barriers{};
	for(auto &buf : src_buffers){
		buffer_memory_barriers.emplace_back(
			buf->get_begin_transfer_barrier(vk::AccessFlags()));
	}

	vk::PipelineStageFlags generating_stages = vk::PipelineStageFlagBits::eTopOfPipe;
	vk::PipelineStageFlags consuming_stages = vk::PipelineStageFlagBits::eTransfer;

	cmd_buf.pipelineBarrier(generating_stages, consuming_stages, vk::DependencyFlags(),
		std::vector<vk::MemoryBarrier>(), buffer_memory_barriers,
		std::vector<vk::ImageMemoryBarrier>());

	for(auto &buf : src_buffers){
		cmd_buf.copyBuffer(buf->host_stage_buffer.buf, buf->device_buffer.buf,
			std::vector<vk::BufferCopy>{
				{
					buf->host_stage_buffer.info.offset,
					buf->device_buffer.info.offset,
					buf->host_stage_buffer.info.range
				}
			});
	}

	buffer_memory_barriers.clear();
	for(auto &buf : src_buffers){
		buffer_memory_barriers.emplace_back(
			buf->get_end_transfer_barrier(vk::AccessFlags()));
	}

	generating_stages = vk::PipelineStageFlagBits::eTransfer;
	consuming_stages = final_consuming_stages;

	cmd_buf.pipelineBarrier(generating_stages, consuming_stages, vk::DependencyFlags(),
		std::vector<vk::MemoryBarrier>(), buffer_memory_barriers,
		std::vector<vk::ImageMemoryBarrier>());
}

void load_to_device_memory_f(
	vk::Device device, vk::CommandBuffer cmd_buffer, vk::Queue queue,
	std::vector<host_to_device_f*> buffers){

	cmd_buffer.begin(vk::CommandBufferBeginInfo(
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	cmd_to_device_memory_f(buffers, cmd_buffer,
		vk::PipelineStageFlagBits::eBottomOfPipe);

	cmd_buffer.end();

	vk::Fence fance = device.createFence(vk::FenceCreateInfo());
	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eBottomOfPipe;
	queue.submit(std::array<vk::SubmitInfo, 1>{
		vk::SubmitInfo()
			.setWaitSemaphoreCount(0)
			.setPWaitSemaphores(nullptr)
			.setPWaitDstStageMask(&pipe_stage_flags)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmd_buffer)
			.setSignalSemaphoreCount(0)
			.setPSignalSemaphores(nullptr)
	}, fance);

	while(device.waitForFences(fance, true, 10000000) != vk::Result::eSuccess);

	device.destroy(fance);
}

struct buffer_to_image_f{
	std::string filename;
	buffer_t buffer;

	int width = 0;
	int height = 0;
};

buffer_to_image_f stage_texture_f(
	std::string path, std::string filename, vk::Device device,
	vk::PhysicalDeviceMemoryProperties mem_prop){
	int num_requested_components = 4;

	int width = 0;
	int height = 0;
	int num_components = 0;

	unsigned char *stbi_data_ptr = stbi_load( std::string(path + filename).c_str(),
		&width, &height, &num_components, num_requested_components);

	if( (!stbi_data_ptr) || (0 >= width) || (0 >= height) || (0 >= num_components) )
		throw std::runtime_error("Could not read image!");

	int data_size = width * height *
		(0 < num_requested_components ? num_requested_components : num_components);

	buffer_to_image_f result{};
	result.filename = filename;
	result.buffer = create_buffer(device, mem_prop, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible |
		vk::MemoryPropertyFlagBits::eHostCoherent, data_size);
	result.width = width;
	result.height = height;

	void *data_ptr = device.mapMemory(result.buffer.mem, result.buffer.info.offset,
		result.buffer.info.range, vk::MemoryMapFlags());

	memcpy(data_ptr, stbi_data_ptr, data_size);

	device.unmapMemory(result.buffer.mem);
	stbi_image_free( stbi_data_ptr );
	return result;
}

void cmd_load_to_device_image_f(vk::CommandBuffer cmd_buffer,
	buffer_to_image_f host_buffer, image_t device_image){

	cmd_buffer.copyBufferToImage(host_buffer.buffer.buf, device_image.img,
		vk::ImageLayout::eTransferDstOptimal, std::array<vk::BufferImageCopy, 1>{
		vk::BufferImageCopy()
		/*	.setBufferOffset(0)
			.setBufferRowLength(host_buffer.width)
			.setBufferImageHeight(host_buffer.height)*/
			.setImageSubresource(vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setMipLevel(0)
				.setBaseArrayLayer(0)
				.setLayerCount(1))
			.setImageOffset(vk::Offset3D(0, 0, 0))
			.setImageExtent(vk::Extent3D(host_buffer.width, host_buffer.height, 1))
		});
}

void load_to_device_memory_f(
	vk::Device device, vk::CommandBuffer cmd_buffer, vk::Queue queue,
	std::map<std::string, image_t> *device_images_ptr,
	std::vector<buffer_to_image_f> *host_buffers_ptr){

	cmd_buffer.begin(vk::CommandBufferBeginInfo(
		vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	for(auto &host_buffer: *host_buffers_ptr){
		cmd_load_to_device_image_f(cmd_buffer, host_buffer,
			device_images_ptr->at(host_buffer.filename));
	}

	cmd_buffer.end();

	vk::Fence fance = device.createFence(vk::FenceCreateInfo());
	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eBottomOfPipe;
	queue.submit(std::array<vk::SubmitInfo, 1>{
		vk::SubmitInfo()
			.setWaitSemaphoreCount(0)
			.setPWaitSemaphores(nullptr)
			.setPWaitDstStageMask(&pipe_stage_flags)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmd_buffer)
			.setSignalSemaphoreCount(0)
			.setPSignalSemaphores(nullptr)
	}, fance);

	while(device.waitForFences(fance, true, 10000000) != vk::Result::eSuccess);

	device.destroy(fance);
}

std::map<std::string, image_t> load_textures_f(vk::PhysicalDevice physical_device,
	vk::Device device, std::string path, const std::vector<material_t> *materials,
	vk::Format format, vk::CommandBuffer cmd_buffer, vk::Queue queue){

	const vk::FormatProperties format_properties =
		physical_device.getFormatProperties(format);
	const vk::PhysicalDeviceMemoryProperties memory_properties =
		physical_device.getMemoryProperties();

	std::vector<std::string> textures_names{};
	for(auto &mat : *materials){
		if(mat.diffuse_texname.size() != 0){
			textures_names.emplace_back(mat.diffuse_texname);
		}
	}

	std::sort(textures_names.begin(), textures_names.end());
	auto last_iter = std::unique(textures_names.begin(), textures_names.end());
	textures_names.erase(last_iter, textures_names.end());

	std::vector<buffer_to_image_f> stage_buffers{};
	stage_buffers.reserve(textures_names.size());
	for(auto &name : textures_names){
		stage_buffers.emplace_back(stage_texture_f(path, name, device, memory_properties));
	}

	std::map<std::string, image_t> device_local_textures{};
	for(auto &stage_buffer : stage_buffers){
		device_local_textures[stage_buffer.filename] = create_image(device, format,
			format_properties, false,
			vk::Extent3D(stage_buffer.width, stage_buffer.height, 1), 1, 1,
			vk::SampleCountFlagBits::e1,
			vk::ImageUsageFlagBits::eColorAttachment |
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::ImageLayout::eTransferDstOptimal,
			vk::ImageAspectFlagBits::eColor, false, memory_properties,
			vk::MemoryPropertyFlagBits::eDeviceLocal);;
	}

	load_to_device_memory_f(device, cmd_buffer, queue,
		&device_local_textures, &stage_buffers);

	for(auto &stage_buffer : stage_buffers){
		stage_buffer.filename.clear();
		destroy(device, stage_buffer.buffer);
		stage_buffer.width = 0;
		stage_buffer.height = 0;
	}
	return device_local_textures;
}

void load_transparencies_f(vk::PhysicalDevice physical_device, vk::Device device,
	vk::CommandBuffer cmd_buf, vk::Queue queue, std::vector<material_t> &materials){

	const vk::PhysicalDeviceMemoryProperties memory_properties =
		physical_device.getMemoryProperties();

	std::vector<buffer_t> tmp_buffers;
	cmd_buf.begin(vk::CommandBufferBeginInfo(
			vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

	for(auto &material : materials){
		constexpr std::size_t data_size = sizeof(float);

		buffer_t buf = create_buffer(device, memory_properties,
			vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible |
			vk::MemoryPropertyFlagBits::eHostCoherent, data_size);
		tmp_buffers.emplace_back(buf);

		void *data_ptr = device.mapMemory(buf.mem, buf.info.offset,
			buf.info.range, vk::MemoryMapFlags());
		memcpy(data_ptr, &material.dissolve, data_size);
		device.unmapMemory(buf.mem);

		material.transparency_buff = create_buffer(device, memory_properties,
			vk::BufferUsageFlagBits::eTransferDst |
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal, data_size);

		cmd_buf.copyBuffer(buf.buf, material.transparency_buff.buf,
			std::vector<vk::BufferCopy>{
				{buf.info.offset, buf.info.offset, buf.info.range}
			});
	}
	cmd_buf.end();

	vk::Fence fance = device.createFence(vk::FenceCreateInfo());
	vk::PipelineStageFlags pipe_stage_flags = vk::PipelineStageFlagBits::eBottomOfPipe;
	queue.submit(std::array<vk::SubmitInfo, 1>{
		vk::SubmitInfo()
			.setWaitSemaphoreCount(0)
			.setPWaitSemaphores(nullptr)
			.setPWaitDstStageMask(&pipe_stage_flags)
			.setCommandBufferCount(1)
			.setPCommandBuffers(&cmd_buf)
			.setSignalSemaphoreCount(0)
			.setPSignalSemaphores(nullptr)
	}, fance);

	while(device.waitForFences(fance, true, 10000000) != vk::Result::eSuccess);
	device.destroy(fance);
	for(auto &buf : tmp_buffers)
		destroy(device, buf);
}

}

namespace{
	void butch(indeced_mash_vk& mash,
		const std::vector<material_range_t> &materials_ranges){

		uint32_t offset = 0;
			for(auto &range : materials_ranges){
				const material_t& material = mash.materials[range.id];
			//	std::cerr << range.id << ' ' << material.dissolve << std::endl;
				if(material.dissolve == 1){

					if(!material.diffuse_texname.empty()){
						mash.texture_ranges.emplace_back(
							material_range_t{range.id, range.range, offset});
					} else {
						mash.no_texture_ranges.emplace_back(
							material_range_t{range.id, range.range, offset});
					}
				} else {
					mash.transparency_ranges.emplace_back(
						material_range_t{range.id, range.range, offset});
				}
				offset += range.range;
			}
	}
}

void pipeline_t::load_scene(const vk::Device &device,
	const vk::PhysicalDevice &physical_device,
	const vk::CommandBuffer &cmd_buffer, const vk::Queue &queue,
	const indeced_mash &mash, vk::Format format){

	stage_vertex_buffer_f vertex_tmp_buffer = create_vertex_stage_buffer_f(device,
		physical_device, mash);
	update_host_vertex_stage_buffer_f(device, mash, &vertex_tmp_buffer);

	load_to_device_memory_f(device, cmd_buffer, queue, std::vector<host_to_device_f*>{
		&vertex_tmp_buffer.vertex_buffer , &vertex_tmp_buffer.index_buffer});

	destroy(device, vertex_tmp_buffer.vertex_buffer.host_stage_buffer);
	destroy(device, vertex_tmp_buffer.index_buffer.host_stage_buffer);

	this->scene_buffer.vertex_buffer = vertex_tmp_buffer.vertex_buffer.device_buffer;
	this->scene_buffer.vertex_count = mash.verteces.size();

	this->scene_buffer.index_buffer = vertex_tmp_buffer.index_buffer.device_buffer;
	this->scene_buffer.index_count = mash.indeces.size();

	this->scene_buffer.textures = load_textures_f(physical_device, device,
		mash.path, &mash.materials, format, cmd_buffer, queue);

	this->scene_buffer.materials = mash.materials;
	load_transparencies_f(physical_device, device, cmd_buffer, queue,
		this->scene_buffer.materials);

	butch(this->scene_buffer, mash.materials_ranges);
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

/*	this->depth.view = create_2d_image_view(device, this->depth.image, format,
		vk::ImageAspectFlagBits::eDepth);*/

	this->depth.info = vk::DescriptorImageInfo()
		.setSampler(vk::Sampler())

		.setImageView(create_2d_image_view(device, this->depth.image, format,
				vk::ImageAspectFlagBits::eDepth))
		.setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
}

void pipeline_t::init_inside_color_buffer(const vk::Device &device, const vk::Format &format,
	const vk::PhysicalDevice &physical_device, vk::Extent2D window_size){

	const vk::FormatProperties format_properties =
		physical_device.getFormatProperties(format);
	const vk::PhysicalDeviceMemoryProperties memory_properties =
		physical_device.getMemoryProperties();

	auto image = create_image(device, format,
		format_properties, false,
		vk::Extent3D(window_size.width, window_size.height, 1), 1, 1,
		vk::SampleCountFlagBits::e1,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eInputAttachment,
		vk::ImageLayout::eUndefined,
		vk::ImageAspectFlagBits::eColor, false, memory_properties,
		vk::MemoryPropertyFlagBits::eDeviceLocal);;

	this->inside_color.format = format;

	this->inside_color.image = image.img;
	this->inside_color.mem = image.mem;
	this->inside_color.info = image.info;
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

buffer_t create_buffer_f(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, std::size_t size,
	vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eUniformBuffer){
	buffer_t buf;
	buf.buf = device.createBuffer(vk::BufferCreateInfo()
		.setSize(size)
		.setUsage(usage));

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
	this->mvp_buffer = create_buffer_f(device, physical_device, get_mvp_buffer_size());

	this->vertex_descriptor = descriptor_t(
		device,
		std::vector<layout_f>{
			{
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
		}
	);

	this->fragment_descriptor = descriptor_t(
		device,
		std::vector<layout_f>{
			{
				vk::DescriptorSetLayoutBinding()
					.setBinding(3)
					.setDescriptorType(vk::DescriptorType::eInputAttachment)
					.setDescriptorCount(1)
					.setStageFlags(vk::ShaderStageFlagBits::eFragment)
					.setPImmutableSamplers(nullptr),
				&this->depth.info,
				nullptr,
				nullptr
			}
		}
	);

	auto vert_size = this->vertex_descriptor.get_size();
	auto frag_size = this->fragment_descriptor.get_size();

	std::vector<vk::DescriptorSetLayoutBinding> texture_layout_binding = {
		vk::DescriptorSetLayoutBinding()
			.setBinding(0)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setPImmutableSamplers(nullptr),
		vk::DescriptorSetLayoutBinding()
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setPImmutableSamplers(nullptr)
	};

	this->texture_layout = device.createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(texture_layout_binding.size())
			.setPBindings(texture_layout_binding.data()));

	uint32_t textures_counter = 0;
	for(auto &material : this->scene_buffer.materials){
		if(!material.diffuse_texname.empty()){
			++textures_counter;
		}
	}

	auto texture_size = pool_size_t();
	texture_size.add(texture_layout_binding[0].descriptorType, textures_counter);
	texture_size.add(texture_layout_binding[1].descriptorType,
		this->scene_buffer.materials.size());

	this->desc_pool = create_descriptor_pool(device, vert_size + frag_size + texture_size);

	std::vector<vk::WriteDescriptorSet> writes{};
	for(auto &material : this->scene_buffer.materials){
		auto tmp_desc_sets = device.allocateDescriptorSets(
			vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(this->desc_pool)
				.setDescriptorSetCount(1)
				.setPSetLayouts(&this->texture_layout));

		material.material_desc = *tmp_desc_sets.begin();
		if(!material.diffuse_texname.empty()){
			writes.emplace_back(
				vk::WriteDescriptorSet()
					.setDstSet(material.material_desc)
					.setDstBinding(texture_layout_binding[0].binding)
					.setDescriptorCount(texture_layout_binding[0].descriptorCount)
					.setDescriptorType(texture_layout_binding[0].descriptorType)
					.setPImageInfo(&this->scene_buffer.textures
						[material.diffuse_texname].info));
		}
		writes.emplace_back(
			vk::WriteDescriptorSet()
				.setDstSet(material.material_desc)
				.setDstBinding(texture_layout_binding[1].binding)
				.setDescriptorCount(texture_layout_binding[1].descriptorCount)
				.setDescriptorType(texture_layout_binding[1].descriptorType)
				.setPBufferInfo(&material.transparency_buff.info));
	}

	device.updateDescriptorSets(writes, std::vector<vk::CopyDescriptorSet>{});

	this->vertex_descriptor.allocate_set(device, desc_pool);
	this->vertex_descriptor.update(device);

	this->fragment_descriptor.allocate_set(device, desc_pool);
	this->fragment_descriptor.update(device);

	this->init_const_range();
	this->init_pipeline_layouts(device);

	this->shader_stages[static_cast<int>(SHADER_TYPE::VERT)] = load_shader_f(device,
		"./shaders/vert_shader.spv", vk::ShaderStageFlagBits::eVertex);

	this->shader_stages[static_cast<int>(SHADER_TYPE::COLOR_FRAG)] = load_shader_f(device,
		"./shaders/frag_color_shader.spv", vk::ShaderStageFlagBits::eFragment);

	this->shader_stages[static_cast<int>(SHADER_TYPE::TEXTURE_FRAG)] = load_shader_f(device,
		"./shaders/frag_texture_shader.spv", vk::ShaderStageFlagBits::eFragment);

	this->shader_stages[static_cast<int>(SHADER_TYPE::TRANC_FRAG)] = load_shader_f(device,
		"./shaders/frag_shader_tranc.spv", vk::ShaderStageFlagBits::eFragment);

	this->pipeline_cache = create_pipeline_cache_f(device);

	this->describing_vertex_data();
	this->init_pipeline(device);
}

void pipeline_t::init_render_pass(const vk::Device &device, const vk::Format &format){
	enum class ATTACHMENT{
		COLOR = 0,
		DEPTH = 1,
		INSIDE_COLOR = 2
	};

	const std::array<vk::AttachmentDescription, 3> attachments {
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
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal),

		vk::AttachmentDescription()
			.setFormat(this->inside_color.format)
			.setSamples(pipeline_t::num_samples)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal),
	};

	vk::AttachmentReference depth_reference = vk::AttachmentReference()
		.setAttachment(static_cast<int>(ATTACHMENT::DEPTH))
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	const std::array<vk::AttachmentReference, 2> inside_and_color_reference{
		vk::AttachmentReference()
			.setAttachment(static_cast<int>(ATTACHMENT::COLOR))
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal),
		vk::AttachmentReference()
			.setAttachment(static_cast<int>(ATTACHMENT::INSIDE_COLOR))
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
	};

	const std::array<vk::AttachmentReference, 1> color_reference{
		vk::AttachmentReference()
			.setAttachment(static_cast<int>(ATTACHMENT::COLOR))
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal)
	};

	const std::array<vk::AttachmentReference, 1> transparency_input{
	//	vk::AttachmentReference()
	//		.setAttachment(static_cast<int>(ATTACHMENT::COLOR))
	//		.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal),
		vk::AttachmentReference()
			.setAttachment(static_cast<int>(ATTACHMENT::DEPTH))
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
	};

	using SUBPASS = PIPELINE_TYPE;
	const std::array<vk::SubpassDescription, static_cast<int>(PIPELINE_TYPE::SIZE)>
		subpass {
			vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setInputAttachmentCount(0)
				.setPInputAttachments(nullptr)
				.setColorAttachmentCount(inside_and_color_reference.size())
				.setPColorAttachments(inside_and_color_reference.data())
				.setPResolveAttachments(nullptr)
				.setPDepthStencilAttachment(&depth_reference)
				.setPreserveAttachmentCount(0)
				.setPPreserveAttachments(nullptr),

			vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setInputAttachmentCount(0)
				.setPInputAttachments(nullptr)
				.setColorAttachmentCount(color_reference.size())
				.setPColorAttachments(color_reference.data())
				.setPResolveAttachments(nullptr)
				.setPDepthStencilAttachment(&depth_reference)
				.setPreserveAttachmentCount(0)
				.setPPreserveAttachments(nullptr),

			vk::SubpassDescription()
				.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
				.setInputAttachmentCount(transparency_input.size())
				.setPInputAttachments(transparency_input.data())
				.setColorAttachmentCount(color_reference.size())
				.setPColorAttachments(color_reference.data())
				.setPResolveAttachments(nullptr)
				.setPDepthStencilAttachment(&depth_reference)
				.setPreserveAttachmentCount(0)
				.setPPreserveAttachments(nullptr),
	};

	const std::array<vk::SubpassDependency, 2> dependencies{
		vk::SubpassDependency()
			.setSrcSubpass(static_cast<uint32_t>(SUBPASS::COLOR))
			.setDstSubpass(static_cast<uint32_t>(SUBPASS::TEXTURE))
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion),

		vk::SubpassDependency()
			.setSrcSubpass(static_cast<uint32_t>(SUBPASS::TEXTURE))
			.setDstSubpass(static_cast<uint32_t>(SUBPASS::TRANSPARENCY))
			.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
			.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion)
	};

	const vk::RenderPassCreateInfo render_pass_info
		= vk::RenderPassCreateInfo()
		.setAttachmentCount(attachments.size())
		.setPAttachments(attachments.data())
		.setSubpassCount(subpass.size())
		.setPSubpasses(subpass.data())
		.setDependencyCount(dependencies.size())
		.setPDependencies(dependencies.data());
	this->render_pass = device.createRenderPass(render_pass_info);
}

std::vector<vk::Framebuffer> pipeline_t::create_framebuffers(const vk::Device &device,
	const vk::PhysicalDevice &physical_device,
	const std::vector<swapchain_buffers_type> &buffers, vk::Extent2D window_size,
	const vk::Format &format){

	this->init_inside_color_buffer(device, format, physical_device, window_size);
	this->init_depth_buffer(device, physical_device, window_size);
	this->init_render_pass(device, format);

	std::array<vk::ImageView, 3> attachments{vk::ImageView{},
		this->depth.info.imageView,
		this->inside_color.info.imageView};

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

namespace{

std::vector<vk::DescriptorPoolSize> get_pool_size_f(
	const std::vector<layout_f> &layouts);

}

void pipeline_t::init_const_range(){
	this->const_range = {};
}

void pipeline_t::init_pipeline_layouts(const vk::Device &device){
	std::vector<vk::DescriptorSetLayout> desc_set_layout{
		vertex_descriptor.layout, fragment_descriptor.layout, texture_layout
	};
/*	desc_set_layout.insert(desc_set_layout.begin(),
		texture_layouts.begin(), texture_layouts.end());*/

	const vk::PipelineLayoutCreateInfo pipeline_layout_info =
		vk::PipelineLayoutCreateInfo()
		.setSetLayoutCount(desc_set_layout.size())
		.setPSetLayouts(desc_set_layout.data())
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
