#include "lava_types.hpp"
#include "lava_engine.hpp"
#include "lava_window_system.hpp"
#include "engine/lava_pipeline.hpp"
#include "engine/lava_image.hpp"
#include "engine/lava_pbr_material.hpp"
#include "ecs/lava_ecs.hpp"
#include "ecs/lava_pbr_render_system.hpp"
#include "imgui.h"


void ecs_render_imgui(LavaECSManager& ecs_manager, size_t camera_entity) {
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	ImGui::Begin("ECS Camera Manager Window");

	if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {

	}

	ImGui::DragFloat("Fov", &camera_camera.fov_, 0.1f, 0.0f, 180.0f);
	ImGui::DragFloat("Camera Rot X", &camera_tr.rot_.x, 0.5f, -360.0f, 360.0f);
	ImGui::DragFloat("Camera Rot Y", &camera_tr.rot_.y, 0.5f, -360.0f, 360.0f);


	ImGui::End();

	
}

void ecs_light_imgui(std::vector<std::optional<TransformComponent>>& transform_vector,
	std::vector<std::optional<LightComponent>>& light_component_vector) {

	auto light_transform_it = transform_vector.begin();
	auto light_transform_end = transform_vector.end();
	auto light_it = light_component_vector.begin();
	auto light_end = light_component_vector.end();

	const char* lightTypes[] = { "Directional", "Point", "Spot" };
	int lights_count = 0;
	ImGui::Begin("ECS Light config");
	
	for (; light_transform_it != light_transform_end || light_it != light_end; light_transform_it++, light_it++)
	{
		if (!light_transform_it->has_value()) continue;
		if (!light_it->has_value()) continue;
	
		ImGui::PushID(lights_count);
		if(ImGui::TreeNode("Light" , "Light %d",lights_count)) {
			int type = (int)light_it->value().type_;

			ImGui::Checkbox("Enable", &light_it->value().enabled_);
			if (ImGui::Combo("Light Type", &type, lightTypes, IM_ARRAYSIZE(lightTypes))) {
				light_it->value().type_ = (LightType)type;
			}
			//if (light_it->value().type_ != LightType::LIGHT_TYPE_DIRECTIONAL) {
				ImGui::DragFloat3("Light Pos", &light_transform_it->value().pos_.x, 0.01f, -10.0f, 10.0f);
			//}
			if (light_it->value().type_ != LightType::LIGHT_TYPE_POINT) {
				ImGui::DragFloat3("Light Rot", &light_transform_it->value().rot_.x, 0.05f, -360.0f, 360.0f);
			}
			ImGui::ColorEdit3("Base color", &light_it->value().base_color_.x);
			ImGui::ColorEdit3("Specular color", &light_it->value().spec_color_.x);
			ImGui::DragFloat("Shininess", &light_it->value().shininess_, 0.5f, 1.0f, 256.0f);
			ImGui::DragFloat("Strength", &light_it->value().strength_, 0.01f, 0.0f, 1.0f);
			if (light_it->value().type_ != LightType::LIGHT_TYPE_DIRECTIONAL) {
				ImGui::DragFloat("Linear Att", &light_it->value().linear_att_, 0.01f, 0.0f, 0.7f);
				ImGui::DragFloat("Quadratic Att", &light_it->value().quad_att_, 0.001f, 0.0f, 1.8f);
				ImGui::DragFloat("Constant Att", &light_it->value().constant_att_, 0.01f, 0.0f, 1.0f);
				if (light_it->value().type_ == LightType::LIGHT_TYPE_SPOT) {
					if (ImGui::DragFloat("CutOff", &light_it->value().cutoff_, 1.00f, 0.0f, 90.0f)) {
						if (light_it->value().cutoff_ > light_it->value().outer_cutoff_) {
							light_it->value().outer_cutoff_ =  
								glm::clamp(light_it->value().outer_cutoff_, light_it->value().cutoff_, 90.0f);
						}
					}
					ImGui::DragFloat("Outer CutOff", &light_it->value().outer_cutoff_, 0.01f, light_it->value().cutoff_+0.01f, 90.0f);
				}
			}
			ImGui::TreePop();
		}
		ImGui::PopID();
		lights_count++;
	}

	ImGui::End();


}



int main(int argc, char* argv[]) {
	std::shared_ptr<LavaWindowSystem>  lava_system = LavaWindowSystem::Get();
	LavaEngine engine;
	LavaECSManager ecs_manager;
	LavaPBRRenderSystem pbr_render_system{ engine };

	///////////////////////
	//////ASSETS START/////
	///////////////////////
	MaterialPBRProperties mat_properties = {};
	mat_properties.name = "PBR Material";
	mat_properties.pipeline_flags = PipelineFlags::PIPELINE_USE_PUSHCONSTANTS | PipelineFlags::PIPELINE_USE_DESCRIPTOR_SET;

	LavaPBRMaterial basic_material(engine, mat_properties);

	MeshProperties mesh_properties = {};
	mesh_properties.name = "Shiba Mesh";
	mesh_properties.type = MESH_GLTF;
	mesh_properties.mesh_path = "../examples/assets/Avocado.glb";
	mesh_properties.material = &basic_material;

	std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);

	//Needs to be call every time an image or property is updated
	basic_material.UpdateDescriptorSet();
	/////////////////////
	//////ASSETS END/////
	/////////////////////

	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(0.0f, 0.0f, -1.0f);
			transform.scale_ = glm::vec3(10.0f, 10.0f, 10.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = mesh_;
		}
	}

	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(0.0f, 0.0f, -2.0f);
			transform.scale_ = glm::vec3(10.0f, 10.0f, 10.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = mesh_;
		}
	}

	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);
		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(-0.5f, -0.5f, -1.0f);
			transform.scale_ = glm::vec3(10.0f, 10.0f, 10.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = mesh_;
		}
	}

	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(0.5f, -0.5f, -1.0f);
			transform.scale_ = glm::vec3(10.0f, 10.0f, 10.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = mesh_;
		}
	}
	




	{
		size_t light_entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(light_entity);
		ecs_manager.addComponent<LightComponent>(light_entity);

		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_SPOT;
			light.base_color_ = glm::vec3(1.0f, 1.0f, 1.0f);
			light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.rot_ = glm::vec3(0.0f, 3.14f, 0.0f);
			tr.pos_ = glm::vec3(0.0f, 0.0f, 1.0f);
		}

	}

	{
		size_t light_entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(light_entity);
		ecs_manager.addComponent<LightComponent>(light_entity);

		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_SPOT;
			light.base_color_ = glm::vec3(1.0f, 1.0f, 1.0f);
			light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.rot_ = glm::vec3(0.0f, 3.14f, 0.0f);
			tr.pos_ = glm::vec3(0.0f, 0.0f, 1.0f);
		}

	}
		
	//Create Camera entity
	size_t camera_entity = ecs_manager.createEntity();
	ecs_manager.addComponent<TransformComponent>(camera_entity);
	ecs_manager.addComponent<CameraComponent>(camera_entity);

	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	camera_tr.rot_ = glm::vec3(0.0f, 0.0f, 0.0f);
	camera_tr.pos_ = glm::vec3(0.5f, 0.0f, 0.0f);
	auto& camera_component = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

	engine.global_scene_data_.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);

	LavaInput* input = engine.window_.get_input();

	while (!engine.shouldClose()) {
		

		//auto &tr = ecs_manager.getComponent<TransformComponent>(entity)->value();
		//tr.rot_ = glm::vec3(0.1f * engine.frame_data_.frame_number_,
		//	0.02f * engine.frame_data_.frame_number_,
		//	0.05f * engine.frame_data_.frame_number_);
		auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();

		//if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {

		//}
		//ImGui::DragFloat("Fov", &camera_camera.fov_, 0.1f, 0.0f, 180.0f);
		//ImGui::DragFloat("Camera Rot X", &camera_tr.rot_.x, 0.5f, 88.0f, 268.0f);
		//ImGui::DragFloat("Camera Rot Y", &camera_tr.rot_.y, 0.5f, -360.0f, 360.0f);


		if (input->isInputDown(KEY_D)) {
			camera_tr.pos_.x += (1.0f * engine.dt_);
		}
		if (input->isInputDown(KEY_A)) {
			camera_tr.pos_.x -= (1.0f * engine.dt_);
		}
		if (input->isInputDown(KEY_W)) {
			camera_tr.pos_.y += (1.0f * engine.dt_);
		}
		if (input->isInputDown(KEY_S)) {
			camera_tr.pos_.y -= (1.0f * engine.dt_);
		}

		engine.allocate_lights(ecs_manager.getComponentList<LightComponent>());
		engine.update_lights(ecs_manager.getComponentList<LightComponent>(),ecs_manager.getComponentList<TransformComponent>());
		engine.updateMainCamera(&camera_component, &camera_tr);

		engine.beginFrame();
		engine.clearWindow();

		engine.renderImgui();
		ecs_render_imgui(ecs_manager, camera_entity);
		ecs_light_imgui(ecs_manager.getComponentList<TransformComponent>(), ecs_manager.getComponentList<LightComponent>());
		pbr_render_system.renderWithShadows(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>(), ecs_manager.getComponentList<LightComponent>());
		//normal_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
		//	ecs_manager.getComponentList<RenderComponent>());

		engine.endFrame();
	}

	return 0;
}