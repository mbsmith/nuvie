/*
    GUILIB:  An example GUI framework library for use with SDL
    Copyright (C) 1997  Sam Lantinga

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    5635-34 Springhouse Dr.
    Pleasanton, CA 94588 (USA)
    slouken@devolution.com
*/

/* Base class for all widgets -- the GUI operates on this class */

#ifndef _GUI_widget_h
#define _GUI_widget_h

#include <list>
#include <stdio.h>
#include <stdarg.h>

#include "SDL.h"
#include "GUI_status.h"
#include "GUI_DragArea.h"
#include "GUI_DragManager.h"
#include "Screen.h"

typedef GUI_status (*GUI_CallbackProc)(void *data);

class GUI_Widget : public GUI_DragArea {

protected:
	/* A generic pointer to user-specified data for the widget.
	 */
	void *widget_data;

	Screen *screen;
	/* The display surface for the widget */
	SDL_Surface *surface;

	int offset_x, offset_y; /* original offsets to parent */

	/* Flag -- whether or not the widget should be freed */
	int status;

	/* should we redraw this widget */
	bool update_display;

	/* the button states for theoretically 3 buttons */
	int pressed[3];

	bool focused;

	std::list<GUI_Widget *>children;
	GUI_Widget *parent;

 	char *error;
	char  errbuf[BUFSIZ];

	GUI_DragManager *gui_drag_manager;

	// SB-X
	/* The time of the last mouse click (SDL_GetTicks()). */
	unsigned int mouseup[3]; /* for 3 buttons */
	unsigned int mousedown[3]; /* waiting for MouseUp */
	bool accept_mouseclick[3]; /* which buttons can be [double]clicked */
	unsigned int delayed_button; /* a MouseClick can be delayed on one button; 0=none */
	unsigned int held_button; /* a MouseDown can be delayed on one button; 0=none */
	bool mouse_moved; /* true if mouse moves while button is pressed */

	bool mouse_over; // initialized here; toggled by GUI

public:
	/* The area covered by the widget */
	SDL_Rect area;

	GUI_Widget(void *data);
	GUI_Widget(void *data, int x, int y, int w, int h);
	virtual ~GUI_Widget();

	int AddWidget(GUI_Widget *widget);

	/* Mark the widget as visible -- this is the default state */
	virtual void Show(void);

	/* Mark the widget as hidden;  no display, no events */
	virtual void Hide(void);

	/* Mark the widget as free, so it will be deleted by the GUI */
	virtual void Delete(void);

	virtual void MoveRelative(int dx,int dy);
	virtual void Move(int new_x, int new_y);
    void MoveRelativeToParent(int dx, int dy);
    bool has_focus() { return focused; }
	void grab_focus();
	virtual void release_focus();
	void moveToFront();
	virtual void PlaceOnScreen(Screen *s, GUI_DragManager *dm, int x, int y);

	virtual int  Status(void);	/* Reports status to GUI */

	/* Set the bounds of the widget.
	   If 'w' or 'h' is -1, that parameter will not be changed.
	 */
	virtual void SetRect(int x, int y, int w, int h);
	virtual void SetRect(SDL_Rect **bounds);

	/* Return the whole area */
	virtual SDL_Rect GetRect()
	  {return area;}

	/* Return the bounds of the widget */
	virtual int X() { return area.x; }
	virtual int Y() { return area.y; }
	virtual int W() { return area.w; }
	virtual int H() { return area.h; }

	/* Check to see if a point intersects the bounds of the widget.
	 */
	virtual int HitRect(int x, int y);
	virtual int HitRect(int x, int y, SDL_Rect &rect);

	/* Set the display surface for this widget */
	virtual void SetDisplay(Screen *s);

	/* Show the widget.
	   If the surface needs to be locked, it will be locked
	   before this call, and unlocked after it returns.
	 */
	virtual void Display(bool full_redraw=false);
	void DisplayChildren(bool full_redraw=false);

	/* Redraw the widget and only the widget */
	virtual void Redraw(void);

	/* should this widget be redrawn */
	inline bool needs_redraw() { return update_display; }
	/* widget has focus or no widget is focused */
	bool widget_has_focus(); // SB-X

	/* GUI idle function -- run when no events pending */
	virtual GUI_status Idle(void);

	/* Widget event handlers.
	   These functions should return a status telling the GUI whether
	   or not the event should be passed on to other widgets.
	   These are called by the default HandleEvent function.
	*/
	virtual GUI_status KeyDown(SDL_Keysym key);
	virtual GUI_status KeyUp(SDL_Keysym key);
	virtual GUI_status MouseDown(int x, int y, int button);
	virtual GUI_status MouseUp(int x, int y, int button);
	virtual GUI_status MouseMotion(int x, int y, Uint8 state);
	virtual GUI_status MouseWheel(sint32 x, sint32 y);
	// <SB-X>
	virtual GUI_status MouseEnter(Uint8 state);
	virtual GUI_status MouseLeave(Uint8 state);
	virtual GUI_status MouseClick(int x, int y, int button);
	virtual GUI_status MouseDouble(int x, int y, int button);
	virtual GUI_status MouseDelayed(int x, int y, int button);
	virtual GUI_status MouseHeld(int x, int y, int button);
	// </SB-X>

	bool drag_accept_drop(int x, int y, int message, void *data);
	void drag_perform_drop(int x, int y, int message, void *data);

	/* Main event handler function.
	   This function gets raw SDL events from the GUI.
	 */
	virtual GUI_status HandleEvent(const SDL_Event *event);

	/* Returns NULL if everything is okay, or an error message if not */
	char *Error(void) {
		return(error);
	}

	/* yields click state: none, pressed, intermediate */
	inline virtual int ClickState(int button)
	{ return pressed[button-1]; }

	/* set click state from remote */
	inline virtual void SetClickState(int button, int value)
	{ if ((button>0) && (button<=3)) pressed[button-1]=value; }

protected:
	/* The constructor, separated out for both access constructors */
	void Init(void *data, int x, int y, int w, int h);

	void setParent(GUI_Widget *widget);

	/* Useful for getting error feedback */
	void SetError(char *fmt, ...) {
		va_list ap;

		va_start(ap, fmt);
		vsprintf(errbuf, fmt, ap);
		va_end(ap);
		error = errbuf;
	}

	// SB-X
	void set_accept_mouseclick(bool set, int button = 0);
	void set_mouseup(int set, int button = 0);
	void set_mousedown(int set, int button = 0);
	int get_mouseup(int button)  { if(button > 0 && button < 4) return(mouseup[button-1]); else return(0); }
	int get_mousedown(int button){ if(button > 0 && button < 4) return(mousedown[button-1]); else return(0); }
	void wait_for_mouseclick(int button){ if(button >= 0 && button < 4) delayed_button = button; }
	void wait_for_mousedown(int button) { if(button >= 0 && button < 4) held_button = button; }
	virtual GUI_status try_mouse_delayed();
};

#endif /* _GUI_widget_h */
