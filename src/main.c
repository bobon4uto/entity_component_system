#define VUPS_IMPLEMENTATION
#include "vups.h"

typedef u32 uid;





#define _VUPS_TT_ENUM_DEF(TYPE, FORMAT,_X)                                                 \
  TYPE_##TYPE,
typedef enum {
  UNKNOWN,
  VUPS_TYPES(_VUPS_TT_ENUM_DEF, 0)
} Vups_Type ;
#define _VUPS_TT_ENUM(TYPE, FORMAT)                                                 \
  TYPE:                                                                        \
  TYPE_##TYPE,
#define _TYPE_ENUM(X, T)                                                         \
  _Generic((X), VUPS_TYPES(T, X) VUPS_NEW_TYPES(T) default: UNKNOWN)
#define TYPE_ENUM(X) _TYPE_ENUM(X, _VUPS_TT_ENUM)

#define _VUPS_TT_JUST_TYPE(TYPE,_F, X)                                                 \
  TYPE: add_##TYPE##_comp(WORLD,(X)),
#define _TYPE_JUST_TYPE(X, T)                                                         \
  _Generic((X), VUPS_TYPES(T, X) VUPS_NEW_TYPES(T) default: assert("UNKNOWN"))
#define TYPE_JUST_TYPE(X) _TYPE_JUST_TYPE(X, _VUPS_TT_JUST_TYPE)

typedef struct Entity {
  uid id;
  int i;
  char_ptr ch;

} Entity;
typedef u8 Byte;
typedef Byte* Byte_Ptr;
DA_TYPE(Byte,Bytes);
DA_TYPE(Byte_Ptr,Byte_Ptrs);
DA_TYPE(Vups_Type,Vups_Types);

typedef struct Archetype  {
  Vups_Types types;
} Archetype;
DA_TYPE(Archetype,Archetypes);

//Archetype_u8_char_ptr 


typedef struct _Entity_Creation_Data  {
  Bytes data;
  Vups_Types types;
} _Entity_Creation_Data;

typedef struct World {
  uid id;
  Entity entity;
  _Entity_Creation_Data _entity_creation_data;
} World;

bool world_start_spawn(World* world) {
  // it doesnt sound all that important, lets kust check if anything
  // is being spawned and return true if not
  return world->_entity_creation_data.data.count == 0;
  // it would be smart to use some lock instead for multithread
}
//void  add_int_comp(World* world, int component) {
//  Bytes* data = &world->_entity_creation_data.data;
//  Vups_Types* types = &world->_entity_creation_data.types;
//
//  v_append_buf(data, &component, sizeof(component));
//  v_append(types, TYPE_ENUM(component));
//}
#define _MK_COMPONENT_TYPE_FUNCTION(TYPE, _F, _X) void  add_##TYPE##_comp(World* world, TYPE component) { \
  Bytes* data = &world->_entity_creation_data.data; \
  Vups_Types* types = &world->_entity_creation_data.types; \
  v_append_buf(data, (Byte*)&component, sizeof(component)); \
  v_append(types, TYPE_##TYPE); \
} \

VUPS_TYPES(_MK_COMPONENT_TYPE_FUNCTION, 0)

#define _MK_COMPONENT_TYPE_ARRAY_TYPE(TYPE,_F,_X) DA_TYPE(TYPE,DA_##TYPE);
VUPS_TYPES(_MK_COMPONENT_TYPE_ARRAY_TYPE, 0)
#define _MK_COMPONENT_TYPE_ARRAY(TYPE,_F,_X) DA_##TYPE TYPE##s = {0};
VUPS_TYPES(_MK_COMPONENT_TYPE_ARRAY, 0)

uid world_end_spawn(World *world) {
  // figure out where to put components (for now just push to columns with corresponding types.)
  
  Vups_Types types = world->_entity_creation_data.types;
  u j = 0;

  for(u i = 0; i<types.count; ++i) {
    switch (types.items[i]) {
#define _MK_COMPONENT_CASE_(TYPE,_F,_X) case TYPE_##TYPE : v_append((&TYPE##s), *((TYPE *) (world->_entity_creation_data.data.items+j)) ); \
      j += sizeof(TYPE); \
      break;
VUPS_TYPES(_MK_COMPONENT_CASE_, 0 )
  // for some reason it breaks if i pass world->...items as X to macro.
  // nevermind i forgor that it was format
      default: assert("IMPOSSIBLE!");
    }
  }

   world->_entity_creation_data.data.count = 0;
   world->_entity_creation_data.types.count = 0;
  return world->id++;
}



#define WORLD world
#define add_X_comp(X) TYPE_JUST_TYPE(X);






int main() {
  World world0 ={0};
  World* world =&world0;
#define MACROBEGIN world_start_spawn(world);
#define MACROFUNC(X) add_X_comp ( (X) );
#define MACROEND world_end_spawn(world)
#include "src/gen/main.gen.c"
  add_X_comp ( 1 );
  add_X_comp ( (Bytes){0} );
PROBE("a");
  //world_spawn("a","b","c");


  Vups_Types types = world->_entity_creation_data.types;
  Bytes data = world->_entity_creation_data.data;
  PROBE(types.count);
  for (u i = 0; i< data.count; ++i) {
    PROBE(data.items[i]);
  }
  for (u i = 0; i< types.count; ++i) {
    PROBE(types.items[i]);
  }
  uid entity_id = world_end_spawn(world);
  (void)entity_id;
#define _PRINT_COMPONENT_TYPE_ARRAY(TYPE,_F,_X) printf(#TYPE " = %zu\n",TYPE##s.count);
VUPS_TYPES(_PRINT_COMPONENT_TYPE_ARRAY, 0)
 // DA_char_ptr cp = char_ptrs;
  printf("%p",char_ptrs.items[0]);
 // printf("%s",char_ptrs.items[0]);
  //for (u i = 0; i< char_ptrs.count; ++i) {
  //  PROBE(char_ptrs.items[i]);
  //}

  return 0;
}














