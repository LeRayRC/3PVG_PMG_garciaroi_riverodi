#ifndef __LAVA_PIPELINE_BUILDER_H_
#define __LAVA_PIPELINE_BUILDER_H_ 1

#include "lava_types.hpp"

class PipelineBuilder {
public:
    std::vector<VkPipelineShaderStageCreateInfo> _shader_stages;

    VkPipelineInputAssemblyStateCreateInfo _input_assembly;
    VkPipelineRasterizationStateCreateInfo _rasterizer;
    VkPipelineColorBlendAttachmentState _color_blend_attachment;
    VkPipelineMultisampleStateCreateInfo _multisampling;
    VkPipelineLayout _pipeline_layout;
    VkPipelineDepthStencilStateCreateInfo _depth_stencil;
    VkPipelineRenderingCreateInfo _render_info;
    VkFormat _color_attachmentformat;

    PipelineBuilder() { Clear(); }

    void Clear();

    VkPipeline BuildPipeline(VkDevice device);

    void SetShaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader);

    void SetInputTopology(VkPrimitiveTopology topology);

    void SetPolygonMode(VkPolygonMode mode);

    void SetCullMode(VkCullModeFlags cull_mode, VkFrontFace front_face);

    void SetMultisamplingNone();

    void DisableBlending();

    void EnableBlending(PipelineBlendMode blend_mode);

    void SetColorAttachmentFormat(VkFormat format);
    void DisableColorAttachment(VkFormat format);

    void SetDepthFormat(VkFormat format);

    void EnableDepthTest(bool depthWriteEnable, VkCompareOp op);

    void DisableDepthtest();
};

#endif