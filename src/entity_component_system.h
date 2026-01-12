#ifndef _ENTITY_COMPONENT_SYSTEM_H
#define _ENTITY_COMPONENT_SYSTEM_H


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

uid world_start_spawn(World* world);

/*#*/typedef void (*System_Fn_Type)(Archetype*);

#define _MK_COMPONENT_TYPE_FUNCTION(TYPE, _F, _X) void  add_##TYPE##_comp(World* world, TYPE component) { \
  Bytes* data = &world->_entity_creation_data.data; \
  Vups_Types* types = &world->_entity_creation_data.types; \
  v_append_buf(data, (Byte*)&component, sizeof(component)); \
  v_append(types, TYPE_##TYPE); \
} \

TYPEDEF_TYPES(_MK_COMPONENT_TYPE_FUNCTION, 0)


bool archetype_compare_to_archetype(Archetype* archetype1,Archetype* archetype2);

void merge_archetypes(Archetype* to, Archetype* from);

u world_merge_archetypes_if_same(World *world);

uid world_end_spawn(World *world);
#define WORLD world
#define add_X_comp(X) TYPE_JUST_TYPE(X);
void test_system_fn(Archetype *a) ;
void world_call_on_all(World* world, System_Fn_Type fn);
String_Holder sh_from_c_str(const char * c_str);
String_Holder sh_(const char * c_str);
void archetype_free(Archetype* a) ;

#endif //_ENTITY_COMPONENT_SYSTEM_H
