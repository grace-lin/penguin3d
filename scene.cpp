#ifdef _WIN32
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/glui.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "scene.h"
#include "keyframe.h"
#include "timer.h"
#include "vector.h"
#include "penguin.h"
#include "equipment.h"

// *************** GLOBAL VARIABLES *************************

const float PI = 3.14159;
const float SPINNER_SPEED = 0.1;

// --------------- USER INTERFACE VARIABLES -----------------

// Window settings
int windowID;				// Glut window ID (for display)
int Win[2];					// window (x,y) size

GLUI* glui_joints;			// Glui window with joint controls for red
GLUI* glui_joints2;			// Glui window with joint controls for cyan
GLUI* glui_keyframe;		// Glui window with keyframe controls
GLUI* glui_render;			// Glui window for render style

char msg[256];				// String used for status message
GLUI_StaticText* status;	// Status message ("Status: <msg>")


// ---------------- ANIMATION VARIABLES ---------------------

// Camera settings
bool updateCamZPos = false;
int  lastX = 0;
int  lastY = 0;
const float ZOOM_SCALE = 0.01;

GLdouble camXPos =  0.0;
GLdouble camYPos =  0.0;
GLdouble camZPos = -10.0;

const GLdouble CAMERA_FOVY = 60.0;
const GLdouble NEAR_CLIP   = 0.1;
const GLdouble FAR_CLIP    = 1000.0;

// Render settings
// README: the different render styles:
enum { WIREFRAME, METALLIC };
int renderStyle = METALLIC;			// README: the selected render style

// Light settings
float lightPos = 0.0;
// Animation settings
int animate_mode = 0;			// 0 = no anim, 1 = animate
//render style
bool wire = false;
// Keyframe settings
const char filenameKF[] = "keyframes.txt";	// file for loading / saving keyframes

Keyframe* keyframes;			// list of keyframes

int maxValidKeyframe   = 0;		// index of max VALID keyframe (in keyframe list)
const int KEYFRAME_MIN = 0;
const int KEYFRAME_MAX = 100;	// README: specifies the max number of keyframes

// Frame settings
char filenameF[128];			// storage for frame filename

int frameNumber = 0;			// current frame being dumped
int frameToFile = 0;			// flag for dumping frames to file

const float DUMP_FRAME_PER_SEC = 24.0;		// frame rate for dumped frames
const float DUMP_SEC_PER_FRAME = 1.0 / DUMP_FRAME_PER_SEC;

// Time settings
Timer* animationTimer;
Timer* frameRateTimer;

const float TIME_MIN = 0.0;
const float TIME_MAX = 200.0;	// README: specifies the max time of the animation
const float SEC_PER_FRAME = 1.0 / 60.0;

// Joint settings
Keyframe* joint_ui_data;


// ***********  FUNCTION HEADER DECLARATIONS ****************


// Initialization functions
void initDS();
void initGlut(int argc, char** argv);
void initGlui();
void initGl();


// Callbacks for handling events in glut
void reshape(int w, int h);
void animate();
void display(void);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);


// Functions to help draw the object
Vector getInterpolatedJointDOFS(float time);
void drawPenguin(int s);
void drawSeeSaw();
void drawGround();

// Image functions
void writeFrame(char* filename, bool pgm, bool frontBuffer);


// ******************** FUNCTIONS ************************



// main() function
// Initializes the user interface (and any user variables)
// then hands over control to the event handler, which calls 
// display() whenever the GL window needs to be redrawn.
int main(int argc, char** argv)
{

    // Process program arguments
    if(argc != 3) {
        printf("Usage: scene [width] [height]\n");
        printf("Using 640x480 window by default...\n");
        Win[0] = 640;
        Win[1] = 480;
    } else {
        Win[0] = atoi(argv[1]);
        Win[1] = atoi(argv[2]);
    }


    // Initialize data structs, glut, glui, and opengl
	initDS();
	initGlut(argc, argv);
	initGlui();
	initGl();

    // Invoke the standard GLUT main event loop
    glutMainLoop();

    return 0;         // never reached
}


// Create / initialize global data structures
void initDS()
{
	keyframes = new Keyframe[KEYFRAME_MAX];
	for( int i = 0; i < KEYFRAME_MAX; i++ )
		keyframes[i].setID(i);

	animationTimer = new Timer();
	frameRateTimer = new Timer();
	joint_ui_data  = new Keyframe();
}


// Initialize glut and create a window with the specified caption 
void initGlut(int argc, char** argv)
{
	// Init GLUT
	glutInit(&argc, argv);

    // Set video mode: double-buffered, color, depth-buffered
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Create window
    glutInitWindowPosition (0, 0);
    glutInitWindowSize(Win[0],Win[1]);
    windowID = glutCreateWindow(argv[0]);

    // Setup callback functions to handle events
    glutReshapeFunc(reshape);	// Call reshape whenever window resized
    glutDisplayFunc(display);	// Call display whenever new frame needed
	glutMouseFunc(mouse);		// Call mouse whenever mouse button pressed
	glutMotionFunc(motion);		// Call motion whenever mouse moves while button pressed
}


// Load Keyframe button handler. Called when the "load keyframe" button is pressed
void loadKeyframeButton(int)
{
	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();

	// Update the 'joint_ui_data' variable with the appropriate
	// entry from the 'keyframes' array (the list of keyframes)
	*joint_ui_data = keyframes[keyframeID];

	// Sync the UI with the 'joint_ui_data' values
	glui_joints->sync_live();
	glui_keyframe->sync_live();

	// Let the user know the values have been loaded
	sprintf(msg, "Status: Keyframe %d loaded successfully", keyframeID);
	status->set_text(msg);
}

// Update Keyframe button handler. Called when the "update keyframe" button is pressed
void updateKeyframeButton(int)
{
	// Get the keyframe ID from the UI
	int keyframeID = joint_ui_data->getID();
	// int keyframeID = 0;

	// Update the 'maxValidKeyframe' index variable
	// (it will be needed when doing the interpolation)
	if (keyframeID > maxValidKeyframe) {
		maxValidKeyframe = keyframeID;
	}

	// Update the appropriate entry in the 'keyframes' array
	// with the 'joint_ui_data' data
	keyframes[keyframeID] = *joint_ui_data;

	// Let the user know the values have been updated
	sprintf(msg, "Status: Keyframe %d updated successfully", keyframeID);
	status->set_text(msg);
}

// Load Keyframes From File button handler. Called when the "load keyframes from file" button is pressed
//
// ASSUMES THAT THE FILE FORMAT IS CORRECT, ie, there is no error checking!
//
void loadKeyframesFromFileButton(int)
{
	// Open file for reading
	FILE* file = fopen(filenameKF, "r");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Read in maxValidKeyframe first
	fscanf(file, "%d", &maxValidKeyframe);

	// Now read in all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fscanf(file, "%d", keyframes[i].getIDPtr());
		fscanf(file, "%f", keyframes[i].getTimePtr());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fscanf(file, "%f", keyframes[i].getDOFPtr(j));
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been loaded
	sprintf(msg, "Status: Keyframes loaded successfully");
	status->set_text(msg);
}

// Save Keyframes To File button handler. Called when the "save keyframes to file" button is pressed
void saveKeyframesToFileButton(int)
{
	// Open file for writing
	FILE* file = fopen(filenameKF, "w");
	if( file == NULL )
	{
		sprintf(msg, "Status: Failed to open file %s", filenameKF);
		status->set_text(msg);
		return;
	}

	// Write out maxValidKeyframe first
	fprintf(file, "%d\n", maxValidKeyframe);
	fprintf(file, "\n");

	// Now write out all keyframes in the format:
	//    id
	//    time
	//    DOFs
	//
	for( int i = 0; i <= maxValidKeyframe; i++ )
	{
		fprintf(file, "%d\n", keyframes[i].getID());
		fprintf(file, "%f\n", keyframes[i].getTime());

		for( int j = 0; j < Keyframe::NUM_JOINT_ENUM; j++ )
			fprintf(file, "%f\n", keyframes[i].getDOF(j));

		fprintf(file, "\n");
	}

	// Close file
	fclose(file);

	// Let the user know the keyframes have been saved
	sprintf(msg, "Status: Keyframes saved successfully");
	status->set_text(msg);
}

// Animate button handler.  Called when the "animate" button is pressed.
void animateButton(int)
{
  // synchronize variables that GLUT uses
  glui_keyframe->sync_live();
  glui_joints->sync_live();

  // toggle animation mode and set idle function appropriately
  if( animate_mode == 0 )
  {
	// start animation
	frameRateTimer->reset();
	animationTimer->reset();

	animate_mode = 1;
	GLUI_Master.set_glutIdleFunc(animate);

	// Let the user know the animation is running
	sprintf(msg, "Status: Animating...");
	status->set_text(msg);
  }
  else
  {
	// stop animation
	animate_mode = 0;
	GLUI_Master.set_glutIdleFunc(NULL);

	// Let the user know the animation has stopped
	sprintf(msg, "Status: Animation stopped");
	status->set_text(msg);
  }
}

// Render Frames To File button handler. Called when the "Render Frames To File" button is pressed.
void renderFramesToFileButton(int)
{
	// Calculate number of frames to generate based on dump frame rate
	int numFrames = int(keyframes[maxValidKeyframe].getTime() * DUMP_FRAME_PER_SEC) + 1;

	// Generate frames and save to file
	frameToFile = 1;
	for( frameNumber = 0; frameNumber < numFrames; frameNumber++ )
	{
		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(frameNumber * DUMP_SEC_PER_FRAME) );

		// Let the user know which frame is being rendered
		sprintf(msg, "Status: Rendering frame %d...", frameNumber);
		status->set_text(msg);

		// Render the frame
		display();
	}
	frameToFile = 0;

	// Let the user know how many frames were generated
	sprintf(msg, "Status: %d frame(s) rendered to file", numFrames);
	status->set_text(msg);
}

// Quit button handler.  Called when the "quit" button is pressed.
void quitButton(int)
{
  exit(0);
}

// Initialize GLUI and the user interface
void initGlui()
{
	GLUI_Panel* glui_panel;
	GLUI_Spinner* glui_spinner;
	GLUI_RadioGroup* glui_radio_group;

    GLUI_Master.set_glutIdleFunc(NULL);

    
	// Create GLUI window (render controls) ************
	//
	glui_render = GLUI_Master.create_glui("General Control", 0, 367, Win[1]+64);

	// ------------------ STYLE -----------------//
	glui_panel = glui_render->add_panel("Render Style");
	glui_radio_group = glui_render->add_radiogroup_to_panel(glui_panel, &renderStyle);
	glui_render->add_radiobutton_to_group(glui_radio_group, "Wireframe");
	glui_render->add_radiobutton_to_group(glui_radio_group, "Metallic");
	
	// ------------------ LIGHT POSITION -----------------//
	glui_panel = glui_render->add_panel("Light source position");
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, 
		"light position:", GLUI_SPINNER_FLOAT, &lightPos);
	glui_spinner->set_float_limits(LIGHT_MIN, LIGHT_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);
    
    // ------------------ EQUIPMENT -----------------//
    // controls for the see saw

	glui_panel = glui_render->add_panel("Equipment");
	glui_spinner = glui_render->add_spinner_to_panel(glui_panel, "see saw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::PLANK));
	glui_spinner->set_float_limits(PLANK_MIN, PLANK_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
    
    // ------------------ CAMERA -----------------//
	glui_render->add_column(false);
	glui_panel = glui_render->add_panel("Camera");
    
	glui_spinner = glui_render->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::CAM_TRANSLATE_X));
	glui_spinner->set_float_limits(CAM_TRANSLATE_X_MIN, CAM_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_render->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::CAM_TRANSLATE_Y));
	glui_spinner->set_float_limits(CAM_TRANSLATE_Y_MIN, CAM_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_render->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::CAM_TRANSLATE_Z));
	glui_spinner->set_float_limits(CAM_TRANSLATE_Z_MIN, CAM_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_render->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::CAM_ROTATE_X));
	glui_spinner->set_float_limits(CAM_ROTATE_X_MIN, CAM_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_render->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::CAM_ROTATE_Y));
	glui_spinner->set_float_limits(CAM_ROTATE_Y_MIN, CAM_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_render->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::CAM_ROTATE_Z));
	glui_spinner->set_float_limits(CAM_ROTATE_Z_MIN, CAM_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);
	//
	// ***************************************************
    
    // *************** Create GLUI window (Red Penguin controls) ***************
	//
	glui_joints = GLUI_Master.create_glui("Red Penguin Control", 0, Win[0]+12, 0);

    // Create controls to specify BODY position and orientation
	glui_panel = glui_joints->add_panel("Body");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY_TRANSLATE_X));
	glui_spinner->set_float_limits(BODY_TRANSLATE_X_MIN, BODY_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY_TRANSLATE_Y));
	glui_spinner->set_float_limits(BODY_TRANSLATE_Y_MIN, BODY_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY_TRANSLATE_Z));
	glui_spinner->set_float_limits(BODY_TRANSLATE_Z_MIN, BODY_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY_ROTATE_X));
	glui_spinner->set_float_limits(BODY_ROTATE_X_MIN, BODY_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY_ROTATE_Y));
	glui_spinner->set_float_limits(BODY_ROTATE_Y_MIN, BODY_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY_ROTATE_Z));
	glui_spinner->set_float_limits(BODY_ROTATE_Z_MIN, BODY_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify head rotation
	glui_panel = glui_joints->add_panel("Head");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "head:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::HEAD));
	glui_spinner->set_float_limits(HEAD_MIN, HEAD_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify beak
	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "beak:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BEAK));
	glui_spinner->set_float_limits(BEAK_MIN, BEAK_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints->add_column(false);

	// Create controls to specify right arm
	glui_panel = glui_joints->add_panel("Right arm");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_PITCH));
	glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_YAW));
	glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER_ROLL));
	glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_ELBOW));
	glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left arm
	glui_panel = glui_joints->add_panel("Left arm");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_PITCH));
	glui_spinner->set_float_limits(SHOULDER_PITCH_MIN, SHOULDER_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_YAW));
	glui_spinner->set_float_limits(SHOULDER_YAW_MIN, SHOULDER_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER_ROLL));
	glui_spinner->set_float_limits(SHOULDER_ROLL_MIN, SHOULDER_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_ELBOW));
	glui_spinner->set_float_limits(ELBOW_MIN, ELBOW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints->add_column(false);

	// Create controls to specify right leg
	glui_panel = glui_joints->add_panel("Right leg");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_PITCH));
	glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_YAW));
	glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP_ROLL));
	glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE));
	glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left leg
	glui_panel = glui_joints->add_panel("Left leg");

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_PITCH));
	glui_spinner->set_float_limits(HIP_PITCH_MIN, HIP_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_YAW));
	glui_spinner->set_float_limits(HIP_YAW_MIN, HIP_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP_ROLL));
	glui_spinner->set_float_limits(HIP_ROLL_MIN, HIP_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE));
	glui_spinner->set_float_limits(KNEE_MIN, KNEE_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
	//
	// ***************************************************
    
    
    // *************** Create GLUI window (Cyan Penguin controls) ***************
	//
	glui_joints2 = GLUI_Master.create_glui("Cyan Penguin Control", 0, Win[0]+12, 300);

    // Create controls to specify body position and orientation
	glui_panel = glui_joints2->add_panel("Body2");

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "translate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY2_TRANSLATE_X));
	glui_spinner->set_float_limits(BODY2_TRANSLATE_X_MIN, BODY2_TRANSLATE_X_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "translate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY2_TRANSLATE_Y));
	glui_spinner->set_float_limits(BODY2_TRANSLATE_Y_MIN, BODY2_TRANSLATE_Y_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "translate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY2_TRANSLATE_Z));
	glui_spinner->set_float_limits(BODY2_TRANSLATE_Z_MIN, BODY2_TRANSLATE_Z_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "rotate x:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY2_ROTATE_X));
	glui_spinner->set_float_limits(BODY2_ROTATE_X_MIN, BODY2_ROTATE_X_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "rotate y:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY2_ROTATE_Y));
	glui_spinner->set_float_limits(BODY2_ROTATE_Y_MIN, BODY2_ROTATE_Y_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "rotate z:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BODY2_ROTATE_Z));
	glui_spinner->set_float_limits(BODY2_ROTATE_Z_MIN, BODY2_ROTATE_Z_MAX, GLUI_LIMIT_WRAP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify head rotation
	glui_panel = glui_joints2->add_panel("Head");

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "head:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::HEAD2));
	glui_spinner->set_float_limits(HEAD2_MIN, HEAD2_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify beak
	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "beak:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::BEAK2));
	glui_spinner->set_float_limits(BEAK2_MIN, BEAK2_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints2->add_column(false);

	// Create controls to specify right arm
	glui_panel = glui_joints2->add_panel("Right arm");

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER2_PITCH));
	glui_spinner->set_float_limits(SHOULDER2_PITCH_MIN, SHOULDER2_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER2_YAW));
	glui_spinner->set_float_limits(SHOULDER2_YAW_MIN, SHOULDER2_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_SHOULDER2_ROLL));
	glui_spinner->set_float_limits(SHOULDER2_ROLL_MIN, SHOULDER2_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_ELBOW2));
	glui_spinner->set_float_limits(ELBOW2_MIN, ELBOW2_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left arm
	glui_panel = glui_joints2->add_panel("Left arm");

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "shoulder pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER2_PITCH));
	glui_spinner->set_float_limits(SHOULDER2_PITCH_MIN, SHOULDER2_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "shoulder yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER2_YAW));
	glui_spinner->set_float_limits(SHOULDER2_YAW_MIN, SHOULDER2_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "shoulder roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_SHOULDER2_ROLL));
	glui_spinner->set_float_limits(SHOULDER2_ROLL_MIN, SHOULDER2_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "elbow:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_ELBOW2));
	glui_spinner->set_float_limits(ELBOW2_MIN, ELBOW2_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);


	glui_joints2->add_column(false);

	// Create controls to specify right leg
	glui_panel = glui_joints2->add_panel("Right leg");

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP2_PITCH));
	glui_spinner->set_float_limits(HIP2_PITCH_MIN, HIP2_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP2_YAW));
	glui_spinner->set_float_limits(HIP2_YAW_MIN, HIP2_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_HIP2_ROLL));
	glui_spinner->set_float_limits(HIP2_ROLL_MIN, HIP2_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::R_KNEE2));
	glui_spinner->set_float_limits(KNEE2_MIN, KNEE2_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create controls to specify left leg
	glui_panel = glui_joints2->add_panel("Left leg");

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "hip pitch:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP2_PITCH));
	glui_spinner->set_float_limits(HIP2_PITCH_MIN, HIP2_PITCH_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "hip yaw:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP2_YAW));
	glui_spinner->set_float_limits(HIP2_YAW_MIN, HIP2_YAW_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "hip roll:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_HIP2_ROLL));
	glui_spinner->set_float_limits(HIP2_ROLL_MIN, HIP2_ROLL_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_spinner = glui_joints2->add_spinner_to_panel(glui_panel, "knee:", GLUI_SPINNER_FLOAT, joint_ui_data->getDOFPtr(Keyframe::L_KNEE2));
	glui_spinner->set_float_limits(KNEE2_MIN, KNEE2_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);
    //
	// ***************************************************

    
	// ************ Create GLUI window (keyframe controls) ************
	// ------------------ KEYFRAME -----------------//
	glui_keyframe = GLUI_Master.create_glui("Keyframe Control", 0, 0, Win[1]+64);

	// Create a control to specify the time (for setting a keyframe)
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Time:", GLUI_SPINNER_FLOAT, joint_ui_data->getTimePtr());
	glui_spinner->set_float_limits(TIME_MIN, TIME_MAX, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	// Create a control to specify a keyframe (for updating / loading a keyframe)
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_spinner = glui_keyframe->add_spinner_to_panel(glui_panel, "Keyframe ID:", GLUI_SPINNER_INT, joint_ui_data->getIDPtr());
	glui_spinner->set_int_limits(KEYFRAME_MIN, KEYFRAME_MAX-1, GLUI_LIMIT_CLAMP);
	glui_spinner->set_speed(SPINNER_SPEED);

	glui_keyframe->add_separator();

	// Add buttons to load and update keyframes
	// Add buttons to load and save keyframes from a file
	// Add buttons to start / stop animation and to render frames to file
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframe", 0, loadKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Load Keyframes From File", 0, loadKeyframesFromFileButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Start / Stop Animation", 0, animateButton);
	glui_keyframe->add_column_to_panel(glui_panel, false);
	glui_keyframe->add_button_to_panel(glui_panel, "Update Keyframe", 0, updateKeyframeButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Save Keyframes To File", 0, saveKeyframesToFileButton);
	glui_keyframe->add_button_to_panel(glui_panel, "Render Frames To File", 0, renderFramesToFileButton);

	glui_keyframe->add_separator();

	// Add status line
	glui_panel = glui_keyframe->add_panel("");
	status = glui_keyframe->add_statictext_to_panel(glui_panel, "Status: Ready");

	// Add button to quit
	glui_panel = glui_keyframe->add_panel("", GLUI_PANEL_NONE);
	glui_keyframe->add_button_to_panel(glui_panel, "Quit", 0, quitButton);
	//
	// ***************************************************

	// Tell GLUI windows which window is main graphics window
	glui_joints->set_main_gfx_window(windowID);
	glui_joints2->set_main_gfx_window(windowID);
	glui_keyframe->set_main_gfx_window(windowID);
	glui_render->set_main_gfx_window(windowID);
}


// Performs most of the OpenGL intialization
void initGl(void)
{
    // glClearColor (red, green, blue, alpha)
    // Ignore the meaning of the 'alpha' value for now
    glClearColor(0.7f,0.7f,0.9f,0.7f);
}


// Calculates the interpolated joint DOF vector
// using Catmull-Rom interpolation of the keyframes
Vector getInterpolatedJointDOFS(float time)
{
	// Need to find the keyframes bewteen which
	// the supplied time lies.
	// At the end of the loop we have:
	//    keyframes[i-1].getTime() < time <= keyframes[i].getTime()
	//
	int i = 0;
	while( i <= maxValidKeyframe && keyframes[i].getTime() < time )
		i++;

	// If time is before or at first defined keyframe, then
	// just use first keyframe pose
	if( i == 0 )
		return keyframes[0].getDOFVector();

	// If time is beyond last defined keyframe, then just
	// use last keyframe pose
	if( i > maxValidKeyframe )
		return keyframes[maxValidKeyframe].getDOFVector();

	// Need to normalize time to (0, 1]
	time = (time - keyframes[i-1].getTime()) / (keyframes[i].getTime() - keyframes[i-1].getTime());

	// Get appropriate data points and tangent vectors
	// for computing the interpolation
	Vector p0 = keyframes[i-1].getDOFVector();
	Vector p1 = keyframes[i].getDOFVector();

	Vector t0, t1;
	if( i == 1 )							// special case - at beginning of spline
	{
		t0 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}
	else if( i == maxValidKeyframe )		// special case - at end of spline
	{
		t0 = (keyframes[i].getDOFVector() - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = keyframes[i].getDOFVector() - keyframes[i-1].getDOFVector();
	}
	else
	{
		t0 = (keyframes[i].getDOFVector()   - keyframes[i-2].getDOFVector()) * 0.5;
		t1 = (keyframes[i+1].getDOFVector() - keyframes[i-1].getDOFVector()) * 0.5;
	}

	// Return the interpolated Vector
	Vector a0 = p0;
	Vector a1 = t0;
	Vector a2 = p0 * (-3) + p1 * 3 + t0 * (-2) + t1 * (-1);
	Vector a3 = p0 * 2 + p1 * (-2) + t0 + t1;

	return (((a3 * time + a2) * time + a1) * time + a0);
}


// Callback idle function for animating the scene
void animate()
{
	// Only update if enough time has passed
	// (This locks the display to a certain frame rate rather
	//  than updating as fast as possible. The effect is that
	//  the animation should run at about the same rate
	//  whether being run on a fast machine or slow machine)
	
	if( frameRateTimer->elapsed() > SEC_PER_FRAME )
	{
		// Tell glut window to update itself. This will cause the display()
		// callback to be called, which renders the object (once you've written
		// the callback).
		glutSetWindow(windowID);
		glutPostRedisplay();

		// Restart the frame rate timer
		// for the next frame
		frameRateTimer->reset();
	}
}


// Handles the window being resized by updating the viewport
// and projection matrices
void reshape(int w, int h)
{
	// Update internal variables and OpenGL viewport
	Win[0] = w;
	Win[1] = h;
	glViewport(0, 0, (GLsizei)Win[0], (GLsizei)Win[1]);

    // Setup projection matrix for new window
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(CAMERA_FOVY, (GLdouble)Win[0]/(GLdouble)Win[1], NEAR_CLIP, FAR_CLIP);
}


// display callback
void display(void)
{
    // Clear the screen with the background colour
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
	glEnable(GL_DEPTH_TEST);

    // Setup the model-view transformation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	// Specify camera transformation
	glTranslatef(camXPos, camYPos, camZPos);


	// Get the time for the current animation step, if necessary
	if( animate_mode )
	{
		float curTime = animationTimer->elapsed();

		if( curTime >= keyframes[maxValidKeyframe].getTime() )
		{
			// Restart the animation
			animationTimer->reset();
			curTime = animationTimer->elapsed();
		}

		// Get the interpolated joint DOFs
		joint_ui_data->setDOFVector( getInterpolatedJointDOFS(curTime) );

		// Update user interface
		joint_ui_data->setTime(curTime);
		glui_keyframe->sync_live();

	}

	glPushMatrix();		
        // move camera
        glTranslatef(joint_ui_data->getDOF(Keyframe::CAM_TRANSLATE_X),
                     joint_ui_data->getDOF(Keyframe::CAM_TRANSLATE_Y),
                     joint_ui_data->getDOF(Keyframe::CAM_TRANSLATE_Z));
        
        glRotatef(joint_ui_data->getDOF(Keyframe::CAM_ROTATE_X), 1.0, 0.0, 0.0);
        glRotatef(joint_ui_data->getDOF(Keyframe::CAM_ROTATE_Y), 0.0, 1.0, 0.0);
        glRotatef(joint_ui_data->getDOF(Keyframe::CAM_ROTATE_Z), 0.0, 0.0, 1.0);
        
		glEnable(GL_RESCALE_NORMAL);
		
		// determine render style and set glPolygonMode appropriately
		if (renderStyle == 0) { 			// WIREFRAME
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			wire = true;
        
            //draw the background
            glPushMatrix();
                glTranslatef(0.0, -2.5, 0.0);
                drawGround();
            glPopMatrix();

            glPushMatrix();
            glTranslatef(7.0, 0.0, -6.0);
                drawTree(TRUNK1, WIDTH1, HEIGHT1);
            glPopMatrix();
            
            glPushMatrix();
                glTranslatef(-6.0, 0.0, -10.0);
                drawTree(TRUNK2, WIDTH2, HEIGHT2);
            glPopMatrix();

            // penguinA
            glPushMatrix();
                glTranslatef(4.0, 0.0, 2.0);
                drawPenguin(1);
            glPopMatrix();
            
            // penguinB
            glPushMatrix();	                
                glTranslatef(-4.0, 0.0, 2.0);
                drawPenguin(2);
            glPopMatrix();
           
            // see saw
            glPushMatrix();	      
                drawSeeSaw();
            glPopMatrix();
            
		} else if (renderStyle == 1) {		// METALLIC
			wire = false;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			
			glEnable(GL_LIGHTING);
			glEnable(GL_LIGHT0);
			
			glPushMatrix();
				glRotatef(lightPos, 0.0, 1.0, 0.0);
				GLfloat light_pos[] = {3, 5, 5, 1};
				glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
			glPopMatrix();
	    
            glPushMatrix();
                GLfloat ambg[] = {0.40, 0.02, 0.02, 1.0};
                GLfloat difg[] = {0.78, 0.57, 0.11, 1.0};
                GLfloat speg[] = {0.99, 0.94, 0.80, 1.0};
                
                glMaterialfv(GL_FRONT, GL_AMBIENT, ambg);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, difg);
                glMaterialfv(GL_FRONT, GL_SPECULAR, speg);
                glMaterialf(GL_FRONT, GL_SHININESS, 0.2 * 128.0);
                glTranslatef(0.0, -2.5, 0.0);
                drawGround();
            glPopMatrix();
	   
            glPushMatrix();                
                GLfloat ambt[] = {0.06, 0.4, 0.4, 1.0};
                GLfloat dift[] = {0.5, 0.8, 0.4, 1.0};	
                GLfloat spet[] = {0.2, 0.5, 0.4, 1.0};
                
                glMaterialfv(GL_FRONT, GL_AMBIENT, ambt);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, dift);
                glMaterialfv(GL_FRONT, GL_SPECULAR, spet);
                glMaterialf(GL_FRONT, GL_SHININESS, 0.2 * 128.0);
                
                glTranslatef(7.0, 0.0, -6.0);
                drawTree(TRUNK1, WIDTH1, HEIGHT1);
            glPopMatrix();
	    
            glPushMatrix();
                GLfloat ambt2[] = {0.36, 0.52, 0.44, 1.0};
                GLfloat dift2[] = {0.17, 0.41, 0.265, 1.0};	
                GLfloat spet2[] = {0.2, 0.5, 0.4, 1.0};
                
                glMaterialfv(GL_FRONT, GL_AMBIENT, ambt2);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, dift2);
                glMaterialfv(GL_FRONT, GL_SPECULAR, spet2);
                glMaterialf(GL_FRONT, GL_SHININESS, 0.1 * 128.0);
                  
                glTranslatef(-6.0, 0.0, -10.0);
                drawTree(TRUNK2, WIDTH2, HEIGHT2);
            glPopMatrix();
	    
			// penguinA
            glPushMatrix();
                GLfloat amb[] = {0.9, 0.9, 0.9, 1.0};
                GLfloat dif[] = {0.7, 0.3, 0.4, 1.0};
                GLfloat spe[] = {0.5, 0.4, 0.4, 1.0};
                
                glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
                glMaterialfv(GL_FRONT, GL_SPECULAR, spe);
                glMaterialf(GL_FRONT, GL_SHININESS, 0.1 * 128.0);
                
                glTranslatef(4.0, 0.0, 2.0);
                drawPenguin(1);
            glPopMatrix();
			
            // penguinB
            glPushMatrix();
                GLfloat amb2[] = {0.4, 0.45, 0.45, 1.0};
                GLfloat dif2[] = {0.4, 0.5, 0.5, 1.0};
                GLfloat spe2[] = {0.04, 0.7, 0.7, 1.0};
                
                glMaterialfv(GL_FRONT, GL_AMBIENT, amb2);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, dif2);
                glMaterialfv(GL_FRONT, GL_SPECULAR, spe2);
                glMaterialf(GL_FRONT, GL_SHININESS, 0.078125 * 128.0);
                
                glTranslatef(-4.0, 0.0, 2.0);
                drawPenguin(2);                
            glPopMatrix();

            // see saw
            glPushMatrix();
                GLfloat ambs[] = {0.40, 0.22, 0.02, 1.0};
                GLfloat difs[] = {0.78, 0.57, 0.11, 1.0};
                GLfloat spes[] = {0.99, 0.94, 0.80, 1.0};
                
                glMaterialfv(GL_FRONT, GL_AMBIENT, ambs);
                glMaterialfv(GL_FRONT, GL_DIFFUSE, difs);
                glMaterialfv(GL_FRONT, GL_SPECULAR, spes);
                glMaterialf(GL_FRONT, GL_SHININESS, 0.2 * 128.0);
                
                drawSeeSaw();
            glPopMatrix();
            
            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHTING);
		}
		
	glPopMatrix();

    // Execute any GL functions that are in the queue just to be safe
    glFlush();

	// Dump frame to file, if requested
	if( frameToFile )
	{
		sprintf(filenameF, "frame%03d.ppm", frameNumber);
		writeFrame(filenameF, false, false);
	}

    // Now, show the frame buffer that we just drew into.
    // (this prevents flickering).
    glutSwapBuffers();
}


// Handles mouse button pressed / released events
void mouse(int button, int state, int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( button == GLUT_RIGHT_BUTTON )
	{
		if( state == GLUT_DOWN )
		{
			lastX = x;
			lastY = y;
			updateCamZPos = true;
		}
		else
		{
			updateCamZPos = false;
		}
	}
}


// Handles mouse motion events while a button is pressed
void motion(int x, int y)
{
	// If the RMB is pressed and dragged then zoom in / out
	if( updateCamZPos )
	{
		// Update camera z position
		camZPos += (x - lastX) * ZOOM_SCALE;
		lastX = x;

		// Redraw the scene from updated camera position
		glutSetWindow(windowID);
		glutPostRedisplay();
	}
}


// draw entire penguin
void drawPenguin(int s) {
    glPushMatrix();
        // set up transformation for whole penguin
        if (s == 1) {
            glTranslatef(joint_ui_data->getDOF(Keyframe::BODY_TRANSLATE_X),
                joint_ui_data->getDOF(Keyframe::BODY_TRANSLATE_Y),
                joint_ui_data->getDOF(Keyframe::BODY_TRANSLATE_Z));
            
            glRotatef(joint_ui_data->getDOF(Keyframe::BODY_ROTATE_X), 1.0, 0.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::BODY_ROTATE_Y), 0.0, 1.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::BODY_ROTATE_Z), 0.0, 0.0, 1.0);
        } else {
            glTranslatef(joint_ui_data->getDOF(Keyframe::BODY2_TRANSLATE_X),
                joint_ui_data->getDOF(Keyframe::BODY2_TRANSLATE_Y),
                joint_ui_data->getDOF(Keyframe::BODY2_TRANSLATE_Z));
            
            glRotatef(joint_ui_data->getDOF(Keyframe::BODY2_ROTATE_X), 1.0, 0.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::BODY2_ROTATE_Y), 0.0, 1.0, 0.0);
            glRotatef(joint_ui_data->getDOF(Keyframe::BODY2_ROTATE_Z), 0.0, 0.0, 1.0);
        }
        glScalef(0.8, 0.8, 0.8);
        
        // draw left leg
        glPushMatrix();
            glColor3f(0.5, 0.5, 0.5);
            
            glTranslatef(0.15, -1.05, 0.0);
            if (s == 1) {
                glRotatef(-joint_ui_data->getDOF(Keyframe::L_HIP_PITCH), 1.0, 0.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_YAW), 0.0, 1.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP_ROLL), 0.0, 0.0, 1.0);
            } else {
                glRotatef(-joint_ui_data->getDOF(Keyframe::L_HIP2_PITCH), 1.0, 0.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP2_YAW), 0.0, 1.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::L_HIP2_ROLL), 0.0, 0.0, 1.0);

            }
            glTranslatef(0.0, -0.25, 0.0);
            
            // draw foot
            glPushMatrix();
                // move the foot relative to the leg
                glTranslatef(0.0, -0.3, -0.1);
                if (s == 1) {
                    glRotatef(-joint_ui_data->getDOF(Keyframe::L_KNEE), 1.0, 0.0, 0.0);
                } else {
                    glRotatef(-joint_ui_data->getDOF(Keyframe::L_KNEE2), 1.0, 0.0, 0.0);
                }
                drawFoot();
            glPopMatrix();
            drawCube(0.05, 0.3);
        glPopMatrix();
        
        // draw right leg
        glPushMatrix();
            glColor3f(0.5, 0.5, 0.5);
            glTranslatef(-0.15, -1.05, 0.0);
            if (s == 1) {
                glRotatef(-joint_ui_data->getDOF(Keyframe::R_HIP_PITCH), 1.0, 0.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP_YAW), 0.0, 1.0, 0.0);
                glRotatef(-joint_ui_data->getDOF(Keyframe::R_HIP_ROLL), 0.0, 0.0, 1.0);
            } else {
                glRotatef(-joint_ui_data->getDOF(Keyframe::R_HIP2_PITCH), 1.0, 0.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::R_HIP2_YAW), 0.0, 1.0, 0.0);
                glRotatef(-joint_ui_data->getDOF(Keyframe::R_HIP2_ROLL), 0.0, 0.0, 1.0);
            }
            glTranslatef(0.0, -0.25, 0.0);
            glScalef(-1.0, 1.0, 1.0);
            
            // draw foot
            glPushMatrix();
                // move the foot relative to the leg
                glTranslatef(0.0, -0.3, -0.1);
                if (s == 1) {
                    glRotatef(-joint_ui_data->getDOF(Keyframe::R_KNEE), 1.0, 0.0, 0.0);
                } else {
                    glRotatef(-joint_ui_data->getDOF(Keyframe::R_KNEE2), 1.0, 0.0, 0.0);
                }
                drawFoot();
            glPopMatrix();
            
            drawCube(0.05, 0.3);
        glPopMatrix();
        
        // draw body
        glPushMatrix();
            glColor3f(0.9, 0.9, 0.8);
            drawBody();
        glPopMatrix();

        // draw head
        glPushMatrix();
            if (s == 1) {
                glRotatef(joint_ui_data->getDOF(Keyframe::HEAD), 0.0, 1.0, 0.0);
            } else {
                glRotatef(joint_ui_data->getDOF(Keyframe::HEAD2), 0.0, 1.0, 0.0);
            }
            glTranslatef(0.0, 1.4, 0.0);
            glColor3f(0.5, 0.5, 0.5);
            
            drawCube(0.7, 0.4);
            
            // draw beak
            glPushMatrix();
                glColor3f(0.9, 0.8, 0.0);
                
                // upper beak
                glTranslatef(0.0, -0.1, 1.0);
                drawBeak(0.04);
                
                // lower beak
                if (s == 1) {
                    glTranslatef(0.0, joint_ui_data->getDOF(Keyframe::BEAK), 0.0);
                } else {
                    glTranslatef(0.0, joint_ui_data->getDOF(Keyframe::BEAK2), 0.0);
                }
                glTranslatef(0.0, -0.06, 0.0);
                drawBeak(0.02);
            glPopMatrix();
        glPopMatrix();
        
        // draw left arm
        glPushMatrix();
            glTranslatef(0.9, 0.0, 0.0);
            if (s == 1) {
                glRotatef(-joint_ui_data->getDOF(Keyframe::L_SHOULDER_PITCH), 1.0, 0.0, 0.0);
                glRotatef(-joint_ui_data->getDOF(Keyframe::L_SHOULDER_YAW), 0.0, 1.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER_ROLL), 0.0, 0.0, 1.0);
            } else {
                glRotatef(-joint_ui_data->getDOF(Keyframe::L_SHOULDER2_PITCH), 1.0, 0.0, 0.0);
                glRotatef(-joint_ui_data->getDOF(Keyframe::L_SHOULDER2_YAW), 0.0, 1.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::L_SHOULDER2_ROLL), 0.0, 0.0, 1.0);
            }
            glTranslatef(0.0, 0.09, 0.0);
            glColor3f(0.5, 0.5, 0.5);
            glScalef(1.0, 0.7, 1.0);
            
            // draw left hand
            glPushMatrix();
                glTranslatef(0.0, -1.08, -0.05);
                if (s == 1) {
                    glRotatef(-joint_ui_data->getDOF(Keyframe::L_ELBOW), 1.0, 0.0, 0.0);
                } else {
                    glRotatef(-joint_ui_data->getDOF(Keyframe::L_ELBOW2), 1.0, 0.0, 0.0);
                }
                glTranslatef(0.0, 1.08, 0.05);
                drawHand();
            glPopMatrix();
            drawArm();
        glPopMatrix();
        
        // draw right arm
        glPushMatrix();
            glTranslatef(-0.9, 0.0, 0.0);
            if (s == 1) {
                glRotatef(-joint_ui_data->getDOF(Keyframe::R_SHOULDER_PITCH), 1.0, 0.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER_YAW), 0.0, 1.0, 0.0);
                glRotatef(-joint_ui_data->getDOF(Keyframe::R_SHOULDER_ROLL), 0.0, 0.0, 1.0);
            } else {
                glRotatef(-joint_ui_data->getDOF(Keyframe::R_SHOULDER2_PITCH), 1.0, 0.0, 0.0);
                glRotatef(joint_ui_data->getDOF(Keyframe::R_SHOULDER2_YAW), 0.0, 1.0, 0.0);
                glRotatef(-joint_ui_data->getDOF(Keyframe::R_SHOULDER2_ROLL), 0.0, 0.0, 1.0);
            }
            glColor3f(0.5, 0.5, 0.5);
            glScalef(-1.0, 0.7, 1.0);
            
            // draw right hand
            glPushMatrix();
                glTranslatef(0.0, -1.08, -0.05);
                if (s == 1) {
                    glRotatef(-joint_ui_data->getDOF(Keyframe::R_ELBOW), 1.0, 0.0, 0.0);
                } else {
                    glRotatef(-joint_ui_data->getDOF(Keyframe::R_ELBOW2), 1.0, 0.0, 0.0);
                }
                glTranslatef(0.0, 1.08, 0.05);
                drawHand();
            glPopMatrix();
            
            drawArm();
        glPopMatrix();
    glPopMatrix();
}


// draw seesaw
void drawSeeSaw() {
    glPushMatrix();
        glColor3f(0.6, 0.4, 0.3);
        // plank
        glPushMatrix();
            glRotatef(joint_ui_data->getDOF(Keyframe::PLANK), 
                0.0, 0.0, 1.0);
            drawSSPlank();
        glPopMatrix();
        
        // right leg
        glPushMatrix();
            glRotatef(45.0, 0.0, 0.0, 1.0);
            drawSSLegs();
        glPopMatrix();
        
        // left leg
        glPushMatrix();
            glScalef(-1.0, 1.0, 1.0);
            glRotatef(45.0, 0.0, 0.0, 1.0);
            drawSSLegs();
        glPopMatrix();
	
    glPopMatrix();
}



void drawTree(float trunk, float width, float height)
{
    glPushMatrix();
        // GLfloat ambt[] = {0.006, 0.22, 0.14, 1.0};
        // GLfloat dift[] = {0.17, 0.41, 0.265, 1.0};
        // GLfloat spet[] = {0.2, 0.5, 0.4, 1.0};
        
        // glMaterialfv(GL_FRONT, GL_AMBIENT, ambt);
        // glMaterialfv(GL_FRONT, GL_DIFFUSE, dift);     

        glColor3f(0.0,0.9,0.9);
        drawCylinder(trunk,height);
        glTranslatef(0.0,height,0.0);
        
        glColor3f(0.0,0.8,1);
        drawPyramid(width, height);
        glTranslatef(0.0,height/4,0.0);
        glColor3f(0.0,0.8,1);
        drawPyramid(width, height/2);
        glTranslatef(0.0,height/4,0.0);
        glColor3f(0.0,0.8,1);
        drawPyramid(width, height/3);
        // glTranslatef(0.0,3*height/2,0.0);
    glPopMatrix();
}

/*
void drawTree(float trunk, float width, float height)
{   
    if(!wire) {
        glPushMatrix();
            GLfloat ambt[] = {0.006, 0.22, 0.14, 1.0};
            GLfloat dift[] = {0.17, 0.41, 0.265, 1.0};
            //GLfloat spet[] = {0.2, 0.5, 0.4, 1.0};	      
            glMaterialfv(GL_FRONT, GL_AMBIENT, ambt);
            glMaterialfv(GL_FRONT, GL_DIFFUSE, dift);     
     
            //glColor3f(0.0,0.9,0.9);
            drawCylinder(trunk,height);
            glTranslatef(0.0,height,0.0);
            
            //glColor3f(0.0,0.8,1);
            drawPyramid(width, height);
            glTranslatef(0.0,height/4,0.0);
            //glColor3f(0.0,0.8,1);
            drawPyramid(width, height/2);
            glTranslatef(0.0,height/4,0.0);
            //glColor3f(0.0,0.8,1);
            drawPyramid(width, height/3);
            // glTranslatef(0.0,3*height/2,0.0);
        glPopMatrix();
     
    } else {
        glPushMatrix();
            glColor3f(0.0,0.9,0.9);
            drawCylinder(trunk,height);
            glTranslatef(0.0,height,0.0);
            
            glColor3f(0.0,0.8,1);
            drawPyramid(width, height);
            glTranslatef(0.0,height/4,0.0);
            glColor3f(0.0,0.8,1);
            drawPyramid(width, height/2);
            glTranslatef(0.0,height/4,0.0);
            glColor3f(0.0,0.8,1);
            drawPyramid(width, height/3);
            // glTranslatef(0.0,3*height/2,0.0);
        glPopMatrix();
    }
}
*/

///////////////////////////////////////////////////////////
//
// BELOW ARE FUNCTIONS FOR GENERATING IMAGE FILES (PPM/PGM)
//
///////////////////////////////////////////////////////////

void writePGM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P5\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			fwrite(&buffer[y*width],sizeof(GLubyte),width,fp);
			/*
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%c",int(buffer[x+y*width];
			}
			*/
		}
	}
	else
	{
		fprintf(fp,"P2\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				fprintf(fp,"%d ",int(buffer[x+y*width]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

#define RED_OFFSET   0
#define GREEN_OFFSET 1
#define BLUE_OFFSET  2

void writePPM(char* filename, GLubyte* buffer, int width, int height, bool raw=true)
{
	FILE* fp = fopen(filename,"wt");

	if( fp == NULL )
	{
		printf("WARNING: Can't open output file %s\n",filename);
		return;
	}

	if( raw )
	{
		fprintf(fp,"P6\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%c%c%c",int(pix[RED_OFFSET]),
									int(pix[GREEN_OFFSET]),
									int(pix[BLUE_OFFSET]));
			}
		}
	}
	else
	{
		fprintf(fp,"P3\n%d %d\n%d\n",width,height,255);
		for(int y=height-1;y>=0;y--)
		{
			for(int x=0;x<width;x++)
			{
				GLubyte* pix = &buffer[4*(x+y*width)];

				fprintf(fp,"%d %d %d ",int(pix[RED_OFFSET]),
									   int(pix[GREEN_OFFSET]),
									   int(pix[BLUE_OFFSET]));
			}
			fprintf(fp,"\n");
		}
	}

	fclose(fp);
}

void writeFrame(char* filename, bool pgm, bool frontBuffer)
{
	static GLubyte* frameData = NULL;
	static int currentSize = -1;

	int size = (pgm ? 1 : 4);

	if( frameData == NULL || currentSize != size*Win[0]*Win[1] )
	{
		if (frameData != NULL)
			delete [] frameData;

		currentSize = size*Win[0]*Win[1];

		frameData = new GLubyte[currentSize];
	}

	glReadBuffer(frontBuffer ? GL_FRONT : GL_BACK);

	if( pgm )
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_LUMINANCE, GL_UNSIGNED_BYTE, frameData);
		writePGM(filename, frameData, Win[0], Win[1]);
	}
	else
	{
		glReadPixels(0, 0, Win[0], Win[1],
					 GL_RGBA, GL_UNSIGNED_BYTE, frameData);
		writePPM(filename, frameData, Win[0], Win[1]);
	}
}

void drawGround()
{
    glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0, 1.0, 0.0);
	    glVertex3f(-15,0,15);
	    glVertex3f(-15,0,-15);
	    glVertex3f(15,0,-15);
	    glVertex3f(15,0,15);
    glEnd();
}