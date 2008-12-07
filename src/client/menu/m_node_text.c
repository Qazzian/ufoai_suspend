/**
 * @file m_node_text.c
 * @todo add getter/setter to cleanup access to extradata from cl_*.c files (check "u.text.")
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

#include "m_main.h"
#include "m_font.h"
#include "m_node_text.h"
#include "m_actions.h"
#include "../cl_video.h" /**< @todo Clean this up and remove this include */
#include "../../common/scripts.h"

#define EXTRADATA(node) (node->u.text)

static void MN_TextNodeDraw(menuNode_t *node);

/**
 * @brief Change the selected line
 */
void MN_TextNodeSelectLine (menuNode_t *node, int num)
{
	EXTRADATA(node).textLineSelected = num;
}

/**
 * @brief Scrolls the text in a textbox up/down.
 * @param[in] node The node of the text to be scrolled.
 * @param[in] offset Number of lines to scroll. Positive values scroll down, negative up.
 * @return Returns qtrue if scrolling was possible otherwise qfalse.
 */
qboolean MN_TextScroll (menuNode_t *node, int offset)
{
	int textScroll_new;

	if (!node || !EXTRADATA(node).rows)
		return qfalse;

	if (EXTRADATA(node).textLines <= EXTRADATA(node).rows) {
		/* Number of lines are less than the height of the textbox. */
		EXTRADATA(node).textScroll = 0;
		return qfalse;
	}

	textScroll_new = EXTRADATA(node).textScroll + offset;

	if (textScroll_new <= 0) {
		/* Goto top line, no matter how big the offset was. */
		EXTRADATA(node).textScroll = 0;
		return qtrue;

	} else if (textScroll_new >= (EXTRADATA(node).textLines + 1 - EXTRADATA(node).rows)) {
		/* Goto last possible line, no matter how big the offset was. */
		EXTRADATA(node).textScroll = EXTRADATA(node).textLines - EXTRADATA(node).rows;
		return qtrue;

	} else {
		EXTRADATA(node).textScroll = textScroll_new;
		return qtrue;
	}
}

/**
 * @brief Call to say to the node the text is updated
 */
static void MN_TextUpdated_f (void)
{
	menuNode_t *node;

	if (Cmd_Argc() != 2) {
		Com_Printf("Usage: %s <nodename>\n", Cmd_Argv(0));
		return;
	}

	node = MN_GetNodeFromCurrentMenu(Cmd_Argv(1));
	if (!node) {
		Com_DPrintf(DEBUG_CLIENT, "MN_TextUpdateLines_f: Node '%s.%s' not found.\n", MN_GetActiveMenu()->name, Cmd_Argv(1));
		return;
	}

	/** @todo fix it better */
	/* bad hack to compute the lines */
	MN_TextNodeDraw(node);
}

/**
 * @brief Scriptfunction that gets the wanted text node and scrolls the text.
 */
static void MN_TextScroll_f (void)
{
	int offset = 0;
	menuNode_t *node;

	if (Cmd_Argc() < 3) {
		Com_Printf("Usage: %s <nodename> <+/-offset>\n", Cmd_Argv(0));
		return;
	}

	node = MN_GetNodeFromCurrentMenu(Cmd_Argv(1));

	if (!node) {
		Com_DPrintf(DEBUG_CLIENT, "MN_TextScroll_f: Node '%s' not found.\n", Cmd_Argv(1));
		return;
	}

	if (!Q_strncmp(Cmd_Argv(2), "reset", 5)) {
		EXTRADATA(node).textScroll = 0;
		return;
	}

	offset = atoi(Cmd_Argv(2));

	if (offset == 0)
		return;

	MN_TextScroll(node, offset);
}

/**
 * @brief Scroll to the bottom
 */
void MN_TextScrollBottom (const char* nodeName)
{
	menuNode_t *node = MN_GetNodeFromCurrentMenu(nodeName);
	if (!node) {
		Com_DPrintf(DEBUG_CLIENT, "Node '%s' could not be found\n", nodeName);
		return;
	}

	if (EXTRADATA(node).textLines > EXTRADATA(node).rows) {
		Com_DPrintf(DEBUG_CLIENT, "\nMN_TextScrollBottom: Scrolling to line %i\n", EXTRADATA(node).textLines - EXTRADATA(node).rows + 1);
		EXTRADATA(node).textScroll = EXTRADATA(node).textLines - EXTRADATA(node).rows + 1;
	}
}

/**
 * @brief Draw a scrollbar, if necessary
 * @note Needs node->textLines to be accurate
 */
static void MN_DrawScrollBar (const menuNode_t *node)
{
	static const vec4_t scrollbarBackground = {0.03, 0.41, 0.05, 0.5};
	static const vec4_t scrollbarColor = {0.03, 0.41, 0.05, 1.0};

	if (EXTRADATA(node).scrollbar && EXTRADATA(node).rows && EXTRADATA(node).textLines > EXTRADATA(node).rows) {
		vec2_t nodepos;
		int scrollbarX;
		float scrollbarY;

		MN_GetNodeAbsPos(node, nodepos);
		scrollbarX = nodepos[0] + (EXTRADATA(node).scrollbarLeft ? 0 : node->size[0] - MN_SCROLLBAR_WIDTH);
		scrollbarY = node->size[1] * EXTRADATA(node).rows / EXTRADATA(node).textLines * MN_SCROLLBAR_HEIGHT;

		R_DrawFill(scrollbarX, nodepos[1],
			MN_SCROLLBAR_WIDTH, node->size[1],
			ALIGN_UL, scrollbarBackground);

		R_DrawFill(scrollbarX, nodepos[1] + (node->size[1] - scrollbarY) * EXTRADATA(node).textScroll / (EXTRADATA(node).textLines - EXTRADATA(node).rows),
			MN_SCROLLBAR_WIDTH, scrollbarY,
			ALIGN_UL, scrollbarColor);
	}
}

/**
 * @brief Get the line number under an absolute position
 * @param[in] node a text node
 * @param[in] x position x on the screen
 * @param[in] y position y on the screen
 * @return The line number under the position (0 = first line)
 */
int MN_TextNodeGetLine (const menuNode_t *node, int x, int y)
{
	assert(node->behaviour->id == MN_TEXT);

	/* if no texh, its not a text list, result is not important */
	if (!node->texh[0])
		return 0;

	MN_NodeAbsoluteToRelativePos(node, &x, &y);
	if (EXTRADATA(node).textScroll)
		return (int) (y / node->texh[0]) + EXTRADATA(node).textScroll;
	else
		return (int) (y / node->texh[0]);
}

static void MN_TextNodeMouseMove (menuNode_t *node, int x, int y) {
	EXTRADATA(node).lineUnderMouse = MN_TextNodeGetLine(node, x, y);
}

/**
 * @brief Handles line breaks and drawing for MN_TEXT menu nodes
 * @sa MN_DrawMenus
 * @param[in] text Text to draw
 * @param[in] font Font string to use
 * @param[in] node The current menu node
 * @param[in] x The fixed x position every new line starts
 * @param[in] y The fixed y position the text node starts
 */
static void MN_TextNodeDrawText (const char *text, const linkedList_t* list, const char* font, menuNode_t* node, int x, int y, int width, int height)
{
	char textCopy[MAX_MENUTEXTLEN];
	char newFont[MAX_VAR];
	const char* oldFont = NULL;
	vec4_t colorHover;
	vec4_t colorSelectedHover;
	char *cur, *tab, *end;
	int lines;
	int x1; /* variable x position */

	if (text) {
		Q_strncpyz(textCopy, text, sizeof(textCopy));
	} else if (list) {
		Q_strncpyz(textCopy, (const char*)list->data, sizeof(textCopy));
	} else
		Sys_Error("MN_DrawTextNode: Called without text or linkedList pointer");
	cur = textCopy;

	/* Hover darkening effect for normal text lines. */
	VectorScale(node->color, 0.8, colorHover);
	colorHover[3] = node->color[3];

	/* Hover darkening effect for selected text lines. */
	VectorScale(node->selectedColor, 0.8, colorSelectedHover);
	colorSelectedHover[3] = node->selectedColor[3];

	/* scrollbar space */
	if (EXTRADATA(node).scrollbar) {
		width -= MN_SCROLLBAR_WIDTH + MN_SCROLLBAR_PADDING;
		if (EXTRADATA(node).scrollbarLeft)
			x += MN_SCROLLBAR_WIDTH + MN_SCROLLBAR_PADDING;
	}

	R_ColorBlend(node->color);

	/*Com_Printf("\n\n\nEXTRADATA(node).textLines: %i \n", EXTRADATA(node).textLines);*/
	lines = 0;
	do {
		/* new line starts from node x position */
		x1 = x;
		if (oldFont) {
			font = oldFont;
			oldFont = NULL;
		}

		/* text styles and inline images */
		if (cur[0] == '^') {
			switch (toupper(cur[1])) {
			case 'B':
				Com_sprintf(newFont, sizeof(newFont), "%s_bold", font);
				oldFont = font;
				font = newFont;
				cur += 2; /* don't print the format string */
				break;
			}
		} else if (!Q_strncmp(cur, TEXT_IMAGETAG, strlen(TEXT_IMAGETAG))) {
			const char *token;
			const image_t *image;
			int y1 = y;
			/* cut the image tag */
			cur += strlen(TEXT_IMAGETAG);
			token = COM_Parse((const char **)&cur);
			/** @todo fix scrolling images */
			if (lines > EXTRADATA(node).textScroll)
				y1 += (lines - EXTRADATA(node).textScroll) * node->texh[0];
			/* don't draw images that would be out of visible area */
			if (y + height > y1 && lines >= EXTRADATA(node).textScroll) {
				/** @todo (menu) once font_t from r_font.h is known everywhere we should scale the height here, too */
				image = R_DrawNormPic(x1, y1, 0, 0, 0, 0, 0, 0, node->align, node->blend, token);
				x1 += image->height;
			}
		}

		/* get the position of the next newline - otherwise end will be null */
		end = strchr(cur, '\n');
		if (end)
			/* set the \n to \0 to draw only this part (before the \n) with our font renderer */
			/* let end point to the next char after the \n (or \0 now) */
			*end++ = '\0';

		/* highlighting */
		if (lines == EXTRADATA(node).textLineSelected && EXTRADATA(node).textLineSelected >= 0) {
			/* Draw current line in "selected" color (if the linenumber is stored). */
			R_Color(node->selectedColor);
		}

		if (node->state && node->mousefx && lines == EXTRADATA(node).lineUnderMouse) {
			/* Hightlight line if mousefx is true. */
			/** @todo what about multiline text that should be highlighted completely? */
			if (lines == EXTRADATA(node).textLineSelected && EXTRADATA(node).textLineSelected >= 0) {
				R_ColorBlend(colorSelectedHover);
			} else {
				R_ColorBlend(colorHover);
			}
		}

		/* tabulation, we assume all the tabs fit on a single line */
		do {
			int tabwidth;
			int numtabs;

			tab = strchr(cur, '\t');
			if (!tab)
				break;

			/* use tab stop as given via menu definition format string
			 * or use 1/3 of the node size (width) */
			if (!node->texh[1])
				tabwidth = width / 3;
			else
				tabwidth = node->texh[1];

			numtabs = strspn(tab, "\t");
			tabwidth *= numtabs;
			while (*tab == '\t')
				*tab++ = '\0';

			/*Com_Printf("tab - first part - lines: %i \n", lines);*/
			R_FontDrawString(font, node->align, x1, y, x, y, tabwidth-1, height, node->texh[0], cur, EXTRADATA(node).rows, EXTRADATA(node).textScroll, &lines, qfalse, LONGLINES_PRETTYCHOP);
			x1 += tabwidth;
			/* now skip to the first char after the \t */
			cur = tab;
		} while (1);

		/*Com_Printf("until newline - lines: %i\n", lines);*/
		/* the conditional expression at the end is a hack to draw "/n/n" as a blank line */
		/* prevent line from being drawn if there is nothing that should be drawn afer it */
		if (cur && (cur[0] || end || list)) {
			/* is it a white line? */
			if (!cur) {
				lines++;
			} else {
				R_FontDrawString(font, node->align, x1, y, x, y, width, height, node->texh[0], cur, EXTRADATA(node).rows, EXTRADATA(node).textScroll, &lines, qtrue, node->longlines);
			}
		}

		if (node->mousefx)
			R_ColorBlend(node->color); /* restore original color */

		/* now set cur to the next char after the \n (see above) */
		cur = end;
		if (!cur && list) {
			list = list->next;
			if (list) {
				Q_strncpyz(textCopy, (const char*)list->data, sizeof(textCopy));
				cur = textCopy;
			}
		}
	} while (cur);

	EXTRADATA(node).textLines = lines;

	MN_DrawScrollBar(node);
}

/**
 * @brief Draws the TEXT_MESSAGESYSTEM node
 * @sa MN_DrawMenus
 * @param[in] font Font string to use
 * @param[in] node The current menu node
 * @param[in] x The fixed x position every new line starts
 * @param[in] y The fixed y position the text node starts
 * @note For efficiency, scrolling is based on the count of messages
 * rather than a count of linewrapped lines. The result is that
 * scrolling of the message window scrolls message by message,
 * which looks better anyway.
 */
static void MN_TextNodeDrawMessageList (const message_t *messageStack, const char *font, menuNode_t *node, int x, int y, int width, int height)
{
	char text[TIMESTAMP_TEXT + MAX_MESSAGE_TEXT];
	const message_t *message;
	int skip_messages;
	int screenLines;

	/* scrollbar space */
	if (EXTRADATA(node).scrollbar) {
		width -= MN_SCROLLBAR_WIDTH + MN_SCROLLBAR_PADDING;
		if (EXTRADATA(node).scrollbarLeft)
			x += MN_SCROLLBAR_WIDTH + MN_SCROLLBAR_PADDING;
	}

	skip_messages = EXTRADATA(node).textScroll;
	screenLines = 0;
	for (message = messageStack; message; message = message->next) {
		if (skip_messages) {
			skip_messages--;
			continue;
		}

		Com_sprintf(text, sizeof(text), "%s%s", message->timestamp, message->text);
		R_FontDrawString(font, node->align, x, y, x, y, width, height, node->texh[0], text, EXTRADATA(node).rows, 0, &screenLines, qtrue, node->longlines);
		if (screenLines > EXTRADATA(node).rows)
			break;
	}

	if (EXTRADATA(node).scrollbar && EXTRADATA(node).rows) {
		int lines = 0;
		for (message = messageStack; message; message = message->next)
			lines++;
		/** @todo This is a hack to allow scrolling all the way
		 * to the last message, if the last page has multiline
		 * messages. Getting an accurate count of the last page
		 * would be nice, if it can be done efficiently enough. */
		if (lines > EXTRADATA(node).rows)
			lines += EXTRADATA(node).rows / 2;
		EXTRADATA(node).textLines = lines;
		MN_DrawScrollBar(node);
	}
}

/**
 * @note need a cleanup/merge/rearchitecture between MN_DrawTextNode2 and MN_DrawTextNode
 */
static void MN_TextNodeDraw (menuNode_t *node)
{
	const menu_t *menu = node->menu;
	const char *font;
	vec2_t nodepos;

	MN_GetNodeAbsPos(node, nodepos);

	if (mn.menuText[EXTRADATA(node).num]) {
		font = MN_GetFont(menu, node);
		MN_TextNodeDrawText(mn.menuText[EXTRADATA(node).num], NULL, font, node, nodepos[0], nodepos[1], node->size[0], node->size[1]);
	} else if (mn.menuTextLinkedList[EXTRADATA(node).num]) {
		font = MN_GetFont(menu, node);
		MN_TextNodeDrawText(NULL, mn.menuTextLinkedList[EXTRADATA(node).num], font, node, nodepos[0], nodepos[1], node->size[0], node->size[1]);
	} else if (EXTRADATA(node).num == TEXT_MESSAGESYSTEM) {
		font = MN_GetFont(menu, node);
		MN_TextNodeDrawMessageList(mn.messageStack, font, node, nodepos[0], nodepos[1], node->size[0], node->size[1]);
	}
}

/**
 * @brief Resets the mn.menuText pointers and the mn.menuTextLinkedList lists
 */
void MN_MenuTextReset (int menuTextID)
{
	assert(menuTextID < MAX_MENUTEXTS);
	assert(menuTextID >= 0);

	mn.menuText[menuTextID] = NULL;
	LIST_Delete(&mn.menuTextLinkedList[menuTextID]);
}


/**
 * @brief Resets the mn.menuText pointers from a func node
 * @note You can give this function a parameter to only delete a specific list
 */
static void MN_MenuTextReset_f (void)
{
	int i;

	if (Cmd_Argc() == 2) {
		i = atoi(Cmd_Argv(1));
		if (i >= 0 && i < MAX_MENUTEXTS)
			MN_MenuTextReset(i);
		else
			Com_Printf("%s: invalid mn.menuText ID: %i\n", Cmd_Argv(0), i);
	} else {
		for (i = 0; i < MAX_MENUTEXTS; i++)
			MN_MenuTextReset(i);
	}
}

/**
 * @brief Calls the script command for a text node that is clickable
 * @note The node must have the click parameter in it's menu definition or there
 * must be a console command that has the same name as the node has + _click
 * attached.
 * @sa MN_TextRightClick
 * @todo Check for scrollbars and when one would click them scroll according to
 * mouse movement, maybe implement a new mousespace (MS_* - @sa cl_input.c)
 */
static void MN_TextNodeClick (menuNode_t * node, int x, int y)
{
	int line = MN_TextNodeGetLine(node, x, y);
	char cmd[MAX_VAR];

	Com_sprintf(cmd, sizeof(cmd), "%s_click", node->name);
	if (Cmd_Exists(cmd)) {
		EXTRADATA(node).textLineSelected = line;
		Cbuf_AddText(va("%s %i\n", cmd, line));
	}
	else if (node->onClick && node->onClick->type == EA_CMD) {
		assert(node->onClick->data);
		EXTRADATA(node).textLineSelected = line;
		Cbuf_AddText(va("%s %i\n", (const char *)node->onClick->data, line));
	}
}

/**
 * @brief Calls the script command for a text node that is clickable via right mouse button
 * @note The node must have the rclick parameter
 * @sa MN_TextClick
 */
static void MN_TextNodeRightClick (menuNode_t * node, int x, int y)
{
	int line = MN_TextNodeGetLine(node, x, y);
	char cmd[MAX_VAR];

	Com_sprintf(cmd, sizeof(cmd), "%s_rclick", node->name);
	if (Cmd_Exists(cmd))
		Cbuf_AddText(va("%s %i\n", cmd, line));
}

static void MN_TextNodeMouseWheel (menuNode_t *node, qboolean down, int x, int y)
{
	if (node->onWheelUp && node->onWheelDown) {
		MN_ExecuteEventActions(node, (down ? node->onWheelDown : node->onWheelUp));
	} else {
		MN_TextScroll(node, (down ? 1 : -1));
		/* they can also have script commands assigned */
		MN_ExecuteEventActions(node, node->onWheel);
	}
}

static void MN_TextNodeLoading (menuNode_t *node)
{
	EXTRADATA(node).textLineSelected = -1; /**< Invalid/no line selected per default. */
	Vector4Set(node->selectedColor, 1.0, 1.0, 1.0, 1.0);
	Vector4Set(node->color, 1.0, 1.0, 1.0, 1.0);
}

static void MN_TextNodeLoaded (menuNode_t *node)
{
	/* auto compute lineheight */
	if (node->texh[0] == 0) {
		if (EXTRADATA(node).rows != 0 && node->size[1] != 0) {
			node->texh[0] = node->size[1] / EXTRADATA(node).rows;
		} else {
			/* the font is used */
			/** @todo clean this up once font_t is known in the client */
			const char *font = MN_GetFont(node->menu, node);
			node->texh[0] = R_FontGetHeight(font) / 2;
		}
	}

	/* auto compute rows */
	if (EXTRADATA(node).rows == 0) {
		if (node->size[1] != 0 && node->texh[0] != 0) {
			EXTRADATA(node).rows = node->size[1] / node->texh[0];
		} else {
			EXTRADATA(node).rows = 1;
			Com_Printf("MN_TextNodeLoaded: node '%s.%s' has no rows value\n", node->menu->name, node->name);
		}
	}

	/* auto compute height */
	if (node->size[1] == 0) {
		node->size[1] = EXTRADATA(node).rows * node->texh[0];
	}

	/* is text slot exists */
	if (EXTRADATA(node).num >= MAX_MENUTEXTS)
		Sys_Error("Error in node %s.%s - max menu num exeeded (num: %i, max: %i)", node->menu->name, node->name, EXTRADATA(node).num, MAX_MENUTEXTS);


#if DEBUG
	if (EXTRADATA(node).rows != (int)(node->size[1] / node->texh[0])) {
		Com_Printf("MN_TextNodeLoaded: rows value (%i) of node '%s.%s' differs from size (%.0f) and format (%.0f) values\n",
			EXTRADATA(node).rows, node->menu->name, node->name, node->size[1], node->texh[0]);
	}
#endif
}

static void MN_TextNodeInitBehaviour (nodeBehaviour_t *behaviour)
{
	Cmd_AddCommand("mn_textscroll", MN_TextScroll_f, NULL);
	Cmd_AddCommand("mn_textreset", MN_MenuTextReset_f, "Resets the mn.menuText pointers");
	Cmd_AddCommand("mn_textupdated", MN_TextUpdated_f, "Event to inform node the text is updated");
}

static const value_t properties[] = {
	{"scrollbar", V_BOOL, offsetof(menuNode_t, u.text.scrollbar), MEMBER_SIZEOF(menuNode_t, u.text.scrollbar)},
	{"scrollbarleft", V_BOOL, offsetof(menuNode_t, u.text.scrollbarLeft), MEMBER_SIZEOF(menuNode_t, u.text.scrollbarLeft)},
	{"lineselected", V_INT, offsetof(menuNode_t, u.text.textLineSelected), MEMBER_SIZEOF(menuNode_t, u.text.textLineSelected)},
	{"num", V_MENUTEXTID, offsetof(menuNode_t, u.text.num), MEMBER_SIZEOF(menuNode_t, u.text.num)},
	{"rows", V_INT, offsetof(menuNode_t, u.text.rows), MEMBER_SIZEOF(menuNode_t, u.text.rows)},
	{"text_scroll", V_INT, offsetof(menuNode_t, u.text.textScroll), MEMBER_SIZEOF(menuNode_t, u.text.textScroll)},

	{"format", V_POS, offsetof(menuNode_t, texh), MEMBER_SIZEOF(menuNode_t, texh)},
	{"longlines", V_LONGLINES, offsetof(menuNode_t, longlines), MEMBER_SIZEOF(menuNode_t, longlines)},
	{NULL, V_NULL, 0, 0}
};

void MN_RegisterTextNode (nodeBehaviour_t *behaviour)
{
	behaviour->name = "text";
	behaviour->id = MN_TEXT;
	behaviour->draw = MN_TextNodeDraw;
	behaviour->leftClick = MN_TextNodeClick;
	behaviour->rightClick = MN_TextNodeRightClick;
	behaviour->mouseWheel = MN_TextNodeMouseWheel;
	behaviour->mouseMove = MN_TextNodeMouseMove;
	behaviour->loading = MN_TextNodeLoading;
	behaviour->loaded = MN_TextNodeLoaded;
	behaviour->properties = properties;
	behaviour->initBehaviour = MN_TextNodeInitBehaviour;
}
