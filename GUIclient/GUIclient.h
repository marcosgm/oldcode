/***************************************************************************
 *   Copyright (C) 2005 by Marcos Garcia, arboRDI S.L.                     *
 *   marcos.gm@gmail.com                                                   *
 ***************************************************************************/

#include "trunkclient.h"
#include <fstream>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/Fl_Secret_Input.H>
#include <FL/Fl_Select_Browser.H>
#include <FL/Fl_Light_Button.H>

using namespace PhoneMgr;
#define DEFAULT_DOMAN "sip.domain.com"


/* We need two windows: login screen and phone screen
Login screen must have 3 inputs...
	Name
	Password
	Domain (default text)
...and 2 buttons
	Quit
	Accept
	
Phone screen must have ...
	TextList (group list) //Change_Group Button NOT NECESSARY
	Status TextField (showing where are we connected)
	Light Button (3 colors, RED when medium is free, RED when somebody is talking and BLUE when we are talking)
	Quit Button
*/
int exitted=1;
string My_user, My_domain, My_pass; //are global bcause are obtained in 1st window but needed in 2nd window.
trunkClient *ph=NULL;

class LoginWin: public Fl_Window
{
	public:
		LoginWin(int w, int h, const char* title );
		~LoginWin();
		Fl_Return_Button* accept;
		Fl_Button* quit;
		Fl_Input* name;
		Fl_Secret_Input* pass;
		Fl_Input* domain;
	private:
		static void cb_accept(Fl_Widget*, void*);
		inline void cb_accept_i();
		static void cb_quit(Fl_Widget*, void*);
		inline void cb_quit_i();
};

class PhoneWin: public Fl_Window, public PhoneMgr::Observer //	Receive Argument arg(Observable::TXTMSG_RECEIVED,command)
{
	public:
		PhoneWin(int w, int h, const char* title);
		~PhoneWin();
		Fl_Button* quit;
		Fl_Select_Browser* list;
		Fl_Multiline_Output* status;
		Fl_Multiline_Output* trunkStatus;
		Fl_Light_Button* trunk;
		Fl_Light_Button* emerg;
		void update(Observable* o, Argument* arg);
		void updateWidgets();

	private:
		string group;
		string associatedGroup;
		
		static void cb_main(Fl_Widget*, void*);
		
		static void cb_quit(Fl_Widget*, void*);
		inline void cb_quit_i();
		static void cb_list(Fl_Widget*, void*);
		inline void cb_list_i(Fl_Widget* w,void* v);
		static void cb_trunk(Fl_Widget*, void*);
		inline void cb_trunk_i(Fl_Widget* w,void* v);
		static void cb_emerg(Fl_Widget*, void*);
		inline void cb_emerg_i(Fl_Widget* w,void* v);
};


/*list->add("Paracetamol");
	list->add("1");
	list->add("a");
	list->add("d");
	list->add("G");
	list->add("r");
	list->add("5");
	list->add("`");
	list->add("+");
	list->add("2");
	list->add("g");
	list->add("K");
	list->add("¿");
	*/
	
	
	//i686-pc-linux-gnu-g++ -I/usr/include/fltk-1.1 -I/usr/include/freetype2 -O2 -march=pentium4 -fomit-frame-pointer -o GUIclient GUIclient.cpp -L/usr/lib/fltk-1.1 /usr/lib/fltk-1.1/libfltk.a -lXft -lpthread -lm -lXext -lX11 -lsupc++ -Wall -pedantic && ./GUIclient

