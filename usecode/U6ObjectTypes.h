class U6UseCode;

const U6ObjectType U6ObjectTypes[] =
{
// (object,frame,distance to trigger,event(s),function,properties)
    { OBJ_U6_EGG,0,0,USE_EVENT_USE/*|USE_EVENT_LOAD*/,&U6UseCode::use_egg,OBJTYPE_NONE },

    { OBJ_U6_OAKEN_DOOR,   255,0,USE_EVENT_USE,&U6UseCode::use_door,OBJTYPE_NONE },
    { OBJ_U6_WINDOWED_DOOR,255,0,USE_EVENT_USE,&U6UseCode::use_door,OBJTYPE_NONE },
    { OBJ_U6_CEDAR_DOOR,   255,0,USE_EVENT_USE,&U6UseCode::use_door,OBJTYPE_NONE },
    { OBJ_U6_STEEL_DOOR,   255,0,USE_EVENT_USE,&U6UseCode::use_door,OBJTYPE_NONE },
    { OBJ_U6_KEY,          255,0,USE_EVENT_USE,&U6UseCode::use_key,OBJTYPE_NONE },

    { OBJ_U6_SIGN,      255,0,USE_EVENT_LOOK,&U6UseCode::look_sign,OBJTYPE_NONE },
    { OBJ_U6_BOOK,      255,0,USE_EVENT_LOOK,&U6UseCode::look_sign,OBJTYPE_NONE },
    { OBJ_U6_SCROLL,    255,0,USE_EVENT_LOOK,&U6UseCode::look_sign,OBJTYPE_NONE },
    { OBJ_U6_PICTURE,   255,0,USE_EVENT_LOOK,&U6UseCode::look_sign,OBJTYPE_NONE },
    { OBJ_U6_SIGN_ARROW,255,0,USE_EVENT_LOOK,&U6UseCode::look_sign,OBJTYPE_NONE },
    { OBJ_U6_TOMBSTONE, 255,0,USE_EVENT_LOOK,&U6UseCode::look_sign,OBJTYPE_NONE },
    { OBJ_U6_CROSS,     255,0,USE_EVENT_LOOK,&U6UseCode::look_sign,OBJTYPE_NONE },
    { OBJ_U6_CODEX,       0,0,USE_EVENT_LOOK,&U6UseCode::look_sign,OBJTYPE_NONE },

    { OBJ_U6_CRATE,        0,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_CRATE,      255,0,USE_EVENT_USE,   &U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_BARREL,       0,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_BARREL,     255,0,USE_EVENT_USE,   &U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_CHEST,        0,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_CHEST,      255,0,USE_EVENT_USE,   &U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_SECRET_DOOR,255,0,USE_EVENT_USE|USE_EVENT_SEARCH,&U6UseCode::use_secret_door,OBJTYPE_NONE },
    { OBJ_U6_BAG,        255,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_BACKPACK,   255,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_DRAWER,     255,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_STONE_LION,   1,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_NONE },
    { OBJ_U6_PLANT,        0,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_NONE },
    { OBJ_U6_GRAVE,        0,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_NONE },
    { OBJ_U6_DEAD_ANIMAL,255,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_DEAD_BODY,  255,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_DEAD_CYCLOPS, 0,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_DEAD_GARGOYLE,0,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },
    { OBJ_U6_REMAINS,    255,0,USE_EVENT_SEARCH,&U6UseCode::use_container,OBJTYPE_CONTAINER },

    { OBJ_U6_V_PASSTHROUGH,255,0,USE_EVENT_USE,&U6UseCode::use_passthrough,OBJTYPE_NONE },
    { OBJ_U6_H_PASSTHROUGH,255,0,USE_EVENT_USE,&U6UseCode::use_passthrough,OBJTYPE_NONE },

    { OBJ_U6_LEVER, 255,0,USE_EVENT_USE,&U6UseCode::use_switch,OBJTYPE_NONE },
    { OBJ_U6_SWITCH,255,0,USE_EVENT_USE,&U6UseCode::use_switch,OBJTYPE_NONE },

    { OBJ_U6_CHURN, 255,0,USE_EVENT_USE,&U6UseCode::use_churn,OBJTYPE_NONE },

    { OBJ_U6_CRANK, 255,0,USE_EVENT_USE,&U6UseCode::use_crank,OBJTYPE_NONE },

    { OBJ_U6_FIREPLACE, 255,0,USE_EVENT_USE,&U6UseCode::use_firedevice,OBJTYPE_NONE },
    { OBJ_U6_CANDLE,    255,0,USE_EVENT_USE,&U6UseCode::use_firedevice,OBJTYPE_NONE },
    { OBJ_U6_CANDELABRA,255,0,USE_EVENT_USE,&U6UseCode::use_firedevice,OBJTYPE_NONE },
    { OBJ_U6_BRAZIER,   0,0,USE_EVENT_USE,&U6UseCode::use_firedevice,OBJTYPE_NONE },
    { OBJ_U6_BRAZIER,   1,0,USE_EVENT_USE,&U6UseCode::use_firedevice,OBJTYPE_NONE },
    { OBJ_U6_TORCH,     255,0,USE_EVENT_USE|USE_EVENT_READY|USE_EVENT_GET|USE_EVENT_DROP,&U6UseCode::torch,OBJTYPE_NONE },

    { OBJ_U6_ORB_OF_THE_MOONS,255,0,USE_EVENT_USE,&U6UseCode::use_orb,OBJTYPE_NONE },
    { OBJ_U6_RED_GATE,  1,0,USE_EVENT_PASS,&U6UseCode::enter_red_moongate,OBJTYPE_NONE },
    { OBJ_U6_LADDER,255,0,USE_EVENT_USE,&U6UseCode::use_ladder,OBJTYPE_NONE },
    { OBJ_U6_CAVE,  255,0,USE_EVENT_PASS,&U6UseCode::enter_dungeon,OBJTYPE_NONE },
    { OBJ_U6_HOLE,  255,0,USE_EVENT_PASS,&U6UseCode::enter_dungeon,OBJTYPE_NONE },

    { OBJ_U6_CLOCK,     255,0,USE_EVENT_LOOK,&U6UseCode::look_clock,OBJTYPE_NONE },
    { OBJ_U6_SUNDIAL,   255,0,USE_EVENT_LOOK,&U6UseCode::look_clock,OBJTYPE_NONE },
    { OBJ_U6_MIRROR,    255,0,USE_EVENT_LOOK,&U6UseCode::look_mirror,OBJTYPE_NONE },
    { OBJ_U6_WELL,      255,0,USE_EVENT_USE,&U6UseCode::use_well,OBJTYPE_NONE },
    { OBJ_U6_POWDER_KEG,255,0,USE_EVENT_USE|USE_EVENT_TIMED,&U6UseCode::use_powder_keg,OBJTYPE_NONE },
    
    { OBJ_U6_BEEHIVE,   255,0,USE_EVENT_USE,&U6UseCode::use_beehive,OBJTYPE_NONE },
	
    { OBJ_U6_POTION,    255,0,USE_EVENT_USE,&U6UseCode::use_potion,OBJTYPE_NONE },
    { OBJ_U6_BUTTER,      0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_WINE,        0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_MEAD,        0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_ALE,         0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_BREAD,       0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_MEAT_PORTION,0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_ROLLS,       0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_CAKE,        0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_CHEESE,      0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_RIBS,        0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_MEAT,        0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_GRAPES,      0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_HAM,         0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_GARLIC,      0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_SNAKE_VENOM, 0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_HORSE_CHOPS, 0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_JAR_OF_HONEY,0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },
    { OBJ_U6_DRAGON_EGG,  0,0,USE_EVENT_USE,&U6UseCode::use_food,OBJTYPE_FOOD },

    { OBJ_U6_FAN, 0, 0,USE_EVENT_USE,&U6UseCode::use_fan,OBJTYPE_NONE },
    
    { OBJ_U6_COW, 255, 0, USE_EVENT_USE,&U6UseCode::use_cow,OBJTYPE_NONE },
    { OBJ_U6_HORSE, 255,0,USE_EVENT_USE,&U6UseCode::use_horse,OBJTYPE_NONE },
    { OBJ_U6_HORSE_WITH_RIDER, 255,0,USE_EVENT_USE,&U6UseCode::use_horse,OBJTYPE_NONE },
    
    { OBJ_U6_SHIP, 255,0,USE_EVENT_USE,&U6UseCode::use_boat,OBJTYPE_NONE },
    { OBJ_U6_SKIFF,255,0,USE_EVENT_USE,&U6UseCode::use_boat,OBJTYPE_NONE },
    { OBJ_U6_RAFT,   0,0,USE_EVENT_USE,&U6UseCode::use_boat,OBJTYPE_NONE },
//    { OBJ_U6_BALLOON_BASKET,0,0,USE_EVENT_USE,&U6UseCode::use_balloon,OBJTYPE_NONE },
    
    { OBJ_U6_QUEST_GATE,  0,0,USE_EVENT_PASS,&U6UseCode::pass_quest_barrier,OBJTYPE_NONE },

    { OBJ_U6_VORTEX_CUBE, 0,0,USE_EVENT_USE,&U6UseCode::use_vortex_cube,OBJTYPE_NONE },
    { OBJ_U6_PULL_CHAIN,  0,0,USE_EVENT_USE,&U6UseCode::use_bell,OBJTYPE_NONE },
    { OBJ_U6_BELL,      255,0,USE_EVENT_USE,&U6UseCode::use_bell,OBJTYPE_NONE },
    { OBJ_U6_SHOVEL,      0,0,USE_EVENT_USE,&U6UseCode::use_shovel,OBJTYPE_NONE },
    { OBJ_U6_FOUNTAIN,    0,0,USE_EVENT_USE,&U6UseCode::use_fountain,OBJTYPE_NONE },
    { OBJ_U6_RUBBER_DUCKY,0,0,USE_EVENT_USE,&U6UseCode::use_rubber_ducky,OBJTYPE_NONE },
    { OBJ_U6_CANNON,    255,0,USE_EVENT_USE|USE_EVENT_MOVE,&U6UseCode::use_cannon,OBJTYPE_NONE },

    { OBJ_U6_PANPIPES,   0,0,USE_EVENT_USE,&U6UseCode::play_instrument,OBJTYPE_NONE },
    { OBJ_U6_HARPSICHORD,0,0,USE_EVENT_USE,&U6UseCode::play_instrument,OBJTYPE_NONE },
    { OBJ_U6_HARP,       0,0,USE_EVENT_USE,&U6UseCode::play_instrument,OBJTYPE_NONE },
    { OBJ_U6_LUTE,       0,0,USE_EVENT_USE,&U6UseCode::play_instrument,OBJTYPE_NONE },
    { OBJ_U6_XYLOPHONE,  0,0,USE_EVENT_USE,&U6UseCode::play_instrument,OBJTYPE_NONE },

    { OBJ_U6_NOTHING, 0, 0, 0, NULL,OBJTYPE_NONE } // end indicator
};