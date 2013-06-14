#include "GUIclient.h"

LoginWin::LoginWin(int w, int h, const char* title ):Fl_Window (w,h,title)
{
	begin();
	color(FL_WHITE);

	name = new Fl_Input(100, 10, 140, 30, "Username:");
	pass = new Fl_Secret_Input(100, 60, 140, 30, "Password:");
	domain = new Fl_Input(100, 110, 140, 30, "Domain:");
	domain->static_value(DEFAULT_DOMAN);
	
	quit = new Fl_Button(35, 150, 70, 30, "&Quit");
	quit->callback(cb_quit, this);
	accept = new Fl_Return_Button(155, 150, 90, 30, "&Accept");
	accept->callback( cb_accept, this );
	
	end();
	resizable(this);
	show();
}
LoginWin::~LoginWin()
{
}
		
void LoginWin::cb_accept(Fl_Widget* w,void* v)
{
	( (LoginWin*)v )->cb_accept_i();

}
void LoginWin::cb_accept_i()
{
	int valid=0;
	if (name->size()>0 && pass->size()>0 && domain->size()>0) valid=1;
		
	if (valid)
	{
		exitted=0;
		My_user=name->value();
		My_pass=pass->value();
		My_domain=domain->value();
		hide();
	}
	else
	{	Fl_Window *err;
		err = new Fl_Window (200,50,"Login Error");
		err->begin();
		Fl_Button *ok;
		ok = new Fl_Return_Button(75, 10, 50, 30, "&Quit");
		ok->callback( cb_quit, err );
		ok->selection_color(FL_RED);
		err->end();
		err->position(100,140);
		err->show();
	}
}


void LoginWin::cb_quit(Fl_Widget* w,void* v)
{
	( (LoginWin*)v )->cb_quit_i();

}

void LoginWin::cb_quit_i()
{
	exitted=1;
	hide();

}



PhoneWin::PhoneWin(int w, int h, const char* title ):Fl_Window (w,h,title)
{
	begin();
	color(FL_WHITE);
		
	quit = new Fl_Button(10, 270, 230, 20, "&Quit");
	quit->callback(cb_quit, this);
	quit->labelcolor(FL_DARK_BLUE);
	quit->color(FL_RED);
	quit->labelfont(FL_TIMES_BOLD);
	quit->labelsize(10);

	trunk = new Fl_Multiline_Output(10, 230, 110, 30);
	trunk->textsize(10);
	trunk->callback(cb_trunk,this);
	
	light = new Fl_Light_Button(130,230,110,30,"Trunking Activity");
	light->labelcolor(FL_WHITE);
	light->color(FL_DARK_BLUE);
	light->callback(cb_light, this);
	light->align(FL_ALIGN_CENTER);
	light->selection_color(FL_GREEN);
	light->down_box(FL_ROUNDED_BOX);
	light->labelsize(10);

	
	status = new Fl_Multiline_Output(10, 170, 230, 50);
	status->textsize(10);
	//status->align(FL_ALIGN_TOP_LEFT);
	
	list= new Fl_Select_Browser(10,10,230,150);
	list->callback( cb_list, this );
	end();
	resizable(this);
	
	ph->addGUIObserver(*this);
	show();
}
PhoneWin::~PhoneWin()
{
}
void PhoneWin::cb_trunk(Fl_Widget* w,void* v)
{
	( (PhoneWin*)v )->cb_trunk_i(w,v);
}
void PhoneWin::cb_trunk_i(Fl_Widget* w,void* v)
{
	if (!ph->isConfigured()) 
	{
		((PhoneWin*)v)->status->value("We aren't configured yet");
		((PhoneWin*)v)->status->damage(1);
	}
	else
	{
		Fl_Multiline_Output *l = (Fl_Multiline_Output *)w;
		RTPListener* rtp = ph->getRTPListenerForActiveSession();
	
		if (rtp==NULL)
		{
			((PhoneWin*)v)->status->value("RTPListener no funciona");
			((PhoneWin*)v)->status->damage(1);
			cerr<<"PhoneWin::cb_light, RTPListener no funciona"<<endl;
		}
		else if (rtp->getStatus()==SILENCIO)
		{ 
			l->value("SILENCE" );
			((PhoneWin*)v)->light->selection_color(FL_GREEN);
			((PhoneWin*)v)->light->value(1);
		}
		else if (rtp->getStatus()==ALGUIENHABLA)
		{ 
			string m ("I LISTEN ");
			m+=rtp->getWhoIsNowTalking();
			l->value(m.c_str());
			((PhoneWin*)v)->light->selection_color(FL_RED);
			((PhoneWin*)v)->light->value(1);
		}
		else 
		{
			l->value("I TALK");
			((PhoneWin*)v)->light->selection_color(FL_YELLOW);
			((PhoneWin*)v)->light->value(1);
		}
		l->damage(1);
		l->parent()->redraw();

	}
}
void PhoneWin::cb_light(Fl_Widget* w,void* v)
{
	( (PhoneWin*)v )->cb_light_i(w,v);
}
void PhoneWin::cb_light_i(Fl_Widget* w,void* v)
{
	if (!ph->isConfigured()) 
	{
		((PhoneWin*)v)->status->value("We aren't configured yet");
		((PhoneWin*)v)->status->damage(1);
	}
	else
	{
		Fl_Light_Button* fllb = ((Fl_Light_Button*)w);
		RTPListener* rtp = ph->getRTPListenerForActiveSession();
		/*
		if status==SILENCE (the light was green) and i was clicked, then i startTalking and put the status to IAMTALKING and the light in YELLOW
		
		if status==IAMTALKING (yellow light) and i was clicked, then i stopTalking and put the status to SILENCE and light green
		
		always check the status at the end to see if status might be ACTIVE, and if so, the light should be RED and status=ACTIVE
		*/
		if (rtp!=NULL)	rtp->attachObserver(*this);//TRUCO PARA FORZAR EL REFRESCO DE LA PANTALLA CUANDO HAY UN CAMBIO DE TRUNKING STATE
		
		if (rtp==NULL) 
		{
			((PhoneWin*)v)->status->value("RTPListener no funciona");
			((PhoneWin*)v)->status->damage(1);
			cerr<<"PhoneWin::cb_light, RTPListener no funciona"<<endl;
		}	
		
		else if (rtp->getStatus()==SILENCIO)
		{
			fllb->selection_color(FL_YELLOW);
			fllb->value(1);
			fllb->damage(1);
			cout <<"NADIE HABLA, AHORA HABLARE YO"<<endl;
			
			if (rtp->startTalking()) ((PhoneWin*)v)->trunk->value("I TALK");
			else 
			{
				string m ("I LISTEN ");
				m+=rtp->getWhoIsNowTalking();
				((PhoneWin*)v)->trunk->value(m.c_str());
			}
		}
		else if (rtp->getStatus()==YOHABLO)
		{
			fllb->selection_color(FL_GREEN);
			fllb->value(1);
			fllb->damage(1);
			cout <<"DEJO DE HABLAR"<<endl;
			rtp->stopTalking();
			((PhoneWin*)v)->trunk->value("SILENCE");
		}
		else if (rtp->getStatus()==ALGUIENHABLA)
		{
			cout <<"ALGUIEN HABLA"<<endl;
			fllb->selection_color(FL_RED);
			fllb->value(1);
			fllb->damage(1);
			string m ("I LISTEN ");
			m+=rtp->getWhoIsNowTalking();
			((PhoneWin*)v)->trunk->value(m.c_str());
		}
		else 
		{
			cerr<<"cb_list_i ELSE, vamos, no tiene sentido"<<endl;
			cerr.flush();
			cout <<rtp->getStatus()<<" las variables valen "<<SILENCIO<<" "<<YOHABLO<<" "<<ALGUIENHABLA<<endl;
		}
		
		((PhoneWin*)v)->trunk->damage(1);		
	
		fllb->parent()->redraw();
	}
}

void PhoneWin::cb_list(Fl_Widget* w,void* v)
{
	( (PhoneWin*)v )->cb_list_i(w,v);
}
void PhoneWin::cb_list_i(Fl_Widget* w,void* v)
{
	Fl_Select_Browser *l = (Fl_Select_Browser *)w;
	int n = l->value();
	string group;
	cout <<"Evento Lista"<<endl;
	if (!ph->isConfigured()) 
	{
		((PhoneWin*)v)->status->value("We aren't configured yet");
		((PhoneWin*)v)->status->damage(1);
	}
	else if (n<=0)
	{
		((PhoneWin*)v)->status->value("Invalid entry selected");
		((PhoneWin*)v)->status->damage(1);
		//cout <<"Ha clickado en una entrada invalida"<<endl;
	}
	else 
	{
		cout <<"Evento Lista OK   -- "<<n<<endl;
		const char *c = (l->text(n));
		group+=c;
		
	
		if (group.find("@")==string::npos)
		{
			((PhoneWin*)v)->status->value("Invalid entry selected");
			((PhoneWin*)v)->status->damage(1);
			//cout <<"Ha clickado en una entrada invalida"<<endl;
		}
		else
		{
			string msg ("Connected to group:\n");
			msg += group;
			((PhoneWin*)v)->status->value(msg.data());
			((PhoneWin*)v)->status->damage(1);
			cout <<"Grupo escogido de la lista es "<<group<<endl;
			string grp = SipUserData::extractUserFromAddress(group); //converts "group@domain.com" into "group"
			ph->joinGroup(grp); //joinGroup needs "group", not "group@domain.com"
		}
	}
	ofstream file("/tmp/groups.tmp.001");
	//file << "uno\ndos\ntres\ncuatro\n";
	file << ph->dumpGroups().data();
	
	file.close();
	
	l->load("/tmp/groups.tmp.001");
}

void PhoneWin::cb_quit(Fl_Widget* w,void* v)
{
	( (PhoneWin*)v )->cb_quit_i();

}

void PhoneWin::cb_quit_i()
{
	hide();
}
void PhoneWin::update(Observable* o, Argument* arg)
{
	RTPListener *rtp;
	//cerr << "PhoneWindow :: Rcvd update: " << arg->n <<" " <<arg->s <<endl;
	switch (arg->n)
	{
		case Observable::TXTMSG_RECEIVED: 
		{	
			cout <<"Received TXT msg "<<arg->s<<endl;
			string txt ("Message Received:\n");
			txt += arg->s;
			status->value(txt.data());
			status->damage(1);
		}
		break;
		case Observable::TRUNKING_EVENT:
		{
			if (ph->isConfigured() && (rtp=ph->getRTPListenerForActiveSession())!=NULL)
			{
				if (rtp->getStatus()==SILENCIO)
				{ 
					trunk->value("SILENCE" );
					light->selection_color(FL_GREEN);
				}
				else if (rtp->getStatus()==ALGUIENHABLA)
				{ 
					string m ("I LISTEN ");
					m+=rtp->getWhoIsNowTalking();
					trunk->value(m.c_str());
					light->selection_color(FL_RED);
				}
				else if (rtp->getStatus()==YOHABLO)
				{
					trunk->value("I TALK");
					light->selection_color(FL_YELLOW);
					
				}
				trunk->damage(1);
				light->value(1);
			}
		}
		break;
	}
	redraw();
}
int main()
{
	LoginWin win(280,200,"IPTrunking Login");
	Fl::run();
	cout<<"after win1"<<endl;
	cout.flush();
	if (exitted!=1)
	{
		ph = new trunkClient(My_user, My_domain, My_pass,"Custom User");
		//cout<< ph->isConfigured()<<endl;
		//win.hide();
		PhoneWin win2(250,300,"IPTrunking Phone");
		Fl::run();
		cout<<"after win2"<<endl;
	}
	
	if (ph!=NULL) delete ph;
	ph=NULL;
	//return Fl::run();
	
	return 0;
}


