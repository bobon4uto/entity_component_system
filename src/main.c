#define VUPS_IMPLEMENTATION

#include "vups.h"

#include "gen/types.h"
#include "entity_component_system.h"

//#define _MK_COMPONENT_TYPE_ARRAY(TYPE,_F,_X) DA_##TYPE TYPE##_da = {0};
//TYPEDEF_TYPES(_MK_COMPONENT_TYPE_ARRAY, 0)


uid world_start_spawn(World* world) {
  Archetype empty = {0};
  if (world->archetypes.count < world->archetypes.capacity) {
    archetype_free(world->archetypes.items + world->archetypes.count);
  }
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


bool archetype_compare_to_archetype(Archetype* archetype1,Archetype* archetype2) {
  if (archetype1->types.count != archetype2->types.count) return false;
  for (u i=0; i<archetype1->types.count; ++i) {
    if (archetype1->types.items[i] != archetype2->types.items[i]) return false;
  }
  return true;
}
void merge_archetypes(Archetype* to, Archetype* from) {
u scount = from->count;
#define _MK_ARCHETYPE_TRANSFER_(TYPE,_F,_X) v_append_buf((&to->TYPE##_da), (from->TYPE##_da.items), (from->TYPE##_da.count) ); /*v_free(&from->TYPE##_da)*/;

TYPEDEF_TYPES(_MK_ARCHETYPE_TRANSFER_, 0 )
//v_free(&from->types); //replaced in favour of a check in archetype creation
  to->count+=scount;

}
u world_merge_archetypes_if_same(World *world) {
  Archetype* new_archetype = world->archetypes.items + world->archetypes.count - 1;
  for (u i=0; i<world->archetypes.count - 1; ++i) {
    if (archetype_compare_to_archetype(world->archetypes.items+i, new_archetype)) {
      merge_archetypes(world->archetypes.items+i, new_archetype);
      world->archetypes.count--;
     // PROBE(i);
      return i;
    }
  }

      //PROBE(world->archetypes.count - 1);
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






Position world_get_position(World* world, uid id) {
  return world->position_table.items[id];
}
void free_system_fn(Archetype *a) {
  if (a->String_Holder_da.count==0) return;
  for (u i=0;i<a->String_Holder_da.count;++i) {
    String_Holder* sh = a->String_Holder_da.items + i;
    //printf("[freeing] sh = %s\n", sh_c_str(sh));
    sh_free(sh);
  }
}
void test_system_fn(Archetype *a) {
//given an archetype, do something
if (a->String_Holder_da.count==0) return;
if (a->String_Holder_da.count!=a->i32_da.count) return;
// or nothing if it doesnt have something mandatory

  for (u i=0;i<a->i32_da.count;++i) {
    i32* num = a->i32_da.items+i;
    //String_Holder* sh = a->String_Holder_da.items + i;
    sh_appendf(a->String_Holder_da.items + i, "%d", *num);
    //printf("!%d>>",*num);
    //printf("[%zu|",sh->count);
    //printf("%zu]\n",sh->capacity);
  }
}

///*#*/ typedef struct Fn_Holder {
//  System_Fn_Type fn;
//} Fn_Holder;

void world_call_on_all(World* world, System_Fn_Type fn) {
  for(u i=0;i<world->archetypes.count;++i) {
    fn(world->archetypes.items+i);
  }
}


///void entity_to_tmp(uid entity) {

  //get every component and push it onto tmp
///}
//void tmp_to_entity_without_type(uid entity, Vups_Type without) {
  // i think this is easier than deleting type from tmp (but maybe the same actually, just skips creating another tmp to store changed value.)
  // also, we slould free skipped type if possible. since it depends on the type (da's can be v_free, but something complex cant, eg da of da's.)
  // its up to the user to define free functions for structs.
  //
  // or... maybe i can generic the hell outa it!!!
//}


///*#*/ typedef struct System {
//  System_Fn_Type fn;
//  Vups_Types types; // types that archetype has to have for system to operate on it
//} System;

String_Holder sh_from_c_str(const char * c_str) {
  String_Holder sh0 = {0};
  String_Holder* sh = &sh0;
  sh_append_c_str(sh,c_str);
  return sh0;
}
String_Holder sh_(const char * c_str) {
  return sh_from_c_str(c_str);
}
void archetype_free(Archetype* a) {

#define _MK_ARCHETYPE_FREE_CALL_(TYPE,_F,_X) v_free(&a->TYPE##_da);

TYPEDEF_TYPES(_MK_ARCHETYPE_FREE_CALL_, 0 )
v_free(&a->types);
a->count = 0;
}
void world_free(World* world) {


  v_free(&world->_entity_creation_data.data);
  v_free(&world->_entity_creation_data.types);
  v_free(&world->position_table);
  for (u i=0;i<world->archetypes.capacity;++i){
    archetype_free(world->archetypes.items + i);

  }
  v_free(&world->archetypes);

}

int main() {
  World world0 ={0};
  World* world =&world0;
#define MACROBEGIN world_start_spawn(world);
#define MACROFUNC(X) add_X_comp ( (X) );
#define MACROEND world_end_spawn(world)
#include "src/gen/main.gen.c"
//_VUPS_TT_JUST_TYPE(typeof(1),"", 1);
//
uid a = world_spawn(5, "a", sh_("hello"));
uid b = world_spawn(6, "a", sh_("world"));
uid c = world_spawn("a",7, (String_Holder){0});
world_spawn("b",8, (String_Holder){0});
uid d = world_spawn("a",9, (String_Holder){0});
(void)a;
(void)b;
(void)c;
(void)d;
world_spawn(11, (String_Holder){0});
world_spawn(12, (String_Holder){0});
world_spawn("b",(u16)15, (String_Holder){0});
world_spawn("b",15, (String_Holder){0});
  //add_X_comp ( (Bytes){0} );
//PROBE("a");
  //world_spawn("a","b","c");




#define get_component(entity,TYPE) (WORLD->archetypes.items[world_get_position(WORLD, entity).archetype_position].TYPE##_da.items + world_get_position(WORLD, entity).number_in_archetype) \


//Position a_pos = world_get_position(world, a); // random access or something
//PROBE(a_pos.archetype_position);
//PROBE(a_pos.number_in_archetype);
//Position b_pos = world_get_position(world, b);
//PROBE(b_pos.archetype_position);
//PROBE(b_pos.number_in_archetype);
//Position b_pos = world_get_position(world, b);
//PROBE(a);
//PROBE(b);
  Vups_Types types = world->archetypes.items[0].types;
  Bytes data = world->_entity_creation_data.data;
  //PROBE(types.count);
  for (u i = 0; i< data.count; ++i) {
  //  PROBE(data.items[i]);
  }
  for (u i = 0; i< types.count; ++i) {
  //  PROBE(types.items[i]);
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
//String_Holder* sh = get_component(c,String_Holder);

 ///printf("??%p??\n", sh->items);
 //printf("%s\n", sh_c_str(sh));
 //sh = get_component(a,String_Holder);
 //printf("??%p??\n", sh->items);
 //printf("%s\n", sh_c_str(sh));

 //sh = get_component(d,String_Holder);

 //printf("??%p??\n", sh->items);
// printf("%s\n", sh_c_str(sh));
 //sh = get_component(e,String_Holder);
// printf("??%p??\n", sh->items);
 //printf("%s\n", sh_c_str(sh));
 fflush(stdout);
 // DA_char_ptr cp = char_ptrs;
  //printf("%p",char_ptrs.items[0]);
 // printf("%s",char_ptrs.items[0]);
  //for (u i = 0; i< char_ptrs.count; ++i) {
  //  PROBE(char_ptrs.items[i]);
  //}





  world_call_on_all(world, free_system_fn);
  world_call_on_all(world, archetype_free);
  world_free(world);

  return 0;
}














