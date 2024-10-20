//void LavaEngine::drawBackground(VkCommandBuffer command_buffer) {
//
//	//Se sustituye todo esto por el compute shader
////	//Limpiamos la imagen con un clear color
////	VkClearColorValue clearValue;
////	clearValue = { {1.0f,0.0f,0.0f,0.0f} };
////
////	//Seleccionamos un rango de la imagen sobre la que actuar
////	VkImageSubresourceRange clearRange = ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
////
////	//Aplicamos el clear color a una imagen 
////vkCmdClearColorImage(command_buffer, draw_image_.image,
////	VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);
//
//
//	//// bind the gradient drawing compute pipeline
//	//vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_);
//	//
//	////IMPORTANT
//	////Importante recordar esto cuando tengamos que hacer binding de description sets en otros shaders
//	//// bind the descriptor set containing the draw image for the compute pipeline
//	//vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, gradient_pipeline_layout_, 0, 1, &draw_image_descriptor_set_, 0, nullptr);
//	//
//	//// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
//	//vkCmdDispatch(command_buffer, std::ceil(draw_image_.image_extent.width / 16.0), std::ceil(draw_image_.image_extent.height / 16.0), 1);
//
//
//	ComputeEffect& effect = backgroundEffects[currentBackgroundEffect];
//
//	// bind the background compute pipeline
//	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, effect.pipeline);
//
//	// bind the descriptor set containing the draw image for the compute pipeline
//	vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, effect.layout, 0, 1, &draw_image_descriptor_set_, 0, nullptr);
//
//	if (effect.use_push_constants) {
//		vkCmdPushConstants(command_buffer, effect.layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(ComputePushConstants), &effect.data);
//	}
//	// execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
//	vkCmdDispatch(command_buffer, std::ceil(swap_chain_.get_draw_image().image_extent.width / 16.0), std::ceil(swap_chain_.get_draw_image().image_extent.height / 16.0), 1);
//
//}

//void LavaEngine::DrawGeometryWithProperties(VkCommandBuffer command_buffer)
//{
//	//begin a render pass  connected to our draw image
//	VkRenderingAttachmentInfo colorAttachment = vkinit::AttachmentInfo(swap_chain_.get_draw_image().image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
//	//
//	VkRenderingInfo renderInfo = vkinit::RenderingInfo(swap_chain_.get_draw_extent(), &colorAttachment, nullptr);
//	vkCmdBeginRendering(command_buffer, &renderInfo);
//
//	//vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);
//
//	//set dynamic viewport and scissor
//	VkViewport viewport = {};
//	viewport.x = 0;
//	viewport.y = 0;
//	viewport.width = swap_chain_.get_draw_extent().width;
//	viewport.height = swap_chain_.get_draw_extent().height;
//	viewport.minDepth = 0.f;
//	viewport.maxDepth = 1.f;
//
//	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
//
//	VkRect2D scissor = {};
//	scissor.offset.x = 0;
//	scissor.offset.y = 0;
//	scissor.extent.width = swap_chain_.get_draw_extent().width;
//	scissor.extent.height = swap_chain_.get_draw_extent().height;
//
//	vkCmdSetScissor(command_buffer, 0, 1, &scissor);
//
//	vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _meshPipeline);
//
//	GPUDrawPushConstants push_constants;
//	push_constants.world_matrix = glm::mat4{ 1.f };
//	//push_constants.vertex_buffer = rectangle.vertex_buffer_address;
//
//	vkCmdPushConstants(command_buffer, _meshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(GPUDrawPushConstants), &push_constants);
//	VkDeviceSize offsets[] = { 0 };
//	vkCmdBindVertexBuffers(command_buffer, 0, 1, &rectangle.vertex_buffer.buffer, offsets);
//	vkCmdBindIndexBuffer(command_buffer, rectangle.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
//
//	vkCmdDrawIndexed(command_buffer, 3, 1, 0, 0, 0);
//
//	//launch a draw command to draw 3 vertices
//	vkCmdDraw(command_buffer, 3, 1, 0, 0);
//
//	vkCmdEndRendering(command_buffer);
//
//	//launch a draw command to draw 3 vertices
//}

//void LavaEngine::createMeshPipeline()
//{
//	VkShaderModule triangleFragShader;
//	if (!LoadShader("../src/shaders/colored_triangle.frag.spv", device_.get_device(), &triangleFragShader)) {
//		printf("Error when building the triangle fragment shader module");
//	}
//	else {
//		printf("Triangle fragment shader succesfully loaded");
//	}
//
//	VkShaderModule triangleVertexShader;
//	if (!LoadShader("../src/shaders/colored_triangle_mesh.vert.spv", device_.get_device(), &triangleVertexShader)) {
//		printf("Error when building the triangle vertex shader module");
//	}
//	else {
//		printf("Triangle vertex shader succesfully loaded");
//	}
//
//	VkPushConstantRange buffer_range{};
//	buffer_range.offset = 0;
//	buffer_range.size = sizeof(GPUDrawPushConstants);
//	buffer_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//
//	VkPipelineLayoutCreateInfo pipeline_layout_info{};
//	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//	pipeline_layout_info.pNext = nullptr;
//	pipeline_layout_info.pSetLayouts = &draw_image_descriptor_set_layout_;
//	pipeline_layout_info.setLayoutCount = 1;
//	pipeline_layout_info.pPushConstantRanges = &buffer_range;
//	pipeline_layout_info.pushConstantRangeCount = 1;
//
//	if (vkCreatePipelineLayout(device_.get_device(), &pipeline_layout_info, nullptr, &_meshPipelineLayout)) {
//#ifndef NDEBUG
//		printf("Compute pipeline creation failed!");
//#endif // !NDEBUG
//	}
//
//	PipelineBuilder pipeline_builder;
//
//	//use the triangle layout we created
//	pipeline_builder._pipeline_layout = _meshPipelineLayout;
//	//connecting the vertex and pixel shaders to the pipeline
//	pipeline_builder.SetShaders(triangleVertexShader, triangleFragShader);
//	//it will draw triangles
//	pipeline_builder.SetInputTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
//	//filled triangles
//	pipeline_builder.SetPolygonMode(VK_POLYGON_MODE_FILL);
//	//no backface culling
//	pipeline_builder.SetCullMode(VK_CULL_MODE_NONE, VK_FRONT_FACE_CLOCKWISE);
//	//no multisampling
//	pipeline_builder.SetMultisamplingNone();
//	//no blending
//	pipeline_builder.DisableBlending();
//	//no depth testing
//	//pipeline_builder.DisableDepthtest();
//	pipeline_builder.EnableDepthTest(true, VK_COMPARE_OP_GREATER_OR_EQUAL);
//
//	//connect the image format we will draw into, from draw image
//	pipeline_builder.SetColorAttachmentFormat(swap_chain_.get_draw_image().image_format);
//	pipeline_builder.SetDepthFormat(swap_chain_.get_depth_image().image_format);
//
//	//finally build the pipeline
//	_meshPipeline = pipeline_builder.BuildPipeline(device_.get_device());
//
//	//clean structures
//	vkDestroyShaderModule(device_.get_device(), triangleFragShader, nullptr);
//	vkDestroyShaderModule(device_.get_device(), triangleVertexShader, nullptr);
//
//	main_deletion_queue_.push_function([&]() {
//		vkDestroyPipelineLayout(device_.get_device(), _meshPipelineLayout, nullptr);
//		vkDestroyPipeline(device_.get_device(), _meshPipeline, nullptr);
//		});
//}