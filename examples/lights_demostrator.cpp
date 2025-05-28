#include "lava/window/lava_window_system.hpp"
#include "lava/common/lava_global_helpers.hpp"
#include "lava/engine/lava_engine.hpp"
#include "lava/engine/lava_pbr_material.hpp"
#include "lava/engine/lava_mesh.hpp"
#include "lava/ecs/lava_ecs.hpp"
#include "lava/ecs/lava_pbr_render_system.hpp"
#include "lava/ecs/lava_update_system.hpp"
#include "lava/ecs/lava_normal_render_system.hpp"
#include "lava/ecs/lava_diffuse_render_system.hpp"
#include "lava/ecs/lava_deferred_render_system.hpp"
#include "lava/common/lava_shapes.hpp"
#include "lava/engine/lava_image.hpp"
#include "imgui.h"


void ecs_render_imgui(LavaECSManager& ecs_manager, size_t camera_entity) {
	auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
	auto& camera_camera = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();
	static bool camera_type = (bool)camera_camera.type_;

	ImGui::Begin("ECS Camera Manager Window");

	if (ImGui::DragFloat3("Camera position", &camera_tr.pos_.x, 0.1f, -100.0f, 100.0f)) {

	}
	if (camera_camera.type_ == CameraType_Perspective) {
		ImGui::DragFloat("Fov", &camera_camera.fov_, 0.1f, 0.0f, 180.0f);
	}
	else {
		ImGui::DragFloat("Size", &camera_camera.size_, 0.01f, 0.0f, 10.0f);

	}
	ImGui::DragFloat("Camera Rot X", &camera_tr.rot_.x, 0.5f, -360.0f, 360.0f);
	ImGui::DragFloat("Camera Rot Y", &camera_tr.rot_.y, 0.5f, -360.0f, 360.0f);

	ImGui::Checkbox("CameraType", &camera_type); {
		camera_camera.type_ = (CameraType)camera_type;
	}

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
	LavaEngine engine(1920,1080);
	LavaECSManager ecs_manager;
	LavaDeferredRenderSystem deferred_render_system{ engine };
	LavaPBRRenderSystem pbr_render_system{ engine };
	LavaUpdateSystem update_system{ engine };

	std::vector<size_t> light_entities;
	//Temple
	std::shared_ptr<LavaPBRMaterial> temple_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
	MeshProperties temple_mesh_properties = {};
	temple_mesh_properties.mesh_path = "../examples/assets/temple/griechischer_tempel.glb";
	temple_mesh_properties.material = temple_material;
	std::shared_ptr<LavaMesh> temple_mesh_ = std::make_shared<LavaMesh>(engine, temple_mesh_properties);
	
	//Horse
	std::shared_ptr<LavaPBRMaterial> horse_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
	MeshProperties horse_mesh_properties = {};
	horse_mesh_properties.mesh_path = "../examples/assets/temple/rossbandiger.glb";
	horse_mesh_properties.material = horse_material;
	std::shared_ptr<LavaMesh> horse_mesh_ = std::make_shared<LavaMesh>(engine, horse_mesh_properties);

	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(0.0f, 0.0f, -2.0f);
			transform.scale_ = glm::vec3(2.0f, 2.0f, 2.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = temple_mesh_;
		}
	}


	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(-0.724f, 0.60f, -1.397f);
			transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
			transform.rot_ = glm::vec3(00.0f, 90.0f, 0.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = horse_mesh_;
		}
	}

	
	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(1.624f, 0.60f, -1.397f);
			transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
			transform.rot_ = glm::vec3(00.0f, -90.0f, 0.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = horse_mesh_;
		}
	}


	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(1.624f, 0.60f, -3.600f);
			transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
			transform.rot_ = glm::vec3(00.0f, -90.0f, 0.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = horse_mesh_;
		}
	}

	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(-0.724f, 0.60f, -3.600f);
			transform.rot_ = glm::vec3(00.0f, 90.0f, 0.0f);
			transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = horse_mesh_;
		}
	}

	//{
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);

	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(3.524f, 0.60f, -0.300f);
	//		transform.rot_ = glm::vec3(00.0f, -90.0f, 0.0f);
	//		transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = horse_mesh_;
	//	}
	//}

	//{
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);

	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(3.524f, 0.60f, -2.300f);
	//		transform.rot_ = glm::vec3(00.0f, -90.0f, 0.0f);
	//		transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = horse_mesh_;
	//	}
	//}


	//{
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);

	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(3.524f, 0.60f, -4.300f);
	//		transform.rot_ = glm::vec3(00.0f, -90.0f, 0.0f);
	//		transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = horse_mesh_;
	//	}
	//}


	////LEFT THREE SIDE
	//{
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);

	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(-2.324f, 0.60f, -0.300f);
	//		transform.rot_ = glm::vec3(00.0f, 90.0f, 0.0f);
	//		transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = horse_mesh_;
	//	}
	//}

	//{
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);

	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(-2.324f, 0.60f, -2.300f);
	//		transform.rot_ = glm::vec3(00.0f, 90.0f, 0.0f);
	//		transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = horse_mesh_;
	//	}
	//}


	//{
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);

	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(-2.324f, 0.60f, -4.300f);
	//		transform.rot_ = glm::vec3(00.0f, 90.0f, 0.0f);
	//		transform.scale_ = glm::vec3(0.08f, 0.08f, 0.08f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = horse_mesh_;
	//	}
	//}



	//


	//LIGHT OK
	{
		size_t light_entity = ecs_manager.createEntity();
		light_entities.push_back(light_entity);
		ecs_manager.addComponent<TransformComponent>(light_entity);
		ecs_manager.addComponent<LightComponent>(light_entity);
	
		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_POINT;
			light.base_color_ = glm::vec3(1.0f, 0.0f, 0.0f);
			light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
			light.cutoff_ = 27.0f;
			light.outer_cutoff_ = 36.410f;
			light.constant_att_ = 1.0f;
			light.quad_att_ = 0.112f;
			light.strength_ = 0.28f;
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.pos_ = glm::vec3(-1.339f, 1.961f, -2.0f);
		}
	}


	{
		size_t light_entity = ecs_manager.createEntity();
		light_entities.push_back(light_entity);
		ecs_manager.addComponent<TransformComponent>(light_entity);
		ecs_manager.addComponent<LightComponent>(light_entity);

		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_POINT;
			light.base_color_ = glm::vec3(0.0f, 1.0f, 0.0f);
			light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
			light.cutoff_ = 27.0f;
			light.outer_cutoff_ = 35.0f;
			light.constant_att_ = 1.0f;
			light.quad_att_ = 0.112f;
			light.strength_ = 0.28f;
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.pos_ = glm::vec3(0.701f, 1.960, -1.100);
		}
	}

	{
		size_t light_entity = ecs_manager.createEntity();
		light_entities.push_back(light_entity);
		ecs_manager.addComponent<TransformComponent>(light_entity);
		ecs_manager.addComponent<LightComponent>(light_entity);

		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_POINT;
			light.base_color_ = glm::vec3(0.0f, 0.0f, 1.0f);
			light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
			light.cutoff_ = 27.0f;
			light.outer_cutoff_ = 36.410f;
			light.constant_att_ = 1.0f;
			light.quad_att_ = 0.112f;
			light.strength_ = 0.28f;
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.pos_ = glm::vec3(0.701f, 1.960, -3.360);
		}
	}


	{
		size_t light_entity = ecs_manager.createEntity();
		light_entities.push_back(light_entity);
		ecs_manager.addComponent<TransformComponent>(light_entity);
		ecs_manager.addComponent<LightComponent>(light_entity);

		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_POINT;
			light.base_color_ = glm::vec3(1.0f, 1.0f, 0.0f);
			light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
			light.cutoff_ = 27.0f;
			light.outer_cutoff_ = 36.410f;
			light.constant_att_ = 1.00f;
			light.quad_att_ = 0.112f;
			light.strength_ = 0.28f;
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.pos_ = glm::vec3(2.781f, 1.961f, -2.0f);
		}
	}

	size_t disco_light_entity = ecs_manager.createEntity();
	{
		ecs_manager.addComponent<TransformComponent>(disco_light_entity);
		ecs_manager.addComponent<LightComponent>(disco_light_entity);

		auto light_component = ecs_manager.getComponent<LightComponent>(disco_light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_SPOT;
			light.base_color_ = glm::vec3(0.2f, 1.0f, 0.2f);
			light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
			light.cutoff_ = 20.0f;
			light.outer_cutoff_ = 30.0f;
			light.constant_att_ = 1.0f;
			light.quad_att_ = 0.112f;
			light.strength_ = 0.28f;
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(disco_light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.rot_ = glm::vec3(-120.0f, 0.0f, 0.0f);
			tr.pos_ = glm::vec3(0.0f, 2.5f, -2.5f);
		}
	}

	
	
	//Create Camera entity
	size_t camera_entity = ecs_manager.createEntity();
	{

		ecs_manager.addComponent<TransformComponent>(camera_entity);
		ecs_manager.addComponent<CameraComponent>(camera_entity);
		ecs_manager.addComponent<UpdateComponent>(camera_entity);

		auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
		camera_tr.rot_ = glm::vec3(-19.0f, 1.5f, 0.0f);
		camera_tr.pos_ = glm::vec3(0.9f, 6.752f, 9.406f);
		auto& camera_component = ecs_manager.getComponent<CameraComponent>(camera_entity)->value();

		auto update_component = ecs_manager.getComponent<UpdateComponent>(camera_entity);
		if (update_component) {
			auto& update = update_component->value();
		
			update.id = camera_entity;
			update.ecs_manager = &ecs_manager;
			update.update_ = [](size_t id, LavaECSManager* ecs_manager, LavaEngine& engine) {
					UpdateCameraWithInput(id, ecs_manager, engine);
				};
		}

		engine.setMainCamera(&camera_component, &camera_tr);
	}
	
	engine.global_scene_data_.ambientColor = glm::vec3(0.2f, 0.2f, 0.2f);

	int count = 0;
	while (!engine.shouldClose()) {

		auto tr_component = ecs_manager.getComponent<TransformComponent>(disco_light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.rot_.y += 0.2f;
		}


		update_system.update(ecs_manager.getComponentList<UpdateComponent>());

		engine.beginFrame();
		engine.clearWindow();

		//deferred_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
		//	ecs_manager.getComponentList<RenderComponent>(), ecs_manager.getComponentList<LightComponent>());
		pbr_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>(), ecs_manager.getComponentList<LightComponent>());
		ecs_render_imgui(ecs_manager, camera_entity);
		ecs_light_imgui(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<LightComponent>());

		engine.endFrame();
		count++;
	}

	return 0;
	
}