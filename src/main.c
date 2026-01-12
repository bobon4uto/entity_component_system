#define VUPS_IMPLEMENTATION
#include "vups.h"

#include "gen/types.h"

/*#*/typedef u32 uid;
typedef struct Position {
  u archetype_position;
  u number_in_archetype;
} Position;
DA_TYPE(Position,Positions);

#define _VUPS_TT_ENUM_DEF(TYPE, FORMAT,_X)                                                 \
  TYPE_##TYPE,
typedef enum {
  UNKNOWN,
  TYPEDEF_TYPES(_VUPS_TT_ENUM_DEF, 0)
} Vups_Type ;
#define _VUPS_TT_ENUM(TYPE, FORMAT)                                                 \
  TYPE:                                                                        \
  TYPE_##TYPE,
#define _TYPE_ENUM(X, T)                                                         \
  _Generic((X), TYPEDEF_TYPES(T, X) VUPS_NEW_TYPES(T) default: UNKNOWN)
#define TYPE_ENUM(X) _TYPE_ENUM(X, _VUPS_TT_ENUM)

#define _VUPS_TT_JUST_TYPE(TYPE,_F, X)                                                 \
  TYPE:  add_##TYPE##_comp(WORLD,*(TYPE*)&(_temp_var)),
#define _TYPE_JUST_TYPE(X, T)  do {                                                       \
  typeof(X) _temp_var = X; \
  _Generic((X), TYPEDEF_TYPES(T, X) VUPS_NEW_TYPES(T) default: assert("UNKNOWN")) ;} while(0)

#define TYPE_JUST_TYPE(X) _TYPE_JUST_TYPE(X, _VUPS_TT_JUST_TYPE)

typedef struct Entity {
  uid id;
  int i;
  char_ptr ch;

} Entity;
/*#*/typedef u8 Byte;
typedef Byte* Byte_Ptr;
DA_TYPE(Byte,Bytes);
DA_TYPE(Byte_Ptr,Byte_Ptrs);
DA_TYPE(Vups_Type,Vups_Types);



#define _MK_COMPONENT_TYPE_ARRAY_TYPE(TYPE,_F,_X) DA_TYPE(TYPE,DA_##TYPE);
TYPEDEF_TYPES(_MK_COMPONENT_TYPE_ARRAY_TYPE, 0)
#define _MK_COMPONENT_TYPE_ARRAY(TYPE,_F,_X) DA_##TYPE TYPE##_da = {0};
TYPEDEF_TYPES(_MK_COMPONENT_TYPE_ARRAY, 0)
/*# we dont want Archetype to contain itself*/typedef struct Archetype  {
  Vups_Types types;
  u count;
#define _MK_COMPONENT_TYPE_ARRAY_AS_STRUCT_PART(TYPE,_F,_X) DA_##TYPE TYPE##_da;
TYPEDEF_TYPES(_MK_COMPONENT_TYPE_ARRAY_AS_STRUCT_PART, 0)
} Archetype;
DA_TYPE(Archetype,Archetypes);

/*#*/typedef struct _Entity_Creation_Data  {
  Bytes data;
  Vups_Types types;
} _Entity_Creation_Data;

/*#*/typedef struct World {
  uid id;
  Archetypes archetypes;
  Positions position_table;

  _Entity_Creation_Data _entity_creation_data;
} World;

uid world_start_spawn(World* world) {
  // it doesnt sound all that important, lets kust check if anything
  // is being spawned and return true if not
  Archetype empty = {0};
  v_append(&world->archetypes, empty);
  return world->id;
  //return world->_entity_creation_data.data.count == 0;
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

TYPEDEF_TYPES(_MK_COMPONENT_TYPE_FUNCTION, 0)


bool archetype_compare_to_archetype(Archetype* archetype1,Archetype* archetype2) {
  if (archetype1->types.count != archetype2->types.count) return false;
  for (u i=0; i<archetype1->types.count; ++i) {
    if (archetype1->types.items[i] != archetype2->types.items[i]) return false;
  }
  return true;
}
void merge_archetypes(Archetype* to, Archetype* from) {

#define _MK_ARCHETYPE_TRANSFER_(TYPE,_F,_X) v_append_buf((&to->TYPE##_da), (from->TYPE##_da.items), (from->TYPE##_da.count) );

TYPEDEF_TYPES(_MK_ARCHETYPE_TRANSFER_, 0 )
  to->count+=from->count;

}
u world_merge_archetypes_if_same(World *world) {
  Archetype* new_archetype = world->archetypes.items + world->archetypes.count - 1;
  for (u i=0; i<world->archetypes.count - 1; ++i) {
    if (archetype_compare_to_archetype(world->archetypes.items+i, new_archetype)) {
      merge_archetypes(world->archetypes.items+i, new_archetype);
      world->archetypes.count--;
      PROBE(i);
      return i;
    }
  }

      PROBE(world->archetypes.count - 1);
  return world->archetypes.count - 1;
}
uid world_end_spawn(World *world) {
  // figure out where to put components (for now just push to columns with corresponding types.)
  
  Vups_Types types = world->_entity_creation_data.types;
  u j = 0;
  Archetype* new_archetype = world->archetypes.items + world->archetypes.count - 1;

  for(u i = 0; i<types.count; ++i) {
    switch (types.items[i]) {
#define _MK_COMPONENT_CASE_(TYPE,_F,_X) case TYPE_##TYPE : v_append((&new_archetype->types), TYPE_##TYPE);v_append((&new_archetype->TYPE##_da), *((TYPE *) (world->_entity_creation_data.data.items+j)) ); \
      j += sizeof(TYPE); \
      break;
TYPEDEF_TYPES(_MK_COMPONENT_CASE_, 0 )
  // for some reason it breaks if i pass world->...items as X to macro.
  // nevermind i forgor that it was format
      default: assert("IMPOSSIBLE!");
    }
  }
  new_archetype->count++;
  u final_position = world_merge_archetypes_if_same(world);
  Position new_entity_position = {.archetype_position=final_position,.number_in_archetype=world->archetypes.items[final_position].count-1};
  v_append(&world->position_table, new_entity_position);

   world->_entity_creation_data.data.count = 0;
   world->_entity_creation_data.types.count = 0;
  return world->id++;
}



#define WORLD world
#define add_X_comp(X) TYPE_JUST_TYPE(X);



Position world_get_position(World* world, uid id) {
  return world->position_table.items[id];
}
void test_system_fn(Archetype *a) {
//given an archetype, do something
printf("systen running");
if (a->String_Holder_da.count==0) return;
if (a->String_Holder_da.count!=a->i32_da.count) return;
// or nothing if it doesnt have something mandatory

  for (u i=0;i<a->i32_da.count;++i) {
    i32* num = a->i32_da.items+i;
    String_Holder* sh = a->String_Holder_da.items + i;
    sh_appendf(a->String_Holder_da.items + i, "%d", *num);
    printf("!%d>>",*num);
    printf("[%zu|",sh->count);
    printf("%zu]\n",sh->capacity);
  }
}
/*#*/typedef void (*System_Fn_Type)(Archetype*);
/*#*/ typedef struct Fn_Holder {
  System_Fn_Type fn;
} Fn_Holder;

void world_call_on_all(World* world, System_Fn_Type fn) {
  for(u i=0;i<world->archetypes.count;++i) {
    fn(world->archetypes.items+i);
  }
}

/*#*/ typedef struct System {
  System_Fn_Type fn;
  Vups_Types types; // types that archetype has to have for system to operate on it
} System;


int main() {
  World world0 ={0};
  World* world =&world0;
#define MACROBEGIN world_start_spawn(world);
#define MACROFUNC(X) add_X_comp ( (X) );
#define MACROEND world_end_spawn(world)
#include "src/gen/main.gen.c"
//_VUPS_TT_JUST_TYPE(typeof(1),"", 1);
//
uid a = world_spawn(5, "a", (String_Holder){0});
uid b = world_spawn(6, "a", (String_Holder){0});
uid c = world_spawn("a",7, (String_Holder){0});
world_spawn("b",8, (String_Holder){0});
uid d = world_spawn("a",9, (String_Holder){0});
world_spawn(11, (String_Holder){0});
world_spawn(12, (String_Holder){0});
world_spawn("b",(u16)15, (String_Holder){0});
uid e =world_spawn("b",15, (String_Holder){0});
  //add_X_comp ( (Bytes){0} );
//PROBE("a");
  //world_spawn("a","b","c");




#define get_component(entity,TYPE) (WORLD->archetypes.items[world_get_position(WORLD, entity).archetype_position].TYPE##_da.items + world_get_position(WORLD, entity).number_in_archetype) \


Position a_pos = world_get_position(world, a); // random access or something
PROBE(a_pos.archetype_position);
PROBE(a_pos.number_in_archetype);
Position b_pos = world_get_position(world, b);
PROBE(b_pos.archetype_position);
PROBE(b_pos.number_in_archetype);
//Position b_pos = world_get_position(world, b);
PROBE(a);
PROBE(b);
  Vups_Types types = world->archetypes.items[0].types;
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
  for (u i=0; i< world->archetypes.count;++i) {
#define _PRINT_COMPONENT_TYPE_ARRAY(TYPE,_F,_X) printf(#TYPE " = %zu\n",world->archetypes.items[i].TYPE##_da.count);
//TYPEDEF_TYPES(_PRINT_COMPONENT_TYPE_ARRAY, 0)
  }
 world_call_on_all(world, test_system_fn);
 fflush(stdout);
i32* numi = get_component(c,i32);
numi = get_component(b,i32);

if (numi) printf("numi= %d\n", *numi);
String_Holder* sh = get_component(c,String_Holder);

 printf("??%p??\n", sh->items);
 printf("%s\n", sh_c_str(sh));
 sh = get_component(a,String_Holder);
 printf("??%p??\n", sh->items);
 printf("%s\n", sh_c_str(sh));

 sh = get_component(d,String_Holder);

 printf("??%p??\n", sh->items);
 printf("%s\n", sh_c_str(sh));
 sh = get_component(e,String_Holder);
 printf("??%p??\n", sh->items);
 printf("%s\n", sh_c_str(sh));
 fflush(stdout);
 // DA_char_ptr cp = char_ptrs;
  //printf("%p",char_ptrs.items[0]);
 // printf("%s",char_ptrs.items[0]);
  //for (u i = 0; i< char_ptrs.count; ++i) {
  //  PROBE(char_ptrs.items[i]);
  //}






  return 0;
}














