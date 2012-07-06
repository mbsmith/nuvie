/*
 *  SaveManager.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Wed Apr 28 2004.
 *  Copyright (c) 2004. All rights reserved.
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

#include "Actor.h"
#include "ActorManager.h"
#include "U6misc.h"
#include "U6LList.h"
#include "SaveManager.h"
#include "NuvieIOFile.h"
#include "NuvieFileList.h"

#include "GUI.h"
#include "Console.h"
#include "SaveDialog.h"
#include "SaveSlot.h"
#include "SaveGame.h"
//#include <direct.h>

#if defined __linux__ || MACOSX
#include <sys/stat.h>
#endif

SaveManager::SaveManager(Configuration *cfg)
{
 config = cfg;
 dialog = NULL;
 actor_manager = NULL;
 obj_manager = NULL;
 savegame = NULL;
 game_type = 0;
}

// setup the savedir variable.
// throws if the directory is not found or inaccessable
bool SaveManager::init()
{
 std::string savedir_key;

 config->value("config/GameType",game_type);

 search_prefix.assign("nuvie");
 search_prefix.append(get_game_tag(game_type));

 savegame = new SaveGame(config);

 savedir_key = config_get_game_key(config);

 savedir_key.append("/savedir");

 config->value(savedir_key, savedir);

 if(savedir.size() == 0)
   {
#ifdef WIN32
    DEBUG(0,LEVEL_WARNING,"savedir config variable not found. Using current directory for saves!\n");
    savedir.assign("");
#elif defined(__linux__)
    DEBUG(0,LEVEL_WARNING,"savedir config variable not found. Using ~/.nuvie for saves.\n");
    savedir = getenv("HOME");
    savedir += "/.nuvie";
    if(directory_exists(savedir.c_str()) == false && !savedir.empty()) {
      // try to create the save dir if it doesn't exist
      DEBUG(0,LEVEL_NOTIFICATION,"creating directory ~/.nuvie\n");
      mkdir(savedir.c_str(), 0700);
    }
#else
    DEBUG(0,LEVEL_WARNING,"savedir config variable not found. Using current directory for saves!\n");
    savedir.assign(".");
#endif
   }

/*
   if(directory_exists(savedir.c_str()) == false && !savedir.empty())
	   // try to create the save dir if it doesn't exist
	   mkdir(savedir.c_str());
*/

   if(directory_exists(savedir.c_str()) == false)
   {
      //DEBUG(0,LEVEL_NOTIFICATION,"creating directory %s\n", savedir.c_str());
	   ConsoleAddInfo(std::string("Creating savegame directory: \"") + savedir + "\"");
      if(mkdir_recursive(savedir.c_str(), 0700) != 0)
      {
         DEBUG(0,LEVEL_ERROR,"savedir '%s' either not found or not accessible!\n", savedir.c_str());
         ConsoleAddError("Setting Save Directory!");
         return false;
      }
   }

   ConsoleAddInfo("Save dir: \"" + savedir + "\"");

 return true;
}

SaveManager::~SaveManager()
{
 delete savegame;
}

bool SaveManager::load_save()
{
	bool newsave = false;

	config->value("config/newgame", newsave, false);

	if(newsave)
	{
		return savegame->load_new();
	}

	return load_latest_save();
}

bool SaveManager::load_latest_save()
{
 NuvieFileList filelist;
 std::string *filename;
 std::string fullpath;

 if(filelist.open(savedir.c_str(), search_prefix.c_str(), NUVIE_SORT_TIME_DESC) == false)
   return false;

 filename = filelist.get_latest();

 filelist.close();

 if(filename != NULL)
   build_path(savedir, filename->c_str(), fullpath);

 if(!filename || savegame->load(fullpath.c_str()) == false) //try to load the latest save
   {
    if(savegame->load_original() == false)          // fall back to savegame/ if no nuvie savegames exist.
      {
       return savegame->load_new();                 // if all else fails try to load a new game.
      }
   }

 return true;
}

void SaveManager::create_dialog()
{
 GUI *gui = GUI::get_gui();

 if(dialog == NULL)
   {
    dialog = new SaveDialog((GUI_CallBack *)this);
    dialog->init(savedir.c_str(), search_prefix.c_str());
    dialog->grab_focus();
    gui->AddWidget(dialog);
   }

 return;
}

bool SaveManager::load(SaveSlot *save_slot)
{
 std::string save_filename;

 if(save_slot->get_filename()->size() == 0)
   return savegame->load_new();

 build_path(savedir, save_slot->get_filename()->c_str(), save_filename);

 return savegame->load(save_filename.c_str());
}

bool SaveManager::save(SaveSlot *save_slot)
{
 std::string save_filename;
 std::string save_fullpath;
 std::string save_desc;



 save_filename.assign(save_slot->get_filename()->c_str());

 if(save_filename.size() == 0)
   save_filename = get_new_savefilename();

 build_path(savedir, save_filename, save_fullpath);

 save_desc = save_slot->get_save_description();
 return savegame->save(save_fullpath.c_str(), &save_desc);
}

std::string SaveManager::get_new_savefilename()
{
 uint32 max_count;
 uint32 count;
 std::string *filename;
 std::string new_filename;
 std::string search_prefix;
 std::string num_str;
 char end_buf[8]; // 000.sav\0
 NuvieFileList filelist;

 max_count = 0;

 search_prefix = "nuvie";
 search_prefix.append(get_game_tag(game_type));

 new_filename = search_prefix;

 filelist.open(savedir.c_str(), search_prefix.c_str(), NUVIE_SORT_TIME_DESC);

 for(;(filename = filelist.next());)
   {
    // search for highest save number here.
    num_str = filename->substr(7,3); //extract the number.
    count = atoi(num_str.c_str());
    if(count > max_count)
      max_count = count;
   }

 filelist.close();

 max_count++;

 snprintf(end_buf, 8, "%03d.sav",max_count);

 new_filename.append(end_buf);

 return new_filename;
}

GUI_status SaveManager::callback(uint16 msg, GUI_CallBack *caller, void *data)
{
 SaveSlot *save_slot;

 if(caller == dialog)
  {
   switch(msg)
     {
       case SAVEDIALOG_CB_DELETE : dialog = NULL; break;

       case SAVEDIALOG_CB_LOAD : save_slot = (SaveSlot *)data;
                                 if(load(save_slot) == false)
                                    return GUI_PASS;
                                 break;

       case SAVEDIALOG_CB_SAVE : save_slot = (SaveSlot *)data;
                                 if(save(save_slot) == false)
                                    return GUI_PASS;
                                 break;
       default : break;
     }
  }

 return GUI_YUM;
}
