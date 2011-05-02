#ifndef __Effect_h__
#define __Effect_h__

#include "SDL.h"
#include "CallBack.h"
#include "Map.h"
#include "ObjManager.h"
#include "AnimManager.h"

//class Actor;
class EffectManager;
class Game;
class MapWindow;
class NuvieAnim;
class Screen;
class TimedAdvance;
class TimedCallback;
class ObjManager;

// Effects add themselves to EffectManager and most start immediately.

/* Effects: * = unwritten or untested
 * Quake - earthquake from cyclops or volcanos
 * Hit - hit actor anim + sfx
 * Explosive - explosion caused by powder keg, volcanos, or cannonball hit
 * ThrowObject - any thrown object or tile
 * Cannonball (FIX: change to UseCodeThrow)
 * Missile - throw object to ground or actor; optionally cause damage
 * *Boomerang - spin Missile and return to sender
 * Drop - throw obj from inventory to ground
 * Sleep - pause game & advance time quickly
 * Fade - fade the mapwindow in or out
 * GameFadeIn - blocks user-input until Fade is complete
 * *Palette - do something with the color palette
 * Vanish - fade from an image of the mapwindow to the real mapwindow
 * *FadeObject - might not need this since Vanish can be used
 * U6WhitePotion - will probably make PaletteEffect to do this
 */


/* Control animation and sounds in the game world.
 */
class Effect : public CallBack
{
protected:
    Game *game;
    EffectManager *effect_manager;
    bool defunct;

    uint32 retain_count;

public:
    Effect();
    ~Effect();

    void retain() { retain_count++; }
    void release() { if(retain_count > 0) retain_count--; }
    bool is_retained() { return retain_count == 0 ? false : true; }

    void delete_self() { defunct = true; }
    void add_anim(NuvieAnim *anim);

    bool is_defunct()  { return(defunct); }
    uint16 callback(uint16, CallBack *, void *)
                       { return(0); }
};

#define CANNON_SPEED 320
/* Toss a cannon ball from one actor to another, or from an object towards
 * a numbered direction.
 */
class CannonballEffect : public Effect
{
    UseCode *usecode;
    NuvieAnim *anim;
//  *sfx;
    Obj *obj;
    MapCoord target_loc; // where cannonball will hit

    void start_anim();

public:
    CannonballEffect(Obj *src_obj, sint8 direction = -1);
//    CannonballEffect(Actor *src_actor, Actor *target_actor); from a ship

    uint16 callback(uint16 msg, CallBack *caller, void *data);

};

class ProjectileEffect : public Effect
{
protected:
	uint16 tile_num;

    MapCoord start_loc; // where explosion will start
    vector<MapCoord> targets;
    uint8 anim_speed;
    bool trail;
    uint16 initial_tile_rotation;
    uint16 finished_tiles;

    vector<MapEntity> hit_entities;

    virtual void start_anim();

public:
    ProjectileEffect() {}
    ProjectileEffect(uint16 tileNum, MapCoord start, MapCoord target, uint8 speed, bool trailFlag, uint16 initialTileRotation);
    ProjectileEffect(uint16 tileNum, MapCoord start, vector<MapCoord> t, uint8 speed, bool trailFlag, uint16 initialTileRotation);

    void init(uint16 tileNum, MapCoord start, vector<MapCoord> t, uint8 speed, bool trailFlag, uint16 initialTileRotation);

    uint16 callback(uint16 msg, CallBack *caller, void *data);

    vector<MapEntity> *get_hit_entities() { return &hit_entities; }
};

class ExpEffect : public ProjectileEffect
{
    UseCode *usecode;
    NuvieAnim *anim;

    Obj *obj;
    uint16 exp_tile_num;

protected:
    void start_anim();
public:
    ExpEffect(uint16 tileNum, MapCoord location);

};

/* Use to add an effect with timed activity. Self-contained timer must be
 * stopped/started with the included methods.
 */
class TimedEffect : public Effect
{
protected:
    TimedCallback *timer;
public:
    TimedEffect()                  { timer = NULL; }
    ~TimedEffect()                 { stop_timer(); }

    void start_timer(uint32 delay);
    void stop_timer();

    void delete_self() { stop_timer(); Effect::delete_self(); }

    virtual uint16 callback(uint16 msg, CallBack *caller, void *data) { if(msg == MESG_TIMED) delete_self(); return(0); } //= 0;
};


/* Shake the visible play area around.
 */
class QuakeEffect : public TimedEffect
{
    MapWindow *map_window;
    static QuakeEffect *current_quake; // do nothing if already active
    sint32 sx, sy; // last map_window movement amount
    MapCoord orig; // map_window location at start
    Actor *orig_actor; // center map_window on actor
    uint32 stop_time;
    uint8 strength; // magnitude

public:
    QuakeEffect(uint8 magnitude, uint32 duration, Actor *keep_on = NULL);
    ~QuakeEffect();
    uint16 callback(uint16 msg, CallBack *caller, void *data);

    void init_directions();
    void recenter_map();
    void stop_quake();
};


/* Hit target actor.
 */
class HitEffect : public Effect
{
public:
    HitEffect(Actor *target, uint32 duration = 300);
    HitEffect(MapCoord location);
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};


/* Create explosion animation and sounds from the source location out to
 * specified radius. Hit actors and objects for `dmg'.
 */
class ExplosiveEffect : public Effect
{
protected:
    NuvieAnim *anim;
//  *sfx;
    MapCoord start_at;
    uint32 radius;
    uint16 hit_damage; // hp taken off actors hit by explosion

    void start_anim();

public:
    ExplosiveEffect(uint16 x, uint16 y, uint32 size, uint16 dmg = 0);
    uint16 callback(uint16 msg, CallBack *caller, void *data);

    // children can override
    virtual void delete_self() { Effect::delete_self(); }
    virtual bool hit_object(Obj *obj) { return(false); } // explosion hit something
                                                         // true return=end effect
};


/* Explosion that sends usecode event to an object on completion.
 */
class UseCodeExplosiveEffect : public ExplosiveEffect
{
     Obj *obj; // explosion came from this object (can be NULL)
     Obj *original_obj; // don't hit this object (chain-reaction avoidance hack)

 public:
    UseCodeExplosiveEffect(Obj *src_obj, uint16 x, uint16 y, uint32 size, uint16 dmg = 0, Obj *dont_hit_me = NULL)
                          : ExplosiveEffect(x, y, size, dmg), obj(src_obj), original_obj(dont_hit_me)
    {
    }
    void delete_self();
    bool hit_object(Obj *hit_obj); // explosion hit something



};


/* Toss object tile from one location to another with a TossAnim, and play a
 * sound effect. The ThrowObjectEffect is constructed with uninitialized
 * parameters and isn't started until start_anim() is called.
 */
class ThrowObjectEffect : public Effect
{
protected:
    ObjManager *obj_manager;
    NuvieAnim *anim; // TossAnim
//  *sfx;
    MapCoord start_at, stop_at; // start_at -> stop_at
    Obj *throw_obj; // object being thrown
    Tile *throw_tile; // graphic to use (default is object's tile)
    uint16 throw_speed; // used in animation
    uint16 degrees; // rotation of tile
    uint8 stop_flags; // TossAnim blocking flags

public:
    ThrowObjectEffect();
    virtual ~ThrowObjectEffect() { }

    void hit_target(); // stops effect
    void start_anim();

    virtual uint16 callback(uint16 msg, CallBack *caller, void *data) = 0;
};


/* Drop an object from an actor's inventory. Object is removed from the actor
 * after starting the effect, and added to the map when the effect is complete.
 * Effect speed is gametype-defined.
 */
class DropEffect : public ThrowObjectEffect
{
    Actor *drop_from_actor;
public:
    DropEffect(Obj *obj, uint16 qty, Actor *actor, MapCoord *drop_loc);

    void hit_target();

    void get_obj(Obj *obj, uint16 qty);
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};

#define MISSILE_DEFAULT_SPEED 200
#define MISSILE_HIT_TARGET  TOSS_TO_BLOCKING
#define MISSILE_HIT_OBJECTS (TOSS_TO_BLOCKING|TOSS_TO_OBJECT)
#define MISSILE_HIT_ACTORS  (TOSS_TO_BLOCKING|TOSS_TO_ACTOR)
#define MISSILE_HIT_ALL     (TOSS_TO_BLOCKING|TOSS_TO_OBJECT|TOSS_TO_ACTOR)

/* Throw a missile towards a target location. If the target is an actor or
 * object, it will be hit for the requested damage. If the target is an empty
 * map location, the object will be added to the map. The missile always stops
 * if hitting a blocking tile.
 *
 * Decide in the attack logic, before constructing this, whether or not it was
 * successful, and use the appropriate constructor. You can set the effect to
 * hit any actors or objects in the way if the attack missed.
 */
class MissileEffect : public ThrowObjectEffect
{
    ActorManager *actor_manager;

    uint16 hit_damage; // hp taken off actor/object hit by missile
    Actor *hit_actor;
    Obj *hit_obj;

public:
    MissileEffect(uint16 tile_num, uint16 obj_n, const MapCoord &source,
                  const MapCoord &target, uint8 dmg, uint8 intercept = MISSILE_HIT_TARGET, uint16 speed = MISSILE_DEFAULT_SPEED);

    void init(uint16 tile_num, uint16 obj_n, const MapCoord &source,
              const MapCoord &target, uint32 dmg, uint8 intercept, uint32 speed);
    void hit_target();
    void hit_blocking();
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};

#if 0
/* Throw an object and bring it back.
 */
class BoomerangEffect : public ThrowObjectEffect
{
// I might even add an arc from the center line for a cool effect.
};


/* Cycle or modify the game palette in some way.
 */
class PaletteEffect : public TimedEffect
{
// palette effects are created from child classes (new BlackPotionEffect();)
// ...and these can include SFX like any other effect
// but PaletteEffect is not abstract (new PaletteEffect(timing & color params...);)
};


#endif


/* For sleeping at inns. Fade-out, advance time, and fade-in.
 */
class SleepEffect : public Effect
{
    TimedAdvance *timer; // timed event
    uint8 stop_hour, stop_minute; // sleep until this time
    std::string stop_time;
public:
    SleepEffect(std::string until);
    SleepEffect(uint8 to_hour);
    ~SleepEffect();

    uint16 callback(uint16 msg, CallBack *caller, void *data);
    void delete_self();
};


typedef enum { FADE_PIXELATED, FADE_CIRCLE, FADE_PIXELATED_ONTOP } FadeType;
typedef enum { FADE_IN, FADE_OUT } FadeDirection;

/* Manipulate the MapWindow for two types of fades. One is a stippled-like fade
 * that draws pixels to random locations on the screen until completely flooded
 * with a set color. The other changes the ambient light until fully black.
 */
class FadeEffect : public TimedEffect
{
protected:
    static FadeEffect *current_fade; // do nothing if already active

    MapWindow *map_window;
    Screen *screen; // for PIXELATED, the overlay is blitted to the screen...
    SDL_Rect *viewport; // ...at the MapWindow coordinates set here
    SDL_Surface *overlay; // this is what gets blitted

    FadeType fade_type; // PIXELATED[_ONTOP] or CIRCLE
    FadeDirection fade_dir; // IN (removing color) or OUT (adding color)
    uint32 fade_speed; // meaning of this depends on fade_type
    uint8 pixelated_color; // color from palette that is being faded to/from
    SDL_Surface *fade_from; // image being faded from or to (or NULL if coloring)
    uint16 fade_x, fade_y; // start fade from this point (to fade_from size)

    uint32 evtime, prev_evtime; // time of last message to callback()
    uint32 pixel_count, colored_total; // number of pixels total/colored

public:
    FadeEffect(FadeType fade, FadeDirection dir, uint32 color = 0, uint32 speed = 0);
    FadeEffect(FadeType fade, FadeDirection dir, SDL_Surface *capture, uint32 speed = 0);
    FadeEffect(FadeType fade, FadeDirection dir, SDL_Surface *capture, uint16 x, uint16 y, uint32 speed = 0);
    ~FadeEffect();
    virtual uint16 callback(uint16 msg, CallBack *caller, void *data);
    
    bool pixelated_fade_out();
    bool pixelated_fade_in();
    bool circle_fade_out();
    bool circle_fade_in();

    void delete_self();

protected:
    void init(FadeType fade, FadeDirection dir, uint32 color, SDL_Surface *capture, uint16 x, uint16 y, uint32 speed);
    void init_pixelated_fade();
    void init_circle_fade();

    inline bool find_free_pixel(uint32 &rnum, uint32 pixel_count);
    uint32 pixels_to_check();
    bool pixelated_fade_core(uint32 pixels_to_check, sint16 fade_to);
//    inline uint32 get_random_pixel(uint16 center_thresh = 0);
};


/* Front-end to FadeEffect that fades in, and resumes game.
 */
class GameFadeInEffect : public FadeEffect
{
public:
    GameFadeInEffect(uint32 color);
    ~GameFadeInEffect();
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};


/* Captures an image of the MapWindow without an object, then places the object
 * on the map and fades to the new image. (or the opposite if FADE_OUT is used)
 */
class FadeObjectEffect : public Effect
{
    ObjManager *obj_manager;
    Obj *fade_obj;
    FadeDirection fade_dir;
public:
    FadeObjectEffect(Obj *obj, FadeDirection dir);
    ~FadeObjectEffect();
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};


/* Do a blocking fade-to (FADE_OUT) from a captured image of the game area, to
 * the active game area. (transparent) This is used for vanish or morph effects.
 */
#define VANISH_WAIT true
#define VANISH_NOWAIT false
class VanishEffect : public Effect
{
    bool input_blocked;
public:
    VanishEffect(bool pause_user = VANISH_NOWAIT);
    ~VanishEffect();
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};

class TileFadeEffect : public TimedEffect
{
	TileAnim *anim;
	Tile *to_tile;
	Tile *anim_tile;
	Actor *actor;
	uint8 color_from, color_to;
	bool inc_reverse;

public:
	TileFadeEffect(MapCoord loc, Tile *from, Tile *to, FadeType type, uint16 speed);
	//TileFadeEffect(MapCoord loc, Tile *from, uint8 color_from, uint8 color_to, bool reverse, uint16 speed);
	//TileFadeEffect(Actor *a, uint8 c_from, uint8 c_to, bool include_return, uint16 speed);
	~TileFadeEffect();
	uint16 callback(uint16 msg, CallBack *caller, void *data);

protected:
	void add_actor_anim(uint8 speed);
};

class TileBlackFadeEffect : public TimedEffect
{
	Actor *actor;
	Obj *obj;
	uint8 color;
	bool reverse;
	uint16 fade_speed;

	uint16 num_anim_running;
public:
	TileBlackFadeEffect(Actor *a, uint8 fade_color, uint16 speed);
	TileBlackFadeEffect(Obj *o, uint8 fade_color, uint16 speed);
	~TileBlackFadeEffect();
	uint16 callback(uint16 msg, CallBack *caller, void *data);
protected:
	void init(uint8 fade_color, uint16 speed);
	void add_actor_anim();
	void add_obj_anim(Obj *o);
	void add_tile_anim(MapCoord loc, Tile *tile);
};

/* Briefly modify the mapwindow colors, disable map-blacking and player
 * movement for a few seconds, then enable both.
 */
class XorEffect : public TimedEffect
{
    MapWindow *map_window;
    uint32 length;
    SDL_Surface *capture; // this is what gets blitted

    void xor_capture(uint8 mod);
    void init_effect();

public:
    /* eff_ms=length of visual effect */
    XorEffect(uint32 eff_ms);
    ~XorEffect() { }

    /* Called by the timer between each effect stage. */
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};

/* Briefly modify the mapwindow colors, disable map-blacking and player
 * movement for a few seconds, then enable both.
 */
class U6WhitePotionEffect : public TimedEffect
{
    MapWindow *map_window;
    uint8 state; // 0=start, 1=eff1, 2=eff2, 3=x-ray, 4=complete
    uint32 start_length, eff1_length, eff2_length, xray_length;
    SDL_Surface *capture; // this is what gets blitted
    Obj *potion; // allows effect to call usecode and delete object

    void xor_capture(uint8 mod);
    void init_effect();

public:
    /* eff_ms=length of visual effect; delay_ms=length of x-ray effect */
    U6WhitePotionEffect(uint32 eff_ms, uint32 delay_ms, Obj *callback_obj=NULL);
    ~U6WhitePotionEffect() { }

    /* Called by the timer between each effect stage. */
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};


/* Pause the game, create an effect, and wait for user input to continue. */
class PauseEffect: public Effect
{
public:
    /* Called by the Effect handler when input is available. */
    uint16 callback(uint16 msg, CallBack *caller, void *data);
    virtual void delete_self() { Effect::delete_self(); }
    PauseEffect();
    ~PauseEffect() { }
};


/* colors for PeerEffect */
const uint8 peer_tilemap[4] =
{
    0x0A, // GROUND/PASSABLE
    0x09, // WATER
    0x07, // WALLS/BLOCKED
    0x0C  // DANGER/DAMAGING
};

#define PEER_TILEW 4
const uint8 peer_tile[PEER_TILEW*PEER_TILEW] =
    { 0,1,0,1,
      1,0,1,0,
      0,1,0,1,
      1,0,1,0 };

/* Display an overview of the current area in the MapWindow. Any new actions
 * cancel the effect and return to the prompt.
 * (area is 48x48 tiles around the player, regardless of MapWindow size)
 */
class PeerEffect : public PauseEffect
{
    MapWindow *map_window;
    SDL_Surface *overlay; // this is what gets blitted
    Obj *gem; // allows effect to call usecode and delete object
    MapCoord area; // area to display (top-left corner)
    uint8 tile_trans; // peer_tile transparency mask (0 or 1)
    uint16 map_pitch;

    inline void blit_tile(uint16 x, uint16 y, uint8 c);
    inline void blit_actor(Actor *actor);
    inline uint8 get_tilemap_type(uint16 wx, uint16 wy, uint8 wz);
    void fill_buffer(uint8 *mapbuffer, uint16 x, uint16 y);
    void peer();

public:
    PeerEffect(uint16 x, uint16 y, uint8 z, Obj *callback_obj=0);
    ~PeerEffect() { }
    void init_effect();
    void delete_self();
};

/* Run an effect asynchronously and keep updating the world until the effect completes. */
class AsyncEffect : public Effect
{
protected:
	Effect *effect;
	bool effect_complete;

public:
    AsyncEffect(Effect *e);
    ~AsyncEffect();
    void run();
    uint16 callback(uint16 msg, CallBack *caller, void *data);
};

#endif /* __Effect_h__ */
