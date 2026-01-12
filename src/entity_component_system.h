#ifndef _ENTITY_COMPONENT_SYSTEM_H
#define _ENTITY_COMPONENT_SYSTEM_H

/*#*/ typedef u32 uid;
typedef struct Position {
  u archetype_position;
  u number_in_archetype;
} Position;
DA_TYPE(Position, Positions);

#define _VUPS_TT_ENUM_DEF(TYPE, FORMAT, _X) TYPE_##TYPE,
typedef enum { UNKNOWN, TYPEDEF_TYPES(_VUPS_TT_ENUM_DEF, 0) } Vups_Type;
#define _VUPS_TT_ENUM(TYPE, FORMAT)                                            \
  TYPE:                                                                        \
  TYPE_##TYPE,
#define _TYPE_ENUM(X, T)                                                       \
  _Generic((X), TYPEDEF_TYPES(T, X) VUPS_NEW_TYPES(T) default: UNKNOWN)
#define TYPE_ENUM(X) _TYPE_ENUM(X, _VUPS_TT_ENUM)

#define _VUPS_TT_JUST_TYPE(TYPE, _F, X)                                        \
  TYPE:                                                                        \
  add_##TYPE##_comp(WORLD, *(TYPE *)&(_temp_var)),
#define _TYPE_JUST_TYPE(X, T)                                                  \
  do {                                                                         \
    typeof(X) _temp_var = X;                                                   \
    _Generic((X),                                                              \
        TYPEDEF_TYPES(T, X) VUPS_NEW_TYPES(T) default: assert("UNKNOWN"));     \
  } while (0)

#define TYPE_JUST_TYPE(X) _TYPE_JUST_TYPE(X, _VUPS_TT_JUST_TYPE)

typedef struct Entity {
  uid id;
  int i;
  char_ptr ch;

} Entity;
/*#*/ typedef u8 Byte;
typedef Byte *Byte_Ptr;
DA_TYPE(Byte, Bytes);
DA_TYPE(Byte_Ptr, Byte_Ptrs);
DA_TYPE(Vups_Type, Vups_Types);

#define _MK_COMPONENT_TYPE_ARRAY_TYPE(TYPE, _F, _X) DA_TYPE(TYPE, DA_##TYPE);
TYPEDEF_TYPES(_MK_COMPONENT_TYPE_ARRAY_TYPE, 0)

/*# we dont want Archetype to contain itself*/ typedef struct Archetype {
  Vups_Types types;
  u count;
#define _MK_COMPONENT_TYPE_ARRAY_AS_STRUCT_PART(TYPE, _F, _X)                  \
  DA_##TYPE TYPE##_da;
  TYPEDEF_TYPES(_MK_COMPONENT_TYPE_ARRAY_AS_STRUCT_PART, 0)
} Archetype;
DA_TYPE(Archetype, Archetypes);

/*#*/ typedef struct _Entity_Creation_Data {
  Bytes data;
  Vups_Types types;
} _Entity_Creation_Data;

/*#*/ typedef struct World {
  uid id;
  Archetypes archetypes;
  Positions position_table;

  _Entity_Creation_Data _entity_creation_data;
} World;

uid world_start_spawn(World *world);

/*#*/ typedef void (*System_Fn_Type)(Archetype *);

#define _MK_COMPONENT_TYPE_FUNCTION(TYPE, _F, _X)                              \
  void add_##TYPE##_comp(World *world, TYPE component) {                       \
    Bytes *data = &world->_entity_creation_data.data;                          \
    Vups_Types *types = &world->_entity_creation_data.types;                   \
    v_append_buf(data, (Byte *)&component, sizeof(component));                 \
    v_append(types, TYPE_##TYPE);                                              \
  }

TYPEDEF_TYPES(_MK_COMPONENT_TYPE_FUNCTION, 0)

bool archetype_compare_to_archetype(Archetype *archetype1,
                                    Archetype *archetype2);

void merge_archetypes(Archetype *to, Archetype *from);

u world_merge_archetypes_if_same(World *world);

uid world_end_spawn(World *world);
#define WORLD world
#define add_X_comp(X) TYPE_JUST_TYPE(X);
void world_call_on_all(World *world, System_Fn_Type fn);
String_Holder sh_from_c_str(const char *c_str);
String_Holder sh_(const char *c_str);
void archetype_free(Archetype *a);
void world_free(World *world);

#define get_component(entity, TYPE)                                            \
  (WORLD->archetypes                                                           \
       .items[world_get_position(WORLD, entity).archetype_position]            \
       .TYPE##_da.items +                                                      \
   world_get_position(WORLD, entity).number_in_archetype)

#ifdef ENTITY_COMPONENT_SYSTEM_IMPLEMENTATION

// #define _MK_COMPONENT_TYPE_ARRAY(TYPE,_F,_X) DA_##TYPE TYPE##_da = {0};
// TYPEDEF_TYPES(_MK_COMPONENT_TYPE_ARRAY, 0)

uid world_start_spawn(World *world) {
  Archetype empty = {0};
  if (world->archetypes.count < world->archetypes.capacity) {
    archetype_free(world->archetypes.items + world->archetypes.count);
  }
  v_append(&world->archetypes, empty);
  return world->id;
  // return world->_entity_creation_data.data.count == 0;
  //  it would be smart to use some lock instead for multithread
}
// void  add_int_comp(World* world, int component) {
//   Bytes* data = &world->_entity_creation_data.data;
//   Vups_Types* types = &world->_entity_creation_data.types;
//
//   v_append_buf(data, &component, sizeof(component));
//   v_append(types, TYPE_ENUM(component));
// }

bool archetype_compare_to_archetype(Archetype *archetype1,
                                    Archetype *archetype2) {
  if (archetype1->types.count != archetype2->types.count)
    return false;
  for (u i = 0; i < archetype1->types.count; ++i) {
    if (archetype1->types.items[i] != archetype2->types.items[i])
      return false;
  }
  return true;
}
void merge_archetypes(Archetype *to, Archetype *from) {
  u scount = from->count;
#define _MK_ARCHETYPE_TRANSFER_(TYPE, _F, _X)                                  \
  v_append_buf((&to->TYPE##_da), (from->TYPE##_da.items),                      \
               (from->TYPE##_da.count)); /*v_free(&from->TYPE##_da)*/          \
  ;

  TYPEDEF_TYPES(_MK_ARCHETYPE_TRANSFER_, 0)
  // v_free(&from->types); //replaced in favour of a check in archetype creation
  to->count += scount;
}
u world_merge_archetypes_if_same(World *world) {
  Archetype *new_archetype =
      world->archetypes.items + world->archetypes.count - 1;
  for (u i = 0; i < world->archetypes.count - 1; ++i) {
    if (archetype_compare_to_archetype(world->archetypes.items + i,
                                       new_archetype)) {
      merge_archetypes(world->archetypes.items + i, new_archetype);
      world->archetypes.count--;
      // PROBE(i);
      return i;
    }
  }

  // PROBE(world->archetypes.count - 1);
  return world->archetypes.count - 1;
}
uid world_end_spawn(World *world) {
  // figure out where to put components (for now just push to columns with
  // corresponding types.)

  Vups_Types types = world->_entity_creation_data.types;
  u j = 0;
  Archetype *new_archetype =
      world->archetypes.items + world->archetypes.count - 1;

  for (u i = 0; i < types.count; ++i) {
    switch (types.items[i]) {
#define _MK_COMPONENT_CASE_(TYPE, _F, _X)                                      \
  case TYPE_##TYPE:                                                            \
    v_append((&new_archetype->types), TYPE_##TYPE);                            \
    v_append((&new_archetype->TYPE##_da),                                      \
             *((TYPE *)(world->_entity_creation_data.data.items + j)));        \
    j += sizeof(TYPE);                                                         \
    break;
      TYPEDEF_TYPES(_MK_COMPONENT_CASE_, 0)
      // for some reason it breaks if i pass world->...items as X to macro.
      // nevermind i forgor that it was format
    default:
      assert("IMPOSSIBLE!");
    }
  }
  new_archetype->count++;
  u final_position = world_merge_archetypes_if_same(world);
  Position new_entity_position = {
      .archetype_position = final_position,
      .number_in_archetype = world->archetypes.items[final_position].count - 1};
  v_append(&world->position_table, new_entity_position);

  world->_entity_creation_data.data.count = 0;
  world->_entity_creation_data.types.count = 0;
  return world->id++;
}

Position world_get_position(World *world, uid id) {
  return world->position_table.items[id];
}

///*#*/ typedef struct Fn_Holder {
//  System_Fn_Type fn;
//} Fn_Holder;

void world_call_on_all(World *world, System_Fn_Type fn) {
  for (u i = 0; i < world->archetypes.count; ++i) {
    fn(world->archetypes.items + i);
  }
}

/// void entity_to_tmp(uid entity) {

// get every component and push it onto tmp
///}
// void tmp_to_entity_without_type(uid entity, Vups_Type without) {
//  i think this is easier than deleting type from tmp (but maybe the same
//  actually, just skips creating another tmp to store changed value.) also, we
//  slould free skipped type if possible. since it depends on the type (da's can
//  be v_free, but something complex cant, eg da of da's.) its up to the user to
//  define free functions for structs.
//
//  or... maybe i can generic the hell outa it!!!
//}

///*#*/ typedef struct System {
//  System_Fn_Type fn;
//  Vups_Types types; // types that archetype has to have for system to operate
//  on it
//} System;

void archetype_free(Archetype *a) {

#define _MK_ARCHETYPE_FREE_CALL_(TYPE, _F, _X) v_free(&a->TYPE##_da);

  TYPEDEF_TYPES(_MK_ARCHETYPE_FREE_CALL_, 0)
  v_free(&a->types);
  a->count = 0;
}
void world_free(World *world) {

  v_free(&world->_entity_creation_data.data);
  v_free(&world->_entity_creation_data.types);
  v_free(&world->position_table);
  for (u i = 0; i < world->archetypes.capacity; ++i) {
    archetype_free(world->archetypes.items + i);
  }
  v_free(&world->archetypes);
}

#endif // ENTITY_COMPONENT_SYSTEM_IMPLEMENTATION

#endif //_ENTITY_COMPONENT_SYSTEM_H
