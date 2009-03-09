/**
 * @file r_lightmap.h
 * @brief lightmap definitions
 */

/*
All original material Copyright (C) 2002-2007 UFO: Alien Invasion team.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef R_LIGHTMAP_H
#define R_LIGHTMAP_H

#include "r_entity.h"

#define	LIGHTMAP_BLOCK_WIDTH	4096
#define	LIGHTMAP_BLOCK_HEIGHT	4096
#define LIGHTMAP_BLOCK_BYTES	4 /* bytes RGBA */
#define DELUXEMAP_BLOCK_BYTES	4 /* bytes RGBA */

#define LIGHTMAP_BYTES			3 /* RGB */

void R_BlendLightmaps(const model_t* mod);
void R_CreateSurfaceLightmap(mBspSurface_t * surf);
void R_EndBuildingLightmaps(void);
void R_BeginBuildingLightmaps(void);

/* in the bsp, they are just rgb, and we work with floats */
#define LIGHTMAP_FBUFFER_SIZE \
	(MAX_MAP_LIGHTMAP * LIGHTMAP_BYTES)

typedef struct lightmaps_s {
	GLuint lightmap_texnum;
	GLuint deluxemap_texnum;

	int size;  /* lightmap block size (NxN) */

	unsigned *allocated; /* block availability */

	byte *sample_buffer; /* RGBA buffers for uploading */
	byte *direction_buffer;
	float fbuffer[LIGHTMAP_FBUFFER_SIZE]; /* RGB buffer for bsp loading */
} lightmaps_t;

extern lightmaps_t r_lightmaps;

void R_LightPoint(const vec3_t p, static_lighting_t *lighting);

#endif
