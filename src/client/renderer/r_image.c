/**
 * @file r_image.c
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
#include "r_error.h"
#include "r_overlay.h"
#include "../../shared/images.h"

#define MAX_IMAGEHASH 256
static image_t *imageHash[MAX_IMAGEHASH];

image_t r_images[MAX_GL_TEXTURES];
int r_numImages;

/* generic environment map */
image_t *r_envmaptextures[MAX_ENVMAPTEXTURES];

/* lense flares */
image_t *r_flaretextures[NUM_FLARETEXTURES];

#define MAX_TEXTURE_SIZE 8192

/**
 * @brief Free previously loaded materials and their stages
 * @sa R_LoadMaterials
 */
void R_ImageClearMaterials (void)
{
	int i;
	const size_t length = lengthof(r_images);

	/* clear previously loaded materials */
	for (i = 0; i < length; i++) {
		image_t *image = &r_images[i];
		material_t *m = &image->material;
		materialStage_t *s = m->stages;

		while (s) {  /* free the stages chain */
			materialStage_t *ss = s->next;
			Mem_Free(s);
			s = ss;
		}

		memset(m, 0, sizeof(*m));

		m->bump = DEFAULT_BUMP;
		m->parallax = DEFAULT_PARALLAX;
		m->specular = DEFAULT_SPECULAR;
		m->hardness = DEFAULT_HARDNESS;
		m->glowscale = DEFAULT_GLOWSCALE;
	}
}

/**
 * @brief Shows all loaded images
 */
void R_ImageList_f (void)
{
	int i;
	image_t *image;
	int texels;

	Com_Printf("------------------\n");
	texels = 0;

	for (i = 0, image = r_images; i < r_numImages; i++, image++) {
		if (!image->texnum)
			continue;
		texels += image->upload_width * image->upload_height;
		switch (image->type) {
		case it_effect:
			Com_Printf("EF");
			break;
		case it_skin:
			Com_Printf("SK");
			break;
		case it_wrappic:
			Com_Printf("WR");
			break;
		case it_chars:
			Com_Printf("CH");
			break;
		case it_static:
			Com_Printf("ST");
			break;
		case it_normalmap:
			Com_Printf("NM");
			break;
		case it_material:
			Com_Printf("MA");
			break;
		case it_lightmap:
			Com_Printf("LM");
			break;
		case it_world:
			Com_Printf("WO");
			break;
		case it_pic:
			Com_Printf("PI");
			break;
		default:
			Com_Printf("  ");
			break;
		}

		Com_Printf(" %4i %4i RGB: %5i idx: %s\n", image->upload_width, image->upload_height, image->texnum, image->name);
	}
	Com_Printf("Total textures: %i (max textures: %i)\n", r_numImages, MAX_GL_TEXTURES);
	Com_Printf("Total texel count (not counting mipmaps): %i\n", texels);
}

/**
 * @brief Generic image-data loading fucntion.
 * @param[in] name (Full) pathname to the image to load. Extension (if given) will be ignored.
 * @param[out] pic Image data.
 * @param[out] width Width of the loaded image.
 * @param[out] height Height of the loaded image.
 * @sa R_FindImage
 */
void R_LoadImage (const char *name, byte **pic, int *width, int *height)
{
	char filenameTemp[MAX_QPATH];
	SDL_Surface *surf;

	if (!name || name[0] == '\0')
		Com_Error(ERR_FATAL, "R_LoadImage: NULL name");

	Com_StripExtension(name, filenameTemp, sizeof(filenameTemp));

	if (Img_LoadImage(filenameTemp, &surf)) {
		const size_t size = (surf->w * surf->h) * 4;
		*width = surf->w;
		*height = surf->h;
		*pic = Mem_PoolAlloc(size, vid_imagePool, 0);
		memcpy(*pic, surf->pixels, size);
		SDL_FreeSurface(surf);
	}
}

void R_ScaleTexture (unsigned *in, int inwidth, int inheight, unsigned *out, int outwidth, int outheight)
{
	int i, j;
	unsigned frac;
	unsigned p1[MAX_TEXTURE_SIZE], p2[MAX_TEXTURE_SIZE];
	const unsigned fracstep = inwidth * 0x10000 / outwidth;

	assert(outwidth <= MAX_TEXTURE_SIZE);

	frac = fracstep >> 2;
	for (i = 0; i < outwidth; i++) {
		p1[i] = 4 * (frac >> 16);
		frac += fracstep;
	}
	frac = 3 * (fracstep >> 2);
	for (i = 0; i < outwidth; i++) {
		p2[i] = 4 * (frac >> 16);
		frac += fracstep;
	}

	for (i = 0; i < outheight; i++, out += outwidth) {
		const int index = inwidth * (int) ((i + 0.25) * inheight / outheight);
		const unsigned *inrow = in + index;
		const int index2 = inwidth * (int) ((i + 0.75) * inheight / outheight);
		const unsigned *inrow2 = in + index2;

		assert(index < inwidth * inheight);
		assert(index2 < inwidth * inheight);

		for (j = 0; j < outwidth; j++) {
			const byte *pix1 = (const byte *) inrow + p1[j];
			const byte *pix2 = (const byte *) inrow + p2[j];
			const byte *pix3 = (const byte *) inrow2 + p1[j];
			const byte *pix4 = (const byte *) inrow2 + p2[j];
			((byte *) (out + j))[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0]) >> 2;
			((byte *) (out + j))[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1]) >> 2;
			((byte *) (out + j))[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2]) >> 2;
			((byte *) (out + j))[3] = (pix1[3] + pix2[3] + pix3[3] + pix4[3]) >> 2;
		}
	}
}

/**
 * @brief Applies brightness and contrast to the specified image while optionally computing
 * the image's average color. Also handles image inversion and monochrome. This is
 * all munged into one function to reduce loops on level load.
 */
void R_FilterTexture (byte *in, int width, int height, imagetype_t type, int bpp)
{
	const float scale = 1.0 / 255.0;

	byte *p = in;
	const byte *end = p + width * height * bpp;
	const float brightness = type == it_lightmap ? r_modulate->value : r_brightness->value;
	const float contrast = r_contrast->value;
	const float saturation = r_saturation->value;
	vec3_t intensity, luminosity, temp;
	int j, mask;
	float max, d;

	enum filter_flags_t {
		FILTER_NONE       = 0,
		FILTER_MONOCHROME = 1U << 0,
		FILTER_INVERT     = 1U << 1
	} filter;

	/* monochrome/invert */
	switch (type) {
	case it_world:
	case it_effect:
	case it_material:
		mask = 1;
		break;
	case it_lightmap:
		mask = 2;
		break;
	default:
		mask = 0;
		break;
	}

	filter = FILTER_NONE;
	if (r_monochrome->integer & mask)
		filter |= FILTER_MONOCHROME;
	if (r_invert->integer & mask)
		filter |= FILTER_INVERT;

	VectorSet(luminosity, 0.2125, 0.7154, 0.0721);

	for (; p != end; p += bpp) {
		VectorCopy(p, temp);
		VectorScale(temp, scale, temp);  /* convert to float */

		VectorScale(temp, brightness, temp);  /* apply brightness */

		max = 0.0;  /* determine brightest component */

		for (j = 0; j < 3; j++) {
			if (temp[j] > max)
				max = temp[j];

			if (temp[j] < 0.0)  /* enforcing positive values */
				temp[j] = 0.0;
		}

		if (max > 1.0)  /* clamp without changing hue */
			VectorScale(temp, 1.0 / max, temp);

		for (j = 0; j < 3; j++) {  /* apply contrast */
			temp[j] -= 0.5;  /* normalize to -0.5 through 0.5 */
			temp[j] *= contrast;  /* scale */
			temp[j] += 0.5;

			if (temp[j] > 1.0)  /* clamp */
				temp[j] = 1.0;
			else if (temp[j] < 0)
				temp[j] = 0;
		}

		/* finally saturation, which requires rgb */
		d = DotProduct(temp, luminosity);

		VectorSet(intensity, d, d, d);
		VectorMix(intensity, temp, saturation, temp);

		for (j = 0; j < 3; j++) {
			temp[j] *= 255;  /* back to byte */

			if (temp[j] > 255)  /* clamp */
				temp[j] = 255;
			else if (temp[j] < 0)
				temp[j] = 0;

			p[j] = (byte)temp[j];
		}

		if (filter & FILTER_MONOCHROME)
			p[0] = p[1] = p[2] = (p[0] + p[1] + p[2]) / 3;

		if (filter & FILTER_INVERT) {
			p[0] = 255 - p[0];
			p[1] = 255 - p[1];
			p[2] = 255 - p[2];
		}
	}
}

/**
 * @brief Calculates the texture size that should be used to upload the texture data
 * @param[in] width The width of the source texture data
 * @param[in] height The heigt of the source texture data
 * @param[out] scaledWidth The resulting width - can be the same as the given @c width
 * @param[out] scaledHeight The resulting height - can be the same as the given @c height
 */
void R_GetScaledTextureSize (int width, int height, int *scaledWidth, int *scaledHeight)
{
	for (*scaledWidth = 1; *scaledWidth < width; *scaledWidth <<= 1) {}
	for (*scaledHeight = 1; *scaledHeight < height; *scaledHeight <<= 1) {}

	while (*scaledWidth > r_config.maxTextureSize || *scaledHeight > r_config.maxTextureSize) {
		*scaledWidth >>= 1;
		*scaledHeight >>= 1;
	}

	if (*scaledWidth > MAX_TEXTURE_SIZE)
		*scaledWidth = MAX_TEXTURE_SIZE;
	else if (*scaledWidth < 1)
		*scaledWidth = 1;

	if (*scaledHeight > MAX_TEXTURE_SIZE)
		*scaledHeight = MAX_TEXTURE_SIZE;
	else if (*scaledHeight < 1)
		*scaledHeight = 1;
}

/**
 * @brief Uploads the opengl texture to the server
 * @param[in] data Must be in RGBA format
 */
void R_UploadTexture (unsigned *data, int width, int height, image_t* image)
{
	unsigned *scaled;
	int scaledWidth, scaledHeight;
	int samples;
	int i, c;
	byte *scan;
	qboolean mipmap = (image->type != it_pic && image->type != it_chars);
	qboolean clamp = (image->type == it_pic);

	R_GetScaledTextureSize(width, height, &scaledWidth, &scaledHeight);

	/* some images need very little attention (pics, fonts, etc..) */
	if (!mipmap && scaledWidth == width && scaledHeight == height) {
		/* no mipmapping for these images - just use GL_NEAREST here to not waste memory */
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, scaledWidth, scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		return;
	}

	if (scaledWidth != width || scaledHeight != height) {  /* whereas others need to be scaled */
		scaled = (unsigned *)Mem_PoolAllocExt(scaledWidth * scaledHeight * sizeof(unsigned), qfalse, vid_imagePool, 0);
		R_ScaleTexture(data, width, height, scaled, scaledWidth, scaledHeight);
	} else {
		scaled = data;
	}

	/* and filter */
	if (image->type == it_effect || image->type == it_world || image->type == it_material || image->type == it_skin)
		R_FilterTexture((byte*)scaled, scaledWidth, scaledHeight, image->type, 4);

	/* scan the texture for any non-255 alpha */
	c = scaledWidth * scaledHeight;
	samples = r_config.gl_compressed_solid_format ? r_config.gl_compressed_solid_format : r_config.gl_solid_format;
	/* set scan to the first alpha byte */
	for (i = 0, scan = ((byte *) scaled) + 3; i < c; i++, scan += 4) {
		if (*scan != 255) {
			samples = r_config.gl_compressed_alpha_format ? r_config.gl_compressed_alpha_format : r_config.gl_alpha_format;
			break;
		}
	}

	image->has_alpha = (samples == r_config.gl_alpha_format || samples == r_config.gl_compressed_alpha_format);
	image->upload_width = scaledWidth;	/* after power of 2 and scales */
	image->upload_height = scaledHeight;

	/* and mipmapped */
	if (mipmap) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_config.gl_filter_min);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_config.gl_filter_max);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		if (r_config.anisotropic) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_config.maxAnisotropic);
			R_CheckError();
		}
		if (r_texture_lod->integer && r_config.lod_bias) {
			glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, r_texture_lod->value);
			R_CheckError();
		}
	} else {
		if (r_config.anisotropic) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
			R_CheckError();
		}
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_config.gl_filter_max);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_config.gl_filter_max);
	}
	R_CheckError();

	if (clamp) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		R_CheckError();
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		R_CheckError();
	}

	glTexImage2D(GL_TEXTURE_2D, 0, samples, scaledWidth, scaledHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, scaled);
	R_CheckError();

	if (scaled != data)
		Mem_Free(scaled);
}

/**
 * @brief Applies blurring to a texture
 * @sa R_BuildLightMap
 */
void R_SoftenTexture (byte *in, int width, int height, int bpp)
{
	byte *out;
	int i, j, k;
	const int size = width * height * bpp;

	/* soften into a copy of the original image, as in-place would be incorrect */
	out = (byte *)Mem_PoolAllocExt(size, qfalse, vid_imagePool, 0);
	if (!out)
		Com_Error(ERR_FATAL, "Mem_PoolAllocExt: failed on allocation of %i bytes for R_SoftenTexture", width * height * bpp);

	memcpy(out, in, size);

	for (i = 1; i < height - 1; i++) {
		for (j = 1; j < width - 1; j++) {
			const byte *src = in + ((i * width) + j) * bpp;  /* current input pixel */

			const byte *u = (src - (width * bpp));  /* and it's neighbors */
			const byte *d = (src + (width * bpp));
			const byte *l = (src - (1 * bpp));
			const byte *r = (src + (1 * bpp));

			byte *dest = out + ((i * width) + j) * bpp;  /* current output pixel */

			for (k = 0; k < bpp; k++)
				dest[k] = (u[k] + d[k] + l[k] + r[k]) / 4;
		}
	}

	/* copy the softened image over the input image, and free it */
	memcpy(in, out, size);
	Mem_Free(out);
}

#define DAN_WIDTH	512
#define DAN_HEIGHT	256

#define DAWN		0.03

/** this is the data that is used with r_dayandnightTexture */
static byte r_dayandnightAlpha[DAN_WIDTH * DAN_HEIGHT];
/** this is the mask that is used to display day/night on (2d-)geoscape */
image_t *r_dayandnightTexture;

/**
 * @brief Applies alpha values to the night overlay image for 2d geoscape
 * @param[in] q The angle the sun is standing against the equator on earth
 */
void R_CalcAndUploadDayAndNightTexture (float q)
{
	int x, y;
	const float dphi = (float) 2 * M_PI / DAN_WIDTH;
	const float da = M_PI / 2 * (HIGH_LAT - LOW_LAT) / DAN_HEIGHT;
	const float sin_q = sin(q);
	const float cos_q = cos(q);
	float sin_phi[DAN_WIDTH], cos_phi[DAN_WIDTH];
	byte *px;

	for (x = 0; x < DAN_WIDTH; x++) {
		const float phi = x * dphi - q;
		sin_phi[x] = sin(phi);
		cos_phi[x] = cos(phi);
	}

	/* calculate */
	px = r_dayandnightAlpha;
	for (y = 0; y < DAN_HEIGHT; y++) {
		const float a = sin(M_PI / 2 * HIGH_LAT - y * da);
		const float root = sqrt(1 - a * a);
		for (x = 0; x < DAN_WIDTH; x++) {
			const float pos = sin_phi[x] * root * sin_q - (a * SIN_ALPHA + cos_phi[x] * root * COS_ALPHA) * cos_q;

			if (pos >= DAWN)
				*px++ = 255;
			else if (pos <= -DAWN)
				*px++ = 0;
			else
				*px++ = (byte) (128.0 * (pos / DAWN + 1));
		}
	}

	/* upload alpha map into the r_dayandnighttexture */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, DAN_WIDTH, DAN_HEIGHT, 0, GL_ALPHA, GL_UNSIGNED_BYTE, r_dayandnightAlpha);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, r_config.gl_filter_max);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, r_config.gl_filter_max);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	R_CheckError();
}

static inline void R_DeleteImage (image_t *image)
{
	image_t *next = image->hashNext;
	image_t *prev = image->hashPrev;

	/* free it */
	glDeleteTextures(1, (GLuint *) &image->texnum);
	R_CheckError();

	if (next) {
		assert(next->hashPrev == image);
		next->hashPrev = prev;
	}
	if (prev) {
		assert(prev->hashNext == image);
		prev->hashNext = next;
	} else {
		const unsigned int hash = Com_HashKey(image->name, MAX_IMAGEHASH);
		assert(imageHash[hash] == image);
		imageHash[hash] = next;
	}

	memset(image, 0, sizeof(*image));
}

static inline image_t *R_GetImage (const char *name)
{
	image_t *image;
	const unsigned hash = Com_HashKey(name, MAX_IMAGEHASH);

	/* look for it */
	for (image = imageHash[hash]; image; image = image->hashNext)
		if (!strcmp(name, image->name))
			return image;

	return NULL;
}

/**
 * @brief Creates a new image from RGBA data. Stores it in the gltextures array
 * and also uploads it.
 * @note This is also used as an entry point for the generated r_noTexture
 * @param[in] name The name of the newly created image
 * @param[in] pic The RGBA data of the image
 * @param[in] width The width of the image (power of two, please)
 * @param[in] height The height of the image (power of two, please)
 * @param[in] type The image type @sa imagetype_t
 */
image_t *R_LoadImageData (const char *name, byte * pic, int width, int height, imagetype_t type)
{
	image_t *image;
	int i;
	size_t len;
	unsigned hash;

	len = strlen(name);
	if (len >= sizeof(image->name))
		Com_Error(ERR_DROP, "R_LoadImageData: \"%s\" is too long", name);
	if (len == 0)
		Com_Error(ERR_DROP, "R_LoadImageData: name is empty");

	/* look for it */
	image = R_GetImage(name);
	if (image) {
		assert(image->texnum);
		Com_Printf("R_LoadImageData: image '%s' is already uploaded\n", name);
		return image;
	}

	/* find a free image_t */
	for (i = 0, image = r_images; i < r_numImages; i++, image++)
		if (!image->texnum)
			break;

	if (i == r_numImages) {
		if (r_numImages >= MAX_GL_TEXTURES)
			Com_Error(ERR_DROP, "R_LoadImageData: MAX_GL_TEXTURES hit");
		r_numImages++;
	}
	image = &r_images[i];
	image->has_alpha = qfalse;
	image->type = type;
	image->width = width;
	image->height = height;
	image->texnum = i + 1;

	Q_strncpyz(image->name, name, sizeof(image->name));
	/* drop extension */
	if (len >= 4 && image->name[len - 4] == '.') {
		image->name[len - 4] = '\0';
		Com_Printf("Image with extension: '%s'\n", name);
	}

	hash = Com_HashKey(image->name, MAX_IMAGEHASH);
	image->hashNext = imageHash[hash];
	if (imageHash[hash])
		imageHash[hash]->hashPrev = image;
	imageHash[hash] = image;

	if (pic) {
		R_BindTexture(image->texnum);
		R_UploadTexture((unsigned *) pic, width, height, image);
	}
	return image;
}

/**
 * @brief Finds or loads the given image
 * @sa R_RegisterImage
 * @param[in] pname Image name Path relative to the game dir (e.g. textures/tex_common/nodraw)
 * @param[in] type The type of the image. This has influence on image filters and texture
 * parameters when uploading the image data
 * @note the image name has to be at least 5 chars long
 * @sa R_LoadTGA
 * @sa R_LoadJPG
 * @sa R_LoadPNG
 */
#ifdef DEBUG
image_t *R_FindImageDebug (const char *pname, imagetype_t type, const char *file, int line)
#else
image_t *R_FindImage (const char *pname, imagetype_t type)
#endif
{
	char lname[MAX_QPATH];
	image_t *image;
	SDL_Surface *surf;

	if (!pname || !pname[0])
		Com_Error(ERR_FATAL, "R_FindImage: NULL name");

	/* drop extension */
	Com_StripExtension(pname, lname, sizeof(lname));

	image = R_GetImage(lname);
	if (image)
		return image;

	if (Img_LoadImage(lname, &surf)) {
		image = R_LoadImageData(lname, surf->pixels, surf->w, surf->h, type);
		SDL_FreeSurface(surf);
		if (image->type == it_world) {
			image->normalmap = R_FindImage(va("%s_nm", image->name), it_normalmap);
			if (image->normalmap == r_noTexture)
				image->normalmap = NULL;
		}
		if (image->type != it_glowmap) {
			image->glowmap = R_FindImage(va("%s_gm", image->name), it_glowmap);
			if (image->glowmap == r_noTexture)
				image->glowmap = NULL;
		}
	}

	/* no fitting texture found */
	if (!image)
		image = r_noTexture;

	return image;
}

qboolean R_ImageExists (const char *pname)
{
	const char **types = Img_GetImageTypes();
	int i;

	for (i = 0; types[i]; i++) {
		if (FS_CheckFile("%s.%s", pname, types[i]) != -1)
			return qtrue;
	}
	return qfalse;
}

/**
 * @brief Free the image and its normalmap (if there is one)
 * @param image The image that should be freed
 */
void R_FreeImage (image_t *image)
{
	/* free image slot */
	if (!image || !image->texnum)
		return;

	/* also free a normalmap if there is one */
	if (image->normalmap)
		R_DeleteImage(image->normalmap);
	R_DeleteImage(image);
}

/**
 * @brief Any image that is a mesh or world texture will be removed here
 * @sa R_ShutdownImages
 */
void R_FreeWorldImages (void)
{
	int i;
	image_t *image;

	R_CheckError();
	for (i = 0, image = r_images; i < r_numImages; i++, image++) {
		if (image->type < it_world)
			continue;			/* keep them */

		/* free it */
		R_FreeImage(image);
	}
}

void R_InitImages (void)
{
	int i;

#ifdef SDL_IMAGE_VERSION
	SDL_version version;

	SDL_IMAGE_VERSION(&version)
	Com_Printf("SDL_image version %i.%i.%i\n", version.major, version.minor, version.patch);
#else
	Com_Printf("could not get SDL_image version\n");
#endif

	r_numImages = 0;

	r_dayandnightTexture = R_LoadImageData("***r_dayandnighttexture***", NULL, DAN_WIDTH, DAN_HEIGHT, it_effect);
	if (r_dayandnightTexture == r_noTexture)
		Com_Error(ERR_FATAL, "Could not create daynight image for the geoscape");

	for (i = 0; i < MAX_ENVMAPTEXTURES; i++) {
		r_envmaptextures[i] = R_FindImage(va("pics/envmaps/envmap_%i", i), it_effect);
		if (r_envmaptextures[i] == r_noTexture)
			Com_Error(ERR_FATAL, "Could not load environment map %i", i);
	}

	for (i = 0; i < NUM_FLARETEXTURES; i++) {
		r_flaretextures[i] = R_FindImage(va("pics/flares/flare_%i", i), it_effect);
		if (r_flaretextures[i] == r_noTexture)
			Com_Error(ERR_FATAL, "Could not load lens flare %i", i);
	}

	R_InitOverlay();
}

/**
 * @sa R_FreeWorldImages
 */
void R_ShutdownImages (void)
{
	int i;
	image_t *image;

	R_CheckError();
	for (i = 0, image = r_images; i < r_numImages; i++, image++) {
		if (!image->texnum)
			continue;			/* free image_t slot */
		R_DeleteImage(image);
	}
	memset(imageHash, 0, sizeof(imageHash));

	R_ShutdownOverlay();
}


typedef struct {
	const char *name;
	int minimize, maximize;
} glTextureMode_t;

static const glTextureMode_t gl_texture_modes[] = {
	{"GL_NEAREST", GL_NEAREST, GL_NEAREST},
	{"GL_LINEAR", GL_LINEAR, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR},
	{"GL_NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR, GL_NEAREST},
	{"GL_LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR}
};
#define NUM_R_MODES (sizeof(gl_texture_modes) / sizeof(glTextureMode_t))

void R_TextureMode (const char *string)
{
	int i, texturemode;
	image_t *image;

	for (i = 0; i < NUM_R_MODES; i++) {
		if (!Q_strcasecmp(gl_texture_modes[i].name, string))
			break;
	}

	if (i == NUM_R_MODES) {
		Com_Printf("bad filter name\n");
		return;
	}

	texturemode = i;

	for (i = 0, image = r_images; i < r_numImages; i++, image++) {
		if (image->type == it_chars || image->type == it_pic || image->type == it_wrappic)
			continue;

		R_BindTexture(image->texnum);
		if (r_config.anisotropic)
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, r_config.maxAnisotropic);
		R_CheckError();
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, gl_texture_modes[texturemode].minimize);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, gl_texture_modes[texturemode].maximize);
		R_CheckError();
	}
}

typedef struct {
	const char *name;
	int mode;
} gltmode_t;

static const gltmode_t gl_alpha_modes[] = {
	{"default", 4},
	{"GL_RGBA", GL_RGBA},
	{"GL_RGBA8", GL_RGBA8},
	{"GL_RGB5_A1", GL_RGB5_A1},
	{"GL_RGBA4", GL_RGBA4},
	{"GL_RGBA2", GL_RGBA2},
};

#define NUM_R_ALPHA_MODES (sizeof(gl_alpha_modes) / sizeof(gltmode_t))

void R_TextureAlphaMode (const char *string)
{
	int i;

	for (i = 0; i < NUM_R_ALPHA_MODES; i++) {
		if (!Q_strcasecmp(gl_alpha_modes[i].name, string))
			break;
	}

	if (i == NUM_R_ALPHA_MODES) {
		Com_Printf("bad alpha texture mode name\n");
		return;
	}

	r_config.gl_alpha_format = gl_alpha_modes[i].mode;
}

static const gltmode_t gl_solid_modes[] = {
	{"default", 3},
	{"GL_RGB", GL_RGB},
	{"GL_RGB8", GL_RGB8},
	{"GL_RGB5", GL_RGB5},
	{"GL_RGB4", GL_RGB4},
	{"GL_R3_G3_B2", GL_R3_G3_B2},
#ifdef GL_RGB2_EXT
	{"GL_RGB2", GL_RGB2_EXT},
#endif
};

#define NUM_R_SOLID_MODES (sizeof(gl_solid_modes) / sizeof(gltmode_t))

void R_TextureSolidMode (const char *string)
{
	int i;

	for (i = 0; i < NUM_R_SOLID_MODES; i++) {
		if (!Q_strcasecmp(gl_solid_modes[i].name, string))
			break;
	}

	if (i == NUM_R_SOLID_MODES) {
		Com_Printf("bad solid texture mode name\n");
		return;
	}

	r_config.gl_solid_format = gl_solid_modes[i].mode;
}

