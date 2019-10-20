#include "utils.hpp"

#include <cmath>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

vk::Image create_depth_image(const vk::Device &device, vk::Format format,
	vk::Extent2D window_size, vk::FormatProperties format_prop,
	vk::SampleCountFlagBits num_samples){

	vk::ImageTiling tiling;
	if(format_prop.linearTilingFeatures
		& vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		tiling = vk::ImageTiling::eLinear;
	else if(format_prop.optimalTilingFeatures
		& vk::FormatFeatureFlagBits::eDepthStencilAttachment)
		tiling = vk::ImageTiling::eOptimal;
	else
		throw std::runtime_error("init_depth_buffer: "
			+ vk::to_string(format) + " unsupported.");

	const vk::ImageCreateInfo image_info = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(format)
		.setExtent(vk::Extent3D(window_size, 1))
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(num_samples)
		.setTiling(tiling)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
	/*	.setSharingMode()
		.setQueueFamilyIndexCount()
		.setPQueueFamilyIndices();*/
		.setInitialLayout(vk::ImageLayout::eUndefined);

	return device.createImage(image_info);
}

uint32_t memory_type_from_properties(const vk::PhysicalDeviceMemoryProperties &mem_prop,
	const vk::MemoryRequirements &mem_req, vk::MemoryPropertyFlags req_mask){

	uint32_t typeBits = mem_req.memoryTypeBits;

	for(uint32_t i = 0; i < mem_prop.memoryTypeCount; i++){
		if ((typeBits & 1) == 1) {
			if ((mem_prop.memoryTypes[i].propertyFlags & req_mask) == req_mask) {
				return i;
			}
		}
		typeBits >>= 1;
	}
	throw std::runtime_error("ERROR: memory_type_from_properties");
}

vk::DeviceMemory allocate_image_memory(const vk::Device &device,
	const vk::PhysicalDevice &physical_device, const vk::Image &depth_image,
	vk::MemoryPropertyFlags requirements_mask){
	const vk::MemoryRequirements mem_req = device.getImageMemoryRequirements(depth_image);
	const vk::PhysicalDeviceMemoryProperties mem_prop
		= physical_device.getMemoryProperties();

	const uint32_t typeIndex
		= memory_type_from_properties(mem_prop, mem_req, requirements_mask);

	const vk::MemoryAllocateInfo allocate_info = vk::MemoryAllocateInfo()
		.setAllocationSize(mem_req.size)
		.setMemoryTypeIndex(typeIndex);
	return device.allocateMemory(allocate_info);
}

vk::ImageView create_2d_image_view(const vk::Device &device,
	const vk::Image &image, const vk::Format &format, vk::ImageAspectFlags aspect_mask){

	const vk::ImageViewCreateInfo image_view_info = vk::ImageViewCreateInfo()
		.setImage(image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format)
		.setSubresourceRange(vk::ImageSubresourceRange(
			aspect_mask, 0, 1, 0, 1));
	return device.createImageView(image_view_info);
}

std::size_t get_mvp_buffer_size(){
	return sizeof(glm::mat4) * 2;
}

void update_mvp_buffer(const camera &cam,
	const vk::Device &device, const buffer_t &buf){
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 2.5, -7), // Camera is at (-5,3,-10), in World Space
		glm::vec3(0, 0, 0),    // and looks at the origin
		glm::vec3(0, 1, 0)    // Head is up (set to 0,-1,0 to look upside-down)
	);
	static float angle = 0.0f;
	glm::mat4 Model = glm::rotate(glm::mat4(1.0f), glm::radians(angle),
		glm::vec3(0.0f, 1.0f, 0.0f));
	angle += 1.0f;
	// Vulkan clip space has inverted Y and half Z.
	glm::mat4 Clip = glm::mat4(
		1.0f,  0.0f, 0.0f, 0.0f,
		0.0f, -1.0f, 0.0f, 0.0f,
		0.0f,  0.0f, 0.5f, 0.0f,
		0.0f,  0.0f, 0.5f, 1.0f);

	glm::mat4 mv = glm::transpose(glm::inverse(Model * View));
	glm::mat4 mvp = Clip * Projection * View * Model;

//	vk::DeviceSize size = device.getMemoryCommitment(mem);

	void *data_ptr = device.mapMemory(buf.mem, 0, buf.info.range, vk::MemoryMapFlags());
	memcpy(data_ptr, &mvp, sizeof(glm::mat4));
	memcpy(data_ptr + sizeof(glm::mat4), &mv, sizeof(glm::mat4));
    device.unmapMemory(buf.mem);
}

struct normal{
	float x = 0, y = 0, z = 0;
};

void add_normal(vertex &v, normal n){
	v.norm_x += n.x;
	v.norm_y += n.y;
	v.norm_z += n.z;
}

void normalize(vertex &v){
	float length = sqrt(pow(v.norm_x, 2) + pow(v.norm_y, 2) + pow(v.norm_z, 2));
	v.norm_x /= length;
	v.norm_y /= length;
	v.norm_z /= length;
}

indeced_mash load_obj(std::string path){
	tinyobj::attrib_t attribs;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warning;
	std::string error;

	if(!tinyobj::LoadObj( &attribs, &shapes, &materials,
		&warning, &error, path.c_str())){

		std::string msg("Could not open: " + path);
		if( 0 < error.size() ) {
			msg += '\n' + error;
		}
		throw std::runtime_error(msg);
	}

	indeced_mash mash;
	for(uint32_t i = 0; i < attribs.vertices.size(); i += 3){
		mash.verteces.emplace_back(vertex{attribs.vertices[i], attribs.vertices[i+2],
			attribs.vertices[i+1], 1});
	}

	std::vector<normal> normals{};
	for(uint32_t i = 0; i < attribs.normals.size(); i += 3){
		normals.emplace_back(normal{attribs.normals[i], attribs.normals[i+2],
			attribs.normals[i+1]});
	}

	for(auto &shape : shapes){
		for(auto &index : shape.mesh.indices){
			mash.indeces.emplace_back(index.vertex_index);

			add_normal(mash.verteces[index.vertex_index], normals[index.normal_index]);
		}
	}

	for(auto &vertex : mash.verteces){
		normalize(vertex);
	}

	return mash;
}
