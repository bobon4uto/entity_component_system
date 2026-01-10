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
typedef uint64_t Component_Big[1000];

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
    Component_Big big;
  } as;
} Component;

DA_TYPE(Component, Components);

typedef struct Entity {
  ID  id;
  Components components;
} Entity;

DA_TYPE(Entity, Entities);
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
DA_TYPE(Component*, Component_Ptrs);

typedef void (*System_Fn)(Component**, u8*);
typedef void (*System_Mass_Fn)(Component_Ptrs, u8*);

typedef struct System {
  ID id;
  System_Fn invoke;
  System_Mass_Fn mass_invoke;
  Component_Types to_use;
  void* anydata;
} System;

Component* get_component_of_type(Entity* entity,Component_Type to_use_type) {
  for (size_t i = 0;i<entity->components.count;++i) {
    Component component = entity->components.items[i];
    if (component.is == to_use_type) {
      return entity->components.items + i;
    }
  }
  return NULL;
}
void get_all_components_of_types(Entity* entity,Component_Types to_use_types, Component_Ptrs* to_use_array) {
  Component_Ptrs tmp_to_use = {0};
  for (size_t i = 0;i<to_use_types.count;++i) {
    Component* component_ptr = get_component_of_type(entity, to_use_types.items[i]);
    if (component_ptr==NULL) {
      v_free(&tmp_to_use);
      return;
    } else {
      v_append(&tmp_to_use, component_ptr);
    }
  }
  v_append_buf(to_use_array,tmp_to_use.items, tmp_to_use.count);
}
void invoke_system(Entities* entities, System* system, u8* args) {
  if (system->mass_invoke == NULL && system->invoke == NULL) {
    return;
  }

  Component_Ptrs to_use = {0};
  for (size_t i = 0; i<entities->count;++i) {
    get_all_components_of_types(entities->items + i,system->to_use, &to_use);
  }
  if (to_use.count ==0) {return;} //no entities match system
  // in the end we get array with sorted components (eg if to use is Health Pos 0th is health 1st is pos 2nd is health, etc...)
  // ow, we could need entity to entity interaction so there should be mass invoke or something
  if (system->mass_invoke!=NULL) {
    system->mass_invoke(to_use, args);
  }
  if (system->invoke!=NULL) {
    for (size_t i = 0; i<entities->count;++i) {
      system->invoke(to_use.items + i*system->to_use.count, args);
    }
  }
  v_free(&to_use);
}


void free_system(System* system){
  (void)(system);
}

DA_TYPE(System, Systems);


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
ID  add_system(Root* root, System system) {
  system.id = root->id;
  v_append(&root->sys,system);
  return root->id++;
}

void invoke(Root* root, System* system, u8* args) {
  invoke_system(&root->ent, system, args);
}
void invoke_all(Root* root, u8* args) {
  for (size_t i = 0; i<root->sys.count;++i){
    invoke_system(&root->ent, root->sys.items + i, args);
  }
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






void example_invoke (Component** components, u8* args) {
  (void)(args);
  //for example we will set health to 0 for anyone whos in range
  Component_Health* health = &components[0]->as.health;
  Component_Position_2D pos = components[1]->as.position;
  // note that ordering is defined with to_use in system
  if (pos.x > 2 && pos.x < 7 && pos.y >2 && pos.y <7) {
    *health = 0;
  }
}
void example_mass_invoke(Component_Ptrs components, u8* args) {
  (void)(args);
  (void)(components);
  //dont know what example to show havent come up with enything interesting yet
}
System create_example_system() {
  System system0 = {0};
  System* system = &system0;
  v_append(&system->to_use,HEALTH);
  v_append(&system->to_use,POSITION);
  for(size_t i =10; i< 1000000; ++i) {
    v_append(&system->to_use,(Component_Type)i);
  }
  system->invoke = example_invoke;
  return system0;
}
Entity create_example_entity(Component_Health health_val, uint32_t x, uint32_t y) {
  Entity entity0 = {0};
  Entity* entity = &entity0;

  Component health = (Component){.is=HEALTH,.as.health=health_val};
  add_component(entity, health);
  Component pos = (Component){.is=POSITION,.as.position=(Component_Position_2D){.x=x,.y=y}};
  add_component(entity, pos);
  return entity0;
}

void inspect_component_value(Component component) {
  switch (component.is) {
  case NONE: printf("NONE"); break;
  case HEALTH: printf("%u",component.as.health); break;
  case POSITION: printf("(%u,%u)",component.as.position.x, component.as.position.y ); break;
  case NAME: printf("NAME"); break;
  }
}
void inspect_component_type(Component_Type type) {
  switch (type) {
  case NONE: printf("NONE"); break;
  case HEALTH: printf("HEALTH"); break;
  case POSITION: printf("POSITION"); break;
  case NAME: printf("NAME"); break;
  }
}
void inspect_component(Component component, u8 indentation) {
  printf("%*s", indentation, "");
  inspect_component_type(component.is);
  printf(" = ");
  inspect_component_value(component);

  printf("\n");
}
void inspect_entity(Entity entity, u8 indentation) {
  printf("%*sid=%zu\n", indentation, "", entity.id);
  ++indentation;
  for (size_t i = 0; i<entity.components.count; ++i) {
    inspect_component(entity.components.items[i], indentation);
  }
}
void inspect_system(System system, u8 indentation) {
  printf("%*sid=%zu\n", indentation, "", system.id);
  ++indentation;
  if (system.mass_invoke != NULL) printf("%*smass_invoke=%p\n", indentation, "", system.mass_invoke);
  if (system.invoke != NULL) printf("%*sinvoke=%p\n", indentation, "", system.invoke);
  ++indentation;
  for (size_t i = 0; i<system.to_use.count; ++i) {
    printf("%*s", indentation, "");
    inspect_component_type(system.to_use.items[i]);
    printf("\n");
  }
}
void inspect_root(Root root, u8 indentation) {
  printf("%*sRoot\n", indentation, "");
  printf("%*sSystems\n", indentation, "");
  ++indentation;
  for (size_t i = 0; i<root.sys.count; ++i) {
    inspect_system(root.sys.items[i], indentation);
  }
  --indentation;
  printf("%*sEntities\n", indentation, "");
  ++indentation;
  for (size_t i = 0; i<root.ent.count; ++i) {
    inspect_entity(root.ent.items[i], indentation);
  }
}

int main (int argc, char** argv) {
  (void)argc;
  (void)argv;

  PTR(Root, root);

  System system = create_example_system();
  add_system(root, system);
  Entity entity = create_example_entity(100, 3,4);
  Entity entity2 = create_example_entity(50,5, 7);
  add_entity(root, entity);
  add_entity(root, entity2);
  add_entity(root,create_example_entity(75, 1,4));
  for (size_t i = 0; i< 100000;++i) {
  Entity newent = create_example_entity(i, i%100,i);
  add_entity(root, newent);

  }
  add_entity(root,create_example_entity(99, 4,4));
  //add_entity(root, entity); be careful, must clone.
  printf("\n");
  printf("\n");
  inspect_entity(root0.ent.items[root0.ent.count-1], 5);
  //inspect_root(root0, 1);
  printf("\n===========\n");
  fflush(stdout);
  invoke_all(root, NULL);
  inspect_entity(root0.ent.items[root0.ent.count-1], 5);
  //inspect_root(root0, 1);


  free_root(root);
  return 0;
}
