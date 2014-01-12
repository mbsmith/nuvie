/*
 *  VideoDialog.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Mon May 10 2004.
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

#include "SDL.h"
#include "nuvieDefs.h"

#include "GUI.h"
#include "GUI_types.h"
#include "GUI_button.h"
#include "GUI_text.h"
#include "GUI_TextToggleButton.h"
#include "GUI_CallBack.h"
#include "GUI_area.h"
#include "U6misc.h"
#include "Dither.h"
#include "Scale.h"
#include "Screen.h"
#include "MapWindow.h"
#include "GUI_Dialog.h"
#include "VideoDialog.h"
#include "Configuration.h"

#define VD_WIDTH 280
#define VD_HEIGHT 153

VideoDialog::VideoDialog(GUI_CallBack *callback)
          : GUI_Dialog(Game::get_game()->get_game_x_offset() + (Game::get_game()->get_game_width() - VD_WIDTH)/2,
                       Game::get_game()->get_game_y_offset() + (Game::get_game()->get_game_height() - VD_HEIGHT)/2,
                       VD_WIDTH, VD_HEIGHT, 244, 216, 131, GUI_DIALOG_UNMOVABLE) {
	callback_object = callback;
	init();
	grab_focus();
}

bool VideoDialog::init() {
	int textY[] = { 11, 24, 37, 50, 63 , 76, 89, 102, 115, 128 };
	int buttonY[] = { 9, 22, 35, 48, 61, 74, 87, 100, 113, 126, 139 };
	int colX[] = { 9, 29, 63, 201, 239};
	int height = 12;
	int yesno_width = 32;
	bool no_fullscreen = false; // no compatible fullscreen setting found
	GUI_Widget *widget;
	GUI *gui = GUI::get_gui();
	GUI_Font *font = gui->get_font();
	Game *game = Game::get_game();
	Screen *screen = game->get_screen();
	uint16 bpp = screen->get_bpp();
	uint16 scr_width = screen->get_width();
	uint16 scr_height = screen->get_height();
	const char* const yesno_text[] = { "no", "yes" };
#define SCALER_AND_SCALE_CANNOT_BE_CHANGED 1 // FIXME need to be able to change these in game
#if SCALER_AND_SCALE_CANNOT_BE_CHANGED
	only2x_button = NULL; scale_button = scaler_button = scale_win_button = scaler_win_button = NULL;
	int scale = screen->get_scale_factor();
	no_fullscreen = !SDL_VideoModeOK(scr_width * scale, scr_height * scale, bpp, SDL_FULLSCREEN);
#else
// scaler
	int num_scalers = screen->get_scaler_reg()->GetNumScalers();
	const char *scaler_text[num_scalers];
	for (int i = 0; i <= num_scalers; i++)
		scaler_text[i] = screen->get_scaler_reg()->GetNameForIndex(i);

	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[0], 0, 0, 0, "Scaler:", font);
	AddWidget(widget);
// scaler(fullscreen)
	int num_scalers_fullscreen, fullscreen_scaler_selection;
	bool no_only2x_scalers = !SDL_VideoModeOK(scr_width * 2, scr_height * 2, bpp, SDL_FULLSCREEN);
	if(no_only2x_scalers) {
		num_scalers_fullscreen = 2;
		fullscreen_scaler_selection = (screen->get_scaler_index() == 1) ? 1 : 0;
	}
	else {
		num_scalers_fullscreen = num_scalers;
		fullscreen_scaler_selection = screen->get_scaler_index();
	}
	scaler_button = new GUI_TextToggleButton(this, colX[2], buttonY[0], 208, height, scaler_text, num_scalers_fullscreen, fullscreen_scaler_selection, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(scaler_button);
// scaler (windowed)
	scaler_win_button = new GUI_TextToggleButton(this, colX[2], buttonY[0], 208, height, scaler_text, num_scalers, screen->get_scaler_index(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(scaler_win_button);
// scale
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[1], 0, 0, 0, "Scale:", gui->get_font());
	AddWidget(widget);
	const char *scale_win_text[10];
	scale_win_text[0] = "1"; scale_win_text[1] = "2"; scale_win_text[2] = "3";
	scale_win_text[3] = "4"; scale_win_text[4] = "5"; scale_win_text[5] = "6";
	scale_win_text[6] = "7"; scale_win_text[7] = "8";
	int scale = screen->get_scale_factor();
	char buff [4];
	itoa (scale, buff, 10); // write current scale to buff
// scale (fullscreen)
	const char *scale_text[10];
	int num_scale = 0;
	int scale_selection = 9;

	for (int i = 1; i < 9; i++) {
		if(SDL_VideoModeOK(scr_width * i, scr_height * i, bpp, SDL_FULLSCREEN)) {
			scale_text[num_scale] = scale_win_text[i - 1];
			if(i == scale)
				scale_selection = num_scale;
			num_scale++;
		}
	}
	if(scale_selection == 9) { // current scale is greater than 8 (or wasn't returned as okay)
		if(screen->is_fullscreen() || (scale > 8 && SDL_VideoModeOK(scr_width * scale, scr_height * scale, bpp, SDL_FULLSCREEN))) {
			scale_selection = num_scale;
			scaler_text[num_scale] = buff; // current scale
			num_scale++;
		} else if (num_scale > 0) {
			scale_selection = 0;
		} else {
			no_fullscreen = true;
		}
	}
	if(no_fullscreen) {
		scale_button = NULL;
		scaler_button->Delete();
		scaler_button = NULL;
	} else {
		scale_button = new GUI_TextToggleButton(this, colX[4], buttonY[1], yesno_width, height, scale_text, num_scale, scale_selection, font, BUTTON_TEXTALIGN_CENTER, this, 0);
		AddWidget(scale_button);
	}
// scale (windowed)
	int num_win_scale, scale_win_selection;
	if(scale < 9) {
		num_win_scale = 8;
		scale_win_selection = scale -1;
	} else {
		num_win_scale = 9;
		scale_win_selection = 8;
		scale_win_text[8] = buff;
	}
	scale_win_button = new GUI_TextToggleButton(this, colX[4], buttonY[1], yesno_width, height, scale_win_text, num_win_scale, scale_win_selection, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(scale_win_button);
// scale (only2x scale button for scalers that aren't point or interlaced)
	only2x_button = new GUI_Button(this, colX[3], buttonY[1], 70, height, "2x only", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(only2x_button);
#endif
// fullscreen_toggle
	fullscreen_button = new GUI_TextToggleButton(this, colX[4], buttonY[2], yesno_width, height, yesno_text, 2, screen->is_fullscreen(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(fullscreen_button);

	if(no_fullscreen && !screen->is_fullscreen()) {
		fullscreen_button->Hide();
	} else {
		widget = (GUI_Widget *) new GUI_Text(colX[0], textY[2], 0, 0, 0, "Fullscreen:", gui->get_font());
		AddWidget(widget);
	}
// show roofs
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[3], 0, 0, 0, "Show roofs:", gui->get_font());
	AddWidget(widget);
	roof_button = new GUI_TextToggleButton(this, colX[4], buttonY[3], yesno_width, height, yesno_text, 2, game->is_roof_mode(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(roof_button);
// needs restart text
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[5], 0, 0, 0, "The following require a restart:", gui->get_font());
	AddWidget(widget);
// lighting (needs reset)
	widget = (GUI_Widget *) new GUI_Text(colX[1], textY[6], 0, 0, 0, "Lighting mode:", gui->get_font());
	AddWidget(widget);
	const char* const lighting_text[] = { "none", "smooth", "original" };
	lighting_button = new GUI_TextToggleButton(this, colX[3], buttonY[6], 70, height, lighting_text, 3, screen->get_old_lighting_style(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(lighting_button);
// fullscreen map (needs reset)
	widget = (GUI_Widget *) new GUI_Text(colX[1], textY[7], 0, 0, 0, "Game map is entire screen:", gui->get_font());
	AddWidget(widget);
	fullscreen_map_button = new GUI_TextToggleButton(this, colX[4], buttonY[7], yesno_width, height, yesno_text, 2, game->is_new_style(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(fullscreen_map_button);
// dithering (needs reset)
	widget = (GUI_Widget *) new GUI_Text(colX[1], textY[8], 0, 0, 0, "Old video graphics:", gui->get_font());
	AddWidget(widget);
	const char* const dither_text[] = { "no", "CGA", "EGA" };
	dither_button = new GUI_TextToggleButton(this, colX[4], buttonY[8], yesno_width, height, dither_text, 3, game->get_dither()->get_mode(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(dither_button);
// cancel/save buttons
	cancel_button = new GUI_Button(this, 84, buttonY[9] + 6, 54, height, "Cancel", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(cancel_button);
	save_button = new GUI_Button(this, 155, buttonY[9] + 6, 40, height, "Save", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(save_button);

	rebuild_buttons(true);
	return true;
}

VideoDialog::~VideoDialog() {
}

void VideoDialog::rebuild_buttons(bool init) {
#if SCALER_AND_SCALE_CANNOT_BE_CHANGED
	return;
#endif
	int scaler;
	bool fullscreen;
	Screen *screen = Game::get_game()->get_screen();

	if(init) {
		scaler = screen->get_scaler_index();
		fullscreen = screen->is_fullscreen();
	} else {
		fullscreen = fullscreen_button->GetSelection();
		if(fullscreen)
			scaler = scaler_button->GetSelection();
		else
			scaler = scaler_win_button->GetSelection();
	}
// scaler buttons
	if(fullscreen) {
		if(scaler_button)
			scaler_button->Show();
		scaler_win_button->Hide();
	} else {
		if(scaler_button)
			scaler_button->Hide();
		scaler_win_button->Show();
	}
// scale buttons
	if(scaler > 1) {
		if(scale_button)
			scale_button->Hide();
		scale_win_button->Hide();
		only2x_button->Show();
	} else {
		only2x_button->Hide();
		if(fullscreen) {
			if(scale_button)
				scale_button->Show();
			scale_win_button->Hide();
		}
		else {
			if(scale_button)
				scale_button->Hide();
			scale_win_button->Show();
		}
	}
}

GUI_status VideoDialog::close_dialog() {
	Delete(); // mark dialog as deleted. it will be freed by the GUI object
	callback_object->callback(0, this, this);
	return GUI_YUM;
}

GUI_status VideoDialog::KeyDown(SDL_keysym key) {
	if(key.sym == SDLK_ESCAPE)
		return close_dialog();
	return GUI_PASS;
}

GUI_status VideoDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if(caller == (GUI_CallBack *)cancel_button) {
		return close_dialog();
	} else if(caller == (GUI_CallBack *)fullscreen_button) {
		rebuild_buttons(false);
	} else if(caller == (GUI_CallBack *)scaler_button) {
		if(scaler_button->GetSelection() > 1) {
			scale_button->Hide();
			only2x_button->Show();
		} else {
			scale_button->Show();
			only2x_button->Hide();
		}
	} else if(caller == (GUI_CallBack *)scaler_win_button) {
		if(scaler_win_button->GetSelection() > 1) {
			scale_win_button->Hide();
			only2x_button->Show();
		} else {
			scale_win_button->Show();
			only2x_button->Hide();
		}
	} else if(caller == (GUI_CallBack *)save_button) {
		Game *game = Game::get_game();
		Screen *screen = Game::get_game()->get_screen();
		Configuration *config = Game::get_game()->get_config();
		bool fullscreen = fullscreen_button->GetSelection();
#if SCALER_AND_SCALE_CANNOT_BE_CHANGED
		if(fullscreen != screen->is_fullscreen())
			screen->toggle_fullscreen();
#else
	// scaler
		int scaler;
		if(fullscreen)
			scaler = scaler_button->GetSelection();
		else
			scaler = scaler_win_button->GetSelection();
		config->set("config/video/scale_method", screen->get_scaler_reg()->GetNameForIndex(scaler));
	// scale
		int scale;
		if(fullscreen)
			scale = scale_button->GetSelection() + 1;
		else
			scale = scale_win_button->GetSelection() + 1;
		config->set("config/video/scale_factor", scale);
#endif
	// fullscreen
		config->set("config/video/fullscreen", fullscreen ? "yes" : "no");
	// roof mode
		bool roof_mode = roof_button->GetSelection();
		game->set_roof_mode(roof_mode);
		game->get_map_window()->set_roof_mode(roof_mode);
		game->get_game_map()->set_roof_mode(roof_mode);
		config->set(config_get_game_key(config) + "/roof_mode", roof_mode ? "yes" : "no");
	// lighting
		const char *lighting_char;
		int lighting = lighting_button->GetSelection();
		if(lighting == 0)
			lighting_char = "none";
		else if(lighting == 1)
			lighting_char = "smooth";
		else
			lighting_char = "original";
		config->set("config/general/lighting", lighting_char);
	// fullscreen_map
		config->set("config/general/fullscreen_map", fullscreen_map_button->GetSelection() ? "yes" : "no");
	// dither
		const char *dither_char;
		uint8 dither = dither_button->GetSelection();
		if(dither == 0)
			dither_char = "none";
		else if(dither == 1)
			dither_char = "cga";
		else
			dither_char = "ega";
		config->set("config/general/dither_mode", dither_char);

		config->write();
		close_dialog();
		return GUI_YUM;
	}

	return GUI_PASS;
}