#define VUPS_IMPLEMENTATION

#include "vups.h"

#include "gen/types.h"
#define ENTITY_COMPONENT_SYSTEM_IMPLEMENTATION
#include "entity_component_system.h"

void free_system_fn(Archetype *a);
void print_system_fn(Archetype *a);
void test_system_fn(Archetype *a);

int main() {
  World world0 = {0};
  World *world = &world0;
#define MACROBEGIN world_start_spawn(world);
#define MACROFUNC(X) add_X_comp((X));
#define MACROEND world_end_spawn(world)
#include "src/gen/main.gen.c"
  world_spawn(5, "a", sh_("hello"));
  uid b = world_spawn(6, "a", sh_("world"));
  uid c = world_spawn("a", 7, (String_Holder){0});
  world_spawn("b", 8, (String_Holder){0});
  world_spawn("a", 9, (String_Holder){0});
  world_spawn(11, (String_Holder){0});
  world_spawn(12, (String_Holder){0});
  world_spawn("b", (u16)15, (String_Holder){0});
  world_spawn("b", 15, (String_Holder){0});
  uid entity_id = world_end_spawn(world);
  (void)entity_id;
  for (u i = 0; i < world->archetypes.count; ++i) {
#define _PRINT_COMPONENT_TYPE_ARRAY(TYPE, _F, _X)                              \
  printf(#TYPE " = %zu\n", world->archetypes.items[i].TYPE##_da.count);
    // TYPEDEF_TYPES(_PRINT_COMPONENT_TYPE_ARRAY, 0)
  }
  world_call_on_all(world, test_system_fn);
  fflush(stdout);
  i32 *numi = get_component(c, i32);
  if (numi)
    printf("c [i32]= %d\n", *numi);
  numi = get_component(b, i32);
  if (numi)
    printf("b [i32]= %d\n", *numi);
  world_call_on_all(world, print_system_fn);
  world_call_on_all(world, free_system_fn);
  world_call_on_all(world, archetype_free);
  world_free(world);
  return 0;
}

void print_system_fn(Archetype *a) {
  if (a->String_Holder_da.count == 0)
    return;
  for (u i = 0; i < a->String_Holder_da.count; ++i) {
    String_Holder *sh = a->String_Holder_da.items + i;
    printf("[%s]", sh_c_str(sh));
  }
}
void free_system_fn(Archetype *a) {
  if (a->String_Holder_da.count == 0)
    return;
  for (u i = 0; i < a->String_Holder_da.count; ++i) {
    String_Holder *sh = a->String_Holder_da.items + i;
    sh_free(sh);
  }
}
void test_system_fn(Archetype *a) {
  if (a->String_Holder_da.count == a->i32_da.count) {
    // it would be faster to calculate beforehand which archetypes are affected
    // and in here operate without the check, but then systems wont be as simple
    // as just functions;
    for (u i = 0; i < a->i32_da.count; ++i) {
      i32 *num = a->i32_da.items + i;
      sh_appendf(a->String_Holder_da.items + i, "%d", *num);
    }
  }
}
