#include "lava_pipelines.hpp"
#include "lava_vulkan_inits.hpp"

void PipelineBuilder::Clear()
{
    // clear all of the structs we need back to 0 with their correct stype

    _input_assembly = { .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };

    _rasterizer = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };

    _color_blend_attachment = {};

    _multisampling = { .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

    _pipeline_layout = {};

    _depth_stencil = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };

    _render_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };

    _shader_stages.clear();
}

VkPipeline PipelineBuilder::BuildPipeline(VkDevice device)
{
    // make viewport state from our stored viewport and scissor.
    // at the moment we wont support multiple viewports or scissors
    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.pNext = nullptr;

    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    // setup dummy color blending. We arent using transparent objects yet
    // the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.pNext = nullptr;

    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &_color_blend_attachment;

    // completely clear VertexInputStateCreateInfo, as we have no need for it
    VkPipelineVertexInputStateCreateInfo _vertex_input_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

    // build the actual pipeline
    // we now use all of the info structs we have been writing into into this one
    // to create the pipeline
    VkGraphicsPipelineCreateInfo pipeline_info = { .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    // connect the renderInfo to the pNext extension mechanism
    pipeline_info.pNext = &_render_info;

    pipeline_info.stageCount = (uint32_t)_shader_stages.size();
    pipeline_info.pStages = _shader_stages.data();
    pipeline_info.pVertexInputState = &_vertex_input_info;
    pipeline_info.pInputAssemblyState = &_input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &_rasterizer;
    pipeline_info.pMultisampleState = &_multisampling;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDepthStencilState = &_depth_stencil;
    pipeline_info.layout = _pipeline_layout;

    VkDynamicState state[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

    VkPipelineDynamicStateCreateInfo dynamic_info = { .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamic_info.pDynamicStates = &state[0];
    dynamic_info.dynamicStateCount = 2;

    pipeline_info.pDynamicState = &dynamic_info;

    // its easy to error out on create graphics pipeline, so we handle it a bit
    // better than the common VK_CHECK case
    VkPipeline new_pipeline;
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info,
        nullptr, &new_pipeline)
        != VK_SUCCESS) {
        printf("\nfailed to create pipeline\n");
        return VK_NULL_HANDLE; // failed to create graphics pipeline
    }
    else {
        return new_pipeline;
    }
}

void PipelineBuilder::SetShaders(VkShaderModule vertex_shader, VkShaderModule fragment_shader)
{
    _shader_stages.clear();

    VkPipelineShaderStageCreateInfo vertex_info{};
    vertex_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_info.pNext = nullptr;
    vertex_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_info.module = vertex_shader;
    vertex_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_info{};
    fragment_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_info.pNext = nullptr;
    fragment_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_info.module = fragment_shader;
    fragment_info.pName = "main";

    _shader_stages.push_back(
        vertex_info);

    _shader_stages.push_back(
        fragment_info);
}

void PipelineBuilder::SetInputTopology(VkPrimitiveTopology topology)
{
    _input_assembly.topology = topology;

    _input_assembly.primitiveRestartEnable = VK_FALSE;
}

void PipelineBuilder::SetPolygonMode(VkPolygonMode mode)
{
    _rasterizer.polygonMode = mode;
    _rasterizer.lineWidth = 1.f;
}

void PipelineBuilder::SetCullMode(VkCullModeFlags cull_mode, VkFrontFace front_face)
{
    _rasterizer.cullMode = cull_mode;
    _rasterizer.frontFace = front_face;
}

void PipelineBuilder::SetMultisamplingNone()
{
    _multisampling.sampleShadingEnable = VK_FALSE;
    // multisampling defaulted to no multisampling (1 sample per pixel)
    _multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    _multisampling.minSampleShading = 1.0f;
    _multisampling.pSampleMask = nullptr;
    // no alpha to coverage either
    _multisampling.alphaToCoverageEnable = VK_FALSE;
    _multisampling.alphaToOneEnable = VK_FALSE;
}

void PipelineBuilder::DisableBlending()
{
    // default write mask
    _color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    // no blending
    _color_blend_attachment.blendEnable = VK_FALSE;
}

void PipelineBuilder::SetColorAttachmentFormat(VkFormat format)
{
    _color_attachmentformat = format;
    // connect the format to the renderInfo  structure
    _render_info.colorAttachmentCount = 1;
    _render_info.pColorAttachmentFormats = &_color_attachmentformat;
}

void PipelineBuilder::SetDepthFormat(VkFormat format)
{
    _render_info.depthAttachmentFormat = format;
}

void PipelineBuilder::DisableDepthtest()
{
    _depth_stencil.depthTestEnable = VK_FALSE;
    _depth_stencil.depthWriteEnable = VK_FALSE;
    _depth_stencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    _depth_stencil.depthBoundsTestEnable = VK_FALSE;
    _depth_stencil.stencilTestEnable = VK_FALSE;
    _depth_stencil.front = {};
    _depth_stencil.back = {};
    _depth_stencil.minDepthBounds = 0.f;
    _depth_stencil.maxDepthBounds = 1.f;
}
