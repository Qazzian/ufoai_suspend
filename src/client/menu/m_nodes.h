/**
 * @file m_nodes.h
 */

/*
Copyright (C) 1997-2008 UFO:AI Team

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

#ifndef CLIENT_MENU_M_NODES_H
#define CLIENT_MENU_M_NODES_H

#include "../cl_renderer.h"
#include "../../common/common.h"
#include "../renderer/r_mesh.h"
#include "../renderer/r_draw.h"
#include "../renderer/r_mesh_anim.h"

/** @brief possible menu node types */
typedef enum mn_s {
	MN_NULL,
	MN_CONFUNC,
	MN_CVARFUNC,
	MN_FUNC,
	MN_ZONE,
	MN_PIC,
	MN_STRING,
	MN_SPINNER,
	MN_TEXT,
	MN_TEXTENTRY,
	MN_BAR,
	MN_TBAR,
	MN_MODEL,
	MN_CONTAINER,
	MN_ITEM,	/**< used to display the model of an item or aircraft */
	MN_MAP,
	MN_BASEMAP,
	MN_BASELAYOUT,
	MN_CHECKBOX,
	MN_SELECTBOX,
	MN_LINESTRIP,
	MN_CINEMATIC, /**< every menu can only have one cinematic */
	MN_RADAR,	/**< tactical mission radar */
	MN_TAB,
	MN_CONTROLS,	/**< menu controls */
	MN_CUSTOMBUTTON,
	MN_WINDOWPANEL,
	MN_BUTTON,
	MN_WINDOW,

	MN_NUM_NODETYPE
} mn_t;

#define MAX_EXLUDERECTS	16

typedef struct excludeRect_s {
	vec2_t pos, size;
} excludeRect_t;

/* extradata struct */
#include "m_node_abstractvalue.h"
#include "m_node_linestrip.h"
#include "m_node_model.h"
#include "m_node_selectbox.h"

/**
 * @brief menu node
 * @todo delete data* when it's possible
 */
typedef struct menuNode_s {
	/* common identification */
	char name[MAX_VAR];
	struct nodeBehaviour_s *behaviour;

	/* common navigation */
	struct menuNode_s *next;
	struct menu_s *menu;		/**< backlink */

	/* common pos */
	vec2_t pos;
	vec2_t size;

	/* common attributes */
	char key[MAX_VAR];
	byte state;					/**< e.g. the line number for text nodes to highlight due to cursor hovering */
	byte textalign;
	int border;					/**< border for this node - thickness in pixel - default 0 - also see bgcolor */
	int padding;				/**< padding for this node - default 3 - see bgcolor */
	qboolean invis;
	qboolean blend;
	qboolean disabled;			/**< true, the node is unactive */
	int mousefx;
	char* text;
	const char* font;	/**< Font to draw text */
	const char* tooltip; /**< holds the tooltip */

	byte align;					/** @todo delete it when its possible */

	/** @todo need a cleanup
	 */
	void* dataImageOrModel;	/**< an image, or a model, this depends on the node type */
	void* dataModelSkinOrCVar; /**< a skin or a cvar, this depends on the node type */

	/* common color */
	vec4_t color;				/**< rgba */
	vec4_t bgcolor;				/**< rgba */
	vec4_t bordercolor;			/**< rgba - see border and padding */
	vec4_t selectedColor;		/**< rgba The color to draw the line specified by textLineSelected in. */

	/* common events */
	struct menuAction_s *click;
	struct menuAction_s *rclick;
	struct menuAction_s *mclick;
	struct menuAction_s *wheel;
	struct menuAction_s *mouseIn;
	struct menuAction_s *mouseOut;
	struct menuAction_s *wheelUp;
	struct menuAction_s *wheelDown;

	vec3_t scale;
	invDef_t *container;		/** The container linked to this node. */
	int horizontalScroll;		/**< if text is too long, the text is horizontally scrolled, @todo implement me */
	int textScroll;				/**< textfields - current scroll position */
	int textLines;				/**< How many lines there are (set by MN_DrawMenus)*/
	int textLineSelected;		/**< Which line is currenlty selected? This counts only visible lines). Add textScroll to this value to get total linecount. @sa selectedColor below.*/
	byte longlines;				/**< what to do with long lines */
	int timeOut;				/**< ms value until invis is set (see cl.time) */
	int timePushed;				/**< when a menu was pushed this value is set to cl.time */
	qboolean timeOutOnce;		/**< timeOut is decreased if this value is true */
	qboolean repeat;			/**< repeat action when "click" is holded */
	int clickDelay;				/**< for nodes that have repeat set, this is the delay for the next click */
	qboolean scrollbar;			/**< if you want to add a scrollbar to a text node, set this to true */
	qboolean scrollbarLeft;		/**< true if the scrollbar should be on the left side of the text node */
	excludeRect_t exclude[MAX_EXLUDERECTS];	/**< exclude this for hover or click functions */
	int excludeNum;				/**< how many exclude rects defined? */
	menuDepends_t depends;
	const value_t *scriptValues;

	/* MN_IMAGE, and more */
	vec2_t texh;				/**< lower right texture coordinates, for text nodes texh[0] is the line height and texh[1] tabs width */
	vec2_t texl;				/**< upper left texture coordinates */

	/* MN_TBAR */
	float pointWidth;			/**< MN_TBAR: texture pixels per one point */
	int gapWidth;				/**< MN_TBAR: tens separator width */

	/* MN_TEXT */
	int lineUnderMouse;	/**< MN_TEXT: The line under the mouse, when the mouse is over the node */
	int num;					/**< textfields: menutexts-id - baselayouts: baseID */
	int height;					/**< textfields: max. rows to show
								 * select box: options count */

	/* BaseLayout */
	int baseid;					/**< the baseid - e.g. for baselayout nodes */

	/** union will contain all extradata for a node */
	union {
		abstractValueExtraData_t abstractvalue;
		lineStripExtraData_t linestrip;	/**< List of lines to draw. (MN_LINESTRIP) */
		modelExtraData_t model;
		optionExtraData_t option;
	} u;

} menuNode_t;

/** @brief node behaviour, how a node work */
typedef struct nodeBehaviour_s {
	/* attributes */
	const char* name;	/**< name of the behaviour: string type of a node */
	int id;				/**< id of the behaviour: will be removed soon */
	qboolean isVirtual; /**< if true, the node dont have any position on the screen */
	const value_t* properties; /**< list of properties of the node */
	int propertyCount;	/**< number of the properties into the propertiesList. Cache value to speedup search */

	/* function */
	void (*draw)(menuNode_t *node);		/**< how to draw a node */
	void (*leftClick)(menuNode_t *node, int x, int y); /**< on left mouse click into the node */
	void (*rightClick)(menuNode_t *node, int x, int y); /**< on left mouse button click into the node */
	void (*middleClick)(menuNode_t *node, int x, int y); /**< on right mouse button click into the node */
	void (*mouseWheel)(menuNode_t *node, qboolean down, int x, int y); /**< on use mouse wheel into the node */
	void (*mouseMove)(menuNode_t *node, int x, int y);
	void (*mouseDown)(menuNode_t *node, int x, int y, int button);	/**< on a mouse button down into the node */
	void (*mouseUp)(menuNode_t *node, int x, int y, int button);	/**< on a mouse button up into the node */
	void (*capturedMouseMove)(menuNode_t *node, int x, int y);
	void (*loading)(menuNode_t *node);		/**< call before script initialisation to init default value */
	void (*loaded)(menuNode_t *node); /**< call one time, when node load from script is finished */

	/** Planed */
	/*
	mouse move event
	void (*mouseEnter)(menuNode_t *node);
	void (*mouseLeave)(menuNode_t *node);
	drag and drop callback
	int  (*dndEnter)(menuNode_t *node);
	void (*dndLeave)(menuNode_t *node);
	int  (*dndMove)(menuNode_t *node);
	int  (*dndEnd)(menuNode_t *node);
	*/
} nodeBehaviour_t;

extern nodeBehaviour_t menuBehaviour;

qboolean MN_CheckNodeZone(menuNode_t* const node, int x, int y);
void MN_UnHideNode(menuNode_t* node);
void MN_HideNode(menuNode_t* node);
void MN_SetNewNodePos(menuNode_t* node, int x, int y);
void MN_GetNodeAbsPos(const menuNode_t* node, vec2_t pos);
void MN_NodeAbsoluteToRelativePos(const menuNode_t* node, int *x, int *y);
menuNode_t* MN_AllocNode(int type);
nodeBehaviour_t* MN_GetNodeBehaviour(const char* name);

void MN_InitNodes(void);

#endif
