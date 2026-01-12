# entity component system
## my attempt at making an entity component system in c
its kind of ugly, dont realy recommend looking into it,  
just a flood of macros and a pinch of codegen.  
also, only creating entities is implemented, you can't delete them.  
final programs look something like this:

```c
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
  // systems (definition below)
  world_call_on_all(world, test_system_fn);

  //random access
  i32 *numi = get_component(c, i32);
  if (numi)
    printf("c [i32]= %d\n", *numi);
  numi = get_component(b, i32);
  if (numi)
    printf("b [i32]= %d\n", *numi);
  world_call_on_all(world, print_system_fn);
  world_call_on_all(world, free_system_fn);
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
```
