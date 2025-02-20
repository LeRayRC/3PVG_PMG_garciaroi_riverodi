#ifndef __LAVA_ECS_H__ 
#define __LAVA_ECS_H__ 1

#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>
//#include "ecs/lava_ecs_components.hpp"

//Entity
//8 bytes -> 64 bits -> 

#define ECS_SUCCESS 1
#define COMPONENT_NOT_EXIST 2
#define ENTITY_NOT_EXIST 3


struct ComponentListBase {
  virtual ~ComponentListBase() {}
  virtual size_t getSize() const = 0;
  virtual void resize(size_t new_size) = 0;
  virtual void remove(size_t entity_id) = 0;
};

template<typename T>
struct ComponentListDerived : ComponentListBase {
  std::vector<std::optional<T>> component_list_;
  size_t getSize() const override{
    return component_list_.size();
  }
  void resize(size_t new_size) override{
    component_list_.resize(new_size);
  }
  void remove(size_t entity_id) override {
    component_list_[entity_id] = std::nullopt;
  }

};


class LavaECSManager
{
public:
  LavaECSManager();
	~LavaECSManager();


  size_t createEntity();
  void deleteEntity(size_t entity_id);

  template<typename T>
  void addComponentType() {

    size_t key = typeid(T).hash_code();
    std::unique_ptr<ComponentListDerived<T>> list = std::make_unique<ComponentListDerived<T>>();
    list->component_list_.resize(last_entity);
    component_list_map_.emplace(key, std::move(list));
  }

  template<typename T>
  std::optional<T>* getComponent(size_t entity) {
    size_t hash = typeid(T).hash_code();

    map_type::iterator it = component_list_map_.find(hash);
    if (it == component_list_map_.end()) return nullptr; //Component does not exist

    ComponentListDerived<T>* cld = static_cast<ComponentListDerived<T>*>(it->second.get());

    if(entity >= cld->getSize()) return nullptr;

    return &cld->component_list_[entity];
  }

  template<typename T>
  int addComponent(size_t entity) {
    size_t hash = typeid(T).hash_code();

    map_type::iterator it = component_list_map_.find(hash);
    if (it == component_list_map_.end()) return COMPONENT_NOT_EXIST; //Component does not exist

    ComponentListDerived<T>* cld = static_cast<ComponentListDerived<T>*>(it->second.get());

    if (entity >= cld->getSize()) return ENTITY_NOT_EXIST;
    cld->component_list_[entity] = T();

    return ECS_SUCCESS;
  }

  template<typename T>
  int removeComponent(size_t entity) {
    size_t hash = typeid(T).hash_code();

    map_type::iterator it = component_list_map_.find(hash);
    if (it == component_list_map_.end()) return COMPONENT_NOT_EXIST; //Component does not exist

    ComponentListDerived<T>* cld = static_cast<ComponentListDerived<T>*>(it->second.get());

    if (entity >= cld->getSize()) return ENTITY_NOT_EXIST;
    cld->component_list_[entity] = std::nullopt;

    return ECS_SUCCESS;
  }

  template<typename T>
  std::vector<std::optional<T>>& getComponentList() {
    size_t hash = typeid(T).hash_code();

    map_type::iterator it = component_list_map_.find(hash);
    //TO FIX
    //if (it == component_list_map_.end())// return nullptr; //Component does not exist

    ComponentListDerived<T>* cld = static_cast<ComponentListDerived<T>*>(it->second.get());
    return cld->component_list_;
  }

private:

  typedef std::unordered_map<size_t, std::unique_ptr<ComponentListBase>> map_type;
  map_type component_list_map_;

  size_t last_entity;
  std::vector<size_t> free_slots;
};


#endif // !__LAVA_ECS_H__ 



