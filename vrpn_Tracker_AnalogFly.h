#ifndef INCLUDED_ANALOGFLY
#define INCLUDED_ANALOGFLY

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vrpn_Tracker.h"
#include "vrpn_Analog.h"
#include "vrpn_Button.h"

#include <quat.h>

// This parameter is passed to the constructor for the AnalogFly; it describes
// the channel mapping and parameters of that mapping, as well as the button that
// will be used to reset the tracker when it is pushed. Any entry which has a NULL
// pointer for the name is disabled.

class vrpn_TAF_axis {
public:
	char *name; int channel; float thresh; float power; float scale;
	vrpn_TAF_axis() { name = NULL; channel = 0; thresh = 0.0; power = 1.0; scale = 1.0; };
};
class vrpn_Tracker_AnalogFlyParam {
public:
	// Name of the Analog device for each axis (x,y,z) and rotation (about x,y,z)
	// Which channel to use from the Analog for each
	// Threshold (distance from zero indistinguishable from zero)
	// Power (to which the value is taken, to allow slow near center and fast away)
	// Scale (which is applied before the power to values that are above threshold)
	vrpn_TAF_axis x,y,z;	// Translation along each of these three axes
	vrpn_TAF_axis sx,sy,sz;	// Rotation in the positive direction about the three axes

	// Button device that is used to reset the matrix to the origin
	char *reset_name; int reset_which;

	vrpn_Tracker_AnalogFlyParam(void) {
		x.name = y.name = z.name = sx.name = sy.name = sz.name = reset_name = NULL;
	}
};

// This class will turn a joystick into a tracker by interpreting the joystick
// positions as velocity inputs and "flying" the user around based on analog values.
// The mapping from analog channels to directions (or orientation changes) is
// described in the vrpn_Tracker_AnalogFlyParam parameter. For translations,
// values above threshold are multiplied by the scale and then taken to the power; the
// result is the number of meters per second to move in that direction. For rotations,
// the result is taken as the number of revolutions per second around the given axis.

class vrpn_TAF_fullaxis {
public:
	vrpn_TAF_axis axis; vrpn_Analog_Remote *ana; double value;
	vrpn_TAF_fullaxis() { ana = NULL; value = 0.0; };
};
class vrpn_Tracker_AnalogFly : public vrpn_Tracker {
  public:
    vrpn_Tracker_AnalogFly(const char *name, vrpn_Connection *trackercon,
			   vrpn_Tracker_AnalogFlyParam *params, float update_rate);

    virtual ~vrpn_Tracker_AnalogFly (void);

    virtual void mainloop (const struct timeval * timeout = NULL);
    virtual void reset (void);

    void update (q_matrix_type &);

    static void handle_joystick (void *, const vrpn_ANALOGCB);
    static int handle_newConnection (void *, vrpn_HANDLERPARAM);

  protected:
    vrpn_Connection	*_connection;	// Connection to send reports on
    double	_update_interval;	// How long to wait between sends
    struct timeval prevtime;		// Time of the previous report

    vrpn_TAF_fullaxis	_x, _y, _z, _sx, _sy, _sz;
    vrpn_Button_Remote	*reset_button;
    int			_which_button;

    q_matrix_type initMatrix, currentMatrix;

    void	update_matrix_based_on_values(double time_interval);
    void	convert_matrix_to_tracker(void);

    int setup_channel(vrpn_TAF_fullaxis *full);
    int teardown_channel(vrpn_TAF_fullaxis *full);

    static void	handle_analog_update(void *userdata, const vrpn_ANALOGCB info);
    static void handle_reset_press(void *userdata, const vrpn_BUTTONCB info);
};

#endif














