
// Copyright Timothy Miller, 1999

#ifndef INCLUDED_GTERM_H
#define INCLUDED_GTERM_H

#define FGCOLOR(attr) (((attr)>>4) & 7)
#define BGCOLOR(attr) (((attr)>>8) & 7)
#define FLAG(attr) ((attr) & 0x0f)

class GTerm;
typedef void (GTerm::*StateFunc)();

struct StateOption {
	int byte;	// char value to look for; -1==end/default
	StateFunc action;
	StateOption *next_state;
};

class GTerm {
public:
	// mode flags
	enum {BOLD=1, BLINK=2, UNDERLINE=4, INVERSE=8,
		NOEOLWRAP=16, CURSORAPPMODE=32, CURSORRELATIVE=64, 
		NEWLINE=128, INSERT=256, KEYAPPMODE=512,
		DEFERUPDATE=1024, DESTRUCTBS=2048, TEXTONLY=4096,
		LOCALECHO=8192, CURSORINVISIBLE=16384} MODES;

private:
	// terminal info
	int width, height, scroll_top, scroll_bot;
	unsigned char *text;
	unsigned short *color;
	char *tab_stops;
	short *linenumbers; // text at text[linenumbers[y]*MAXWIDTH]
	unsigned char* dirty_startx;
    	unsigned char* dirty_endx;
	int pending_scroll; // >0 means scroll up
	bool doing_update;
    	bool force_wrap;
    	bool inverse_mode;
	
	// terminal state
	int cursor_x, cursor_y;
	int save_x, save_y, save_attrib;
	int fg_color, bg_color;
	int mode_flags;
 	char charset[2]; //G0,G1 charset
    	int cur_charset;
	StateOption *current_state;
	static StateOption normal_state[];
    	static StateOption esc_state[];
    	static StateOption bracket_state[];
    	static StateOption q_bracket_state[];
	static StateOption cset_shiftin_state[];
    	static StateOption cset_shiftout_state[];
	static StateOption hash_state[];
	static StateOption nonstd_state[];
	static StateOption vt52_normal_state[], vt52_esc_state[];
	static StateOption vt52_cursory_state[], vt52_cursorx_state[];

	// utility functions
	void update_changes();
	void scroll_region(int start_y, int end_y, int num);	// does clear
	void shift_text(int y, int start_x, int end_x, int num); // ditto
	void clear_area(int start_x, int start_y, int end_x, int end_y);
	void changed_line(int y, int start_x, int end_x);
	void move_cursor(int x, int y);
	int calc_color(int fg, int bg, int flags);
    	void translate_charset(unsigned char *buf, unsigned char *ptr);

	// action parameters
	int nparam, param[30];
	unsigned char *input_data;
	int data_len;
    	int q_mode; //in question mode('?') when parsing
    	int got_param, quote_mode;

	// terminal actions
	void normal_input();
	void set_q_mode();
	void set_quote_mode();
	void clear_param();
	void param_digit();
	void next_param();

	// non-printing characters
	void cr(), lf(), ff(), bell(), tab(), bs();

	// escape sequence actions
	void keypad_numeric();
	void keypad_application();
	void save_cursor();
	void restore_cursor();
	void set_tab();
	void index_down();
	void index_up();
	void next_line();
	void reset();

	void cursor_left();
	void cursor_down();
	void cursor_right();
	void cursor_up();
	void cursor_position();
	void device_attrib();
	void delete_char();
	void set_mode();
	void clear_mode();
	void request_param();
	void set_margins();
	void delete_line();
	void status_report();
	void erase_display();
	void erase_line();
	void insert_line();
	void set_colors();
	void clear_tab();
	void insert_char();
	void screen_align();
	void erase_char();
    	void charset_g1();
    	void charset_g0();
    	void cset_g0_a();
    	void cset_g0_b();
    	void cset_g0_0();
    	void cset_g0_1();
    	void cset_g0_2();
    	void cset_g1_a();
    	void cset_g1_b();
    	void cset_g1_0();
    	void cset_g1_1();
    	void cset_g1_2();
    	void cursor_ypos();

	// vt52 stuff
	void vt52_cursory();
	void vt52_cursorx();
	void vt52_ident();

public:
	GTerm(int w, int h);
	virtual ~GTerm();

	// function to control terminal
	virtual void ProcessInput(int len, unsigned char *data);
	virtual void ResizeTerminal(int width, int height);
	int Width() { return width; }
	int Height() { return height; }
	virtual void Update();
	virtual void ExposeArea(int x, int y, int w, int h);
	virtual void Reset();
	
	int GetMode() { return mode_flags; }
	void SetMode(int mode) { mode_flags = mode; }
	void set_mode_flag(int flag);
	void clear_mode_flag(int flag);

	// manditory child-supplied functions
	virtual void DrawText(int fg_color, int bg_color, int flags, 
		int x, int y, int len, unsigned char *string) = 0;
	virtual void DrawCursor(int fg_color, int bg_color, int flags, 
		int x, int y, unsigned char c) = 0;

	// optional child-supplied functions
	virtual void MoveChars(int sx, int sy, int dx, int dy, int w, int h) { }
	virtual void ClearChars(int bg_color, int x, int y, int w, int h) { }
	virtual void SendBack(char *data) { }
	virtual void ModeChange(int state) { }
	virtual void Bell() { }
	virtual void RequestSizeChange(int w, int h) { }
};

#endif
