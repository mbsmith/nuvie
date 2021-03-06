/*
 *  PCSpeakerSfxManager.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Wed Feb 9 2011.
 *  Copyright (c) 2011. All rights reserved.
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

#include <string>
#include <map>

#include "nuvieDefs.h"
#include "mixer.h"
#include "decoder/PCSpeakerStream.h"
#include "PCSpeakerSfxManager.h"


PCSpeakerSfxManager::PCSpeakerSfxManager(Configuration *cfg, Audio::Mixer *m) : SfxManager(cfg, m)
{

}

PCSpeakerSfxManager::~PCSpeakerSfxManager()
{

}

bool PCSpeakerSfxManager::playSfx(SfxIdType sfx_id, uint8 volume)
{
	return playSfxLooping(sfx_id, NULL, volume);
}


bool PCSpeakerSfxManager::playSfxLooping(SfxIdType sfx_id, Audio::SoundHandle *handle, uint8 volume)
{
	Audio::AudioStream *stream = NULL;

	if(sfx_id == NUVIE_SFX_BLOCKED)
	{
		stream = new PCSpeakerFreqStream(311, 0xa);
	}
	else if(sfx_id == NUVIE_SFX_SUCCESS)
	{
		stream = new PCSpeakerFreqStream(2000, 0xa);
	}
	else if(sfx_id == NUVIE_SFX_FAILURE)
	{
		stream = new PCSpeakerSweepFreqStream(800, 2000, 50, 1);
	}
	else if(sfx_id == NUVIE_SFX_ATTACK_SWING)
	{
		stream = new PCSpeakerSweepFreqStream(400, 750, 150, 5);
	}
	else if(sfx_id == NUVIE_SFX_RUBBER_DUCK)
	{
		stream = new PCSpeakerSweepFreqStream(5000, 8000, 50, 1);
	}
	else if(sfx_id == NUVIE_SFX_HIT)
	{
		stream = new PCSpeakerRandomStream(0x2710, 0x320, 1);
	}
	else if(sfx_id == NUVIE_SFX_BROKEN_GLASS)
	{
		stream = makePCSpeakerGlassSfxStream(mixer->getOutputRate());
	}
	else if(sfx_id == NUVIE_SFX_CORPSER_DRAGGED_UNDER)
	{
		stream = new PCSpeakerSweepFreqStream(1200, 2000, 40, 1);
	}
	else if(sfx_id == NUVIE_SFX_CORPSER_REGURGITATE)
	{
		stream = new PCSpeakerRandomStream(0x258, 0x1b58, 1);
	}
	else if(sfx_id >= NUVIE_SFX_CASTING_MAGIC_P1 && sfx_id <= NUVIE_SFX_CASTING_MAGIC_P1_8)
	{
		uint8 magic_circle = sfx_id - NUVIE_SFX_CASTING_MAGIC_P1 + 1;
		stream = makePCSpeakerMagicCastingP1SfxStream(mixer->getOutputRate(), magic_circle);
	}
	else if(sfx_id >= NUVIE_SFX_CASTING_MAGIC_P2 && sfx_id <= NUVIE_SFX_CASTING_MAGIC_P2_8)
	{
		uint8 magic_circle = sfx_id - NUVIE_SFX_CASTING_MAGIC_P2 + 1;
		stream = makePCSpeakerMagicCastingP2SfxStream(mixer->getOutputRate(), magic_circle);
	}
	else if(sfx_id == NUVIE_SFX_BELL)
	{
		stream = new PCSpeakerStutterStream(-1, 0x4e20, 0x3e80, 1, 0x7d0);
	}
	else if(sfx_id == NUVIE_SFX_AVATAR_DEATH)
	{
		stream = makePCSpeakerAvatarDeathSfxStream(mixer->getOutputRate());
	}
	else if(sfx_id == NUVIE_SFX_KAL_LOR)
	{
		stream = makePCSpeakerKalLorSfxStream(mixer->getOutputRate());
	}
	else if(sfx_id == NUVIE_SFX_SLUG_DISSOLVE)
	{
		stream = makePCSpeakerSlugDissolveSfxStream(mixer->getOutputRate());
	}
	else if(sfx_id == NUVIE_SFX_HAIL_STONE)
	{
		stream = makePCSpeakerHailStoneSfxStream(mixer->getOutputRate());
	}
  else if(sfx_id == NUVIE_SFX_EARTH_QUAKE)
  {
    stream = makePCSpeakerEarthQuakeSfxStream(mixer->getOutputRate());
  }

	if(stream)
	{
		sfx_duration = stream->getLengthInMsec();
		playSoundSample(stream, handle, volume);
		return true;
	}

	return false;
}

void PCSpeakerSfxManager::playSoundSample(Audio::AudioStream *stream, Audio::SoundHandle *looping_handle, uint8 volume)
{
	Audio::SoundHandle handle;

	if(looping_handle)
	{
		Audio::LoopingAudioStream *looping_stream = new Audio::LoopingAudioStream((Audio::RewindableAudioStream *)stream, 0);
		mixer->playStream(Audio::Mixer::kPlainSoundType, looping_handle, looping_stream, -1, volume);
	}
	else
	{
		mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, stream, -1, volume);
	}

}
