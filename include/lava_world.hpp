#ifndef __LAVA_WORLD_H__
#define __LAVA_WORLD_H__ 1

#include <memory>

class LavaWorld
{
public:
	static LavaWorld& GetWorld() {
		if (!instance) {
			instance = std::make_unique<LavaWorld>();
		}
		return *instance;
	}
	
	void setECSManager(class LavaECSManager* ecs_manager) {
		this->ecs_manager_ = ecs_manager;
	}

	class LavaECSManager* getECSManager() const {
		return ecs_manager_;
	}

	~LavaWorld();
private:
	LavaWorld();
	class LavaECSManager* ecs_manager_;

	friend std::unique_ptr<LavaWorld> std::make_unique<LavaWorld>();
	static std::unique_ptr<LavaWorld> instance;
	
};



#endif // !__LAVA_WORLD_H__
