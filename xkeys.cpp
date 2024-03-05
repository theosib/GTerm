
// Copyright Timothy Miller, 1999

#include "gterm.hpp"
#include "xintfc.hpp"
#include <X11/keysym.h>

keyseq cursorkeys[] = {
	{XK_Left,  "\033[D"},
	{XK_Up,    "\033[A"},
	{XK_Right, "\033[C"},
	{XK_Down,  "\033[B"},
	{XK_KP_Left,	"\033[D"},
	{XK_KP_Up, 	"\033[A"},
	{XK_KP_Right,	"\033[C"},
	{XK_KP_Down,	"\033[B"},
	{XK_VoidSymbol, 0}};

keyseq cursorappkeys[] = {
	{XK_Left,  "\033OD"},
	{XK_Up,    "\033OA"},
	{XK_Right, "\033OC"},
	{XK_Down,  "\033OB"},
	{XK_KP_Left,	"\033OD"},
	{XK_KP_Up, 	"\033OA"},
	{XK_KP_Right,	"\033OC"},
	{XK_KP_Down,	"\033OB"},
	{XK_VoidSymbol, 0}};

keyseq keypadkeys[] = {
	{XK_KP_Divide, 	"/"},
	{XK_KP_Multiply, "*"},
	{XK_KP_Subtract, "-"},
	{XK_KP_Home, 	"\033[\000"},
	{XK_KP_7, 	"7"},
	{XK_KP_8,	"8"},
	{XK_KP_Page_Up,	"\033[5~"},
	{XK_KP_9,	"9"},
	{XK_KP_Add,	"+"},
	{XK_KP_4,	"4"},
	{XK_KP_Begin,	"\033[s"},
	{XK_KP_5,	"5"},
	{XK_KP_6,	"6"},
	{XK_KP_End,	"\033[e"},
	{XK_KP_1,	"1"},
	{XK_KP_2,	"2"},
	{XK_KP_Page_Down, "\033[6~"},
	{XK_KP_3,	"3"},
	{XK_KP_Insert,	"\033[2~"},
	{XK_KP_0,	"0"},
	{XK_KP_Delete,	"\033[3~"},
	{XK_KP_Decimal,	"."},
	{XK_KP_Enter,	"\r"},
	{XK_VoidSymbol, 0}};
	
keyseq keypadappkeys[] = {
	{XK_KP_Divide, 	"\033Oo"},
	{XK_KP_Multiply, "\033Oj"},
	{XK_KP_Subtract, "\033Om"},
	{XK_KP_Home, 	"\033[\000"},
	{XK_KP_7, 	"\033Ow"},
	{XK_KP_8,	"\033Ox"},
	{XK_KP_Page_Up,	"\033[5~"},
	{XK_KP_9,	"\033Oy"},
	{XK_KP_Add,	"\033Ok"},
	{XK_KP_4,	"\033Ot"},
	{XK_KP_Begin,	"\033[s"},
	{XK_KP_5,	"\033Ou"},
	{XK_KP_6,	"\033Ov"},
	{XK_KP_End,	"\033[e"},
	{XK_KP_1,	"\033Oq"},
	{XK_KP_2,	"\033Or"},
	{XK_KP_Page_Down, "\033[6~"},
	{XK_KP_3,	"\033Os"},
	{XK_KP_Insert,	"\033[2~"},
	{XK_KP_0,	"\033Op"},
	{XK_KP_Delete,	"\033[3~"},
	{XK_KP_Decimal,	"\033On"},
	{XK_KP_Enter,	"\033OM"},
	{XK_VoidSymbol, 0}};
	
	
keyseq otherkeys[] = {
	{XK_Home,      "\033[\000"},
	{XK_Page_Up,   "\033[5~"},
	{XK_Page_Down, "\033[6~"},
	{XK_End,       "\033[e"},
	{XK_Insert,    "\033[2~"},
	{XK_F1,	"\033[11~"},
	{XK_F2,	"\033[12~"},
	{XK_F3,	"\033[13~"},
	{XK_F4,	"\033[14~"},
	{XK_F5,	"\033[15~"},
	{XK_F6,	"\033[17~"},
	{XK_F7,	"\033[18~"},
	{XK_F8,	"\033[19~"},
	{XK_F9,	"\033[20~"},
	{XK_F10,	"\033[21~"},
	{XK_F11,	"\033[23~"},
	{XK_F12,	"\033[24~"},
	{XK_VoidSymbol, 0}};

char *find_key(KeySym keysym, keyseq *table)
{
	while (table->keysym != XK_VoidSymbol) {
		if (table->keysym == keysym) return table->str;
		table++;
	}
	return 0;
}

