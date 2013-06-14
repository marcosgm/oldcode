/***************************************************************************
 *   Copyright (C) 2005 by Marcos Garcia, arboRDI S.L.                     *
 *   marcos.gm@gmail.com                                                   *
 ***************************************************************************/
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
using namespace std;

//---------------------------------------------------
 
class SimpleWindow : public Fl_Window{
 
	public:
		SimpleWindow(int w, int h, const char* title );
		~SimpleWindow();
		Fl_Button* copy;
		Fl_Button* quit;
		Fl_Input* inp;
		Fl_Output* out;
   
	private:
		static void cb_copy(Fl_Widget*, void*);
		inline void cb_copy_i();
 
		static void cb_quit(Fl_Widget*, void*);
		inline void cb_quit_i();
};

//----------------------------------------------------

int main (){
 
	SimpleWindow win(300,200,"SimpleWindow");
	return Fl::run();
}

//----------------------------------------------------

SimpleWindow::SimpleWindow(int w, int h, const char* title):Fl_Window(w,h,title){
   
	begin();
	copy = new Fl_Button( 10, 150, 70, 30, "C&opy");
	copy->callback( cb_copy, this );
    
	quit = new Fl_Button(100, 150, 70, 30, "&Quit");
	quit->callback(cb_quit, this);
  
	inp = new Fl_Input(50, 50, 140, 30, "Input:");
	out = new Fl_Output(50, 100, 140, 30, "Output:");
	end();
	resizable(this);
	show();
}

//----------------------------------------------------

SimpleWindow::~SimpleWindow(){}

//----------------------------------------------------

void SimpleWindow::cb_copy(Fl_Widget* o, void* v) {
 
   //SimpleWindow* T=(SimpleWindow*)v;
   //T->cb_copy_i();
   
   // or just the one line below
	( (SimpleWindow*)v )->cb_copy_i();
}


void SimpleWindow::cb_copy_i() {

	out->value(inp->value()); 
}

//----------------------------------------------------

void SimpleWindow::cb_quit(Fl_Widget* , void* v) {

	( (SimpleWindow*)v )->cb_quit_i();
}


void SimpleWindow::cb_quit_i() {

	hide();
}

//----------------------------------------------------