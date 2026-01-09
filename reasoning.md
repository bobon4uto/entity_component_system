# dump of my thoughts 
entities    rock  player
health      NULL  10
position    1,1   1,2
 so...
 rock id = 0
 player id = 1
                       0    1
 health components   = NULL 10
 position components = 1,1  1,2

 or...

 health components   = 10 (of id 1)
 position components = 1,1 (of id 0)  1,2 (of id 1)

 always an index overhead for second option, getting entitys components is not trivial
 ...but when we have a lot of components that apply only to 1 or 2 entities, this saves space. were is 2026 memorys not cheaph
 solve not trivial problem and its good?
 entity can hold POINTERS OH NO
 but bacically systems wont care about pointers(eh? no, they will)
 so idea is
 player entity
 id, ptr_to_health, ptr_to_pos
 if system needs to know bofa, it needs to use pointers

 but how often does it need that? it doesnt sound all that needed.
 maybe we should merge components that turn out to be connected?
 then is should be fine

 at the end we only need pointers when retrieving information to show it to the user,
 it should happen once per frame, which should be fine
but then first option is just.. simpler

to get a component you just index array and if null that entity doesnt have that component.

wait, health =0 is also NULL, thats the problem of THIS approach.
maybe pad it with mandatory bool?
then sipmlicity is hindered a bit, but only a bit, we still can store entity as just id.

i want to tackle simpler approach.

WAIT!!!
when we define component, it should be polymorphyc? so, we can use the type of component to indicate its noEXISTANCEn

I would like to use union as { component_of_health, component_of_pos }
but then health will have the same size as pos, and if we have bulky component types, ALL components will be bulky.
so we need to do proper transparancy or something?
or just define Heapth_Component type and so on. 
maybe we should do not type of component but size? type is inferred by the parent array, and for string type components it would be great to have that.
initially my thought was size will allow us to easier define new types of components but more i think of it - not really, but still size instead of type should be good.
   we can generalize health to "numeric component"
   and then just push it to array where all health comnponents reside
how to make something like polymorphyc struct? eeeeh...
it wouldve been easier ina dynamic language like lua, but its like giving your job to lua lol
i think one should think closely to arena, because all the arrays for components are just that.
are they called anon or transparent structs though?

I noticed that its STILL not easy to access Entity's components if they have variable lenght,  
but i presume its for the better? I think systems actually shouldn't  do direct access, they just iterate arrays.
then when second option isnt that bad, besides, empty element in simple aproach will still be here and not  really empty, look at that:

1.
[size=2, data="hi"]\ [size=0]\ [size=5, data="hello"]
notice that when we iterate we have to skip size=0
[size=2, id=0, data="hi"]\ [size=5, id=2, data="hello"]
and in second approach, there is NOTHING!
this again will make it more intuitive and remove empty fields.
my main concern with 1 approach actually is that size=0 means skip sizeof(size_t) since size itself is still there.
it can be fixed by defining 1 as bacically 0, but then all others have to be +1. in second approach we can skip sizeof(mandatory fields) and never worry about it.

well i realize that variable size component makes everything hard but i wont back off from this idea, as i thing its very important also ordering seems to matter still but in the way that all the components of the enmtity have to be in the same place, that makes operating on multiple compondents with one system possible (otherwise how does one iterate arrays with gaps?).
take the situation where we iterate health and pos, for 1 approach its easy

         0    1
healths nil  100
positis 1,1  1,2

we just iterate all.

healths 100 (of 1)
positis 1,1(of 0)  1,2 (of 1)

there if we just iterate and operate on health and pos we will be in trouble. so, we have to skip carefully, we see that 0th entity doesnt have health so we cant operate on it, so we skip all the field that it has (you already see how thats multipointer hell?)
but in what situation does one need to operate on health and pos?
aoe attack you dummie

ahhh...

maybe one should merge components that are related, but then health+pos seems really odd.

again my main problem is that i dont want to iterate halffull array, but if we sepatate sizes???

           en1  en2
helthsize  0    4
helths         100
possize    8    8
poss      1,1  1,2

yo, i think that works
BUT

when we iterate, we still have to be carefull, eg

            en1  en2  en3
helthsize   4    0    4
helths     200       100
possize     8    8    8
poss       1,1  1,1  1,2


when we iterate, en2 will be skipped, but if we call something like attack, it will attack en3, since we're pointing to it.
if we pass size to attack it should be fine (attack will check if size is not 0 and then attack)

it looks like two variants merged, kindof. because we no longer need id field, and size is now separate from main array

wait isnt that bad? now we acces 2 separate arrays ... it does appear counter-productive.
note that this approach is bacically 1st but we sepaarate size

i dream of a situation when i add entity without components and arrays stay the same, thats 2nd approach
first approach requires to add zeroed components...
oh no, what if we add component to entity?
there is no space in current setup, even for 1st approach, if component size is variable, it may not fit.
so we do need id. 
what if everytime system needs multiple components it orders the arrays? eg


helths     200/1 100/3
poss       1,1/1 1,1/2 1,2/3

            en1  en3   en2
helths     200/1 100/3
poss       1,1/1 1,2/3 1,1/2

but then when we operate on 3+ its hard.


after doing a bit of research online i read that firt approach is dense table
https://github.com/SanderMertens/ecs-faq
also i noticed that in flecs they make new tables each time there is a new entity that has components that no other antity has (e.g. health but no pos for my previous examples.)
https://github.com/SanderMertens/flecs :src/storage/table.h:141
k

so I also saw a bit of other things but i dont really want to copycat every detail, so i think i will get the general idea and roll with it.

also maybe its smart to do the simplest possible implementation and layout the api, and then do the normal one with api in mind.
whatsw the simplest implementation? Entity has array of pointers to every component. systems iterate over one array that has all entities and iterate
every array of components. and checks if that component is what it needs and does the thingy.

I realize that im tryimg to make a general ECS that can do anything, but really i just want some small ecs that will work and not have the advantage of
changing on the fly. Foe mr it will be fine if ECS is compile-time dependent. so, I think instead of doing all of that i can just make Component of type something, no?

so i managed to create something like a ECS, but using OOP type thing not data oriented. it was pretty straightforward, but I havent tested it on anything big, oh wait but i just can hloldup
so it was fine untill i started to increase component size, in static approach it matters a lot. if we limit size to 100 uint64's, we can have millions of entities, but when we go higher it will drastically shrink it. 
ah, and also implementaation isnt full, ya cant delete anything, only create. well, yiu can remove a component, but it will be just turned to type NONE, its still there.
I would like to tackle table-based approachm but imma sleep for now bb


