/**
 * @file lava_swap_chain.hpp
 * @author Carlos Garcia Roig (garciaroi@esat-alumni.com)
 * @author Daniel Rivero Diaz (riverodi@esat-alumni.com)
 * @brief Lava Swap Chain's header file
 * @version 0.1
 * @date 2024-10-12
 *
 * @copyright Academic Project ESAT 2024/2025
 *
 */

#ifndef __LAVA_SWAP_CHAIN_H__
#define __LAVA_SWAP_CHAIN_H__ 1

#include "lava_types.hpp"
#include "lava_vulkan_helpers.hpp"

class LavaSwapChain {
public:

	LavaSwapChain(class LavaDevice& use_device, class LavaSurface& use_surface, VkExtent2D window_extent, VmaAllocator allocator);
	~LavaSwapChain();

	VkSwapchainKHR get_swap_chain() const { return swap_chain_; }

	AllocatedImage get_draw_image() const { return draw_image_; }
	AllocatedImage get_depth_image() const { return depth_image_; }
	std::vector<VkImage> get_swap_chain_images() const { return swap_chain_images_; }

	std::vector<VkImageView> get_swap_chain_image_views() const { return swap_chain_image_views_; }

	VkExtent2D get_draw_extent() const { return draw_extent_; }

	VkFormat get_swap_chain_image_format() const { return swap_chain_image_format_; }

	//VkSampler get_shadowmap_sampler() const { return shadowmap_sampler_; }
	//AllocatedImage get_shadowmap_image() const { return shadowmap_image_; }
private:
	LavaSwapChain() = delete;

	void createSwapChain(class LavaSurface& use_surface, VkExtent2D window_extent);

	void createImageViews();

	//Resources for drawing outside of the swap chain
	AllocatedImage draw_image_;
	AllocatedImage depth_image_;
	VkExtent2D draw_extent_;
	//AllocatedImage shadowmap_image_;
	//VkSampler shadowmap_sampler_;


	//Main Swap Chain Resources
	VkSwapchainKHR swap_chain_;
	std::vector<VkImage> swap_chain_images_;
	std::vector<VkImageView> swap_chain_image_views_;
	VkFormat swap_chain_image_format_;


	//Require Device for creation and destruction
	class LavaDevice* device_;

	//Require Allocator for creation and destruction
	VmaAllocator allocator_;
};

#endif // !__LAVA_SWAP_CHAIN_H__