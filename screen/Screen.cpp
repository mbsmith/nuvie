/*
 *  Screen.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sun Mar 09 2003.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cmath>
#include <SDL.h>

#include "nuvieDefs.h"
#include "Configuration.h"

#include "Surface.h"
#include "Scale.h"
#include "Screen.h"
#include "MapWindow.h"

#define sqr(a) ((a)*(a))

//Ultima 6 light globe sizes.

static const uint8 globeradius[]   = { 32, 64, 128 };
static const uint8 globeradius_2[] = { 16, 32,  64 };

Screen::Screen(Configuration *cfg)
{
 std::string str_lighting_style;
 config = cfg;
 
 update_rects = NULL;
 shading_data = NULL;
 updatingalphamap = true;
 std::string x;
 config->value( "config/general/lighting", str_lighting_style );

 if( str_lighting_style == "none" )
	 lighting_style = 0;
 else if( str_lighting_style == "smooth" )
	 lighting_style = 1;
 else
	 lighting_style = 2;
 
 max_update_rects = 10;
 num_update_rects = 0;
 memset( shading_globe, 0, sizeof(uint8*) * 3 );
}

Screen::~Screen()
{
 delete surface;
 if (update_rects) free(update_rects);
 if (shading_data) free(shading_data);

 for( int i = 0; i < 3; i++ )
   {
    if(shading_globe[i])
       free(shading_globe[i]);
   }

 SDL_Quit();
}

bool Screen::init(uint16 new_width, uint16 new_height)
{
 std::string str;
 
 width = new_width;
 height = new_height;
 
  	/* Initialize the SDL library */
	if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n",
			SDL_GetError());
   return false;
	}

 update_rects = (SDL_Rect *)malloc(sizeof(SDL_Rect) * max_update_rects);
 if(update_rects == NULL)
   return false;

 config->value("config/video/scale_method", str, "---");
 scaler_index = scaler_reg.GetIndexForName(str);
 if(scaler_index == -1)
   { 
		//config.set("config/video/scale_method","SuperEagle",true);
		scaler_index = scaler_reg.GetIndexForName("SuperEagle");
	 }

config->value("config/video/scale_factor", scale_factor, 1);

fullscreen = false;

 //scaled_surface = SDL_SetVideoMode(width, height, 8, SDL_SWSURFACE | SDL_HWPALETTE);
 //pitch = scaled_surface->pitch;
 //bpp = scaled_surface->format->BitsPerPixel;
 
 //printf("surface pitch = %d\n",pitch);
 
// memcpy(palette,0,768);
 
 set_screen_mode();

 return true;
}

bool Screen::set_palette(uint8 *p)
{
 if(surface == NULL || p == NULL)
   return false;

 //SDL_SetColors(scaled_surface,palette,0,256);
 for (int i = 0; i < 256; ++i)
   {
		uint32  r = p[i*3];
		uint32  g = p[i*3+1];
		uint32  b = p[i*3+2];

		uint32	c= ((r>>RenderSurface::Rloss)<<RenderSurface::Rshift) | ((g>>RenderSurface::Gloss)<<RenderSurface::Gshift) | ((b>>RenderSurface::Bloss)<<RenderSurface::Bshift);

		surface->colour32[i] = c;
	 }
  
 return true;
}

bool Screen::rotate_palette(uint8 pos, uint8 length)
{
 uint32 tmp_colour;
 uint8 i;
 
 tmp_colour = surface->colour32[pos+length-1];
 
 for(i= length-1;i > 0; i-- )
    surface->colour32[pos + i] = surface->colour32[pos + i - 1];

 surface->colour32[pos] = tmp_colour;
   
 return true;
}

bool Screen::clear(uint16 x, uint16 y, sint16 w, sint16 h,SDL_Rect *clip_rect)
{
 uint8 *pixels;
 uint16 i;
 uint16 x1,y1;
  
 pixels = (uint8 *)surface->pixels;
 
 if(clip_rect)
   {
    x1 = x;
    y1 = y;
    if(x < clip_rect->x)
      x = clip_rect->x;
      
    if(y < clip_rect->y)
      y = clip_rect->y;
      
    if(x1 + w > clip_rect->x + clip_rect->w)
      {
       w -= (x1 + w) - (clip_rect->x + clip_rect->w);
       if(w <= 0)
         return false;
      }

    if(y1 + h > clip_rect->y + clip_rect->h)
      {
       h -= (y1 + h) - (clip_rect->y + clip_rect->h);
       if(h <= 0)
         return false;
      }
   }
   
 pixels += y * surface->pitch + (x * surface->bytes_per_pixel);

 for(i=0;i<h;i++)
    {
     memset(pixels,0,w*surface->bytes_per_pixel);
     pixels += surface->pitch;
    }

 return true; 
}

bool Screen::fill(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h)
{
 if(surface->bits_per_pixel == 16)
    return fill16(colour_num, x, y, w, h);

 return fill32(colour_num, x, y, w, h); 
}

bool Screen::fill16(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h)
{
 uint16 *pixels;
 uint16 i,j;
  
 pixels = (uint16 *)surface->pixels;

 pixels += y * surface->w + x;

 for(i=0;i<h;i++)
    {
     for(j=0;j<w;j++)
        pixels[j] = (uint16)surface->colour32[colour_num];
        
     pixels += surface->w;
    }

 return true; 
}

bool Screen::fill32(uint8 colour_num, uint16 x, uint16 y, sint16 w, sint16 h)
{
 uint32 *pixels;
 uint16 i,j;

  
 pixels = (uint32 *)surface->pixels;

 pixels += y * surface->w + x;

 for(i=0;i<h;i++)
    {
     for(j=0;j<w;j++)
        pixels[j] = surface->colour32[colour_num];
        
     pixels += surface->w;
    }

 return true; 
}

void *Screen::get_pixels()
{
 //if(scaled_surface == NULL)
 //   return NULL; 
 
 //return scaled_surface->pixels;
 return NULL;
}
   
SDL_Surface *Screen::get_sdl_surface()
{
 if(surface)
   return surface->get_sdl_surface();

 return NULL;
}

bool Screen::blit(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans, SDL_Rect *clip_rect)
{
 uint16 src_x = 0;
 uint16 src_y = 0;
 
 if(clip_rect)
  {
   if(clip_rect->x > dest_x)
      {
       src_x = clip_rect->x - dest_x;
       if(src_x > dest_x + src_w)
         return false;
       src_w -= clip_rect->x - dest_x;
       dest_x = clip_rect->x;
      }
      
   if(clip_rect->y > dest_y)
     {
      src_y = clip_rect->y - dest_y;
      if(src_y > dest_y + src_h)
         return false;
      src_h -= clip_rect->y - dest_y;
      dest_y = clip_rect->y;
     }
     
   if(dest_x + src_w > clip_rect->x + clip_rect->w)
     {
      if(clip_rect->x + clip_rect->w - dest_x <= 0)
        return false;
      
      src_w = clip_rect->x + clip_rect->w - dest_x;
     }

   if(dest_y + src_h > clip_rect->y + clip_rect->h)
     {
      if(clip_rect->y + clip_rect->h - dest_y <= 0)
        return false;
      
      src_h = clip_rect->y + clip_rect->h - dest_y;
     }
     
   src_buf += src_y * src_pitch + src_x;
  }

 if(surface->bits_per_pixel == 16)
   return blit16(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans);

 return blit32(dest_x, dest_y, src_buf, src_bpp, src_w, src_h, src_pitch, trans);
}

inline bool Screen::blit16(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans)
{
 uint16 *pixels;
 uint16 i,j;
  
 pixels = (uint16 *)surface->pixels;
 
 pixels += dest_y * surface->w + dest_x;

 if(trans)
  {
   for(i=0;i<src_h;i++)
     {
      for(j=0;j<src_w;j++)
        {
         if(src_buf[j] != 0xff)
           pixels[j] = (uint16)surface->colour32[src_buf[j]];
        }
      src_buf += src_pitch;
      pixels += surface->w; //pitch;
     }
  }
 else
  {
   for(i=0;i<src_h;i++)
     {
      for(j=0;j<src_w;j++)
        {
         pixels[j] = (uint16)surface->colour32[src_buf[j]];
        }
      src_buf += src_pitch;
      pixels += surface->w; //surface->pitch;
     }
  }

 return true;
}

bool Screen::blit32(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch, bool trans)
{
 uint32 *pixels;
 uint16 i,j;
  
 pixels = (uint32 *)surface->pixels;
 
 pixels += dest_y * surface->w + dest_x;

      
 if(trans)
  {
   for(i=0;i<src_h;i++)
     {
      for(j=0;j<src_w;j++)
        {
         if(src_buf[j] != 0xff)
           pixels[j] = surface->colour32[src_buf[j]];
        }
      src_buf += src_pitch;
      pixels += surface->w; //pitch;
     }
  }
 else
  {
   for(i=0;i<src_h;i++)
     {
      for(j=0;j<src_w;j++)
        {
         pixels[j] = surface->colour32[src_buf[j]];
        }
      src_buf += src_pitch;
      pixels += surface->w; //surface->pitch;
     }
  }

 return true;
}

void Screen::blitbitmap(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color)
{
 if(surface->bits_per_pixel == 16)
   blitbitmap16(dest_x, dest_y, src_buf, src_w, src_h, fg_color, bg_color);
 else
   blitbitmap32(dest_x, dest_y, src_buf, src_w, src_h, fg_color, bg_color);

 return;
}

void Screen::blitbitmap16(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color)
{
 uint16 *pixels;
 uint16 i,j;
  
 pixels = (uint16 *)surface->pixels;
 
 pixels += dest_y * surface->w + dest_x;

 for(i=0;i<src_h;i++)
  {
   for(j=0;j<src_w;j++)
    {
     if(src_buf[j])
       pixels[j] = (uint16)surface->colour32[fg_color];
     else
       pixels[j] = (uint16)surface->colour32[bg_color];
    }
   src_buf += src_w;
   pixels += surface->w; //surface->pitch;
  }

 return;
}

void Screen::blitbitmap32(uint16 dest_x, uint16 dest_y, unsigned char *src_buf, uint16 src_w, uint16 src_h, uint8 fg_color, uint8 bg_color)
{
 uint32 *pixels;
 uint16 i,j;
  
 pixels = (uint32 *)surface->pixels;
 
 pixels += dest_y * surface->w + dest_x;

 for(i=0;i<src_h;i++)
  {
   for(j=0;j<src_w;j++)
    {
     if(src_buf[j])
       pixels[j] = surface->colour32[fg_color];
     else
       pixels[j] = surface->colour32[bg_color];
    }
   src_buf += src_w;
   pixels += surface->w; //surface->pitch;
  }

 return;
}

//4 is pure-light
//0 is pitch-black
//Globe of r 1 is just a single tile of 2

static const char TileGlobe[][11*11] = 
{
{
	1,1,1,
	1,2,1,
	1,1,1
},
{
    0,0,0,0,0,
    0,1,2,1,0,
    0,2,3,2,0,
    0,1,2,1,0,
    0,0,0,0,0
},
{
    0,0,1,1,1,0,0,
    0,1,2,2,2,1,0,
    1,2,3,3,3,2,1,
    1,2,3,4,3,2,1,
    1,2,3,3,3,2,1,
    0,1,2,2,2,1,0,
    0,0,1,1,1,0,0
},
{
    0,0,0,1,1,1,0,0,0,
    0,0,1,2,2,2,1,0,0,
    0,1,2,3,3,3,2,1,0,
    1,2,3,3,4,3,3,2,1,
    1,2,3,4,4,4,3,2,1,
    1,2,3,3,4,3,3,2,1,
    0,1,2,3,3,3,2,1,0,
    0,0,1,2,2,2,1,0,0,
    0,0,0,1,1,1,0,0,0
},
{
    0,0,1,1,2,2,2,1,1,0,0,
    0,1,1,2,3,3,2,2,1,1,0,
    1,1,2,3,3,3,3,2,2,1,1,
    1,2,3,3,4,4,4,3,2,2,1,
    2,2,3,4,4,4,4,4,3,2,2,
    2,2,3,4,4,4,4,4,3,2,2,
    2,2,3,4,4,4,4,4,3,2,2,
    1,2,2,3,4,4,4,3,2,2,1,
    1,1,2,2,3,3,3,2,2,1,1,
    0,1,1,2,2,2,2,2,1,1,0,
    0,0,1,1,2,2,2,1,1,0,0,
},
{
    1,1,2,2,2,2,2,2,2,1,1,
    1,2,2,3,3,3,3,3,2,2,1,
    2,2,3,3,4,4,4,3,3,2,2,
    2,3,3,4,4,4,4,4,3,3,2,
    2,3,4,4,4,4,4,4,4,3,2,
    2,3,4,4,4,4,4,4,4,3,2,
    2,3,4,4,4,4,4,4,4,3,2,
	2,3,3,4,4,4,4,4,3,3,2,
	2,2,3,3,4,4,4,3,3,2,2,
	1,2,2,3,3,3,3,3,2,2,1,
    1,1,2,2,2,2,2,2,2,1,1
}
};

void Screen::clearalphamap8( uint16 x, uint16 y, uint16 w, uint16 h, uint8 opacity )
{
	switch( lighting_style )
	{
	case 0:
		return;
	default:
	case 1:
		shading_ambient = opacity;
		break;
	case 2:
		if( opacity < 0xFF )
            shading_ambient = 0;
        else
            shading_ambient = 0xFF;
		break;
	}

    if( shading_data == NULL )
    {
        shading_rect.x = x;
        shading_rect.y = y;
        shading_rect.w = w;
        shading_rect.h = h;
        shading_data = (unsigned char*)malloc(sizeof(char)*shading_rect.w*shading_rect.h);
        if( shading_data == NULL )
        {
            /* We couldn't allocate memory for the opacity map, so just disable lighting */
            shading_ambient = 0xFF;
            return;
        }
        buildalphamap8();
    }
    if( shading_ambient == 0xFF )
    {
    }
    else
    {
        memset( shading_data, shading_ambient, sizeof(char)*shading_rect.w*shading_rect.h );
    }
    updatingalphamap = true;

    //Light globe around the avatar
    if( lighting_style == 2 )
        drawalphamap8globe( 5, 5, opacity/64+3 ); //range (0..3)+3 or (3..6)
    else if( lighting_style != 0 )
        drawalphamap8globe( 5, 5, 3 );
}

void Screen::buildalphamap8()
{
    //Build three globes for 3 intensities
    //16x16, 32x32, 64x64
    shading_globe[0] = (uint8*)malloc(sqr(globeradius[0]));
    shading_globe[1] = (uint8*)malloc(sqr(globeradius[1]));
    shading_globe[2] = (uint8*)malloc(sqr(globeradius[2]));

    for( int i = 0; i < 3; i++ )
        for( int y = 0; y < globeradius[i]; y++ )
            for( int x = 0; x < globeradius[i]; x++ )
            {
                float r;
                //Distance from center
                r  = sqrt( sqr((y-globeradius_2[i]))+sqr((x-globeradius_2[i])) );
                //Unitize
                r /= sqrt( sqr(globeradius_2[i])+sqr(globeradius_2[i]) );
                //Calculate brightness
                r  = (float)exp(-(10*r*r));
                //Fit into a byte
                r *= 255;
                //Place it
                shading_globe[i][y*globeradius[i]+x] = (uint8)r;
            }

    //Get the three shading tiles (for original-style dithered lighting)
    Game *game = Game::get_game();
    int game_type;
    config->value("config/GameType",game_type);
 
    if(game_type == NUVIE_GAME_U6)
    {
        shading_tile[0] = game->get_map_window()->get_tile_manager()->get_tile(444)->data;
        shading_tile[1] = game->get_map_window()->get_tile_manager()->get_tile(445)->data;
        shading_tile[2] = game->get_map_window()->get_tile_manager()->get_tile(446)->data;
        shading_tile[3] = game->get_map_window()->get_tile_manager()->get_tile(447)->data;
    }
    else
    {
        shading_tile[0] = game->get_map_window()->get_tile_manager()->get_tile(268)->data;
        shading_tile[1] = game->get_map_window()->get_tile_manager()->get_tile(269)->data;
        shading_tile[2] = game->get_map_window()->get_tile_manager()->get_tile(270)->data;
        shading_tile[3] = game->get_map_window()->get_tile_manager()->get_tile(271)->data;
    }
}


void Screen::drawalphamap8globe( sint16 x, sint16 y, uint16 r )
{
    sint16 i,j, globe;
    if( shading_ambient == 0xFF )
        return;
	if( lighting_style == 0 )
		return;
    if( lighting_style == 2 )
    {        
		globe = r+1;
		if( r > 5 )
			r = 5;
		for( j = 0; j <= r*2; j++ )
			for( i = 0; i <= r*2; i++ )
			{
				if( x + i - r < 0 || x + i - r >= 11 )
					continue;
				if( y + j - r < 0 || y + j - r >= 11  )
					continue;
				shading_data[(y+j-r)*11+(x+i-r)] = MIN( shading_data[(y+j-r)*11+(x+i-r)] + TileGlobe[globe-2][j*(r*2+1)+i], 4 );
			}
        return;
    }
    r--;
    x = x*16+8;
    y = y*16+8;
    //The x and y are relative to (0,0) of the screen itself, and are absolute coordinates, so are i and j
    for(i=-globeradius_2[r];i<globeradius_2[r];i++)
        for(j=-globeradius_2[r];j<globeradius_2[r];j++)
        {
            if( (y-shading_rect.y+i)-1 < 0 ||
                (x-shading_rect.x+j)-1 < 0 ||
                (y-shading_rect.y+i)+1 > shading_rect.h ||
                (x-shading_rect.x+j)+1 > shading_rect.w )
                continue;
            shading_data[(y-shading_rect.y+i)*shading_rect.w+(x-shading_rect.x+j)] = MIN( shading_data[(y-shading_rect.y+i)*shading_rect.w+(x-shading_rect.x+j)] + shading_globe[r][(i+globeradius_2[r])*globeradius[r]+(j+globeradius_2[r])], 255 );
        }
}


void Screen::blitalphamap8()
{
    //pixel = (dst*(1-alpha))+(src*alpha)   for an interpolation
    //pixel = pixel * alpha                 for a reduction
    //We use a reduction here

    if( shading_ambient == 0xFF )
        return;
	if( lighting_style == 0 )
		return;

    uint16 i,j;
    Game *game = Game::get_game();
    updatingalphamap = false;

    if( lighting_style == 2 )
    {
        for( j = 0; j < 11; j++ )
        {
            for( i = 0; i < 11; i++ )
            {
                if( shading_data[j*11+i] < 4 )
                    blit(i*16,j*16,shading_tile[shading_data[j*11+i]],8,16,16,16,true,game->get_map_window()->get_clip_rect());
            }
        }
        return;
    }

    switch( surface->bits_per_pixel )
    {
    case 16:
        uint16 *pixels16;
        pixels16 = (uint16 *)surface->pixels;

        pixels16 += shading_rect.y*surface->w;

        for(i=shading_rect.y;i<shading_rect.h+shading_rect.y;i++)
        {
            for(j=shading_rect.x;j<shading_rect.w+shading_rect.x;j++)
            {
                pixels16[j] = ( ( (unsigned char)(( (float)(( pixels16[j] & 0xF800 ) >> 11)) * (float)(shading_data[(j-shading_rect.x)+(i-shading_rect.y)*shading_rect.w])/255.0f) ) << 11) | //R
                              ( ( (unsigned char)(( (float)(( pixels16[j] & 0x07E0 ) >> 5 )) * (float)(shading_data[(j-shading_rect.x)+(i-shading_rect.y)*shading_rect.w])/255.0f) ) << 5 ) | //G
                              ( ( (unsigned char)(( (float)(( pixels16[j] & 0x001F ) >> 0 )) * (float)(shading_data[(j-shading_rect.x)+(i-shading_rect.y)*shading_rect.w])/255.0f) ) << 0 );  //B
            
                //Red = 0xF800 = 1111 1000 0000 0000
                //Grn = 0x07E0 = 0000 0111 1110 0000
                //Blu = 0x001F = 0000 0000 0001 1111
            }
            pixels16 += surface->w;
        }
        return;
        break;
    case 24:
    case 32:
        uint32 *pixels;
        pixels = (uint32 *)surface->pixels;

        pixels += shading_rect.y*surface->w;

        for(i=shading_rect.y;i<shading_rect.h+shading_rect.y;i++)
        {
            for(j=shading_rect.x;j<shading_rect.w+shading_rect.x;j++)
            {
                pixels[j] = ( ( (unsigned char)(( (float)(( pixels[j] & 0xFF0000 ) >> 16)) * (float)(shading_data[(j-shading_rect.x)+(i-shading_rect.y)*shading_rect.w])/255.0f) ) << 16) | //R
                            ( ( (unsigned char)(( (float)(( pixels[j] & 0x00FF00 ) >> 8 )) * (float)(shading_data[(j-shading_rect.x)+(i-shading_rect.y)*shading_rect.w])/255.0f) ) << 8 ) | //G
                            ( ( (unsigned char)(( (float)(( pixels[j] & 0x0000FF ) >> 0 )) * (float)(shading_data[(j-shading_rect.x)+(i-shading_rect.y)*shading_rect.w])/255.0f) ) << 0 );  //B
            }
            pixels += surface->w;
        }
        return;
        break;
    default:
        std::cout << "Screen::blitalphamap8() cannot handle your screen surface depth of " << surface->bits_per_pixel << std::endl;
        break;
        return;
    }    

}

SDL_Surface *Screen::create_sdl_surface_from(unsigned char *src_buf, uint16 src_bpp, uint16 src_w, uint16 src_h, uint16 src_pitch)
{
 SDL_Surface *new_surface;
 uint16 i,j;
 
 new_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, src_w, src_h, surface->bits_per_pixel,
                                    surface->Rmask, surface->Gmask, surface->Bmask, 0);

 if(surface->bits_per_pixel == 16)
   {
    uint16 *pixels = (uint16 *)new_surface->pixels;

    for(i=0;i<src_h;i++)
      {
       for(j=0;j<src_w;j++)
         {
          pixels[j] = (uint16)surface->colour32[src_buf[j]];
         }
       src_buf += src_pitch;
       pixels += src_pitch; //surface->pitch;
      }
   }
 else
   {
    uint32 *pixels = (uint32 *)new_surface->pixels;

    for(i=0;i<src_h;i++)
      {
       for(j=0;j<src_w;j++)
         {
          pixels[j] = surface->colour32[src_buf[j]];
         }
       src_buf += src_pitch;
       pixels += src_pitch; //surface->pitch;
      }
   }

 return new_surface;

}

uint16 Screen::get_pitch()
{
 return (uint16)surface->pitch;
}

uint16 Screen::get_bpp()
{
 return surface->bits_per_pixel;
}

void Screen::update()
{
 //if(scaled_surface)
 if(scaler)
  {
   scaler->Scale(surface->format_type, surface->pixels,		// type, source
                 0, 0, surface->w, surface->h,							// x, y, w, h
				         surface->pitch/surface->bytes_per_pixel, surface->h,	// pixels/line, pixels/col
				         sdl_surface->pixels,									// dest
				         sdl_surface->pitch/sdl_surface->format->BytesPerPixel,	// destpixels/line
                 scale_factor); 
  }
 else
  {
   uint8 *src = (uint8 *)surface->pixels;
   uint8 *dest = (uint8 *)sdl_surface->pixels;
   
   memcpy(dest,src,surface->w*surface->bytes_per_pixel);
   dest += sdl_surface->pitch;
   src += surface->pitch;
  }
  
SDL_UpdateRect(sdl_surface,0,0,0,0);

 return;
}

void Screen::update(uint16 x, uint16 y, uint16 w, uint16 h)
{
 //if(scaled_surface)
 if(x >= 2)
   x -= 2;
 if(y >= 2)
   y -= 2;
 if(w <= surface->w-2)
   w += 2;
 if(h <= surface->h-2)
   h += 2;

 if(scaler)
  {   
   scaler->Scale(surface->format_type, surface->pixels,		// type, source
                 x, y, w, h,							// x, y, w, h
                 surface->pitch/surface->bytes_per_pixel, surface->h,	// pixels/line, pixels/col
                 sdl_surface->pixels,									// dest
                 sdl_surface->pitch/sdl_surface->format->BytesPerPixel,	// destpixels/line
                 scale_factor); 
  }
 else
  {
   uint8 *src = (uint8 *)surface->pixels;
   uint8 *dest = (uint8 *)sdl_surface->pixels;
   src += y * surface->pitch + (surface->bytes_per_pixel * x);
   dest += y * sdl_surface->pitch + (sdl_surface->format->BytesPerPixel * x);
    
   memcpy(dest,src,w*surface->bytes_per_pixel);
   dest += sdl_surface->pitch;
   src += surface->pitch;
  }
   
 if(num_update_rects == max_update_rects)
   {
    update_rects = (SDL_Rect *)realloc(update_rects,sizeof(SDL_Rect) * (max_update_rects + 10));
    max_update_rects += 10;
   }
  
 update_rects[num_update_rects].x = x*scale_factor;
 update_rects[num_update_rects].y = y*scale_factor;
 update_rects[num_update_rects].w = w*scale_factor;
 update_rects[num_update_rects].h = h*scale_factor;
 
 num_update_rects++;
 
 //SDL_UpdateRect(sdl_surface,x*scale_factor,y*scale_factor,w*scale_factor,h*scale_factor);

 return;
}

void Screen::preformUpdate()
{
 SDL_UpdateRects(sdl_surface,num_update_rects,update_rects);
 num_update_rects = 0;
}

void Screen::lock()
{
// SDL_LockSurface(scaled_surface);
 
 return;
}

void Screen::unlock()
{
 //SDL_UnlockSurface(scaled_surface);
 
 return;
}

bool Screen::initScaler()
{
 std::string scaler_name;
 
 return true;
}


void Screen::set_screen_mode()
{
	// Get info. about video.
	const SDL_VideoInfo *vinfo = SDL_GetVideoInfo();
	uint32 flags = 0;

	if (scale_factor == 0) scale_factor = 1;
	scaler = 0;

	// Get BPP
	int bpp = vinfo->vfmt->BitsPerPixel;

	// If we can't use the format, force 16 bit
	if (bpp != 16 && bpp != 32)
		bpp = 16;

  
	std::cout << "Attempting to set vid mode: " << width << "x" << height << "x" << bpp << "x" << scale_factor;

	// Is Fullscreen?
	if (fullscreen) {
		flags |= SDL_FULLSCREEN;
		std::cout << " Fullscreen";
	}
	else
		flags |= SDL_RESIZABLE;

	// Opengl Stuff 
#ifdef WANT_OPENGL
	if (useOpengl) {
		std::cout << " OpenGL" << std::endl;

		// Want double-buffering.
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 16 );

		//SDL_GL_LoadLibrary("opengl32.dll");

		// Allocate surface: 640x480x32bpp.
		sdl_surf = SDL_SetVideoMode(width*scale_factor, height*scale_factor, bpp, flags|SDL_OPENGL|SDL_GL_DOUBLEBUFFER|SDL_HWSURFACE);
		if (sdl_surf) {
			opengl = new OpenGL(width, height, palette);
			surface = CreateRenderSurface(opengl);
			surface->w = width;
			surface->h = height;

			mouse->init_opengl(screen);
			ShapeManager::get().init_opengl(screen);

			std::cout << opengl->get_texture_mem_used() << " bytes of textures loaded so far" << std::endl;

			// Hide system mouse cursor   
			SDL_ShowCursor(0); 

			return;
		}

		std::cout <<  "Setting up OpenGL Failed. Trying" << std::endl;
	}
#endif //WANT_OPENGL

	if (vinfo->hw_available && doubleBuffer && fullscreen) {
		flags |= SDL_HWSURFACE|SDL_DOUBLEBUF;
		std::cout << " Hardware Double Buffered" << std::endl;
	}
	else {
		flags |= SDL_SWSURFACE;
		std::cout << " Software Surface" << std::endl;
	}

	// Old Software rendering. Try a scaler_index first, 
	if (!try_scaler(width, height, flags, bpp)) {

		scale_factor = 1;
		scaler = 0;
		sdl_surface = SDL_SetVideoMode(width, height, bpp, flags);

		// Couldn't create it, so disable double buffering
		if (!sdl_surface) {
			flags &= ~(SDL_HWSURFACE|SDL_DOUBLEBUF);
			flags |= SDL_SWSURFACE;
			scale_factor = 1;
			scaler = 0;
			sdl_surface = SDL_SetVideoMode(width, height, bpp, flags);
		}

		if (!sdl_surface) {
			std::cerr << "Unable to create surface. Attempting 320x200x1 Software Surface" << std::endl;
			width = 320;
			height = 200;
			sdl_surface = SDL_SetVideoMode(width, height, bpp, flags);
			if (!sdl_surface) {
				std::cerr << "Unable to create surface. Exiting" << std::endl;
				exit (-1);
			}
		}

		if (sdl_surface->flags & SDL_HWSURFACE) {
			std::cout << "Created Double Buffered Surface" << std::endl;
			surface = CreateRenderSurface (width, height, bpp);
		}
		else {
			surface = CreateRenderSurface(sdl_surface);
		}
	}
#ifdef UNDER_CE
	SHFullScreen(GetActiveWindow(), SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
#endif
	// Hide system mouse cursor   
#ifndef UNDER_CE
//	SDL_ShowCursor(0); 
#endif

	surface->set_format(sdl_surface->format);
																				

//	if (zbuffer) screen->create_zbuffer();
}


bool Screen::try_scaler(int w, int h, uint32 flags, int hwdepth)
{
	// Try the universal scalers
	if (scale_factor > 1 && scaler_index >= 0) {

		scaler = scaler_reg.GetScaler(scaler_index);

		// If the scaler wasn't found, use the Point scaler
		if (!scaler) {
			std::cout << "Couldn't find scaler for scaler_index" << scaler_index << "." << std::endl;
			scaler = scaler_reg.GetPointScaler();
		}
		// If the scaler selected is 2x only, and we are in a > than 2x mode, use Point
		else if (scale_factor > 2 && scaler->flags & SCALER_FLAG_2X_ONLY) 
		{
			std::cout << "Scaler " << scaler->name << " only supports 2x. " << scale_factor << "x requested" << std::endl;
			scaler = scaler_reg.GetPointScaler();
		}
		// If it requires 16 bit, force that. However, if it fails use point
		else if (scaler->flags & SCALER_FLAG_16BIT_ONLY)
		{
			if ( !SDL_VideoModeOK(w, h, 16, flags)) {
				std::cout << scaler->name << " requires 16 bit colour. Couldn't set mode." << std::endl;
				scaler = scaler_reg.GetPointScaler();
			}
			else if (hwdepth != 16) {
				std::cout << scaler->name << " requires 16 bit colour. Forcing." << std::endl;
				hwdepth = 16;
			}
		}
		// If it requires 32 bit, force that. However, if it fails use point
		else if (scaler->flags & SCALER_FLAG_32BIT_ONLY)
		{
			if ( !SDL_VideoModeOK(w, h, 32, flags)) {
				std::cout << scaler->name << " requires 32 bit colour. Couldn't set mode." << std::endl;
				scaler = scaler_reg.GetPointScaler();
			}
			else if (hwdepth != 32) {
				std::cout << scaler->name << " requires 32 bit colour. Forcing." << std::endl;
				hwdepth = 32;
			}
		}

		std::cout << "Using scaler: " << scaler->name << std::endl;

		// Attempt to set Video mode
		if ( !SDL_VideoModeOK(w, h, hwdepth, flags))
		{
			hwdepth = 0;

			// Try 32 bit (but only if allowed)
			if (SDL_VideoModeOK(w, h, 32, flags) && !(scaler->flags & SCALER_FLAG_16BIT_ONLY))
				hwdepth = 32;
			// Try 16 bit (but only if allowed)
			else if (SDL_VideoModeOK(w, h, 16, flags) && !(scaler->flags & SCALER_FLAG_32BIT_ONLY))
				hwdepth = 16;
		}

		// Oh no, it didn't work
		if (hwdepth != 16 && hwdepth != 32)
		{
			std::cout << scaler->name << " requires 16/32 bit colour. Couldn't set mode." << std::endl;
		}
		else if ((sdl_surface = SDL_SetVideoMode(w*scale_factor, h*scale_factor, hwdepth, flags)))
		{
			/* Create render surface */
			surface = CreateRenderSurface (w, h, hwdepth);
			return true;
		}

		// Output that scaled surface creation failed
		std::cout << "Couldn't create " << scaler->name << " scaled surface" << std::endl;
		delete surface;
		scaler = 0;
		surface = 0;
		sdl_surface = 0;
		return false;
	}

	return false;
}
