
// Copyright Timothy Miller, 1999

#ifndef INCLUDED_XINTFC_H
#define INCLUDED_XINTFC_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

class XTerm : public GTerm {
public:
       XTerm() : GTerm(80, 24) { /*SetMode(TEXTONLY);*/ }

        void DrawText(int fg_color, int bg_color, int flags,
                int x, int y, int len, unsigned char *string);
        void DrawCursor(int fg_color, int bg_color, int flags,
                int x, int y, unsigned char c);

        void MoveChars(int sx, int sy, int dx, int dy, int w, int h);
        void ClearChars(int bg_color, int x, int y, int w, int h);
	void SendBack(char *data);
	void Bell();
	void RequestSizeChange(int w, int h);
};

typedef struct {
        KeySym keysym;
        char *str;
} keyseq;

extern keyseq cursorkeys[];
extern keyseq cursorappkeys[];
extern keyseq keypadkeys[];
extern keyseq keypadappkeys[];
extern keyseq otherkeys[];

char *find_key(KeySym keysym, keyseq *table);


// stuff related to pseudo terminals
extern int master_fd;
int spawn(char *exe);
void restore_ttyp();

#endif
