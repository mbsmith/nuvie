/*
 *  ObjManager.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sat Mar 15 2003.
 *  Copyright (c) 2003. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "nuvieDefs.h"

#include "Configuration.h"

#include "EggManager.h"
#include "TileManager.h"
#include "ObjManager.h"

#include "U6misc.h"
#include "U6LList.h"
#include "NuvieIOFile.h"

static const int obj_egg_table[4] = {0,   // NUVIE_GAME_NONE
                                     335, // NUVIE_GAME_U6
                                     0,   // NUVIE_GAME_MD
                                     230};  // NUVIE_GAME_SE

iAVLKey get_iAVLKey(const void *item)
{
 return ((ObjTreeNode *)item)->key;
}

ObjManager::ObjManager(Configuration *cfg, EggManager *em)
{
 uint8 i;
 std::string show_eggs_key;

 config = cfg;
 egg_manager = em;
 
 memset(actor_inventories,0,sizeof(actor_inventories));

 surface = iAVLAllocTree(get_iAVLKey);

 for(i=0;i<5;i++)
  {
   dungeon[i] = iAVLAllocTree(get_iAVLKey);
  }

 last_obj_blk_x = 0;
 last_obj_blk_y = 0;
 last_obj_blk_z = OBJ_TEMP_INIT;
 
 show_eggs_key = config_get_game_key(config);
 show_eggs_key.append("/show_eggs");
 
 config->value(show_eggs_key, show_eggs);

 config->value("config/GameType",game_type);
 
}

ObjManager::~ObjManager()
{
 //FIX ME. need to free objects.
}

bool ObjManager::loadObjs(TileManager *tm)
{
 std::string path, key;
 char *filename;
 char x,y;
 uint16 len;
 uint8 i;

 tile_manager = tm;

 loadBaseTile();

 key = config_get_game_key(config);
 key.append("/gamedir");
 
 config->value(key,path);

 filename = get_objblk_path((char *)path.c_str());

 len = strlen(filename);

 i = 0;

 for(y = 'a';y < 'i'; y++)
  {
   for(x = 'a';x < 'i'; x++)
    {
     filename[len-1] = y;
     filename[len-2] = x;
     //printf("Loading %s\n",filename);
     if(loadObjSuperChunk(filename,0) == false)
       throw "Loading objects";
     i++;
    }
  }

 filename[len-1] = 'i';

 for(i=1,x = 'a';x < 'f';x++,i++) //Load dungeons
  {
   filename[len-2] = x;
   //printf("Loading %s\n",filename);
   loadObjSuperChunk(filename,i);
  }


 loadWeightTable();

 delete filename;

 //print_egg_list();
 
 return true;
}

/*
U6LList *ObjManager::get_obj_superchunk(uint16 x, uint16 y, uint8 level)
{
 uint16 i;

 if(level == 0)
   {
    i = y * 8 + x;
    return surface[i];
   }

 return dungeon[level-1];
}
*/

bool ObjManager::is_boundary(uint16 x, uint16 y, uint8 level)
{
 U6Link *link;
 U6LList *obj_list;
 Obj *obj;
 Tile *tile, *tile1;
 uint16 tile_num;
 bool check_tile;
 uint16 i,j;

 for(j=y;j<=y+1;j++)
   {
    for(i=x;i<=x+1;i++)
      {
       obj_list = get_obj_list(i,j,level);

       if(obj_list != NULL)
         {
          link = obj_list->end();

          for(check_tile = false;link != NULL;link = link->prev)
            {
             obj = (Obj *)link->data;
             tile_num = get_obj_tile_num(obj->obj_n)+obj->frame_n;
             tile = tile_manager->get_original_tile(tile_num);

             if(obj->x == x && obj->y == y)
               { check_tile = true; }
             if(tile->dbl_width && obj->x == x+1 && obj->y == y)
                { tile_num--; check_tile = true; }
             if(tile->dbl_height && obj->x == x && obj->y == y+1)
                { tile_num--; check_tile = true; }
             if(obj->x == x+1 && obj->y == y+1 && tile->dbl_width && tile->dbl_height)
                { tile_num -= 2; check_tile = true; }
             if(check_tile)
               {
                tile1 = tile_manager->get_tile(tile_num);
                if(tile1->flags2 & TILEFLAG_BOUNDARY)
                  return true;

                check_tile = false;
               }
            }
         }
      }
   }

 return false;
}

/* no longer needed.
//FIX this needs to be moved magicnumbers :(
bool ObjManager::is_door(Obj * obj)
{
 //for U6
 if((obj->obj_n >= 297 && obj->obj_n <= 300) || obj->obj_n == 334 || obj->obj_n == 213) //OBJ_U6_MOUSEHOLE)
   return true;

 return false;
}
*/

uint8 ObjManager::is_passable(uint16 x, uint16 y, uint8 level)
{
 U6Link *link;
 U6LList *obj_list;
 Obj *obj;
 Tile *tile, *tile1;
 uint16 tile_num;
 bool check_tile;
 bool object_at_location = false;
 uint16 i,j;

 for(i=x;i<=x+1;i++)
   {
    for(j=y;j<=y+1;j++)
      {
       obj_list = get_obj_list(i,j,level);
       if(i == x && j == y && obj_list)
         {
          if(obj_list->end() != NULL)
            object_at_location = true;
         }   
       if(obj_list != NULL)
         {
          link = obj_list->end();
            
          for(check_tile = false;link != NULL;link = link->prev)
            {
             obj = (Obj *)link->data;
             tile_num = get_obj_tile_num(obj->obj_n)+obj->frame_n;
             tile = tile_manager->get_original_tile(tile_num);

             if(obj->x == x && obj->y == y)
               { check_tile = true; }
             if(tile->dbl_width && obj->x == x+1 && obj->y == y)
                { tile_num--; check_tile = true; }
             if(tile->dbl_height && obj->x == x && obj->y == y+1)
                { tile_num--; check_tile = true; }
             if(obj->x == x+1 && obj->y == y+1 && tile->dbl_width && tile->dbl_height)
                { tile_num -= 3; check_tile = true; }
             if(check_tile)
               {
                tile1 = tile_manager->get_original_tile(tile_num);
                if(tile1->passable == false)
                  return OBJ_NOT_PASSABLE;
                check_tile = false;
               }
            }
         }
      }
   }

 if(object_at_location)
   return OBJ_PASSABLE;
            
 return OBJ_NO_OBJ;
}

bool ObjManager::is_forced_passable(uint16 x, uint16 y, uint8 level)
{
 U6LList *obj_list;
 U6Link *link;
 Obj *obj;
 Tile *tile;
 
 obj_list = get_obj_list(x,y,level);
 
 if(obj_list)
  {
   for(link=obj_list->start();link!=NULL;link=link->next)
     {
      obj = (Obj *)link->data;
      tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n)+obj->frame_n);
      if(tile->flags3 & TILEFLAG_FORCED_PASSABLE)
        return true;
     }
  }
  
 return false;
}

//gets the linked list of objects at a perticular location.

U6LList *ObjManager::get_obj_list(uint16 x, uint16 y, uint8 level)
{
 iAVLTree *obj_tree;
 iAVLKey key;
 ObjTreeNode *item;

 obj_tree = get_obj_tree(level);
 key = get_obj_tree_key(x,y,level);

 item = (ObjTreeNode *)iAVLSearch(obj_tree,key);
 if(item)
  return item->obj_list;

 return NULL;
}

Tile *ObjManager::get_obj_tile(uint16 obj_n, uint8 frame_n)
{
 return  tile_manager->get_tile(get_obj_tile_num(obj_n)+frame_n);
}

Tile *ObjManager::get_obj_tile(uint16 x, uint16 y, uint8 level, bool top_obj)
{
 Obj *obj;
 Tile *tile;
 uint16 tile_num;

 obj = get_obj(x,y,level, top_obj);
 if(obj == NULL)
   return NULL;

 tile_num = get_obj_tile_num(obj->obj_n)+obj->frame_n;
 tile = tile_manager->get_tile(tile_num);

 if(tile->dbl_width && obj->x == x+1 && obj->y == y)
   tile_num--;
 if(tile->dbl_height && obj->x == x && obj->y == y+1)
   tile_num--;
 if(obj->x == x+1 && obj->y == y+1 && tile->dbl_width && tile->dbl_height)
   tile_num -= 2;

 return tile_manager->get_original_tile(tile_num);
}

Obj *ObjManager::get_obj(uint16 x, uint16 y, uint8 level, bool top_obj)
{
 Obj *obj;
 Tile *tile;

 obj = get_objBasedAt(x,y,level,top_obj);
 if(obj != NULL)
   return obj;

 obj = get_objBasedAt(x+1,y+1,level,top_obj);
 if(obj != NULL)
  {
   tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n)+obj->frame_n);
   if(tile->dbl_width && tile->dbl_height)
     return obj;
  }

 obj = get_objBasedAt(x,y+1,level,top_obj);
 if(obj != NULL)
  {
   tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n)+obj->frame_n);
   if(tile->dbl_height)
     return obj;
  }

 obj = get_objBasedAt(x+1,y,level,top_obj);
 if(obj != NULL)
  {
   tile = tile_manager->get_tile(get_obj_tile_num(obj->obj_n)+obj->frame_n);
   if(tile->dbl_width)
     return obj;
  }


 return NULL;
}

Obj *ObjManager::get_obj_of_type_from_location(uint16 obj_n, uint16 x, uint16 y, uint8 z)
{
 U6LList *obj_list;
 U6Link *link;
 Obj *obj;
 
 obj_list = get_obj_list(x,y,z);
 
 if(obj_list == NULL)
  return NULL;
  
 for(link=obj_list->start();link != NULL; link=link->next) 
   {
    obj = (Obj *)link->data;
    if(obj->obj_n == obj_n)
      return obj;
   }

 return NULL;
}

// x, y in world coords
Obj *ObjManager::get_objBasedAt(uint16 x, uint16 y, uint8 level, bool top_obj)
{
 U6Link *link;
 U6LList *obj_list;
 Obj *obj;

 obj_list = get_obj_list(x,y,level);

 if(obj_list != NULL)
   {
    if(top_obj)
       link = obj_list->end();
    else
       link = obj_list->start();

    if(link != NULL)
       {
        obj = (Obj *)link->data;
        return obj;
       }
   }

 return NULL;
}

bool ObjManager::add_obj(Obj *obj, bool addOnTop)
{
 return add_obj(get_obj_tree(obj->z), obj, addOnTop);
}

bool ObjManager::remove_obj(Obj *obj)
{
 U6LList *obj_list;

 obj_list = get_obj_list(obj->x,obj->y,obj->z);

 if(obj_list != NULL)
   {
    obj_list->remove(obj);
   }

 if(obj->status & OBJ_STATUS_TEMPORARY)
   temp_obj_list_remove(obj);

 if(obj->obj_n == obj_egg_table[game_type])
   {
    egg_manager->remove_egg(obj);
   }

 return true;
}

// remove all objects of type obj_n from location (x,y,z)

bool ObjManager::remove_obj_type_from_location(uint16 obj_n, uint16 x, uint16 y, uint8 z)
{
 U6LList *obj_list;
 U6Link *link;
 Obj *obj;
 bool objects_deleted = false;
 
 obj_list = get_obj_list(x,y,z);

 if(obj_list != NULL)
   {
    for(link=obj_list->start();link != NULL; ) 
     {
      obj = (Obj *)link->data;
      link=link->next;
      
      if(obj->obj_n == obj_n)
        {
         obj_list->remove(obj);
         delete_obj(obj);
         objects_deleted = true;
        }
     }
   }

 return objects_deleted;
}

void ObjManager::delete_obj(Obj *obj)
{
 U6Link *link;

 if(obj == NULL)
   return;

 if(obj->container)
   {
    link = obj->container->start();
    for(;link != NULL;link=link->next)
      {
       delete_obj((Obj *)link->data);
      }
    delete obj->container;
   }
   
 delete obj;
 
 return;
}

Obj *ObjManager::copy_obj(Obj *obj)
{
 Obj *new_obj;
 
 if(obj == NULL)
   return NULL;

 new_obj = new Obj;

 new_obj->obj_n = obj->obj_n;
 new_obj->frame_n = obj->frame_n;
   
 new_obj->status = obj->status;
 new_obj->qty = obj->qty;
 new_obj->quality = obj->quality;

 new_obj->x = obj->x;
 new_obj->y = obj->y;
 new_obj->z = obj->z;

 // should we copy container???
 
 return new_obj;
}

bool ObjManager::move(Obj *obj, uint16 x, uint16 y, uint8 level)
{
 U6LList *list;

 list = get_obj_list(obj->x, obj->y, obj->z);

 if(list == NULL)
  return false;

 list->remove(obj);

 obj->x = x;
 obj->y = y;
 obj->z = level;

 add_obj(get_obj_tree(level),obj,true); // add the object on top of the stack

 return true;
}

const char *ObjManager::look_obj(Obj *obj, bool show_prefix)
{
 const char *desc;
 if(obj == NULL)
  return NULL;

 desc = tile_manager->lookAtTile(get_obj_tile_num(obj->obj_n)+obj->frame_n,obj->qty,show_prefix);

 return desc;
}

const char *ObjManager::get_obj_name(Obj *obj)
{
 return tile_manager->lookAtTile(get_obj_tile_num(obj->obj_n),0,false);
}

float ObjManager::get_obj_weight(Obj *obj, bool include_container_items, bool scale)
{
 float weight;
 U6Link *link;

 weight = obj_weight[obj->obj_n];

 if(obj->qty > 1)
   weight *= obj->qty;

 //weight /= 10;
 if(obj->container != NULL && include_container_items == true)
   {
    for(link=obj->container->start();link != NULL;link=link->next)
      weight += get_obj_weight(reinterpret_cast<Obj*>(link->data), false); //don't scale container objects yet.
   }

 if(scale)
   weight /= 10;

 return weight;
}


uint16 ObjManager::get_obj_tile_num(uint16 obj_num) //assume obj_num is < 1024 :)
{
 return obj_to_tile[obj_num];
}


/* Animate all visible tiles of an object `loop_count' times.
 */
void ObjManager::animate_forwards(Obj *obj, uint32 loop_count)
{
    tile_manager->set_anim_loop(get_obj_tile_num(obj->obj_n), loop_count, 0);
}


/* Animate in reverse all visible tiles of an object `loop_count' times.
 */
void ObjManager::animate_backwards(Obj *obj, uint32 loop_count)
{
    tile_manager->set_anim_loop(get_obj_tile_num(obj->obj_n), loop_count, 1);
}


U6LList *ObjManager::get_actor_inventory(uint16 actor_num)
{
 if(actor_num >= 256)
   return NULL;

 if(actor_inventories[actor_num] == NULL)
   {
    actor_inventories[actor_num] = new U6LList();
   }

 return actor_inventories[actor_num];
}

bool ObjManager::actor_has_inventory(uint16 actor_num)
{
 if(actor_inventories[actor_num] != NULL)
  {
   if(actor_inventories[actor_num]->start() != NULL)
     return true;
  }

 return false;
}

Obj *ObjManager::find_next_obj(Obj *prev_obj)
{
 if(prev_obj == NULL)
   return NULL;

 return find_obj(prev_obj->obj_n, prev_obj->quality, prev_obj->z, prev_obj);
}

Obj *ObjManager::find_obj(uint16 obj_n, uint8 quality, uint8 level, Obj *prev_obj)
{
 iAVLTree *obj_tree = get_obj_tree(level);
 iAVLCursor cursor;
 ObjTreeNode *node;
 U6Link *link;
 Obj *new_obj;
 bool passed_prev_obj = false;

 if(prev_obj == NULL)
   passed_prev_obj = true;

 node = (ObjTreeNode *)iAVLFirst(&cursor,obj_tree);
   
 for(;node != NULL;)
  {
   link = ((U6LList *)(node->obj_list))->start();
   for(;link != NULL;link=link->next)
    {
     if( ((Obj *)(link->data))->obj_n == obj_n && ((Obj *)(link->data))->quality == quality )
       {
        new_obj = (Obj *)link->data;
        if(new_obj == prev_obj)
          passed_prev_obj = true;
        else
          {
           if(passed_prev_obj)  
             return new_obj;
          }
       }
    }

   node = (ObjTreeNode *)iAVLNext(&cursor);
  }
  
 return NULL;
}

bool ObjManager::loadBaseTile()
{
 std::string filename;
 NuvieIOFileRead basetile;
 uint16 i;

 config_get_path(config,"basetile",filename);
  
 if(basetile.open(filename) == false)
   return false;

 for(i=0;i<1024;i++)
  obj_to_tile[i] = basetile.read2();

 return true;
}

bool ObjManager::loadWeightTable()
{
 std::string filename;
 NuvieIOFileRead tileflag;

 config_get_path(config,"tileflag",filename);

 if(tileflag.open(filename) == false)
   return false;

 tileflag.seek(0x1000);

 tileflag.readToBuf(obj_weight,1024);

 return true;
}


bool ObjManager::loadObjSuperChunk(char *filename, uint8 level)
{
 NuvieIOFileRead file;
 U6LList *list;
 uint16 num_objs;
 Obj *obj;
 uint16 i;
 U6LList *inventory_list;
 iAVLTree *obj_tree;

 obj_tree = get_obj_tree(level);
 if(obj_tree == NULL)
   throw "Getting obj_tree";

 if(file.open(filename) == false)
   return false;

 list = new U6LList();

 num_objs = file.read2();

 for(i=0;i<num_objs;i++)
  {
   obj = loadObj(&file,i);
 
   list->add(obj);

   if(obj->obj_n == obj_egg_table[game_type])
     {
      egg_manager->add_egg(obj);
     }

   if(obj->status & OBJ_STATUS_IN_INVENTORY) //object in actor's inventory
     {
      inventory_list = get_actor_inventory(obj->x);
      inventory_list->add(obj);

     }
   else
     {
      if(obj->status & OBJ_STATUS_IN_CONTAINER)
        {
         addObjToContainer(list,obj);

        }
      else
        {
         if(show_eggs || obj->obj_n != obj_egg_table[game_type]) // show remaining objects, hiding eggs if neccecary.
            add_obj(obj_tree,obj);

        }

     }

  }



 delete list;

 return true;
}

bool ObjManager::add_obj(iAVLTree *obj_tree, Obj *obj, bool addOnTop)
{
 ObjTreeNode *node;
 U6LList *obj_list;
 iAVLKey key;

 key = get_obj_tree_key(obj);

 node = (ObjTreeNode *)iAVLSearch(obj_tree,key);

 if(node == NULL)
   {
    obj_list = new U6LList();

    node = (ObjTreeNode *)malloc(sizeof(struct ObjTreeNode));
    node->key = key;
    node->obj_list = obj_list;

    iAVLInsert(obj_tree, node);
   }
 else
   {
    obj_list = node->obj_list;
   }

 if(addOnTop)
   obj_list->add(obj);
 else
   obj_list->addAtPos(0,obj);
 
 if(obj->status & OBJ_STATUS_TEMPORARY)
   temp_obj_list_add(obj);

 return true;
}

bool ObjManager::addObjToContainer(U6LList *list, Obj *obj)
{
 U6Link *link;
 Obj *c_obj; //container object

 if(obj->y == 0)
   {
    link = list->gotoPos(obj->x);
    if(link != NULL)
       c_obj = (Obj *)link->data;
   }
 else // traverse back through the object list to the first object that is NOT in a container.
   {
	for(link=list->end();link != NULL;link=link->prev)
	  {
	  
	   if(link->data)
		 {
		  c_obj = (Obj *)link->data;
		  if((c_obj->status & OBJ_STATUS_IN_CONTAINER) == 0)
		     break;
		 }
		 
	  }
   }
	 
 if(link && c_obj) // we've found our container.
   {
    if(c_obj->container == NULL)
	  c_obj->container = new U6LList();
    c_obj->container->addAtPos(0,obj);
	
	//printf("Add to container %s", tile_manager->lookAtTile(get_obj_tile_num(obj->obj_n)+obj->frame_n,0,false));
	//printf(" -> %s (%x,%x,%x)\n", tile_manager->lookAtTile(get_obj_tile_num(c_obj->obj_n)+c_obj->frame_n,0,false),c_obj->x,c_obj->y,c_obj->z);

    return true;
   }

 return false;
}

Obj *ObjManager::loadObj(NuvieIOFileRead *file, uint16 objblk_n)
{
 uint8 b1,b2;
 Obj *obj;

 obj = new Obj;
 obj->objblk_n = objblk_n;

 obj->status = file->read1();

 obj->x = file->read1(); // h
 b1 = file->read1();
 obj->x += (b1 & 0x3) << 8;

 obj->y = (b1 & 0xfc) >> 2;
 b2 = file->read1();
 obj->y += (b2 & 0xf) << 6;

 obj->z = (b2 & 0xf0) >> 4;

 b1 = file->read1();
 b2 = file->read1();
 obj->obj_n = b1;
 obj->obj_n += (b2 & 0x3) << 8;

 obj->frame_n = (b2 & 0xfc) >> 2;

 obj->qty = file->read1();
 obj->quality = file->read1();

 return obj;
}

char *ObjManager::get_objblk_path(char *path)
{
 char *filename;
 uint16 len;

 if(path == NULL)
   return NULL;

 len = strlen(path);

 if(len == 0)
   return NULL;

 filename = new char [len+19]; // + room for /savegame/objblkxx\0

 strcpy(filename,path);
 if(filename[len-1] != U6PATH_DELIMITER)
   {
    filename[len] = U6PATH_DELIMITER;
    filename[len+1] = '\0';
   }

 strcat(filename,"savegame/objblkxx");

 return filename;
}

iAVLTree *ObjManager::get_obj_tree(uint8 level)
{
 if(level == 0)
   return surface;

 if(level > 5)
   return NULL;

 return dungeon[level-1];
}

inline iAVLKey ObjManager::get_obj_tree_key(Obj *obj)
{
 return get_obj_tree_key(obj->x, obj->y, obj->z);
}

iAVLKey ObjManager::get_obj_tree_key(uint16 x, uint16 y, uint8 level)
{
 if(level == 0)
   return y * 1024 + x;
 else
   return y * 256 + x;
}

void ObjManager::update(uint16 x, uint16 y, uint8 z)
{
 uint16 cur_blk_x, cur_blk_y;
 
 // We're changing levels so clean out all temp objects on the current level.
 if(last_obj_blk_z != z)
   {
    if(last_obj_blk_z != OBJ_TEMP_INIT)
      temp_obj_list_clean_level(last_obj_blk_z);

    last_obj_blk_z = z;
    
    return;
   }

 cur_blk_x = x >> 5; // x / 32;
 cur_blk_y = y >> 5; // y / 32;
 
 //FIX for level change. we want to remove all temps on level change.
 if(cur_blk_x != last_obj_blk_x || cur_blk_y != last_obj_blk_y)
   {
    last_obj_blk_x = cur_blk_x;
    last_obj_blk_y = cur_blk_y;

    temp_obj_list_clean_area(x, y);
    egg_manager->spawn_eggs(x, y, z);
   }

 return;
}

bool ObjManager::temp_obj_list_add(Obj *obj)
{
 if(obj == NULL)
  return false;

 temp_obj_list.push_back(obj);

 return true;
}

bool ObjManager::temp_obj_list_remove(Obj *obj)
{
 temp_obj_list.remove(obj);
 return true;
}

// clean objects from a whole level.
void ObjManager::temp_obj_list_clean_level(uint8 z)
{
 std::list<Obj *>::iterator obj;
 std::list<Obj *>::iterator tmp_obj;
 
 for(obj = temp_obj_list.begin(); obj != temp_obj_list.end();)
   {
    if((*obj)->z == z)
      {
       printf("Removing obj %s.\n", tile_manager->lookAtTile(get_obj_tile_num((*obj)->obj_n)+(*obj)->frame_n,0,false));
       tmp_obj = obj;
       tmp_obj++;
       remove_obj(*obj); // this calls temp_obj_list_remove() 
       delete *obj;
       obj = tmp_obj;
      } 
    else
      obj++;
   }

 return;
}


// Clean objects more than 32 tiles from position
void ObjManager::temp_obj_list_clean_area(uint16 x, uint16 y)
{
 std::list<Obj *>::iterator obj;
 std::list<Obj *>::iterator tmp_obj;
 sint16 dist_x, dist_y;
 
 for(obj = temp_obj_list.begin(); obj != temp_obj_list.end();)
   {
    dist_x = abs((sint16)(*obj)->x - x);
    dist_y = abs((sint16)(*obj)->y - y);

    if(dist_x > 32 || dist_y > 32)
      {
       printf("Removing obj %s.\n", tile_manager->lookAtTile(get_obj_tile_num((*obj)->obj_n)+(*obj)->frame_n,0,false));
       tmp_obj = obj;
       tmp_obj++;
       remove_obj(*obj);
       delete *obj;
       obj = tmp_obj;
      }
    else
      obj++;
   }

 return;
}

/*
inline U6LList *ObjManager::get_schunk_list(uint16 x, uint16 y, uint8 level)
{
 uint16 sx, sy;

 if(level == 0)
   {
    sx = x / 128;
    sy = y / 128;
    return surface[sy * 8 + sx];
   }

 return dungeon[level-1];
}
*/
//prints a human readable list of object number / names.

void ObjManager::print_object_list()
{
 uint16 i;

 for(i=0;i<1024;i++)
  {
   printf("%04d: %s\n",i,tile_manager->lookAtTile(get_obj_tile_num(i),0,false));
  }

 return;
}

void ObjManager::print_egg_list()
{
 iAVLTree *obj_tree;
 ObjTreeNode *tree_node;
 iAVLCursor cursor;
 U6LList *obj_list;
 U6Link *link;
 Obj *obj;
 uint8 i;
 
 for(i=0;i <= 5;i++)
 {
  obj_tree = get_obj_tree(i);
  tree_node = (ObjTreeNode *)iAVLFirst(&cursor,obj_tree);
  
  for(;tree_node != NULL;tree_node = (ObjTreeNode *)iAVLNext(&cursor) )
   {
    obj_list = (U6LList *)tree_node->obj_list;
    for(link = obj_list->start(); link != NULL; link = link->next)
     {
      obj = (Obj *)link->data;
      if(obj->obj_n == 335)
       {
        print_obj(obj,false);
       }
     }
   }
 }

 return;
}

void ObjManager::print_obj(Obj *obj, bool in_container, uint8 indent)
{
 U6Link *link;
 Obj *container_obj;

 printf("\n");
 print_indent(indent);
 printf("%s ",tile_manager->lookAtTile(get_obj_tile_num(obj->obj_n)+obj->frame_n,0,false));
 
 if(in_container == false)
   printf("at %x, %x, %x (%d,%d,%d)",obj->x, obj->y, obj->z, obj->x, obj->y, obj->z);
 printf("\n");

 print_indent(indent);
 printf("objblk_n: %d\n", obj->objblk_n);
 
 print_indent(indent);
 printf("obj_n: %d\n",obj->obj_n);

 print_indent(indent);
 printf("frame_n: %d\n",obj->frame_n);

 print_indent(indent);
 printf("Tile: %d\n", get_obj_tile_num(obj->obj_n));

 print_indent(indent);
 printf("Status: ");
 print_b(obj->status);
 if(obj->status != 0)
  {
   printf(" ( ");
   if(obj->status & OBJ_STATUS_IN_CONTAINER)
     printf("CONT ");
   if(obj->status & OBJ_STATUS_IN_INVENTORY)
     printf("INV ");
   if(obj->status & OBJ_STATUS_TEMPORARY)
     printf("TEMP ");
  
   printf(")");
  }

 printf("\n");
  
 if(in_container)
  {
   print_indent(indent);
   printf("parent_id = %d, y = %d, z = %d\n", obj->x, obj->y, obj->z);
  }

 print_indent(indent);
 printf("Quantity: %d\n",obj->qty);
 print_indent(indent);
 printf("Quality: %d\n",obj->quality);
 
 if(obj->container)
   {
    print_indent(indent);
    printf("Container\n");
    print_indent(indent);
    printf("---------");
    
    for(link = obj->container->start(); link != NULL; link = link->next)
     {
      container_obj = (Obj *)link->data;
      print_obj(container_obj, true, indent + 2);
     }
     
    print_indent(indent);
    printf("---------\n");
   }    
  
 if(in_container == false)
   printf("\n");

 return;
}

Obj *new_obj(uint16 obj_n, uint8 frame_n, uint16 x, uint16 y, uint16 z)
{
 Obj *obj;
 
 obj = new Obj();
 
 obj->obj_n = obj_n;
 obj->frame_n = frame_n;

 obj->x = x; 
 obj->y = y; 
 obj->z = z;
 
 return obj; 
}
