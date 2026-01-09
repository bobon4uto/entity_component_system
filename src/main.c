#define VUPS_IMPLEMENTATION

#include "vups.h"


typedef size_t ID;
typedef struct {
  size_t capacity;
  size_t count;
  u8 *items;
} Vec;



typedef enum Component_Type {
  NONE,
  HEALTH,
  POSITION,
  NAME,
} Component_Type;
typedef void* Component_None;
typedef uint32_t Component_Health;

typedef struct Vec2u8 {
  uint8_t x;
  uint8_t y;
} Vec2u8;

typedef struct Vec2u16 {
  uint16_t x;
  uint16_t y;
} Vec2u16;

typedef struct Vec2u32 {
  uint32_t x;
  uint32_t y;
} Vec2u32;

typedef struct Vec2u64 {
  uint64_t x;
  uint64_t y;
} Vec2u64;

typedef Vec2u32 Vec2;
typedef Vec2 Component_Position_2D;
typedef String_Holder Component_Name;


typedef struct Component {
  Component_Type is;
  union {
    Component_None none;
    Component_Health health;
    Component_Position_2D position;
    Component_Name name;
  } as;
} Component;

DA_TYPE(Component, Components);

typedef struct Entity {
  ID  id;
  Components components;
} Entity;

size_t add_component(Entity* entity, Component component) {
  v_append(&entity->components, component);
  return entity->components.count;
}

size_t find_component(Entity* entity, Component_Type component_type) {
  for (size_t i = 0; i < entity->components.capacity; ++i) {
    if (entity->components.items[i].is ==component_type) {
      return i;
    }
  }
  return entity->components.capacity; //this means not found (capacity will always be out of range, since it's end+1)
}

void free_component(Component* component) {
  // only define for components that need it.
  switch (component->is) {
    case  NAME:
      v_free(&component->as.name);
      break;
    default:
      break;
  }
}
size_t free_component_by_index(Entity* entity, size_t index) {
  free_component(entity->components.items + index);
  return index;
}
size_t free_component_by_type(Entity* entity, Component_Type component_type) {
  size_t i = find_component(entity, component_type);
  free_component_by_index(entity, i);
  return i;
}

size_t remove_component(Entity* entity, Component_Type component_type) {
  size_t i = free_component_by_type(entity, component_type);
  entity->components.items[i].is = NONE;
  return i;
}

void free_entity(Entity* entity) {
  for (size_t i = 0; i < entity->components.count; ++i) {
    free_component_by_index(entity, i);
  }
  v_free(&entity->components);
}

DA_TYPE(Component_Type, Component_Types);

typedef struct Root Root;
typedef void (*System_Fn)(Root*, Component_Types, Component_Types);

typedef struct System {
  ID id;
  System_Fn invoke;
  Component_Types to_read;
  Component_Types to_write;
} System;

void invoke_system(Root* root, System* system) {
  system->invoke(root, system->to_read, system->to_write);
}

void free_system(System* system){
  (void)(system);
}

DA_TYPE(System, Systems);
DA_TYPE(Entity, Entities);


typedef struct Root {
  ID id; // this is a smallest free id 
         // (e.g. 10, when there is already id = 9)
  Systems  sys;
  Entities ent;
  Arena _a;
} Root;

ID  add_entity(Root* root, Entity entity) {
  entity.id = root->id;
  v_append(&root->ent,entity);
  return root->id++;
}

void free_root(Root* root) {
  for (size_t i = 0; i < root->sys.count; ++i) {
    free_system(root->sys.items + i);
  }
  v_free(&root->sys);

  for (size_t i = 0; i < root->ent.count; ++i) {
    free_entity(root->ent.items + i);
  }
  v_free(&root->ent);

}

int main (int argc, char** argv) {
  (void)argc;
  (void)argv;

  PTR(Root, root);

  PROBE(root);
  Entity entity ={0};
  //here one has to add components

  //
  add_entity(root, entity);
  


  PROBE(root->sys.count);
  PROBE(root->ent.count);
  PROBE(root->id);
  free_root(root);
  return 0;
}
