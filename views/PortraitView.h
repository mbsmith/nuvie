#ifndef __PortraitView_h__
#define __PortraitView_h__

/*
 *  PortraitView.h
 *  Nuive
 *
 *  Created by Eric Fry on Tue May 20 2003.
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

#include "U6def.h"
#include "Configuration.h"
#include "View.h"
#include "Screen.h"
#include "Text.h"
#include "TileManager.h"
#include "ObjManager.h"
#include "Party.h"
#include "Portrait.h"

class PortraitView : public View {
 
 uint8 cur_actor_num;
 Portrait *portrait;
 
 unsigned char *portrait_data;
 
 //cursor pos
 
 public:
 PortraitView(Configuration *cfg);
 ~PortraitView();
 
 bool init(Screen *s, Text *t, Party *p, TileManager *tm, ObjManager *om, Portrait *port);
 void update_display();
 bool handle_input(SDLKey *input);
 
 void set_portrait(uint8 actor_num, char *name);
 
 protected:
 
};

#endif /* __PortraitView_h__ */

