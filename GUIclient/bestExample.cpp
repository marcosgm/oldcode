// FLTK demo program for CS559

// Fall, 2000, Mark Pingel

 

#include <FL/Fl.H>

#include <FL/Fl_Window.H>

#include <FL/Fl_Check_Button.H>

#include <FL/Fl_Light_Button.H>

#include <FL/Fl_Round_Button.H>

 

// Create pointers to a check button and a light button

Fl_Check_Button* check_b;

Fl_Light_Button* light_b;

 
Fl_Round_Button* round_b;
 

 

// Callback function for the check button.

// Turn the label red and change it to read Checked if the

// check button is checked.

void checkbutton_cb(Fl_Widget* w) {

	Fl_Check_Button* flcb = ((Fl_Check_Button*)w);

	if ( flcb->value() == 1 ) {

		flcb->labelcolor(FL_RED);

		flcb->label("Checked");

                               

	}

	else {

		flcb->labelcolor(FL_CYAN);

		flcb->label("Check Button");

	}             

	flcb->damage(1);

	flcb->parent()->redraw();

}

 

// Callback function for the light button.

// Turn the label yellow and change it to read Light On if the

// light button is pressed.  A yellow square will also appear in

// the box to the left of the label if selected

void lightbutton_cb(Fl_Widget* w) {

	Fl_Light_Button* fllb = ((Fl_Light_Button*)w);

	if ( fllb->value() == 1 ) {

		check_b->hide();

	}

	else {

		check_b->show();

	}             

	fllb->damage(1);

	fllb->parent()->redraw();

}

 

void round_b_cb(Fl_Widget* w) {

	if ( ((Fl_Round_Button*)w)->value() == 1 ) {

		light_b->activate();

	}

	else {

		light_b->deactivate();

	}

	w->damage(1);

	w->parent()->redraw();

}
 
 

int main(int argc, char ** argv) {      

                // Create a window to add our buttons to

	Fl_Window* w = new Fl_Window(190,100);

               

	w->label("559 Demo Window");

                // Begin adding children to this window

	w->begin();

 

                                // Instantiate the check button

                                // x , y , width, height, label

	check_b = new Fl_Check_Button(25,15,140,20,"Check Button");

                                // Instantiate the light button

                                // x , y , width, height, label

	light_b = new Fl_Light_Button(25,45,140,20,"Hide Check Button");

 
                                // Make all of the default colors for the labels cyan

	check_b->labelcolor(FL_CYAN);

	light_b->labelcolor(FL_CYAN);

	round_b = new Fl_Round_Button(25, 75, 140, 20, "Activate Light B");

	round_b->labelcolor(FL_CYAN);

	round_b->callback(round_b_cb);

                                // Register the callbacks for each of the buttons

	check_b->callback(checkbutton_cb);

	light_b->callback(lightbutton_cb);

 

                // Stop adding children to this window

	w->end();

                // Display the window

	w->show();

                // Run

	return Fl::run();

               

}
