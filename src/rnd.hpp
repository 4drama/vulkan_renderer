#ifndef RND_HPP
#define RND_HPP

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan\vulkan.hpp>
#include "windows.h"

#include "utils.hpp"
#include "pipeline.hpp"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class renderer{
public:
	renderer();
	~renderer() = default;

	void set_scene(scene_t *scene_ptr);
	void main_loop();
private:
	HINSTANCE hInstance;
	HWND hWnd;
	WNDCLASSEX wcex;

	std::vector<const char *> instance_extension_names;

	vk::Instance instance;
	vk::SurfaceKHR surface;

	vk::PhysicalDeviceFeatures features;
	std::vector<const char *> device_extension_names;

	vk::PhysicalDevice physical_device;
	uint32_t graphics_queue_family_index;
	uint32_t present_queue_family_index;

	vk::Device device;

	vk::Format format;		// нужен для в randerpass
	vk::SwapchainKHR swapchain;
//	uint32_t swapchainImageCount;
	std::vector<swapchain_buffers_type> buffers;

	vk::Queue graphics_queue;
	vk::Queue present_queue;

	vk::CommandPool graphics_cmd_pool;
//	vk::CommandPool present_cmd_pool;

	std::vector<vk::Framebuffer> framebuffers;
	uint32_t current_frame;

	pipeline_t pipeline;

	vk::CommandBuffer cmd_buffer;

//	scene_t *scene_ptr = nullptr;

	void init_window();
	void init_instance();
	void init_surface();
	void init_physical_device();
	void init_device();
	void init_swapchain();
	void init_queues();
	void init_command_pools();
	void init_command_buffers();

//	void init_depth_buffer();

	void init_framebuffers();
//	void init_graphic_pipeline();

	void draw();

	[[nodiscard]] vk::Extent2D get_window_size() const noexcept;
};

#endif
