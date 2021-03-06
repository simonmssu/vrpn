// phan_client.C - simplest example: generates a flat horizontal plane

#include <stdlib.h>
#include <stdio.h>
#include "vrpn_ForceDevice.h"
#include <vrpn_Tracker.h>
#include <vrpn_Button.h>

#define PHANTOM_SERVER "Tracker0@localhost"

/*****************************************************************************
 *
   Callback handler
 *
 *****************************************************************************/

void    VRPN_CALLBACK handle_force_change(void *userdata, const vrpn_FORCECB f)
{
	static vrpn_FORCECB lr;        // last report
	static int first_report_done = 0;

	if ((!first_report_done) || 
	    ((f.force[0] != lr.force[0]) || (f.force[1] != lr.force[1])
	      || (f.force[2] != lr.force[2])))
	   printf("force is (%f,%f,%f)\n", f.force[0], f.force[1], f.force[2]);
	first_report_done = 1;
	lr = f;
}

void    VRPN_CALLBACK handle_tracker_change(void *userdata, const vrpn_TRACKERCB t)
{
	static vrpn_TRACKERCB lr; // last report
	static float dist_interval_sq = 0.004f;

	if ((lr.pos[0] - t.pos[0])*(lr.pos[0] - t.pos[0]) + 
	    (lr.pos[1] - t.pos[1])*(lr.pos[1] - t.pos[1]) +
	    (lr.pos[2] - t.pos[2])*(lr.pos[2] - t.pos[2]) > dist_interval_sq){
		printf("Sensor %d is now at (%g,%g,%g)\n", t.sensor,
			t.pos[0], t.pos[1], t.pos[2]);
		lr = t;
	}
}

void	VRPN_CALLBACK handle_button_change(void *userdata, const vrpn_BUTTONCB b)
{
	static int count=0;
	static int buttonstate = 1;
	int done = 0;

	if (b.state != buttonstate) {
	     printf("button #%d is in state %d\n", b.button, b.state);
	     buttonstate = b.state;
	     count++;
	}
	if (count > 4)
		done = 1;
	*(int *)userdata = done;
}

int main(int argc, char *argv[])
{
        int     done = 0;
        vrpn_ForceDevice_Remote *forceDevice;
	vrpn_Tracker_Remote *tracker;
	vrpn_Button_Remote *button;

        // initialize the force device
        forceDevice = new vrpn_ForceDevice_Remote(PHANTOM_SERVER);
	forceDevice->register_force_change_handler(NULL, handle_force_change);

	// initialize the tracker 
	tracker = new vrpn_Tracker_Remote(PHANTOM_SERVER);
	tracker->register_change_handler(NULL, handle_tracker_change);

	// initialize the button 
	button = new vrpn_Button_Remote(PHANTOM_SERVER);
	button->register_change_handler(&done, handle_button_change);

        while (!forceDevice->connectionPtr()->connected()) {
           forceDevice->mainloop();
        }

        // Set plane and surface parameters
        forceDevice->set_plane(0.0, 1.0, 0.0, 0.0);
        forceDevice->setSurfaceKspring(0.8f); 	// spring constant - units of
						// dynes/cm
        forceDevice->setSurfaceKdamping(0.0);	// damping constant - 
                                                // units of dynes*sec/cm
        forceDevice->setSurfaceFstatic(0.0); 	// set static friction
        forceDevice->setSurfaceFdynamic(0.0);	// set dynamic friction
        forceDevice->setRecoveryTime(10);	// recovery occurs over 10
                                                // force update cycles

        forceDevice->setSurfaceBuzzAmplitude(0.0);
        forceDevice->setSurfaceBuzzFrequency(60.0);
        forceDevice->setSurfaceTextureAmplitude(0.0);
        forceDevice->setSurfaceTextureWavelength(0.01f);

        // enable force device and send first surface
        forceDevice->startSurface();  

/*
	// test spring constraint instead

        vrpn_float32 p [3];
        p[0] = 0.0f;
        p[1] = 0.0f;
        p[2] = 0.0f;
        vrpn_float32 d [3];
        d[0] = 0.0f;
        d[1] = 1.0f;
        d[2] = 0.0f;

	//forceDevice->setConstraintMode(vrpn_ForceDevice::POINT_CONSTRAINT);
	//forceDevice->setConstraintPoint(p);
          // works
	//forceDevice->setConstraintMode(vrpn_ForceDevice::LINE_CONSTRAINT);
	//forceDevice->setConstraintLinePoint(p);
	//forceDevice->setConstraintLineDirection(d);
          // works

	forceDevice->setConstraintMode(vrpn_ForceDevice::PLANE_CONSTRAINT);
	forceDevice->setConstraintPlanePoint(p);
	forceDevice->setConstraintPlaneNormal(d);
        forceDevice->setConstraintKSpring(10.0);
        forceDevice->enableConstraint(1);
*/

        // main loop
        while (! done ){
                // Let the forceDevice send its planes to remote force device
                forceDevice->mainloop();
		// Let tracker receive position information from remote tracker
		tracker->mainloop();
		// Let button receive button status from remote button
		button->mainloop();
                // we may call forceDevice->set_plane(...) followed by
                //      forceDevice->sendSurface() here to change the plane
                // for example: using position information from a tracker we can
                //      compute a plane to approximate a complex surface at that
                //      position and send that approximation 15-30 times per
                //      second to simulate the complex surface
        }
	// shut off force device
        forceDevice->stopSurface();

}   /* main */
