#include <QTimer>
#include <QTimerEvent>
#include <QWidget>
#include <QPainter>
#include <QApplication>
#include <QLayout> 
#include <QPixmap>


#include "world.h"
#include "robot.h"
#include "worldpoint.h"
#include "defs.h"
#include "limbic-system-simulator.h"


/***********************************************
 * GNU GENERAL PUBLIC LICENSE
 * Version 3, 29 June 2007
 * (C) 2017, Bernd Porr, bernd.porr@glasgow.ac.uk
 ***********************************************/


#define BORDERS 1



LimbicMainWindow::LimbicMainWindow( QWidget *parent, 
				    const char *) : QMainWindow( parent ) {
	actualStep=0;
	maxx=MAXX;
	maxy=MAXY;
	stepsBelow=0;
	fX0=NULL;
	foodDelay=FOOD_DELAY;
	numOfFoodContactsDuringReversal=0;
	numOfFoodContactsFromReversal=0;
	isReversal=0;

	// Creates the world
	world=new World(maxx,maxy);
	world->setContactsFilename("contacts.dat");

	// array of robots
	robot=NULL;

	robot=new Robot*[MAXROBOT];
	for(int i=0; i<MAXROBOT;i++) {
		robot[i]=new Robot(world,i,0,maxx/2,maxy/2-i*30,10,20);
		robot[i]->setPhi(START_ANGLE);
	}

	//Borders, food!

#ifdef BORDERS
	fprintf(stderr,"Borders\n");
	world->drawBorders();
#endif
	fprintf(stderr,"Food\n");

#ifdef DRAW_FOOD

	world->drawFood(0,PLACE_X1-FOOD_DIAMETER/2,
			PLACE_Y1-FOOD_DIAMETER/2,
			FOOD_DIAMETER,0,1); // real food at index 0
	world->drawFood(0,
			PLACE_X2+FOOD_DIAMETER/2,
			PLACE_Y2+FOOD_DIAMETER/2,
			FOOD_DIAMETER,1,0); // fake food at index 1
#endif	
	// place field

	fprintf(stderr,"Place field\n");
	
	int index = 0;
	
	world->drawPlacefield(0,PLACE_X1,PLACE_Y1,PLACEFIELD_DIAMETER,index);
	index++;
	world->drawPlacefield(0,PLACE_X2,PLACE_Y2,PLACEFIELD_DIAMETER,index);
	index++;

   
	startTimer( TIMER_INTERVAL );
}


LimbicMainWindow::~LimbicMainWindow() {
	for(int i=0;i<MAXROBOT;i++) {
		delete robot[i];
	}
	delete robot;

}


void LimbicMainWindow::writeQuicktime() {
	saveAsQuicktime = 1;
	world->openQuicktime("/tmp/limbic.mov");
}



//
// Handles paint events for the LimbicMainWindow widget.
//

void LimbicMainWindow::paintEvent( QPaintEvent * ) {
	// will paint in this object
	int w = width();
	int h = height();

	QPixmap pm( w, h );

	QPainter *paint = new QPainter( &pm );

	if(w <= 0 || h <= 0) 
		return;

	world->fillPainter(*paint);

	delete paint;

	QPainter paint2(this);
	paint2.drawPixmap(0,0,pm);
}

//
// Handles timer events for the LimbicMainWindow widget.
//

void LimbicMainWindow::timerEvent( QTimerEvent * )
{
	for ( int i=0; i<ITERATIONS_BEFORE_DISPLAY; i++ ) {
		doSimStep();
	}
	repaint();
}


void LimbicMainWindow::doSimStep() {

	if (actualStep>PARTIAL_MOVE_BELOW_STEP) { // 
		for(int j=0;j<ROBOTS_WHICH_MOVE;j++) {
			robot[j]->move(actualStep);
			robot[j]->docCoord(actualStep);
		}
	} else { // all robots one step
		for(int j=0;j<MAXROBOT;j++) {
			robot[j]->move(actualStep);
			robot[j]->docCoord(actualStep);
		}
	}
	//plot new food...

#ifdef DRAW_FOOD
	
	if (world->getNewFood()){
		if (foodDelay>0) {
			foodDelay--;
		} else {
			// fprintf(stderr,"step #%d: drawing new food\n",actualStep);
			int newIndex = world->getFreeFoodIndex();
			
			int cx1 = PLACE_X1;
			int cy1 = PLACE_Y1;
			
			int cx2 = PLACE_X2;
			int cy2 = PLACE_Y2;
			int dx,dy;
			int sub = PLACEFIELD_DIAMETER/2;
			
			// let's find out if there's still real food out there
			// this is the index number of the food bit which is actually rewarding
			int r=world->getRewardIndex();
			// get info if that has been eaten
			int reward_exists=world->getFoodValid(r);
			if (!(world->getSwapFlag())) {
				// real food is on the left
				if (isReversal) {
					fprintf(stderr,"Final reversal contacts: %d\n",
						numOfFoodContactsDuringReversal);
					close();
				}
			} else {
				// real food is on the right (reversal)
				if (!isReversal) {
					// first step
					isReversal=1;
					numOfFoodContactsFromReversal=
						world->getNumberOfFoodContacts();
					fprintf(stderr,"########### REVERSAL STARTED ###########\n");
				}
				numOfFoodContactsDuringReversal=
					world->getNumberOfFoodContacts()-
					numOfFoodContactsFromReversal;
				fprintf(stderr,"Reversal contacts so far: %d\n",
					numOfFoodContactsDuringReversal);
			}


			    
			if (reward_exists) {
				// reward is still out there
				// we have to place fake food
				if (world->getSwapFlag()) {
					// we place the fake food on the left
					dx=(int)(float)rand()% (PLACEFIELD_DIAMETER/2);
					dy=(int)(float)rand()% (PLACEFIELD_DIAMETER/2);
					world->drawFood(actualStep, 
							cx1+dx-sub, 
							cy1+dy-sub, 
							FOOD_DIAMETER,
							newIndex,
							0);
				} else {
					// we plae the fake food on the right
					dx=(int)(float)rand()% (PLACEFIELD_DIAMETER/2);
					dy=(int)(float)rand()% (PLACEFIELD_DIAMETER/2);
					world->drawFood(actualStep, 
							cx2+dx-sub, 
							cy2+dy-sub, 
							FOOD_DIAMETER,
							newIndex,
							0);
				}
			} else {
				// no rewarding food bit out there
				// we have to generate real rewarding food
				// if ((actualStep%(REVERSAL_STEP*2))<REVERSAL_STEP) {
				if (!(world->getSwapFlag())) {
					dx=(int)(float)rand()% (PLACEFIELD_DIAMETER/2);
					dy=(int)(float)rand()% (PLACEFIELD_DIAMETER/2);
					world->drawFood(actualStep, 
							cx1+dx-sub, 
							cy1+dy-sub, 
							FOOD_DIAMETER,
							newIndex,
							1);
				} else {
					dx=(int)(float)rand()% (PLACEFIELD_DIAMETER/2);
					dy=(int)(float)rand()% (PLACEFIELD_DIAMETER/2);
					world->drawFood(actualStep, 
							cx2+dx-sub, 
							cy2+dy-sub, 
							FOOD_DIAMETER,
							newIndex,
							1); 
				}
			}
		}
	} else {
		foodDelay=FOOD_DELAY+(rand()%FOOD_DELAY);
	}
#endif	

	if (saveAsQuicktime) {
		world->docQuicktime(actualStep);
	}
	
        actualStep++;
	if (actualStep==MAXSTEP) {
		close();
	}
}


void LimbicMainWindow::closeEvent(QCloseEvent *) {
        if (saveAsQuicktime) {
	        world->closeQuicktime();
        }
}



void single_food_run(int argc, char **argv,int quicktime) {
	QApplication a( argc, argv );
	LimbicMainWindow* limbicbots=new LimbicMainWindow();	// create widget
	if (quicktime) limbicbots->writeQuicktime();
	limbicbots->resize( MAXX, MAXY );	// start up with size 400x250
#ifdef SHOW_SIM
	limbicbots->show();				// show widget
#endif
	a.exec();				// run event loop
}






void statistics_food_run(int argc, char **argv) {
	FILE* f=fopen("perf.dat","wt");
	QApplication* a=NULL;
	LimbicMainWindow* limbicbots=NULL;
	a=new QApplication( argc, argv ); // create application object

	// loop through different learning rates. This hasn't been tested.
       	for(float phi=0.0001;phi<2*M_PI;phi=phi+M_PI/50) {
		fprintf(stderr,"phi=%e\n",phi);
		limbicbots=new LimbicMainWindow();	// create widget
		if (!limbicbots) {
			fprintf(stderr,"Cound not create limbicbots class\n");
			exit(1);
		}
		limbicbots->robot[0]->setPhi(phi);
		char tmp[128];
		limbicbots->resize( MAXX, MAXY );	// start up with size 400x250
		sprintf(tmp,"reversal learning benchmark: phi=%e",phi);
		// limbicbots->setCaption(tmp);
#ifdef SHOW_SIM
		limbicbots->show();				// show widget
#endif
		a->exec();				// run event loop
		fprintf(f,"%e %d\n",phi,limbicbots->numOfFoodContactsFromReversal);
		fflush(f);
		srandom((unsigned int)(limbicbots->actualStep%65536));
		delete limbicbots;
	}
	delete a;
	fclose(f);
}






int main( int argc, char **argv ) {
        int c;
	int qt = 0;
	int stats = 0;
	
	while (-1 != (c = getopt(argc, argv, "abq"))) {
		switch (c) {
		case 'a':
			stats = 0;
                case 'b':
			stats = 1;
                case 'q':
			qt = 1;
		case 'h':
			fprintf(stderr,"%s: help\n",argv[0]);
			fprintf(stderr," -a: single food run\n");
			fprintf(stderr," -b: statistics\n");
			fprintf(stderr," -q: quicktime export\n");
                }
        }
	// default behaviour without any valid arguments
	if (!stats) {
		single_food_run(argc,argv,qt);
	} else {
		statistics_food_run(argc,argv);
	}
	return 0;	
}