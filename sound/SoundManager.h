/*
 *  SoundManager.h
 *  Nuvie
 *
 *  Created by Adrian Boeing on Wed Jan 21 2004.
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

//notes: this file should be moved to the nuvie main directory instead of sound.

//priorities:
//todo: 
//-sample loading partialy implemented, will do later (now is 21/01/04)
//-make songs fade in & out - add query/callback for end of song so that they can cycle 
//-make samples sound from mapwindow
//-make samples fade in & out according to distance
//-try and use original .m files

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H
#include "Sound.h"
#include "nuvieDefs.h"
#include "Configuration.h"
#include "NuvieIOFile.h"


class SoundManager {
public:
	SoundManager();
	~SoundManager();
	
	bool nuvieStartup(Configuration *config);
	bool Init();
	void Update(); //updates the active sounds

private:
	bool LoadSongs(string directory, string scriptname);
	bool LoadObjectSamples(string directory, string scriptname);
	bool LoadTileSamples(string directory, string scriptname);

	Sound* SoundExists(string name); //have we loaded this sound before?

	Sound* RequestTileSound(int id);
	Sound* RequestObjectSound(int id);
	Sound* RequestSong(string group); //request a song from this group
	
	map<int,SoundCollection *> m_TileSampleMap;
	map<int,SoundCollection *> m_ObjectSampleMap;
	map<string,SoundCollection *> m_MusicMap;
	list<Sound *> m_Songs;
	list<Sound *> m_Samples;
	Configuration *m_Config;

	//state info:
	string m_CurrentGroup;
	Sound *m_pCurrentSong;
	list<Sound *> m_ActiveSounds;
public:
	static bool g_MusicFinished;
};

#endif
