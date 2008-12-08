/**
 * @file r_surface.c
 * @brief surface-related refresh code
 */

/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#include "r_local.h"
#include "r_lightmap.h"
#include "r_light.h"
#include "r_error.h"

/**
 * @brief this is the currently handled bsp model
 * @note Remember that we can have loaded more than one bsp at the same time
 */
static const model_t* bufferMapTile = NULL;

/**
 * @sa R_SetVertexBufferState
 * @param[in] mod The loaded maptile (more than one bsp loaded at the same time)
 * @note r_vertexbuffers must be set to 1 to use this
 */
static inline void R_SetVertexArrayState (const model_t* mod)
{
	R_BindArray(GL_VERTEX_ARRAY, GL_FLOAT, mod->bsp.verts);

	R_BindArray(GL_TEXTURE_COORD_ARRAY, GL_FLOAT, mod->bsp.texcoords);

	if (texunit_lightmap.enabled) {  /* lightmap texcoords */
		R_SelectTexture(&texunit_lightmap);
		R_BindArray(GL_TEXTURE_COORD_ARRAY, GL_FLOAT, mod->bsp.lmtexcoords);
		R_SelectTexture(&texunit_diffuse);
	}

	if (r_state.lighting_enabled) { /* normal vectors for lighting */
		R_BindArray(GL_NORMAL_ARRAY, GL_FLOAT, mod->bsp.normals);

		/* tangent vectors for bump mapping */
		if (r_bumpmap->value)
			R_BindArray(GL_TANGENT_ARRAY, GL_FLOAT, mod->bsp.tangents);
	}
}

/**
 * @sa R_SetVertexArrayState
 * @param[in] mod The loaded maptile (more than one bsp loaded at the same time)
 */
static inline void R_SetVertexBufferState (const model_t* mod)
{
	R_BindBuffer(GL_VERTEX_ARRAY, GL_FLOAT, mod->bsp.vertex_buffer);

	R_BindBuffer(GL_TEXTURE_COORD_ARRAY, GL_FLOAT, mod->bsp.texcoord_buffer);

	if (texunit_lightmap.enabled) {  /* lightmap texcoords */
		R_SelectTexture(&texunit_lightmap);
		R_BindBuffer(GL_TEXTURE_COORD_ARRAY, GL_FLOAT, mod->bsp.lmtexcoord_buffer);
		R_SelectTexture(&texunit_diffuse);
	}

	if (r_state.lighting_enabled) { /* normal vectors for lighting */
		R_BindBuffer(GL_NORMAL_ARRAY, GL_FLOAT, mod->bsp.normal_buffer);

		/* tangent vectors for bump mapping */
		if (r_bumpmap->value)
			R_BindBuffer(GL_TANGENT_ARRAY, GL_FLOAT, mod->bsp.tangent_buffer);
	}
}

static void R_ResetArrayState (void)
{
	R_BindBuffer(0, 0, 0);

	R_BindDefaultArray(GL_VERTEX_ARRAY);

	R_BindDefaultArray(GL_TEXTURE_COORD_ARRAY);

	if (texunit_lightmap.enabled) {
		R_SelectTexture(&texunit_lightmap);
		R_BindDefaultArray(GL_TEXTURE_COORD_ARRAY);
		R_SelectTexture(&texunit_diffuse);
	}

	if (r_state.lighting_enabled) {
		R_BindDefaultArray(GL_NORMAL_ARRAY);

		/* tangent vectors for bump mapping */
		if (r_bumpmap->value)
			R_BindDefaultArray(GL_TANGENT_ARRAY);
	}
	bufferMapTile = NULL;
}

/**
 * @brief Set the surface state according to surface flags and bind the texture
 * @sa R_DrawSurfaces
 */
static void R_SetSurfaceState (const mBspSurface_t *surf)
{
	const model_t* mod = r_mapTiles[surf->tile];
	image_t *image;

	if (r_state.blend_enabled) {  /* alpha blend */
		vec4_t color = {1.0, 1.0, 1.0, 1.0};
		switch (surf->texinfo->flags & (SURF_BLEND33 | SURF_BLEND66)) {
		case SURF_BLEND33:
			color[3] = 0.33;
			break;
		case SURF_BLEND66:
			color[3] = 0.66;
			break;
		}

		R_Color(color);
	}

	if (bufferMapTile != mod) {
		bufferMapTile = mod;

		if (qglBindBuffer && r_vertexbuffers->integer)
			R_SetVertexBufferState(mod);
		else
			R_SetVertexArrayState(mod);
	}

	image = surf->texinfo->image;
	R_BindTexture(image->texnum);  /* texture */

	if (texunit_lightmap.enabled)  /* lightmap */
		R_BindLightmapTexture(surf->lightmap_texnum);

	if (r_state.lighting_enabled && r_bumpmap->value) {
		if (image->normalmap) {
			R_BindDeluxemapTexture(surf->deluxemap_texnum);
			R_BindNormalmapTexture(image->normalmap->texnum);

			R_EnableBumpmap(qtrue, &image->material);
		} else
			R_EnableBumpmap(qfalse, NULL);
	}

	R_CheckError();
}

/**
 * @brief Use the vertex, texture and normal arrays to draw a surface
 * @sa R_DrawSurfaces
 */
static inline void R_DrawSurface (const mBspSurface_t *surf)
{
	glDrawArrays(GL_POLYGON, surf->index, surf->numedges);

	refdef.brush_count++;
}

/**
 * @brief General surface drawing function, that draw the surface chains
 * @note The needed states for the surfaces must be set before you call this
 * @sa R_DrawSurface
 * @sa R_SetSurfaceState
 */
static void R_DrawSurfaces (const mBspSurfaces_t *surfs)
{
	int i;

	for (i = 0; i < surfs->count; i++) {
		if (surfs->surfaces[i]->frame != r_locals.frame)
			continue;

		R_SetSurfaceState(surfs->surfaces[i]);
		R_DrawSurface(surfs->surfaces[i]);
	}

	/* reset state */
	if (r_state.lighting_enabled) {
		if (r_state.bumpmap_enabled)
			R_EnableBumpmap(qfalse, NULL);
	}

	/* and restore array pointers */
	R_ResetArrayState();

	R_Color(NULL);
}

/**
 * @brief Draw the surface chain with multitexture enabled and light enabled
 * @sa R_DrawBlendSurfaces
 */
void R_DrawOpaqueSurfaces (const mBspSurfaces_t *surfs)
{
	if (!surfs->count)
		return;

	R_EnableTexture(&texunit_lightmap, qtrue);

	R_EnableLighting(r_state.default_program, qtrue);
	R_DrawSurfaces(surfs);
	R_EnableLighting(NULL, qfalse);

	R_EnableTexture(&texunit_lightmap, qfalse);
}

/**
 * @brief Draw the surfaces via warp shader
 * @sa R_DrawBlendWarpSurfaces
 */
void R_DrawOpaqueWarpSurfaces (mBspSurfaces_t *surfs)
{
	if (!surfs->count)
		return;

	R_EnableWarp(r_state.warp_program, qtrue);
	R_DrawSurfaces(surfs);
	R_EnableWarp(NULL, qfalse);
}

void R_DrawAlphaTestSurfaces (mBspSurfaces_t *surfs)
{
	if (!surfs->count)
		return;

	R_EnableAlphaTest(qtrue);
	R_EnableLighting(r_state.default_program, qtrue);
	R_DrawSurfaces(surfs);
	R_EnableLighting(NULL, qfalse);
	R_EnableAlphaTest(qfalse);
}

/**
 * @brief Draw the surface chain with multitexture enabled and blend enabled
 * @sa R_DrawOpaqueSurfaces
 */
void R_DrawBlendSurfaces (const mBspSurfaces_t *surfs)
{
	if (!surfs->count)
		return;

	assert(r_state.blend_enabled);
	R_EnableTexture(&texunit_lightmap, qtrue);
	R_DrawSurfaces(surfs);
	R_EnableTexture(&texunit_lightmap, qfalse);
}

/**
 * @brief Draw the alpha surfaces via warp shader and with blend enabled
 * @sa R_DrawOpaqueWarpSurfaces
 */
void R_DrawBlendWarpSurfaces (mBspSurfaces_t *surfs)
{
	if (!surfs->count)
		return;

	assert(r_state.blend_enabled);
	R_EnableWarp(r_state.warp_program, qtrue);
	R_DrawSurfaces(surfs);
	R_EnableWarp(NULL, qfalse);
}
