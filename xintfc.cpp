
// Copyright Timothy Miller, 1999

#include "pseudo.hpp"
#include "gterm.hpp"
#include "xintfc.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

Display *display;
Window window, root;
GC gc;
int screen;
unsigned long fgcolor, bgcolor;
Colormap cmap;
int master_fd;

XFontStruct *font_info = NULL;
int font_height, font_width;

XFontStruct *load_font(char *name)
{       
        XFontStruct *inf;

	if (font_info) XFreeFont(display, font_info);

        inf = XLoadQueryFont(display, name);
        if (!inf) {
                fprintf(stderr, "Can't load this font\n");
                exit(-1);
        }
        font_info = inf;
	font_width = inf->max_bounds.width;
	font_height = inf->ascent + inf->descent;

        return inf;
}

int colors[8], cursor_color;

int alloccolor(int red, int green, int blue)
{
	XColor col;

	col.red = red;
	col.green = green;
	col.blue = blue;
	XAllocColor(display, cmap, &col);
	XFlush(display);
	return col.pixel;
}

void set_hints(Window d, int w, int h)
{
	XSizeHints hints;

	hints.width_inc = w;
	hints.height_inc = h;
	hints.flags = PResizeInc;
	XSetWMNormalHints(display, d, &hints);
}

int init_x(char *winname)
{
	display = XOpenDisplay("");
	if (!display) return -1;
	screen = DefaultScreen(display);
	root = RootWindow(display, screen);

	bgcolor = WhitePixel(display, screen);
	fgcolor = BlackPixel(display, screen);
	gc = XCreateGC(display, root, 0, 0);
	
        font_info = load_font("6x13");
        XSetFont(display, gc, font_info->fid);

	window = XCreateSimpleWindow(display, root,
		1, 1, 80*font_width, 24*font_height, 1, bgcolor, fgcolor);

	set_hints(window, font_width, font_height);

	XSetBackground(display, gc, bgcolor);
	XSetForeground(display, gc, fgcolor);

	cmap = XDefaultColormapOfScreen(DefaultScreenOfDisplay(display));
	colors[0] = alloccolor(0, 0, 0);
	colors[1] = alloccolor(65535, 0, 0);
	colors[2] = alloccolor(0, 65535, 0);
	colors[3] = alloccolor(65535, 65535, 0);
	colors[4] = alloccolor(0, 0, 65535);
	colors[5] = alloccolor(65535, 0, 65535);
	colors[6] = alloccolor(0, 65535, 65535);
	colors[7] = alloccolor(65535, 65535, 65535);
	cursor_color = alloccolor(32768, 32768, 32768);

	XMapRaised(display, window);

        XSelectInput(display, window, ExposureMask | KeyPressMask |
		StructureNotifyMask | KeyPressMask | StructureNotifyMask);

	XStoreName(display, window, winname);

	XFlush(display);

	return ConnectionNumber(display);
}

void bye_x()
{
	XFreeGC(display, gc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

void key_hit(GTerm *t, KeySym keysym, char *kbuf, int count)
{
	int mode;
	char *str = 0, *crlf = "\r\n";

	mode = t->GetMode();

	if (mode & GTerm::KEYAPPMODE) {
		str = find_key(keysym, keypadappkeys);
	} else {
		str = find_key(keysym, keypadkeys);
	}

	if (!str) {
		if (mode & GTerm::CURSORAPPMODE) {
			str = find_key(keysym, cursorappkeys);
		} else {
			str = find_key(keysym, cursorkeys);
		}
	}

	if (!str) {
		str = find_key(keysym, otherkeys);
	}

	if (!str && !count) return;

	if (!str) {
		str = kbuf;
		str[count] = 0;
	}

	if ((mode & GTerm::NEWLINE) && str[0] == 13 && str[1] == 0) 
		str = crlf;
	count = strlen(str);
	write(master_fd, str, count);
	if (mode & GTerm::LOCALECHO) 
		t->ProcessInput(count, (unsigned char *)str);
}

int check_event(GTerm *t)
{
        XEvent event;
	int x1, y1, x2, y2;
        KeySym keysym;
        char kbuf[20];
        int charcount;
	int ret = 0;

	while (XCheckMaskEvent(display, -1, &event)) {
	    	switch (event.type) {
	    	case MappingNotify:
			XRefreshKeyboardMapping((XMappingEvent *)&event);
			break;
		case KeyPress:
                        charcount = XLookupString((XKeyEvent *)&event, kbuf,
                                20, &keysym, 0);
			key_hit(t, keysym, kbuf, charcount);
			if (t->GetMode() & GTerm::LOCALECHO) ret = 1;
			break;
	    	case Expose:
	    	case GraphicsExpose:
			x1 = event.xexpose.x / font_width;
			y1 = event.xexpose.y / font_height;
			x2 = (event.xexpose.x+event.xexpose.width-1)/font_width;
			y2 = (event.xexpose.y+event.xexpose.height-1)/font_height;
			t->ExposeArea(x1, y1, x2-x1+1, y2-y1+1);
			ret = 1;
			break;
		case ConfigureNotify:
			t->ResizeTerminal(event.xconfigure.width / font_width,
				event.xconfigure.height / font_height);
			break;
	    	}
	}

	return ret;
}

void XTerm::DrawText(int fg_color, int bg_color, int flags,
                int x, int y, int len, unsigned char *string)
{
	int t;

	if (fg_color == 0 && (flags&BOLD)) {
		fg_color = 7;
		flags &= ~BOLD;
	}

	if (fg_color == 0 && (flags&BOLD)) {
		fg_color = 7;
		flags &= ~BOLD;
	}
	if (flags & INVERSE) {
		t = fg_color;
		fg_color = bg_color;
		bg_color = t;
	}

	XSetBackground(display, gc, colors[bg_color]);
	XSetForeground(display, gc, colors[fg_color]);

	XDrawImageString(display, window, gc, x*font_width, y*font_height+font_info->ascent,
		(char *)string, len);

	if (flags&BOLD) 
	    XDrawString(display, window, gc, x*font_width+1, y*font_height+font_info->ascent,
		(char *)string, len);

	if (flags&UNDERLINE)
	    XDrawLine(display, window, gc, x*font_width, y*font_height+font_height-1, (x+len)*font_width-1, y*font_height+font_height-1);
}

void XTerm::DrawCursor(int fg_color, int bg_color, int flags,
                int x, int y, unsigned char c)
{
	unsigned char str[2];

	str[0] = c;
	str[1] = 0;
	
	DrawText(7-fg_color, 7-bg_color, flags, x, y, 1, str);

#if 0
	XSetForeground(display, gc, cursor_color);
	XDrawLine(display, window, gc, x*font_width, y*font_height, x*font_width+font_width-1, y*font_height);
	XDrawLine(display, window, gc, x*font_width+font_width-1, y*font_height, x*font_width+font_width-1, y*font_height+font_height-1);
	XDrawLine(display, window, gc, x*font_width+font_width-1, y*font_height+font_height-1, x*font_width, y*font_height+font_height-1);
	XDrawLine(display, window, gc, x*font_width, y*font_height+font_height-1, x*font_width, y*font_height);
#endif
}

void XTerm::MoveChars(int sx, int sy, int dx, int dy, int w, int h)
{
	XCopyArea(display, window, window, gc,
		sx*font_width, sy*font_height, w*font_width, h*font_height, dx*font_width, dy*font_height);

	// Must always check for GraphicsExpose events immediately
	XSync(display, False);
	check_event(this);
}

void XTerm::ClearChars(int bg_color, int x, int y, int w, int h)
{
	XSetForeground(display, gc, colors[bg_color]);
	XFillRectangle(display, window, gc, x*font_width, y*font_height, w*font_width, h*font_height);
}

void XTerm::SendBack(char *data)
{
	write(master_fd, data, strlen(data));
}

void XTerm::Bell()
{
	XBell(display, 0);
}

void XTerm::RequestSizeChange(int w, int h)
{
	if (w != Width() || h != Height()) {
		XResizeWindow(display, window, w*font_width, h*font_height);
		ResizeTerminal(w, h);
	}
}

static volatile int got_sigchild = 0;

static void sigchild_handler(int signum)
{
	got_sigchild++;
}

int main()
{
	unsigned char buf[1000];
	XTerm term;
	fd_set rset;
	PseudoTerminal p;
	int retval = 0,xfd, high, i, ret, need_timeout = 1;
	struct timeval tv;
	struct sigaction child_action;

	/* setup X11 */
	xfd = init_x("Terminal");
	if (xfd<0) exit(-1);

	child_action.sa_handler = sigchild_handler;
	sigemptyset (&child_action.sa_mask);
	child_action.sa_flags |= SA_NOCLDSTOP;
	sigaction(SIGCHLD,&child_action,NULL);

	/* spawn shell in pseudo terminal */
	if(!p.init() || !p.spawn("/bin/sh")) {
            fprintf(stderr, 
                "Error: Could not open pty or could not spawn shell.\n");
	    exit(-1);
        }
	master_fd = p.get_master();

	/* we want non-blocking reads from pty output */
	fcntl(master_fd, F_SETFL, O_NONBLOCK);

	/* configure terminal for deferred display updates */
	term.set_mode_flag(GTerm::DEFERUPDATE);

	i = 1;
	while (i>=0) {
		/* flush Xlib buffer and then check for any new events */
		XFlush(display);
		if (check_event(&term)) need_timeout = 1;

		/* select on input from terminal and x server */
		FD_ZERO(&rset);
		FD_SET(master_fd, &rset);     high = master_fd;
		FD_SET(xfd, &rset);     if (xfd>high) high=xfd;

		/* delay for terminal update */
		tv.tv_sec = 0;
		tv.tv_usec = 10000;

		ret = select(high+1, &rset, NULL, NULL,
			need_timeout ? (&tv) : NULL);

		if (!ret) {	/* timer expired */
			term.Update();
			need_timeout = 0;
		}

		if (FD_ISSET(master_fd, &rset)) {
			i = read(master_fd, buf, 1000);
			if (i>0) term.ProcessInput(i, buf);
			need_timeout = 1;
		}
		
		while(got_sigchild) {
			got_sigchild = 0;
			if(waitpid(p.get_pid(),&retval,WNOHANG)==p.get_pid() &&
			   (WIFEXITED(retval)||WIFSIGNALED(retval))) {
			   	i = -1;
			}
		}

		/* no need to check xfd since that is done
		unconditionally at top of loop */
	}

	p.done();
	return retval;
}

