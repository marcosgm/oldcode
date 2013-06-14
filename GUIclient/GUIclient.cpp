#include "GUIclient.h"

//for SIGNAL
#include <unistd.h>     /* standard unix functions, like getpid()       */
#include <sys/types.h>  /* various type definitions, like pid_t         */
#include <signal.h>

void catch_trunk(int sig_num)
{
	/* re-set the signal handler again to catch_int, for next time */
	signal(SIGUSR2, catch_trunk);
	/* and print the message */
	printf("Trunking by SIGNAL USR2");
	if (ph->getState() == SILENCE+ASSOCIATED) ph->touch(psm::TRUNK_I_ON);
	else if (ph->getState() == TALKING+ASSOCIATED) ph->touch(psm::TRUNK_I_OFF);
	
	fflush(stdout);
  
}

void catch_emerg(int sig_num)
{
	/* re-set the signal handler again to catch_int, for next time */
	signal(SIGUSR1, catch_emerg);
	/* and print the message */
	printf("Emergency by SIGNAL USR1");
	if (ph->getState() == TALKING+UNASSOCIATED) ph->touch(psm::EMERG_I_OFF);
	else if (ph->getState() != LISTENING+UNASSOCIATED) ph->touch(psm::EMERG_I_ON);
	
	fflush(stdout);
  
}
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
	quit->box(FL_PLASTIC_UP_BOX);
	accept = new Fl_Return_Button(155, 150, 90, 30, "&Accept");
	accept->callback( cb_accept, this );
	accept->box(FL_PLASTIC_UP_BOX);
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
		ok->box(FL_PLASTIC_UP_BOX);
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

void timer_handler(void*)
{
	//updateWidgets();
	cout<<"Callb"<<endl;
	Fl::repeat_timeout(2.0,timer_handler);
};

PhoneWin::PhoneWin(int w, int h, const char* title ):Fl_Window (w,h,title)
{
	begin();
	color(FL_WHITE);
	
	callback(cb_main, this);
		
	quit = new Fl_Button(10, 270, 110, 20, "&Quit");
	quit->callback(cb_quit, this);
	quit->labelcolor(FL_DARK_BLUE);
	quit->color(FL_RED);
	quit->box(FL_PLASTIC_UP_BOX);
	quit->labelfont(FL_TIMES_BOLD);
	quit->labelsize(10);

	trunkStatus = new Fl_Multiline_Output(10, 230, 110, 30);
	trunkStatus->textsize(10);
	//Fl::add_timeout(2.0,timer_handler);
		
	trunk = new Fl_Light_Button(130,230,110,60,"Trunk\n Activity");
	trunk->labelcolor(FL_WHITE);
	trunk->color(FL_DARK_BLUE);
	trunk->callback(cb_trunk, this);
	trunk->align(FL_ALIGN_CENTER);
	trunk->selection_color(FL_RED);
	trunk->down_box(FL_ROUND_UP_BOX);
	trunk->box(FL_PLASTIC_UP_BOX);
	trunk->labelsize(12);
	trunk->shortcut ('t');
	//trunk->shortcut (0x1008ff1c); //codigo X del boton de "Record" de la PDA --> no funciona en GPE
	trunk->when(FL_WHEN_CHANGED);
	
	emerg = new Fl_Light_Button(220, 170, 25, 50,"");
	emerg->color(FL_RED);
	emerg->callback(cb_emerg, this);
	emerg->selection_color(FL_GREEN);
	emerg->down_box(FL_ROUND_UP_BOX);//FL_PLASTIC_UP_BOX
	emerg->box(FL_PLASTIC_UP_BOX);
	emerg->labelsize(12);
	emerg->shortcut ('e');
	emerg->when(FL_WHEN_CHANGED);
	
	status = new Fl_Multiline_Output(10, 170, 210, 50);
	status->textsize(10);
	//status->align(FL_ALIGN_TOP_LEFT);
	
	list= new Fl_Select_Browser(10,10,230,150);
	list->callback( cb_list, this );
	end();
	resizable(this);
	
	ph->addGUIObserver(*this);
	show();
	
	ofstream file("/tmp/groups.tmp.001");
	//file << "uno\ndos\ntres\ncuatro\n";
	file << ph->dumpGroups().data();
	file.close();
	list->load("/tmp/groups.tmp.001");
	list->add("NO GROUP", NULL);
	
	updateWidgets();
	
	ofstream pid("/tmp/GUIclient.pid");
	//file << "uno\ndos\ntres\ncuatro\n";
	pid << getpid();
	pid.close();
	
	signal(SIGUSR2, catch_trunk); //Set the handler for  trunking with the button in the PDA (uses "kill 'GUIclient.pid' -s 12" --> 12 is known with kill -l SIGUSR2)
	
	signal(SIGUSR1, catch_emerg); //Set the handler for  emergency with the button in the PDA (uses "kill 'GUIclient.pid' -s 10" --> 12 is known with kill -l SIGUSR1)

}
PhoneWin::~PhoneWin()
{
}
void PhoneWin::updateWidgets()
{

	if (!ph->isConfigured()) 
	{
		status->value("We aren't configured yet");
		status->damage(1);
		return;
	}
	if (ph->weAreAssociated())
	{
		trunkStatus->color(FL_WHITE);
		if (ph->getVoiceState()==TALKING)
		{ 
			trunkStatus->value("I TALK");
			trunk->selection_color(FL_YELLOW);
			emerg->selection_color(FL_RED);

		}
		else if (ph->getVoiceState()==LISTENING)
		{ 
			string m ("I LISTEN ");
			RTPListener *rtp=NULL;
			rtp = ph->getRTPListenerForActiveSession();
			if (rtp!=NULL) m+=rtp->getWhoIsNowTalking();
			trunkStatus->value(m.c_str());
			trunk->selection_color(FL_RED);
			emerg->selection_color(FL_GREEN);
		}
		else if (ph->getVoiceState()==SILENCE)
		{
			trunkStatus->value("SILENCE");
			trunk->selection_color(FL_GREEN);
			emerg->selection_color(FL_GREEN);
		}
		else trunkStatus->value(ph->dumpStateString().c_str()); //MAY NEVER ENTER HERE
		
		string msg ("Connected to group:\n");
		msg += associatedGroup;
		status->value(msg.data());
		status->damage(1);
	}
	else //we are not associated, only EMERGENCY
	{
		trunkStatus->color(FL_RED);
		if (ph->getVoiceState()==TALKING)
		{ 
			trunkStatus->value("EMERGENCY!!");
			trunk->selection_color(FL_RED);
			emerg->selection_color(FL_YELLOW);
		}
		else if (ph->getVoiceState()==LISTENING)
		{ 
			string m ("EMERGENCY: ");
			RTPListener *rtp=NULL;
			rtp = ph->getRTPEmergency();
			if (rtp!=NULL) m+=rtp->getWhoIsNowTalking();
			trunkStatus->value(m.c_str());
			trunk->selection_color(FL_RED);
			emerg->selection_color(FL_RED);
		}
		else if (ph->getVoiceState()==SILENCE)
		{
			trunkStatus->value("NO ACTIVITY");
			trunk->selection_color(FL_RED);
			emerg->selection_color(FL_GREEN);
		}
		else trunkStatus->value(ph->dumpStateString().c_str()); //MAY NEVER ENTER HERE

	}
	trunk->value(1);
	emerg->value(1);
	trunkStatus->damage(1);
	redraw();	
	damage(1);
}
void PhoneWin::cb_main(Fl_Widget* w,void* v)
{
	( (PhoneWin*)v )->updateWidgets();
}
void PhoneWin::cb_trunk(Fl_Widget* w,void* v)
{
	( (PhoneWin*)v )->cb_trunk_i(w,v);
}
void PhoneWin::cb_trunk_i(Fl_Widget* w,void* v)
{
	Fl_Light_Button* fllb = ((Fl_Light_Button*)w);
	if (!ph->isConfigured()) 
	{
		((PhoneWin*)v)->status->value("We aren't configured yet");
		((PhoneWin*)v)->status->damage(1);
		return;
	}

	if (ph->getState() == SILENCE+ASSOCIATED) ph->touch(psm::TRUNK_I_ON);
	else if (ph->getState() == TALKING+ASSOCIATED) ph->touch(psm::TRUNK_I_OFF);
	else 
	{
		cout<<"Trunking pressed when it was not allowed, "<<ph->dumpStateString()<<endl; 
		fllb->value(1);
		fllb->damage(1);
		return;
	}
	updateWidgets();
}

void PhoneWin::cb_emerg(Fl_Widget* w,void* v)
{
	( (PhoneWin*)v )->cb_emerg_i(w,v);
}
void PhoneWin::cb_emerg_i(Fl_Widget* w,void* v)
{
	Fl_Light_Button* efllb = ((Fl_Light_Button*)w);

	if (ph->getVoiceState() == SILENCE || ph->getState() == ASSOCIATED+LISTENING) ph->touch(psm::EMERG_I_ON);
	else if (ph->getState() == UNASSOCIATED+TALKING) ph->touch(psm::EMERG_I_OFF);
	else 
	{
		cout<<"Emergency pressed when it was not allowed "<<ph->dumpStateString()<<endl; 
		efllb->value(1);
		efllb->damage(1);
		return;
	}
	updateWidgets();
}

void PhoneWin::cb_list(Fl_Widget* w,void* v)
{
	( (PhoneWin*)v )->cb_list_i(w,v);
}
void PhoneWin::cb_list_i(Fl_Widget* w,void* v)
{
	Fl_Select_Browser *l = (Fl_Select_Browser *)w;
	int n = l->value();
	//cout <<"Evento Lista"<<endl;
	if (!ph->isConfigured()) 
	{
		((PhoneWin*)v)->status->value("We aren't configured yet");
		((PhoneWin*)v)->status->damage(1);
		updateWidgets();
		return;
	}
	if (n<=0)
	{
		((PhoneWin*)v)->status->value("Invalid entry selected");
		((PhoneWin*)v)->status->damage(1);
		//cout <<"Ha clickado en una entrada invalida"<<endl;
		updateWidgets();
		return;
	}
		//cout <<"Evento Lista OK   -- "<<n<<endl;
	const char *c = (l->text(n));
	group=c;
	
	if (group == "NO GROUP")
	{
		((PhoneWin*)v)->status->value("Unassociating ourselves");
		((PhoneWin*)v)->status->damage(1);
		if (ph->weAreAssociated()) ph->exitFromGroup();
		associatedGroup = "No Group";
		ph->touch(psm::PSM_INIT);
		
	}
	else if (group.find("@")==string::npos)
	{
		((PhoneWin*)v)->status->value("Invalid entry selected");
		((PhoneWin*)v)->status->damage(1);
		//cout <<"Ha clickado en una entrada invalida"<<endl;
	}
	else if ( ph->weAreAssociated()==FALSE && ph->getVoiceState()!=SILENCE)
	{
		((PhoneWin*)v)->status->value("You cannot change the group\nduring an emergency");
		((PhoneWin*)v)->status->damage(1);
		//cout <<"Ha clickado en una entrada invalida"<<endl;
	}
	else
	{
		associatedGroup=group;
		string msg ("Connected to group:\n");
		msg += associatedGroup;
		((PhoneWin*)v)->status->value(msg.data());
		((PhoneWin*)v)->status->damage(1);
		cout <<"Grupo escogido de la lista es "<<associatedGroup<<endl;
		string grp = SipUserData::extractUserFromAddress(associatedGroup); //converts "group@domain.com" into "group"
		ph->joinGroup(grp); //joinGroup needs "group", not "group@domain.com"
		sleep(1);
	}
	
	ofstream file("/tmp/groups.tmp.001");
	//file << "uno\ndos\ntres\ncuatro\n";
	file << ph->dumpGroups().data();
	
	file.close();
	
	l->load("/tmp/groups.tmp.001");
	l->add("NO GROUP", NULL);
	updateWidgets();
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
			//either ASSOCIATED or UNASSOCIATED, we draw in status the current state via the callback
			//cb_trunkStatus_i(trunkStatus,this);	
			//cout <<"Event TRUNKINGEVENT"<<endl;
			//cout.flush();	
			
			updateWidgets();
			//damage(1);
			//Fl::handle(FL_MOVE, this);
			//Fl::handle(FL_FOCUS, this);
			//Fl::belowmouse(trunkStatus);
			//Fl::pushed(trunkStatus);
			//Fl::focus(trunkStatus);
			/* NADA DE ESTO SIRVE PARA ACTUALIZAR AUTOMATICAMENTE LA PANTALLA
			Fl::belowmouse(this);
			damage(1);
			hotspot(trunkStatus);
			make_current();
			flush();
			set_modal();*/
		}break;
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
		string winLabel ("IPTrunking: User ");
		winLabel+=My_user;
		PhoneWin win2(250,300,winLabel.c_str());
		Fl::run();
		cout<<"after win2"<<endl;
	}
	
	if (ph!=NULL) delete ph;
	ph=NULL;
	//return Fl::run();
	
	return 0;
}


		
/*//en /PhoneWin::cb_trunk_i
		if status==SILENCE (the trunk was green) and i was clicked, then i startTalking and put the status to IAMTALKING and the trunk in YELLOW
		
		if status==IAMTALKING (yellow trunk) and i was clicked, then i stopTalking and put the status to SILENCE and trunk green
		
		always check the status at the end to see if status might be ACTIVE, and if so, the trunk should be RED and status=ACTIVE
		*/
		//if (rtp!=NULL)	rtp->attachObserver(*this);//TRUCO PARA FORZAR EL REFRESCO DE LA PANTALLA CUANDO HAY UN CAMBIO DE TRUNKING STATE
		

