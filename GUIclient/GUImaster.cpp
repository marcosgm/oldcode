/***************************************************************************
 *   Copyright (C) 2005 by Marcos Garcia, arboRDI S.L.                     *
 *   marcos.gm@gmail.com                                                   *
 ***************************************************************************/
#include "trunkmaster.h"
#include <fstream>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Output.H>

using namespace PhoneMgr;
#define DEFAULT_DOMAN "sip.domain.com"
string My_user, My_domain, My_pass; //are global bcause are obtained in 1st window but needed in 2nd window.
trunkMaster *ph=NULL;

class Updater;
class MonitorWin: public Fl_Window, public PhoneMgr::Observer
{
	public:
		MonitorWin(int w, int h, const char* title );
		~MonitorWin();
		Fl_Multiline_Output* list;
		Fl_Button* quit;
		void update(Observable* o, Argument* arg);

	private:
		static void cb_list(Fl_Widget*, void*);
		inline void cb_list_i(Fl_Widget* w,void* v);
		static void cb_quit(Fl_Widget*, void*);
		inline void cb_quit_i();
};

MonitorWin::MonitorWin(int w, int h, const char* title ):Fl_Window (w,h,title)
{
	begin();
	color(FL_WHITE);

	list = new Fl_Multiline_Output(10, 10, 260, 160);
	list->textsize(11);
	list->callback (cb_list, this);
	quit = new Fl_Button(10, 170, 260, 20, "&Quit");
	quit->callback(cb_quit, this);
	
	end();
	resizable(this);
	ph->addGUIObserver(*this);

	show();

}
MonitorWin::~MonitorWin()
{
}


void MonitorWin::cb_list(Fl_Widget* w,void* v)
{
	( (MonitorWin*)v )->cb_list_i(w,v);
}

void MonitorWin::cb_list_i(Fl_Widget* w,void* v)
{
	Fl_Multiline_Output *l = (Fl_Multiline_Output *)w;
	l->value( ph->dumpUsersInGroupList().data());
	l->damage(1);
}

void MonitorWin::cb_quit(Fl_Widget* w,void* v)
{
	( (MonitorWin*)v )->cb_quit_i();

}

void MonitorWin::cb_quit_i()
{	
	hide();
}

void MonitorWin::update(Observable* o, Argument* arg)
{
	//cerr << "MonitorWin :: Rcvd update: " << Observable::getEventString(arg->n) <<" " <<arg->s <<endl;
	switch (arg->n)
	{
		case Observable::INVITE_RECEIVED: //trunkMaster
		case Observable::BYE_RECEIVED: //trunkMaster
		{	
			cout <<"Received INVITE or BYE msg "<<arg->s<<endl;
			list->value( ph->dumpUsersInGroupList().data());
			list->damage(1);
			redraw();
		}
		break;
	}
	//Observable::TXTMSG_RECEIVED

}

int main()
{
	string admin("administrador");
	ph = new trunkMaster(admin,"sip.domain.com","1234","Site Admin", "http://www.domain.com:8080/serv.gestion/servlet/ServletXML?XMLGroup=");
	MonitorWin win(280,200,"IPTrunking Master");
	Fl::run();
	cout <<"End"<<endl;
	cout.flush();

	//cout<< ph->isConfigured()<<endl;
	//cout <<ph->dumpUsersCurrentlyInGroups()<<endl;
	//cout << ph->dumpUsersInGroupList()<<endl;
	//cout<< ph->isConfigured()<<endl;
	//win.hide();
	
	if (ph!=NULL) delete ph;
	ph=NULL;
	//return Fl::run();
	
	return 0;
}


