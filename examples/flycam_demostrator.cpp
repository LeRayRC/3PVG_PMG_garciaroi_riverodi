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


class HyperSpaceEffectLine {
public:
	HyperSpaceEffectLine(LavaECSManager& ecs_manager, LavaEngine& engine,
		glm::vec3 init_pos, float speed) : 
		ecs_manager_{ ecs_manager },
		engine_{engine},
		speed_{speed},
		init_pos_{init_pos}
	{
		std::shared_ptr<LavaPBRMaterial> basic_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
		MeshProperties mesh_properties = {};

		mesh_properties.mesh_path = "../examples/assets/hyperspace_star_wars.glb";
		mesh_properties.material = basic_material;
		mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);
		basic_material->UpdateBaseColorImage(engine.default_texture_image_white);
		


		entity_id = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity_id);
		ecs_manager.addComponent<RenderComponent>(entity_id);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity_id);
		if (transform_component) {

			auto& transform = transform_component->value();
			transform.pos_ = init_pos;
			transform.scale_ = glm::vec3(0.01f, 0.001f, 0.001f);
			transform.rot_ = glm::vec3(0.01f, -90.0f, 0.01f);
		}

		//glm::vec3(cosf(step * i), sin(step * i), 8.0f + rand() % 5);

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity_id);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = mesh_;
		}
	}
	~HyperSpaceEffectLine(){}
public:
	void reset() {
		auto& transform_component = ecs_manager_.getComponent<TransformComponent>(entity_id)->value();
		transform_component.pos_ = init_pos_;

		speed_ = (float)(1 + rand() % 15);
	}
	void update() {
		auto& transform_component = ecs_manager_.getComponent<TransformComponent>(entity_id)->value();
		
		transform_component.pos_.z -= speed_ * static_cast<float>(engine_.dt_);

		if (transform_component.pos_.z < -10.0f) {
			reset();
		}
	}
	LavaEngine& engine_;
	std::shared_ptr<LavaPBRMaterial> basic_material;
	size_t entity_id;
	float speed_;
	glm::vec3 init_pos_;
	std::shared_ptr<LavaMesh> mesh_;
	LavaECSManager& ecs_manager_;
};

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
	LavaPBRRenderSystem pbr_render_system{ engine };
	LavaNormalRenderSystem normal_render_system{ engine };
	LavaDiffuseRenderSystem diffuse_render_system{ engine };
	LavaDeferredRenderSystem deferred_render_system{ engine };
	LavaUpdateSystem update_system{ engine };

	std::shared_ptr<LavaPBRMaterial> basic_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
	MeshProperties mesh_properties = {};

	mesh_properties.mesh_path = "../examples/assets/x-wing_cockpit_no_window.glb";
	mesh_properties.material = basic_material;

	std::shared_ptr<LavaMesh> mesh_ = std::make_shared<LavaMesh>(engine, mesh_properties);



	std::shared_ptr<LavaImage> sun_texture = std::make_shared<LavaImage>(&engine, "../examples/assets/textures/sun.jpg");
	std::shared_ptr<LavaImage> forest_texture = std::make_shared<LavaImage>(&engine, "../examples/assets/textures/forest.png");

	
	int num_lines = 500;
	std::vector<HyperSpaceEffectLine> hyperspace_lines;

	const float MIN_ANGLE_RAD = 0.0f * (3.14159f / 180.0f);  // 30 grados en radianes
	const float MAX_ANGLE_RAD = 180.0f * (3.14159f / 180.0f); // 150 grados en radianes
	const float ANGLE_RANGE = MAX_ANGLE_RAD - MIN_ANGLE_RAD;


	for (int i = 0; i < num_lines; i++) {
		float angle = MIN_ANGLE_RAD + (((float)rand() / RAND_MAX) * ANGLE_RANGE);
		
		hyperspace_lines.push_back(HyperSpaceEffectLine(ecs_manager, engine,
			glm::vec3(cosf(angle), sin(angle) - 0.5f, 8.0f + rand() % 5),(float)(1+rand()%15))
		);
	}


	std::shared_ptr<LavaPBRMaterial> cube_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
	std::shared_ptr<LavaMesh> cube_mesh = CreateCube24v(engine, cube_material);


	std::shared_ptr<LavaPBRMaterial> sphere_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
	//sphere_material.UpdateBaseColorImage(sun_texture);
	//std::shared_ptr<LavaMesh> sphere_mesh = CreateSphere(engine, &sphere_material);
	std::shared_ptr<LavaMesh> sphere_mesh = CreateCube8v(sphere_material);


	std::shared_ptr<LavaPBRMaterial> terrain_material = std::make_shared<LavaPBRMaterial>(engine, MaterialPBRProperties());
	terrain_material->UpdateBaseColorImage(forest_texture);

	std::shared_ptr<LavaMesh> terrain_mesh = CreateTerrain(terrain_material, 
		32,32,8.0f,1.0f, 0.15f, {20,20});



	{
		size_t entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(entity);
		ecs_manager.addComponent<RenderComponent>(entity);

		auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
		if (transform_component) {
			auto& transform = transform_component->value();
			transform.pos_ = glm::vec3(0.0f, 0.0f, -2.0f);
			transform.scale_ = glm::vec3(1.0f, 1.0f, 1.0f);
		}

		auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
		if (render_component) {
			auto& render = render_component->value();
			render.mesh_ = mesh_;
		}
	}

	//{
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);
	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(-0.5f, 5.0f, -1.0f);
	//		transform.scale_ = glm::vec3(20.0f, 20.0f, 20.0f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = sphere_mesh;
	//	}
	//}
	// 
	//{
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);

	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(0.5f, -0.5f, -1.0f);
	//		transform.scale_ = glm::vec3(10.0f, 10.0f, 10.0f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = mesh_;
	//	}
	//}

	//for (int i = 0; i < 20; i++) {
	//	size_t entity = ecs_manager.createEntity();
	//	ecs_manager.addComponent<TransformComponent>(entity);
	//	ecs_manager.addComponent<RenderComponent>(entity);

	//	auto transform_component = ecs_manager.getComponent<TransformComponent>(entity);
	//	if (transform_component) {
	//		auto& transform = transform_component->value();
	//		transform.pos_ = glm::vec3(1.0f + (0.5f * (rand() % 10)), -1.5f + (0.5f * (rand() % 10)),-2.0f + (0.5f * (rand()%10)));
	//		transform.scale_ = glm::vec3(10.0f, 10.0f, 10.0f);
	//	}

	//	auto render_component = ecs_manager.getComponent<RenderComponent>(entity);
	//	if (render_component) {
	//		auto& render = render_component->value();
	//		render.mesh_ = mesh_;
	//	}
	//}

	{
		size_t light_entity = ecs_manager.createEntity();
		ecs_manager.addComponent<TransformComponent>(light_entity);
		ecs_manager.addComponent<LightComponent>(light_entity);
	
		auto light_component = ecs_manager.getComponent<LightComponent>(light_entity);
		if (light_component) {
			auto& light = light_component->value();
			light.enabled_ = true;
			light.type_ = LIGHT_TYPE_DIRECTIONAL;
			light.base_color_ = glm::vec3(1.0f, 1.0f, 1.0f);
			light.spec_color_ = glm::vec3(0.0f, 0.0f, 0.0f);
			light.cutoff_ = 40.0f;
			light.outer_cutoff_ = 62.72f;

		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.rot_ = glm::vec3(-115.0f, -2.0f, 0.0f);
			tr.pos_ = glm::vec3(0.0f, 0.0f, 1.6f);
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
			light.cutoff_ = 34.0f;
			light.outer_cutoff_ = 56.410f;
			light.constant_att_ = 1.0f;
			light.quad_att_ = 0.112f;
			light.strength_ = 0.28f;
		}
		auto tr_component = ecs_manager.getComponent<TransformComponent>(light_entity);
		if (tr_component) {
			auto& tr = tr_component->value();
			tr.rot_ = glm::vec3(-110.0f, 0.0f, -0.5f);
			tr.pos_ = glm::vec3(0.03f, 0.06f, -1.68f);
		}
	
	}
	
	//Create Camera entity
	size_t camera_entity = ecs_manager.createEntity();
	{

		ecs_manager.addComponent<TransformComponent>(camera_entity);
		ecs_manager.addComponent<CameraComponent>(camera_entity);
		ecs_manager.addComponent<UpdateComponent>(camera_entity);

		auto& camera_tr = ecs_manager.getComponent<TransformComponent>(camera_entity)->value();
		camera_tr.rot_ = glm::vec3(-25.0f, 180.0f, 0.0f);
		camera_tr.pos_ = glm::vec3(0.0f, 0.0f, -2.0f);
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

		if (count > 2) {

			for (auto& line : hyperspace_lines) {
				line.update();
			}
		}


		update_system.update(ecs_manager.getComponentList<UpdateComponent>());

		engine.beginFrame();
		engine.clearWindow();


		pbr_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<RenderComponent>(), ecs_manager.getComponentList<LightComponent>());
		//diffuse_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
		//	ecs_manager.getComponentList<RenderComponent>());

		//deferred_render_system.render(ecs_manager.getComponentList<TransformComponent>(),
		//	ecs_manager.getComponentList<RenderComponent>(), ecs_manager.getComponentList<LightComponent>());

		ecs_render_imgui(ecs_manager, camera_entity);
		ecs_light_imgui(ecs_manager.getComponentList<TransformComponent>(),
			ecs_manager.getComponentList<LightComponent>());

		engine.endFrame();
		count++;
	}

	return 0;
	
}