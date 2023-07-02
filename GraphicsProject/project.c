/******************************************************************************
 *
 * Computer Graphics Programming 2020 Project Template v1.0 (11/04/2021)
 *
 * Based on: Animation Controller v1.0 (11/04/2021)
 *
 * This template provides a basic FPS-limited render loop for an animated scene,
 * plus keyboard handling for smooth game-like control of an object such as a
 * character or vehicle.
 *
 * A simple static lighting setup is provided via initLights(), which is not
 * included in the animationalcontrol.c template. There are no other changes.
 *
 ******************************************************************************/

#include <Windows.h>
#include <freeglut.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

 /******************************************************************************
  * Animation & Timing Setup
  ******************************************************************************/

// Target frame rate (number of Frames Per Second).
#define TARGET_FPS 60				

// Ideal time each frame should be displayed for (in milliseconds).
const unsigned int FRAME_TIME = 1000 / TARGET_FPS;

// Frame time in fractional seconds.
// Note: This is calculated to accurately reflect the truncated integer value of
// FRAME_TIME, which is used for timing, rather than the more accurate fractional
// value we'd get if we simply calculated "FRAME_TIME_SEC = 1.0f / TARGET_FPS".
const float FRAME_TIME_SEC = (1000 / TARGET_FPS) / 1000.0f;

// Time we started preparing the current frame (in milliseconds since GLUT was initialized).
unsigned int frameStartTime = 0;

/******************************************************************************
 * Some Simple Definitions of Motion
 ******************************************************************************/

#define MOTION_NONE 0				// No motion.
#define MOTION_CLOCKWISE -1			// Clockwise rotation.
#define MOTION_ANTICLOCKWISE 1		// Anticlockwise rotation.
#define MOTION_BACKWARD -1			// Backward motion.
#define MOTION_FORWARD 1			// Forward motion.
#define MOTION_LEFT -1				// Leftward motion.
#define MOTION_RIGHT 1				// Rightward motion.
#define MOTION_DOWN -1				// Downward motion.
#define MOTION_UP 1					// Upward motion.
#define WIDTH 200
#define HEIGHT 200
#define SCALE 0.2f
 // Represents the motion of an object on four axes (Yaw, Surge, Sway, and Heave).
 // 
 // You can use any numeric values, as specified in the comments for each axis. However,
 // the MOTION_ definitions offer an easy way to define a "unit" movement without using
 // magic numbers (e.g. instead of setting Surge = 1, you can set Surge = MOTION_FORWARD).
 //
typedef GLubyte Texture3D[100][100][3];

typedef struct {
	unsigned char* data;
	int width;
	int height;
} ImageData;

typedef struct {
	int r;
	int g;
	int b;
	float greyscale;

} pixel;

typedef struct {
	double x;
	double y;
	double z;
	int alive;
	double velocityx;
	double velocityz;
	int colorCode;
} Spotlight;

typedef struct {
	int Yaw;		// Turn about the Z axis	[<0 = Clockwise, 0 = Stop, >0 = Anticlockwise]
	int Surge;		// Move forward or back		[<0 = Backward,	0 = Stop, >0 = Forward]
	int Sway;		// Move sideways (strafe)	[<0 = Left, 0 = Stop, >0 = Right]
	int Heave;		// Move vertically			[<0 = Down, 0 = Stop, >0 = Up]
} motionstate4_t;

typedef struct {
	float x, y, z;
} vec3d;

typedef struct {
	float x, y;
} vec2d;

typedef struct {
	int vIndex[3], tIndex[3], nIndex[3];
} meshObjectFace;

typedef struct {
	int vertexCount;
	vec3d* vertices;
	int texCoordCount;
	vec2d* texCoords;
	int normalCount;
	vec3d* normals;
	int faceCount;
	meshObjectFace* faces;
} meshObject;

typedef struct {
	GLfloat angle;
	GLfloat axisX;
	GLfloat axisY;
	GLfloat axisZ;
}Electron;
/******************************************************************************
 * Keyboard Input Handling Setup
 ******************************************************************************/

 // Represents the state of a single keyboard key.Represents the state of a single keyboard key.
typedef enum {
	KEYSTATE_UP = 0,	// Key is not pressed.
	KEYSTATE_DOWN		// Key is pressed down.
} keystate_t;

// Represents the states of a set of keys used to control an object's motion.
typedef struct {
	keystate_t MoveForward;
	keystate_t MoveBackward;
	keystate_t MoveLeft;
	keystate_t MoveRight;
	keystate_t MoveUp;
	keystate_t MoveDown;
	keystate_t TurnLeft;
	keystate_t TurnRight;
} motionkeys_t;

// Current state of all keys used to control our "player-controlled" object's motion.
motionkeys_t motionKeyStates = {
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP,
	KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP, KEYSTATE_UP };

// How our "player-controlled" object should currently be moving, solely based on keyboard input.
//
// Note: this may not represent the actual motion of our object, which could be subject to
// other controls (e.g. mouse input) or other simulated forces (e.g. gravity).
motionstate4_t keyboardMotion = { MOTION_NONE, MOTION_NONE, MOTION_NONE, MOTION_NONE };

// Define all character keys used for input (add any new key definitions here).
// Note: USE ONLY LOWERCASE CHARACTERS HERE. The keyboard handler provided converts all
// characters typed by the user to lowercase, so the SHIFT key is ignored.

#define KEY_MOVE_FORWARD	'w'
#define KEY_MOVE_BACKWARD	's'
#define KEY_MOVE_LEFT		'a'
#define KEY_MOVE_RIGHT		'd'
#define KEY_RENDER_FILL		'l'
#define KEY_EXIT			27 // Escape key.

// Define all GLUT special keys used for input (add any new key definitions here).

#define SP_KEY_MOVE_UP		GLUT_KEY_UP
#define SP_KEY_MOVE_DOWN	GLUT_KEY_DOWN
#define SP_KEY_TURN_LEFT	GLUT_KEY_LEFT
#define SP_KEY_TURN_RIGHT	GLUT_KEY_RIGHT

/******************************************************************************
 * GLUT Callback Prototypes
 ******************************************************************************/

void display(void);
void reshape(int width, int h);
void keyPressed(unsigned char key, int x, int y);
void specialKeyPressed(int key, int x, int y);
void keyReleased(unsigned char key, int x, int y);
void specialKeyReleased(int key, int x, int y);
void idle(void);

/******************************************************************************
 * Animation-Specific Function Prototypes (add your own here)
 ******************************************************************************/
void drawOrigin(void);
void main(int argc, char** argv);
void init(void);
void think(void);
void initLights(void);
void drawPropeller(GLfloat x, GLfloat y, GLfloat z);
void drawChopper(GLfloat x, GLfloat y, GLfloat z);
void loadImage(void);
void drawTerrain(float terrainScale);
void CrossProduct(GLfloat* v1, GLfloat* v2, GLfloat* crossProduct);
void Normalize(GLfloat* v);
void loadTexture(char str[], Texture3D* texture);
void drawSkyCylinder(float radius, float height, int numSegments);
void terrainColorPicker(GLfloat height);
void drawHelipad(float radius, float height, int numSegments);
void drawCube(float posX, float posY, float posZ, float size);
void drawWindmill(double rotation, GLfloat x, GLfloat y, GLfloat z);
void drawSpotlight(GLenum light, Spotlight spotlight);
void drawBitmapString(const char* str, float x, float y, float r, float g, float b);
void resetSpotlight(Spotlight* spotlight);
void flashColors(GLfloat coneColours[][4]);
void drawAtom(void);

/******************************************************************************
 * Animation-Specific Setup (Add your own definitions, constants, and globals here)
 ******************************************************************************/

 // Render objects as filled polygons (1) or wireframes (0). Default filled.
int renderFillEnabled = 1;
GLfloat cameraPosition[] = { 1, 1, 5 };
GLfloat bladeRotation[] = { 0, 90 };
GLfloat windmillBladeRotation[] = { 0, 90 };
GLfloat heliCoord[] = { 0.0f, 10.05f, -38.0f };
GLint windowWidth = 500;
GLint windowHeight = 400;
const float PI = 3.14159265358979323846f;
const int NUM_SEGMENTS = 100;
GLUquadricObj* myQuadric;
GLUquadricObj* cone;
GLUquadricObj* windMill;
GLfloat heliX = 210;
GLfloat heliY = 0;
GLfloat heliZ = 0;
GLfloat lastFrameTime = 0.0f;
GLfloat helicopterVelocityY = 0.0f;
const GLfloat gravity = -0.8f;
GLint gravityon = 1;
GLint taperX = 0;
GLfloat rx, pitch = 0;
GLfloat moveSpeed = 0.02f;
GLfloat sideMoveSpeed = 0.00f;
GLuint texture_id;
GLuint texture_id2;
int imageWidth, imageHeight;
GLfloat bladeSpeed = 0.0f;
pixel imageData[HEIGHT][WIDTH];
Texture3D groundTexture;
Texture3D concreteTexture;
Texture3D waterTexture;
int grounded = 1;
const float TERRAINCOLOUR1[] = { 0.0275f, 0.3608f, 0.0431f, 1.0f };
const float TERRAINCOLOUR2[] = { 0.0588f, 0.4000f, 0.0196f, 1.0f };
const float TERRAINCOLOUR3[] = { 0.1216f, 0.4471f, 0.0392f, 1.0f };
const float TERRAINCOLOUR4[] = { 0.2039f, 0.3137f, 0.0941f, 1.0f };
const float TERRAINCOLOUR5[] = { 0.3020f, 0.5569f, 0.2431f, 1.0f };
const float TERRAINCOLOUR6[] = { 0.3020f, 0.5569f, 0.2431f, 1.0f };
const float TERRAINCOLOUR7[] = { 0.3882f, 0.5765f, 0.3412f, 1.0f };
const float TERRAINCOLOUR8[] = { 0.4431f, 0.5882f, 0.3882f, 1.0f };
const float TERRAINCOLOUR9[] = { 0.4431f, 0.5608f, 0.3490f, 1.0f };
const float TERRAINCOLOUR10[] = { 0.4431f, 0.5098f, 0.3059f, 1.0f };
const float TERRAINCOLOUR11[] = { 0.4431f, 0.4784f, 0.2627f, 1.0f };
const float TERRAINCOLOUR12[] = { 0.4471f, 0.4039f, 0.1843f, 1.0f };
const float TERRAINCOLOUR13[] = { 0.4471f, 0.3686f, 0.1373f, 1.0f };
const float TERRAINCOLOUR14[] = { 0.4431f, 0.3412f, 0.0941f, 1.0f };
const float TERRAINCOLOUR15[] = { 0.4431f, 0.3412f, 0.0941f, 1.0f };
const float TERRAINCOLOUR16[] = { 0.5176f, 0.3804f, 0.1922f, 1.0f };
const float TERRAINCOLOUR17[] = { 0.6353f, 0.5019f, 0.3137f, 1.0f };
const float TERRAINCOLOUR18[] = { 0.7255f, 0.6000f, 0.4471f, 1.0f };
const float TERRAINCOLOUR19[] = { 0.8509f, 0.7294f, 0.6157f, 1.0f };
const float SKYBLUE[] = { 0.271f, 0.678f, 0.922f, 1.0f };
const float GOLD[] = { 0.9804f, 0.9019f, 0.6196f, 1.0f };
const float BLUE[] = { 0.2784f, 0.5725f, 0.9019f, 1.0f };
const float BODYCOLOR[] = { 0.75f, 0.75f, 0.75f, 1.0f };
const float BLACK[] = { 0.0f, 0.0f, 0.0f, 1.0f };
const float WHITE[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const float CREAM[] = { 0.9686f, 0.9608f, 0.8f, 1.0f };
const float RED[] = { 0.7176f, 0.1608f, 0.1608f, 1.0f };
meshObject obj;
GLfloat lightX = 0.0f;
double windmillRotation[20];
GLfloat lightVelocityX = 0.03;
Spotlight spotlights[25];
GLenum lights[25];

const double windmillCoordinates[][3] = {
		{14.127081, 9.732650, -1.002306},
	{-7.250275, 10.658718, 66.065704},
	{43.784729, 9.575411, -69.858345},
	{-80.844543, 6.885019, -23.915012},
	{-0.916259, 9.763863, -26.361935},
	{-44.359699, 9.938830, -6.498155},
	{43.469288, 8.574178, 9.094833},
	{66.182175, 8.805546, 49.777252}
};

GLfloat coneColours[][4] = {
	{ 10.0f, 0.0f, 0.0f, 0.20f },   
	{ 10.0f, 5.0f, 0.0f, 0.20f },    
	{ 10.0f, 10.0f, 0.0f, 0.20f },   
	{ 0.0f, 10.0f, 0.0f, 0.20f },    
	{ 0.0f, 0.0f, 10.0f, 0.20f },    
	{ 4.0f, 0.0f, 8.0f, 0.20f },    
	{ 8.0f, 0.0f, 10.0f, 0.20f }    
};
GLfloat lightColours[][4] = {
	{ 10.0f, 0.0f, 0.0f, 1.0f },    
	{ 10.0f, 5.0f, 0.0f, 1.0f },    
	{ 10.0f, 10.0f, 0.0f, 1.0f },   
	{ 0.0f, 10.0f, 0.0f, 1.0f },    
	{ 0.0f, 0.0f, 10.0f, 1.0f },    
	{ 4.0f, 0.0f, 8.0f, 1.0f },     
	{ 8.0f, 0.0f, 10.0f, 1.0f }     
};
int score = 0;
int verticalSpeedResetSwitch = 0;
int tip = 1;
GLfloat orbitRadius = 3.0f; 
GLfloat orbitSpeed = 0.05f; 
GLfloat electronAngle = 0.0f;  
int numElectrons = 1;

/******************************************************************************
 * Entry Point (don't put anything except the main function here)
 ******************************************************************************/
void main(int argc, char** argv)
{
	// Initialize the OpenGL window.
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Project");

	// Set up the scene.
	init();
	
	// Disable key repeat (keyPressed or specialKeyPressed will only be called once when a key is first pressed).
	glutSetKeyRepeat(GLUT_KEY_REPEAT_OFF);

	// Register GLUT callbacks.
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyPressed);
	glutSpecialFunc(specialKeyPressed);
	glutKeyboardUpFunc(keyReleased);
	glutSpecialUpFunc(specialKeyReleased);
	glutIdleFunc(idle);

	// Record when we started rendering the very first frame (which should happen after we call glutMainLoop).
	frameStartTime = (unsigned int)glutGet(GLUT_ELAPSED_TIME);

	// Enter the main drawing loop (this will never return).
	glutMainLoop();
}

/******************************************************************************
 * GLUT Callbacks (don't add any other functions here)
 ******************************************************************************/

 /*
	 Called when GLUT wants us to (re)draw the current animation frame.

	 Note: This function must not do anything to update the state of our simulated
	 world. Animation (moving or rotating things, responding to keyboard input,
	 etc.) should only be performed within the think() function provided below.
 */
void display(void) {



	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, 1.2f, 1.0f, 300.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	//Control 'l'
	if (!renderFillEnabled) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	}
	if (renderFillEnabled) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	//Camera calculations
	GLfloat objectRadius = 5.0f; 
	GLfloat objectRotationX = heliX * PI / 180.0f;
	GLfloat objectRotationY = heliY * PI / 180.0f;
	GLfloat objectRotationZ = heliZ * PI / 180.0f;

	GLfloat cameraDistance = 4.0f;

	GLfloat objectX = heliCoord[0] + objectRadius * cos(objectRotationX);
	GLfloat objectY = heliCoord[1] + objectRadius * sin(objectRotationY);
	GLfloat objectZ = heliCoord[2] + objectRadius * sin(objectRotationZ);

	GLfloat cameraX = heliCoord[0] + cameraDistance * sin(objectRotationX);
	GLfloat cameraY = objectY + 1;
	GLfloat cameraZ = heliCoord[2] + cameraDistance * cos(objectRotationX);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture_id);

	glDisable(GL_TEXTURE_2D);

	gluLookAt(cameraX, cameraY, cameraZ,
	heliCoord[0], heliCoord[1], heliCoord[2],
	0.0f, 1.0f, 0.0f);

	//Sky
	drawSkyCylinder(100, 80, 50);
	
	//Ground
	drawTerrain(1);
	
	//Sky atom :)
	glPushMatrix();
	glTranslatef(0.0, 25.0, 0.0);
	glScalef(3.0, 3.0, 3.0);
	drawAtom();
	glPopMatrix();

	//Helicopter
	glPushMatrix();
	drawChopper(heliCoord[0], heliCoord[1], heliCoord[2]);
	glPopMatrix();

	//Helipad
	glPushMatrix();
	glTranslatef(0.0f, 9.35f, -38.0f);
	glColor3f(1.0, 1.0, 1.0);
	drawHelipad(3.0, 0.05, 40);
	glPopMatrix();

	//Spotlights
	drawSpotlight(GL_LIGHT1,spotlights[0], coneColours[spotlights[0].colorCode], lightColours[spotlights[0].colorCode]);
	drawSpotlight(GL_LIGHT2, spotlights[1], coneColours[spotlights[1].colorCode], lightColours[spotlights[1].colorCode]);
	drawSpotlight(GL_LIGHT3,spotlights[2], coneColours[spotlights[2].colorCode], lightColours[spotlights[2].colorCode]);
	drawSpotlight(GL_LIGHT4,spotlights[3], coneColours[spotlights[3].colorCode], lightColours[spotlights[3].colorCode]);
	drawSpotlight(GL_LIGHT5, spotlights[4], coneColours[spotlights[4].colorCode], lightColours[spotlights[4].colorCode]);
	drawSpotlight(GL_LIGHT6, spotlights[5], coneColours[spotlights[5].colorCode], lightColours[spotlights[5].colorCode]);
	drawSpotlight(GL_LIGHT7, spotlights[6], coneColours[spotlights[6].colorCode], lightColours[spotlights[6].colorCode]);
	
	//Windmills
	int numLines = sizeof(windmillCoordinates) / sizeof(windmillCoordinates[0]);

	for (int i = 0; i < numLines; i++) {

		double x = windmillCoordinates[i][0];
		double y = windmillCoordinates[i][1];
		double z = windmillCoordinates[i][2];
		drawWindmill(windmillRotation[i], x, y, z);
	}

	
	//HUD
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, windowWidth, 0, windowHeight, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	
	char scoreString[256];
	sprintf_s(scoreString, sizeof(scoreString), "Score: %d", score);

	drawBitmapString(scoreString, 10, windowHeight - 20, 1.0f, 1.0f, 1.0f);
	if (tip == 1) {
		drawBitmapString("Catch the spotlights to score points", 320, windowHeight - 70, 1.0f, 1.0f, 1.0f);
		drawBitmapString(" and add electrons to the sky atom", 322, windowHeight - 97, 1.0f, 1.0f, 1.0f);
	}
	glutSwapBuffers();
}


/*
	Called when the OpenGL window has been resized.
*/
void reshape(int width, int h)
{// update the new width
	windowWidth = width;
	// update the new height
	windowHeight = h;

	// update the viewport to still be all of the window
	glViewport(0, 0, windowWidth, windowHeight);

	// change into projection mode so that we can change the camera properties
	glMatrixMode(GL_PROJECTION);

	// load the identity matrix into the projection matrix
	glLoadIdentity();

	// gluPerspective(fovy, aspect, near, far)
	gluPerspective(45, (float)windowWidth / (float)windowHeight, 1, 20);

	// change into model-view mode so that we can change the object positions
	glMatrixMode(GL_MODELVIEW);
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is pressed.
*/
void keyPressed(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is pressed, we do two things:
			(1) Update motionKeyStates to record that the key is held down. We use
				this later in the keyReleased callback.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. The most recent key always "wins" (e.g. if
				you're holding down KEY_MOVE_LEFT then also pressed KEY_MOVE_RIGHT,
				our object will immediately start moving right).
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_FORWARD;
		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_DOWN;
		keyboardMotion.Surge = MOTION_BACKWARD;
		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_LEFT;
		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_DOWN;
		keyboardMotion.Sway = MOTION_RIGHT;
		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Rather than using literals (e.g. "t" for spotlight), create a new KEY_
			definition in the "Keyboard Input Handling Setup" section of this file.
			For example, refer to the existing keys used here (KEY_MOVE_FORWARD,
			KEY_MOVE_LEFT, KEY_EXIT, etc).
		*/
	case KEY_RENDER_FILL:
		renderFillEnabled = !renderFillEnabled;
		break;
	case KEY_EXIT:
		exit(0);
		break;
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is pressed.
*/
void specialKeyPressed(int key, int x, int y)
{
	switch (key) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyPressed.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_UP;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_DOWN;
		keyboardMotion.Heave = MOTION_DOWN;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_DOWN;

		keyboardMotion.Yaw = MOTION_ANTICLOCKWISE;
		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_DOWN;

		keyboardMotion.Yaw = MOTION_CLOCKWISE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			Rather than directly using the GLUT constants (e.g. GLUT_KEY_F1), create
			a new SP_KEY_ definition in the "Keyboard Input Handling Setup" section of
			this file. For example, refer to the existing keys used here (SP_KEY_MOVE_UP,
			SP_KEY_TURN_LEFT, etc).
		*/
	}
}

/*
	Called each time a character key (e.g. a letter, number, or symbol) is released.
*/
void keyReleased(unsigned char key, int x, int y)
{
	switch (tolower(key)) {

		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			Whenever one of our movement keys is released, we do two things:
			(1) Update motionKeyStates to record that the key is no longer held down;
				we need to know when we get to step (2) below.
			(2) Update the relevant axis in keyboardMotion to set the new direction
				we should be moving in. This gets a little complicated to ensure
				the controls work smoothly. When the user releases a key that moves
				in one direction (e.g. KEY_MOVE_RIGHT), we check if its "opposite"
				key (e.g. KEY_MOVE_LEFT) is pressed down. If it is, we begin moving
				in that direction instead. Otherwise, we just stop moving.
		*/
	case KEY_MOVE_FORWARD:
		motionKeyStates.MoveForward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveBackward == KEYSTATE_DOWN) ? MOTION_BACKWARD : MOTION_NONE;
		taperX = 1;

		break;
	case KEY_MOVE_BACKWARD:
		motionKeyStates.MoveBackward = KEYSTATE_UP;
		keyboardMotion.Surge = (motionKeyStates.MoveForward == KEYSTATE_DOWN) ? MOTION_FORWARD : MOTION_NONE;
		taperX = 2;

		break;
	case KEY_MOVE_LEFT:
		motionKeyStates.MoveLeft = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveRight == KEYSTATE_DOWN) ? MOTION_RIGHT : MOTION_NONE;

		break;
	case KEY_MOVE_RIGHT:
		motionKeyStates.MoveRight = KEYSTATE_UP;
		keyboardMotion.Sway = (motionKeyStates.MoveLeft == KEYSTATE_DOWN) ? MOTION_LEFT : MOTION_NONE;

		break;

		/*
			Other Keyboard Functions (add any new character key controls here)

			Note: If you only care when your key is first pressed down, you don't have to
			add anything here. You only need to put something in keyReleased if you care
			what happens when the user lets go, like we do with our movement keys above.
			For example: if you wanted a spotlight to come on while you held down "t", you
			would need to set a flag to turn the spotlight on in keyPressed, and update the
			flag to turn it off in keyReleased.
		*/
	}
}

/*
	Called each time a "special" key (e.g. an arrow key) is released.
*/
void specialKeyReleased(int key, int x, int y)
{
	switch (key) {
		/*
			Keyboard-Controlled Motion Handler - DON'T CHANGE THIS SECTION

			This works as per the motion keys in keyReleased.
		*/
	case SP_KEY_MOVE_UP:
		motionKeyStates.MoveUp = KEYSTATE_UP;
		gravityon = 1;

		keyboardMotion.Heave = (motionKeyStates.MoveDown == KEYSTATE_DOWN) ? MOTION_DOWN : MOTION_NONE;
		break;
	case SP_KEY_MOVE_DOWN:
		motionKeyStates.MoveDown = KEYSTATE_UP;
		gravityon = 1;
		keyboardMotion.Heave = (motionKeyStates.MoveUp == KEYSTATE_DOWN) ? MOTION_UP : MOTION_NONE;
		break;
	case SP_KEY_TURN_LEFT:
		motionKeyStates.TurnLeft = KEYSTATE_UP;

		keyboardMotion.Yaw = (motionKeyStates.TurnRight == KEYSTATE_DOWN) ? MOTION_CLOCKWISE : MOTION_NONE;

		break;
	case SP_KEY_TURN_RIGHT:
		motionKeyStates.TurnRight = KEYSTATE_UP;
		keyboardMotion.Yaw = (motionKeyStates.TurnLeft == KEYSTATE_DOWN) ? MOTION_ANTICLOCKWISE : MOTION_NONE;
		break;

		/*
			Other Keyboard Functions (add any new special key controls here)

			As per keyReleased, you only need to handle the key here if you want something
			to happen when the user lets go. If you just want something to happen when the
			key is first pressed, add you code to specialKeyPressed instead.
		*/
	}
}

/*
	Called by GLUT when it's not rendering a frame.

	Note: We use this to handle animation and timing. You shouldn't need to modify
	this callback at all. Instead, place your animation logic (e.g. moving or rotating
	things) within the think() method provided with this template.
*/
void idle(void)
{
	// Wait until it's time to render the next frame.

	unsigned int frameTimeElapsed = (unsigned int)glutGet(GLUT_ELAPSED_TIME) - frameStartTime;
	if (frameTimeElapsed < FRAME_TIME)
	{
		// This frame took less time to render than the ideal FRAME_TIME: we'll suspend this thread for the remaining time,
		// so we're not taking up the CPU until we need to render another frame.
		unsigned int timeLeft = FRAME_TIME - frameTimeElapsed;
		Sleep(timeLeft);
	}

	// Begin processing the next frame.

	frameStartTime = glutGet(GLUT_ELAPSED_TIME); // Record when we started work on the new frame.

	think(); // Update our simulated world before the next call to display().

	glutPostRedisplay(); // Tell OpenGL there's a new frame ready to be drawn.
}

/******************************************************************************
 * Animation-Specific Functions (Add your own functions at the end of this section)
 ******************************************************************************/

 /*
	 Initialise OpenGL and set up our scene before we begin the render loop.
 */
void init(void)
{
	srand(time(NULL));
	glEnable(GL_DEPTH_TEST);
	loadImage();


	loadTexture("waterTexture.ppm", &waterTexture);
	loadTexture("concreteTexture.ppm", &concreteTexture);
	loadTexture("mountaintexture1.ppm", &groundTexture);

	initLights();
	myQuadric = gluNewQuadric();
	cone = gluNewQuadric();
	windMill = gluNewQuadric();
	gluQuadricDrawStyle(myQuadric, GL_LINE);

	
	glEnable(GL_FOG);
	GLfloat fogColor[4] = { 1,1,1,1 };
	glFogfv(GL_FOG_COLOR, fogColor);
	glFogf(GL_FOG_MODE, GL_EXP);
	glFogf(GL_FOG_DENSITY, 0.02);

	for (int i = 0; i < 24; i++) {
		windmillRotation[i] = ((double)rand() / RAND_MAX) * 360.0;
	}


	for (int i = 0; i < 24; i++) {
		Spotlight spotlight;
		spotlight.x = (double)rand() / RAND_MAX * 200 - 100; // Random value between -100 and 100
		spotlight.y = 10.0;
		spotlight.z = (double)rand() / RAND_MAX * 200 - 100;
		spotlight.alive = 1;
		spotlight.velocityx = -0.2f + ((float)rand() / RAND_MAX) * 0.4f;
		spotlight.velocityz = -0.2f + ((float)rand() / RAND_MAX) * 0.4f;
		spotlight.colorCode = i; // Random value between 0 and 10
		spotlights[i] = spotlight;
	}
	for (int i = 0; i < 24; i++) {
		lights[i] = GL_LIGHT1 + i;
		
	}

}
void drawBitmapString(const char* str, float x, float y, float r, float g, float b) {
	glColor3f(r, g, b);
	glRasterPos2f(x, y);

	while (*str) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *str);
		str++;
	}
}

void drawSkyCylinder(float radius, float height, int numSegments) {
	float segmentAngle = 2.0f * 3.15f / numSegments;
	float segmentHeight = height / numSegments;
	glEnable(GL_COLOR_MATERIAL);
	glColor4fv(BLUE);
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= numSegments; i++) {
		float angle = i * segmentAngle;
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		glVertex3f(x, height / 2.0, z);
		glVertex3f(x, -height / 2.0, z);
	}
	glEnd();


	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0, height / 2.0, 0.0);
	for (int i = 0; i <= numSegments; i++) {
		float angle = i * segmentAngle;
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		glVertex3f(x, height / 2.0, z);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0, -height / 2.0, 0.0);
	for (int i = 0; i <= numSegments; i++) {
		float angle = i * segmentAngle;
		float x = radius * cos(angle);
		float z = radius * sin(angle);

		glVertex3f(x, -height / 2.0, z);
	}
	glEnd();
	glDisable(GL_COLOR_MATERIAL);
}

void drawCube(float posX, float posY, float posZ, float size) {
	float halfSize = size / 2.0f;

	float vertices[8][3] = {
		{posX - halfSize, posY - halfSize, posZ + halfSize},
		{posX + halfSize, posY - halfSize, posZ + halfSize},
		{posX + halfSize, posY + halfSize, posZ + halfSize},
		{posX - halfSize, posY + halfSize, posZ + halfSize},
		{posX - halfSize, posY - halfSize, posZ - halfSize},
		{posX + halfSize, posY - halfSize, posZ - halfSize},
		{posX + halfSize, posY + halfSize, posZ - halfSize},
		{posX - halfSize, posY + halfSize, posZ - halfSize}
	};


	int faces[6][4] = {
		{0, 1, 2, 3},
		{1, 5, 6, 2},
		{5, 4, 7, 6},
		{4, 0, 3, 7},
		{3, 2, 6, 7},
		{4, 5, 1, 0}
	};


	float normals[6][3] = {
		{0.0f, 0.0f, 1.0f},
		{1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, -1.0f},
		{-1.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f},
		{0.0f, -1.0f, 0.0f}
	};


	glBegin(GL_QUADS);
	GLfloat ambientMat[] = { 1.0, 1.0, 0.0, 0.6 };
	GLfloat diffuseMat[] = { 1.0, 1.0, 0.0, 0.6 };
	GLfloat specularMat[] = { 1.0, 1.0, 0.0, 0.6 };
	GLfloat shine = 100.0;

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMat);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularMat);
	glMaterialf(GL_FRONT, GL_SHININESS, shine);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, diffuseMat);

	glEnable(GL_NORMALIZE);
	for (int i = 0; i < 6; i++) {
		glNormal3fv(normals[i]);
		for (int j = 0; j < 4; j++) {
			glVertex3fv(vertices[faces[i][j]]);
		}
	}
	glEnd();


}


void drawHelipad(float radius, float height, int numSegments) {

	float segmentAngle = 2.0f * 3.15f / numSegments;
	float segmentHeight = height / numSegments;
	glEnable(GL_TEXTURE_2D);
	GLfloat mat_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat mat_diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat mat_emission[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat mat_shininess = 64.0;

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	GLuint textureID;
	glGenTextures(1, &concreteTexture);
	glBindTexture(GL_TEXTURE_2D, concreteTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, concreteTexture);

	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= numSegments; i++) {
		float angle = i * segmentAngle;
		float x = radius * cos(angle);
		float z = radius * sin(angle);
		float nx = cos(angle);
		float ny = 0.0f;
		float nz = sin(angle);
		glNormal3f(nx, ny, nz);


		glVertex3f(x, height / 2.0, z);


		glVertex3f(x, -height / 2.0, z);
	}
	glEnd();


	glBegin(GL_TRIANGLE_FAN);
	glNormal3f(0.0f, 1.0f, 0.0f);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(0.0, height / 2.0, 0.0);
	for (int i = 0; i <= numSegments; i++) {
		float angle = i * segmentAngle;
		float x = radius * cos(angle);
		float z = radius * sin(angle);


		glTexCoord2f(x, z);
		glVertex3f(x, height / 2.0, z);
	}
	glEnd();

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.0);
	glNormal3f(0.0f, -1.0f, 0.0f);
	glVertex3f(0.0, -height / 2.0, 0.0);
	for (int i = 0; i <= numSegments; i++) {
		float angle = i * segmentAngle;
		float x = radius * cos(angle);
		float z = radius * sin(angle);


		glTexCoord2f(x, z);
		glVertex3f(x, -height / 2.0, z);
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);

	float symbolWidth = radius * 0.6f;
	float symbolHeight = height * 0.04f;
	float symbolDepth = radius * 0.1f;

	float symbolX = 0.0f;
	float symbolY = height / 2.0f + 0.001f;
	float symbolZ = 0.0f;

	GLfloat ymat_ambient[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat ymat_diffuse[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	GLfloat ymat_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat ymat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat ymat_shininess = 64.0f;

	glMaterialfv(GL_FRONT, GL_AMBIENT, ymat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, ymat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, ymat_specular);
	glMaterialfv(GL_FRONT, GL_EMISSION, ymat_emission);
	glMaterialf(GL_FRONT, GL_SHININESS, ymat_shininess);

	glBegin(GL_QUADS);
	glNormal3f(0.0f, 1.0f, 0.0f);

	glEnd();
	glPushMatrix();
	glScalef(1.1, 0.06, 0.5);
	drawCube(0, 0, 0, 1);
	glPopMatrix();
	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 0.0, 0.8);
	glScalef(3.0, 0.06, 0.5);
	drawCube(0, 0, 0, 1);
	glPopMatrix();
	glPushMatrix();
	glRotatef(90, 0.0, 1.0, 0.0);
	glTranslatef(0.0, 0.0, -0.8);
	glScalef(3.0, 0.06, 0.5);
	drawCube(0, 0, 0, 1);
	glPopMatrix();

	GLfloat mat_ambient_reset[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat mat_diffuse_reset[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat mat_specular_reset[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat mat_emission_reset[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat mat_shininess_reset = 64.0f;

	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient_reset);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse_reset);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_reset);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_reset);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess_reset);

}
void loadTexture(char str[], Texture3D* texture) {

	FILE* fileID;
	int maxValue, i, red, green, blue;
	char tempChar;
	char headerLine[100];

	fopen_s(&fileID, str, "r");

	fscanf_s(fileID, "%[^\n] ", headerLine, _countof(headerLine));

	if ((headerLine[0] != 'P') || (headerLine[1] != '3')) {
		printf("This is not a PPM file!\n");
		exit(0);
	}

	fscanf_s(fileID, "%c", &tempChar, 1);

	while (tempChar == '#') {
		fscanf_s(fileID, "%[^\n] ", headerLine, _countof(headerLine));
		fscanf_s(fileID, "%c", &tempChar, 1);
	}

	ungetc(tempChar, fileID);

	fscanf_s(fileID, "%d %d %d", &imageWidth, &imageHeight, &maxValue);

	int totalPixels = imageWidth * imageHeight;

	for (int i = 0; i < totalPixels; i++) {
		fscanf_s(fileID, "%d %d %d", &red, &green, &blue);

		int row = i / imageWidth;
		int col = i % imageWidth;
		(*texture)[row][col][0] = (GLubyte)red;
		(*texture)[row][col][1] = (GLubyte)green;
		(*texture)[row][col][2] = (GLubyte)blue;
	}

	fclose(fileID);

}


void drawTerrain(float terrainScale) {

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_COLOR_MATERIAL);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, groundTexture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_UNSIGNED_BYTE, groundTexture);

	glBegin(GL_TRIANGLES);
	GLfloat ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
	int x, z;
	int size = 100;
	int step = 1;


	for (x = 0; x < 200; x += step) {
		for (z = 0; z < 200; z += step) {

			GLfloat height1 = imageData[x][z].greyscale / 100.0f * 4;
			GLfloat height2 = imageData[x + step][z].greyscale / 100.0f * 4;
			GLfloat height3 = imageData[x][z + step].greyscale / 100.0f * 4;
			GLfloat height4 = imageData[x + step][z + step].greyscale / 100.0f * 4;

			terrainColorPicker(height1);

			GLfloat normal[3];
			GLfloat edge1[3] = { step, height2 - height1, 0 };
			GLfloat edge2[3] = { 0, height3 - height1, step };
			CrossProduct(edge1, edge2, normal);
			Normalize(normal);

			glNormal3fv(normal);

			glTexCoord2f((float)(x - 100), (float)(z - 100));
			glVertex3f(x - 100, height1, z - 100);

			glTexCoord2f((float)(x + step - 100), (float)(z - 100));
			glVertex3f(x + step - 100, height2, z - 100);

			glTexCoord2f((float)(x - 100), (float)(z + step - 100));
			glVertex3f(x - 100, height3, z + step - 100);

			glTexCoord2f((float)(x + step - 100), (float)(z - 100));
			glVertex3f(x + step - 100, height2, z - 100);

			glTexCoord2f((float)(x + step - 100), (float)(z + step - 100));
			glVertex3f(x + step - 100, height4, z + step - 100);

			glTexCoord2f((float)(x - 100), (float)(z + step - 100));
			glVertex3f(x - 100, height3, z + step - 100);
		}
	}

	glEnd();
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_TEXTURE_2D);
}


void drawAtom() {

	GLfloat matAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat matDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat matSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat matShininess = 50.0f;
	GLfloat redAmbient[] = { 8.0f, 0.0f, 0.0f, 1.0f };

	glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, matShininess);

	glutSolidSphere(1.0f, 20, 20);

	GLfloat angleIncrement = 2 * PI / numElectrons;

	for (int i = 0; i < numElectrons; ++i) {

		GLfloat electronX = orbitRadius * sin(electronAngle + i * angleIncrement);
		GLfloat electronY = 0.0f;
		GLfloat electronZ = orbitRadius * cos(electronAngle + i * angleIncrement);

		if (i % 2 == 0) {
			electronX = orbitRadius * sin(electronAngle + i * angleIncrement);
			electronY = 0.0f;
			electronZ = orbitRadius * cos(electronAngle + i * angleIncrement);
		}
		if (i % 3 == 0) {
			electronX = orbitRadius * cos(electronAngle + i * angleIncrement);
			electronY = orbitRadius * sin(electronAngle + i * angleIncrement);
			electronZ = 0.0f;
		}
		if (i % 4 == 0) {
			electronX = 0.0f;
			electronY = orbitRadius * sin(electronAngle + i * angleIncrement);
			electronZ = orbitRadius * cos(electronAngle + i * angleIncrement);
		}

		GLfloat electronRotationAngle = electronAngle + i * angleIncrement;

		glPushMatrix();
		glMaterialfv(GL_FRONT, GL_AMBIENT, redAmbient);
		glTranslatef(electronX, electronY, electronZ);
		glRotatef(electronRotationAngle * 180.0f / PI, 0.0f, 1.0f, 0.0f);
		glutSolidSphere(0.2f, 10, 10);
		glPopMatrix();
	}

}

void terrainColorPicker(GLfloat height) {

	if (height >= 0.0f && height < 0.5f) {
		glColor4fv(TERRAINCOLOUR1);

	}
	else
		if (height >= 0.5f && height < 1.0f) {
			glColor4fv(TERRAINCOLOUR2);
		}
		else
			if (height >= 1.0f && height < 1.5f) {
				glColor4fv(TERRAINCOLOUR3);
			}
			else
				if (height >= 1.5f && height < 2.0f) {
					glColor4fv(TERRAINCOLOUR4);
				}
				else
					if (height >= 2.0f && height < 2.5f) {
						glColor4fv(TERRAINCOLOUR5);
					}
					else
						if (height >= 2.5f && height < 3.0f) {
							glColor4fv(TERRAINCOLOUR6);
						}
						else
							if (height >= 3.0f && height < 3.5f) {
								glColor4fv(TERRAINCOLOUR7);
							}
							else
								if (height >= 3.5f && height < 4.0f) {
									glColor4fv(TERRAINCOLOUR8);
								}
								else
									if (height >= 4.0f && height < 4.5f) {
										glColor4fv(TERRAINCOLOUR9);
									}
									else
										if (height >= 4.5f && height < 5.0f) {
											glColor4fv(TERRAINCOLOUR10);
										}
										else
											if (height >= 5.0f && height < 5.5f) {
												glColor4fv(TERRAINCOLOUR11);
											}
											else
												if (height >= 5.5f && height < 6.0f) {
													glColor4fv(TERRAINCOLOUR12);
												}
												else
													if (height >= 6.0f && height < 6.5f) {
														glColor4fv(TERRAINCOLOUR13);
													}
													else
														if (height >= 6.5f && height < 7.0f) {
															glColor4fv(TERRAINCOLOUR14);
														}
														else
															if (height >= 7.0f && height < 7.5f) {
																glColor4fv(TERRAINCOLOUR15);
															}
															else
																if (height >= 7.5f && height < 8.0f) {
																	glColor4fv(TERRAINCOLOUR16);
																}
																else
																	if (height >= 8.0f && height < 8.5f) {
																		glColor4fv(TERRAINCOLOUR17);
																	}
																	else
																		if (height >= 8.5f && height < 9.0f) {
																			glColor4fv(TERRAINCOLOUR18);
																		}
																		else
																			if (height >= 9.5f && height < 10.0f) {
																				glColor4fv(TERRAINCOLOUR19);
																			}

}

void CrossProduct(GLfloat* v1, GLfloat* v2, GLfloat* crossProduct)
{
	crossProduct[0] = v1[1] * v2[2] - v1[2] * v2[1];
	crossProduct[1] = v1[2] * v2[0] - v1[0] * v2[2];
	crossProduct[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void Normalize(GLfloat* v)
{
	GLfloat length = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
	v[0] /= length;
	v[1] /= length;
	v[2] /= length;
}

void loadImage(void)
{
	FILE* fileID;
	int maxValue, totalPixels, i, red, green, blue;
	char tempChar;
	char headerLine[100];
	float RGBScaling;

	fopen_s(&fileID, "terrain.ppm", "r");

	fscanf_s(fileID, "%[^\n] ", &headerLine, _countof(headerLine));

	if ((headerLine[0] != 'P') || (headerLine[1] != '3'))
	{
		printf("This is not a PPM file!\n");
		exit(0);
	}

	fscanf_s(fileID, "%c", &tempChar, 1);

	while (tempChar == '#')
	{
		fscanf_s(fileID, "%[^\n] ", headerLine, _countof(headerLine));
		fscanf_s(fileID, "%c", &tempChar, 1);
	}

	ungetc(tempChar, fileID);

	fscanf_s(fileID, "%d %d %d", &imageWidth, &imageHeight, &maxValue);

	totalPixels = imageWidth * imageHeight;

	RGBScaling = 255.0 / maxValue;

	for (int i = 0; i < 40000; i++) {

		fscanf_s(fileID, "%d %d %d", &red, &green, &blue);

		int row = i / 200;
		int col = i % 200;
		imageData[row][col].r = red;
		imageData[row][col].g = green;
		imageData[row][col].b = blue;
		imageData[row][col].greyscale = 0.2126 * red + 0.7152 * green + 0.0722 * blue;

	}

	fclose(fileID);


}

void drawPropeller(GLfloat x, GLfloat y, GLfloat z) {

	glPushMatrix();
	glTranslatef(0.0f, 0.3f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	glRotatef(bladeRotation[0], 0.0f, 1.0f, 0.0f);
	glScalef(1.5, 0.005, 0.08);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	glTranslatef(x + 0.0f, y + 0.3f, z + 0.0f);
	glRotatef(bladeRotation[1], 0.0f, 1.0f, 0.0f);
	glScalef(1.5, 0.005, 0.08);
	glutSolidCube(1.0f);
	glPopMatrix();
}


void drawChopper(GLfloat x, GLfloat y, GLfloat z) {

	GLfloat ambientMat[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat diffuseMat[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat specularMat[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat shine = 100.0;

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMat);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularMat);
	glMaterialf(GL_FRONT, GL_SHININESS, shine);

	glEnable(GL_NORMALIZE);

	glTranslatef(x, y, z);
	glRotatef(heliX, 0.0f, 1.0f, 0.0f);
	glRotatef(rx, 1.0f, 0.0f, 0.0f);
	glRotatef(pitch, 0.0f, 0.0f, 1.0f);
	glTranslatef(-x, -y, -z);


	glPushMatrix();
	glTranslatef(x + 0.0f, y + 0.0f, z + 0.0f);
	glRotatef(15.0f, 1.0f, 0.0f, 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.26, 0.05, 0.6, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glScalef(1, 1, 1);
	glTranslatef(x + 0.0f, y + 0.0f, z + 0.0f);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLUE);
	gluSphere(myQuadric, 0.25, 100, 100);
	drawPropeller(x, y, z);


	glPushMatrix();
	glTranslatef(x, y + 0.25, z);
	glRotatef(90.0f, 0.0f, 0.2f, 0.0f);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLUE);
	gluCylinder(myQuadric, 0.05, 0.05, 0.1, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x + 0.2, y - 0.3, z - 0.2);
	glRotatef(90.0, 0.0, 0.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.02, 0.02, 0.7, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x - 0.2, y - 0.3, z - 0.2);
	glRotatef(90.0, 0.0, 0.0, 1.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.02, 0.02, 0.7, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x - 0.2, y - 0.25, z - 0.28);
	glRotatef(30.0, 1.0, 0.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.02, 0.02, 0.1, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x + 0.2, y - 0.25, z - 0.28);
	glRotatef(30.0, 1.0, 0.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.02, 0.02, 0.1, 50, 50);
	glPopMatrix();

	//TAIL
	glPushMatrix();
	glTranslatef(x, y - 0.15, z + 0.58);
	glRotatef(0.0, 1.0, 0.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLUE);
	gluCylinder(myQuadric, 0.05, 0.05, 0.15, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x + 0.04, y - 0.15, z + 0.70);
	glRotatef(90.0, 0.0, 1.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.02, 0.02, 0.05, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x + 0.09, y - 0.15, z + 0.70);
	glRotatef(bladeRotation[0], 1.0f, 0.0f, 0.0f);
	glScalef(0.008, 0.25, 0.05);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	glutSolidCube(1.0f);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x + 0.09, y - 0.15, z + 0.70);
	glRotatef(bladeRotation[1], 1.0f, 0.0f, 0.0f);
	glScalef(0.008, 0.25, 0.05);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	glutSolidCube(1.0f);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x + 0.12, y - 0.22, z - 0.02);
	glRotatef(90.0, 1.0, 1.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.01, 0.01, 0.11, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x + 0.09, y - 0.195, z + 0.15);
	glRotatef(90.0, 1.0, 1.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.01, 0.01, 0.14, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x - 0.20, y - 0.3, z - 0.02);
	glRotatef(90.0, -1.0, 1.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.01, 0.01, 0.11, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x - 0.19, y - 0.3, z + 0.15);
	glRotatef(90, -1.0, 1.0, 0.0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	gluCylinder(myQuadric, 0.01, 0.01, 0.14, 50, 50);
	glPopMatrix();

}

void drawWindmill(double rotation, GLfloat x, GLfloat y, GLfloat z) {

	GLfloat ambientMat[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat diffuseMat[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat specularMat[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat shine = 100.0;

	glMaterialfv(GL_FRONT, GL_AMBIENT, ambientMat);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuseMat);
	glMaterialfv(GL_FRONT, GL_SPECULAR, specularMat);
	glMaterialf(GL_FRONT, GL_SHININESS, shine);

	glEnable(GL_NORMALIZE);

	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(rotation, 0, 1.0, 0);
	glTranslatef(-x, -y, -z);


	glPushMatrix();
	glTranslatef(x, y, z);
	glRotatef(90, 1.0, 0, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, RED);
	gluCylinder(windMill, 1.5, 1.8, 1.5, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x, y + 1.4, z);
	glRotatef(90, 1.0, 0, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, CREAM);
	gluCylinder(windMill, 1.2, 1.5, 1.5, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x, y + 2.8, z);
	glRotatef(90, 1.0, 0, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, RED);
	gluCylinder(windMill, 0.9, 1.2, 1.5, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x, y + 4.5, z);
	glRotatef(90, 1.0, 0, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, RED);
	gluSphere(windMill, 0.7, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x, y + 4.2, z);
	glRotatef(90, 1.0, 0, 0);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, CREAM);
	gluCylinder(windMill, 0.6, 0.9, 1.5, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x, y + 4.5, z + 0.8);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, CREAM);
	glRotatef(90, 1.0, 0, 0);
	gluSphere(windMill, 0.4, 50, 50);
	glPopMatrix();


	glPushMatrix();
	glTranslatef(x + 0.0f, y + 4.6f, z + 1.1f);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	glRotatef(windmillBladeRotation[0], 0.0f, 1.0f, 0.0f);
	glScalef(4.5, 0.010, 0.3);
	glutSolidCube(1.0f);
	glPopMatrix();


	glPushMatrix();
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, BLACK);
	glTranslatef(x + 0.0f, y + 4.6f, z + 1.1f);
	glRotatef(90, 1.0f, 0.0f, 0.0f);
	glRotatef(windmillBladeRotation[1], 0.0f, 1.0f, 0.0f);
	glScalef(4.5, 0.010, 0.3);
	glutSolidCube(1.0f);
	glPopMatrix();

	glPopMatrix();
}

void drawSpotlight(GLenum light, Spotlight spotlight, GLfloat coneDiffuse[], GLfloat lightDiffuse[]) {

	glPushMatrix();

	int i = spotlight.colorCode;

	GLfloat ambientLight[] = { 10.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_position[] = { spotlight.x, spotlight.y, spotlight.z, 1.0f };
	GLfloat spot_direction[] = { 0.0f, -1.0f, 0.0f };

	glLightfv(light, GL_DIFFUSE, lightDiffuse);

	if (spotlight.alive == 1) {
		glEnable(light);
	}
	else {

		glDisable(light);

	}

	glLightfv(light, GL_POSITION, light_position);
	glLightfv(light, GL_SPOT_DIRECTION, spot_direction);
	glLightf(light, GL_SPOT_CUTOFF, 25.0f);


	GLfloat noAmbient[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat noSpecular[] = { 0.0f, 0.0f, 0.0f, 1.0f };


	if (spotlight.alive == 1) {
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, coneDiffuse);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTranslatef(spotlight.x, spotlight.y - 6, spotlight.z);
		glRotatef(-90, 1.0f, 0.0f, 0.0f);
		gluCylinder(cone, 2.0, 0.3, 4.5, 50, 50);
	}

	glPopMatrix();

}
/*
	Advance our animation by FRAME_TIME milliseconds.

	Note: Our template's GLUT idle() callback calls this once before each new
	frame is drawn, EXCEPT the very first frame drawn after our application
	starts. Any setup required before the first frame is drawn should be placed
	in init().
*/
void think(void)
{
	
	electronAngle += orbitSpeed;
	lightX += lightVelocityX;
	printf("hx: %f, hz: %f\n", heliCoord[0], heliCoord[2]);

	if (motionKeyStates.MoveForward == KEYSTATE_UP) {
		if (rx < 0) {
			rx++;
		}

	}
	if (motionKeyStates.MoveBackward == KEYSTATE_UP) {
		if (rx > 0) {
			rx--;
		}

	}
	if (motionKeyStates.MoveLeft == KEYSTATE_UP) {
		if (pitch > 0) {
			pitch--;
		}

	}
	if (motionKeyStates.MoveRight == KEYSTATE_UP) {
		if (pitch < 0) {
			pitch++;
		}
	}


	int currentFrameTime = glutGet(GLUT_ELAPSED_TIME);


	float deltaTime = (currentFrameTime - lastFrameTime) / 1000.0f;

	lastFrameTime = currentFrameTime;


	int x = round(heliCoord[0]);
	int z = round(heliCoord[2]);
	x += 100;
	z += 100;


	GLfloat ground = imageData[x][z].greyscale / 100.0f * 4;


	if (heliCoord[1] > (ground + 0.5)) {

		if (gravityon == 1) {

			if (heliCoord[1] > 37) {
				helicopterVelocityY = 0;
			}

			if (!(heliCoord[0] > -1.0f && heliCoord[0] < 1.0f)) {
				if (!(heliCoord[0] > -1.0f && heliCoord[0] < 1.0f)) {
					helicopterVelocityY -= 1 - ((bladeSpeed * 2) / 10);

					if (round(heliCoord[1]) <= 38) {
						heliCoord[1] += helicopterVelocityY / 250;
					}
				}
			}
			else {
				if (heliCoord[1] > 4.7f) {
					
					
					helicopterVelocityY -= 1 - ((bladeSpeed * 2) / 10);
					
					if (round(heliCoord[1]) <= 38) {
						heliCoord[1] += helicopterVelocityY / 250;
					}
				}
				else {
					helicopterVelocityY = 0;
					moveSpeed = 0;
				}
			}

		}
	}
	else {
		grounded = 1;
		
		if (verticalSpeedResetSwitch == 0) {
			helicopterVelocityY = 0;
			verticalSpeedResetSwitch = 1;
		}
		sideMoveSpeed = 0;
		moveSpeed = 0;
		gravityon = 0;
	}
	for (int i = 0; i < 2; i++) {

		if (bladeRotation[i] == 360.0f) {
			bladeRotation[i] = bladeSpeed;
		}
		else {
			bladeRotation[i] += bladeSpeed * 8;
		}
	}


	for (int i = 0; i < 2; i++) {

		if (windmillBladeRotation[i] == 360.0f) {
			windmillBladeRotation[i] = bladeSpeed;
		}
		else {
			windmillBladeRotation[i] += 0.7;
		}
	}


	/*
		Keyboard motion handler: complete this section to make your "player-controlled"
		object respond to keyboard input.
	*/
	if (keyboardMotion.Yaw != MOTION_NONE) {

		if (keyboardMotion.Yaw == MOTION_ANTICLOCKWISE) {

			if (heliX == 360.0f) {
				heliX = 0.0f;
			}
			else {
				heliX += 1.0f;
			}


		}
		if (keyboardMotion.Yaw == MOTION_CLOCKWISE) {

			if (heliX == 0.0f) {
				heliX = 360.0f;
			}
			else {
				heliX -= 1.0f;
			}

		}
		/* TEMPLATE: Turn your object right (clockwise) if .Yaw < 0, or left (anticlockwise) if .Yaw > 0 */
	}

	if (keyboardMotion.Surge != MOTION_NONE) {
		if (keyboardMotion.Surge > 0) {

			if (rx > -20.0f) {
				rx--;
			}
		}
		else if (keyboardMotion.Surge < 0) {

			if (rx < 20.0f) {
				rx++;
			}
		}
	}

	if (!grounded) {
		if ((keyboardMotion.Surge != MOTION_NONE) || taperX > 0) {
			/* TEMPLATE: Move your object backward if .Surge < 0, or forward if .Surge > 0 */
			float radians = heliX * PI / 180.0f;

			float deltaX = moveSpeed * sin(radians);
			float deltaZ = moveSpeed * cos(radians);
			if (keyboardMotion.Surge > 0) {

				taperX = 0;


				heliCoord[0] -= deltaX;
				heliCoord[2] -= deltaZ;

				if (moveSpeed < 0.3f) {
					moveSpeed += 0.01f;
				}
			}
			else if (keyboardMotion.Surge < 0) {

				taperX = 0;


				heliCoord[0] -= deltaX;
				heliCoord[2] -= deltaZ;
				if (moveSpeed > -0.2f) {
					moveSpeed -= 0.01f;
				}
			}

			if (taperX == 1) {
				heliCoord[0] -= deltaX;
				heliCoord[2] -= deltaZ;

				if (moveSpeed > 0.0f) {
					moveSpeed -= 0.001f;

				}
				else {
					taperX = 0;
				}
			}
			if (taperX == 2) {
				heliCoord[0] -= deltaX;
				heliCoord[2] -= deltaZ;

				if (moveSpeed < 0.0f) {
					moveSpeed += 0.001f;

				}
				else {
					taperX = 0;
				}
			}
		}

		if (keyboardMotion.Sway != MOTION_NONE) {
			/* TEMPLATE: Move (strafe) your object left if .Sway < 0, or right if .Sway > 0 */

			float radians = heliX * PI / 180.0f;

			float deltaX = sideMoveSpeed * sin(radians);
			float deltaZ = sideMoveSpeed * cos(radians);
			if (keyboardMotion.Sway > 0) {
				
				if (pitch > -20.0f) {
					pitch--;
				}

				heliCoord[0] += deltaZ;
				heliCoord[2] -= deltaX;
				
				if (sideMoveSpeed< 0.2f) {
					sideMoveSpeed += 0.005;
				}
			}
			else if (keyboardMotion.Sway < 0) {
				
				if (pitch < 20.0f) {
					pitch++;
				}
				
				heliCoord[0] += deltaZ;
				heliCoord[2] -= deltaX;
				
				if (sideMoveSpeed > -0.2f) {
					sideMoveSpeed -= 0.005;
				}
			}
		}
		else {
			float radians = heliX * PI / 180.0f;
			float deltaX = sideMoveSpeed * sin(radians);
			float deltaZ = sideMoveSpeed * cos(radians);
		
			if (sideMoveSpeed > 0) {
				
				sideMoveSpeed -= 0.001;
				
			
				heliCoord[0] += deltaZ;
				heliCoord[2] -= deltaX;
			}
			else if(sideMoveSpeed < 0) {
				sideMoveSpeed += 0.001;
				heliCoord[0] += deltaZ;
				heliCoord[2] -= deltaX;
			}
		}

	}


	if (keyboardMotion.Heave != MOTION_NONE) {

		/* TEMPLATE: Move your object down if .Heave < 0, or up if .Heave > 0 */
		if (keyboardMotion.Heave < 0) {
			
				helicopterVelocityY -= 1;
				heliCoord[1] += helicopterVelocityY / 400;
			
		}
		if (keyboardMotion.Heave > 0 && heliCoord[1] < 37.0f) {
			
			if (bladeSpeed > 2.0f) {
				tip = 0;
				helicopterVelocityY += 1;
				gravityon = 0;
				heliCoord[1] += helicopterVelocityY / 400;
				if (heliCoord[1] > ground + 0.5f) {
					verticalSpeedResetSwitch = 0;
				}
				grounded = 0;
			}

			if (bladeSpeed < 5.0f) {
				bladeSpeed += 0.02f;
			}

		}


	}
	else {
		
		if (bladeSpeed > 0.0f) {
			bladeSpeed -= 0.01f;

		}
	}

	
	for (int i = 0; i < 7; i++) {
		spotlights[i].x += spotlights[i].velocityx;
		spotlights[i].z += spotlights[i].velocityz;
	}
	for (int i = 0; i < 7; i++) {
		if (spotlights[i].x > 95 || spotlights[i].x < -95) {
			spotlights[i].velocityx *= -1;
		}
		if (spotlights[i].z > 95 || spotlights[i].z < -95) {
			spotlights[i].velocityz *= -1;
		}
	}
	


	for (int i = 0; i < 7; i++) {
		
		if (heliCoord[0] < (spotlights[i].x + 0.5f)) {

			if (heliCoord[0] > (spotlights[i].x - 0.5f)) {

				if (heliCoord[2] > (spotlights[i].z - 0.5f)) {

					if (heliCoord[2] < (spotlights[i].z + 0.5f)) {
						if (heliCoord[1] < spotlights[i].y)
						{
							if (spotlights[i].alive == 1) {
								score += 1;
								numElectrons++;
								spotlights[i].alive = 0;
							
								resetSpotlight(&spotlights[i]);

							}
							
						}
					}
				}
			}
		}
	}

	flashColors(coneColours);
}

void flashColors(GLfloat coneColours[][4]) {
	for (int i = 0; i < 7; i++) {

		while (coneColours[i][3] > 0.00f) {
			coneColours[i][3] -= 0.001f;
		
		}

		while (coneColours[i][3] < 0.20f) {
			coneColours[i][3] += 0.001f;
		
		}
	}
}
void resetSpotlight(Spotlight* spotlight) {
	
	spotlight->x = (double)rand() / RAND_MAX * 200 - 100;
	spotlight->y = 10.0;
	spotlight->z = (double)rand() / RAND_MAX * 200 - 100;
	spotlight->velocityx = -0.2f + ((float)rand() / RAND_MAX) * 0.4f;
	spotlight->velocityz = -0.2f + ((float)rand() / RAND_MAX) * 0.4f;
	spotlight->colorCode = rand() % 7;
	spotlight->alive = 1;
}
/*
	Initialise OpenGL lighting before we begin the render loop.

	Note (advanced): If you're using dynamic lighting (e.g. lights that move around, turn on or
	off, or change colour) you may want to replace this with a drawLights function that gets called
	at the beginning of display() instead of init().
*/
void initLights(void)
{

	GLfloat globalAmbient[] = { 0.9f, 0.6f, 0.6f, 1 };
	GLfloat lightPosition[] = { 0.0f, 40.0f, 0.0f, 1.0f };
	GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat diffuseLightRed[] = { 6.0f, 0.0f, 0.0f, 1.0f };
	GLfloat specularLight[] = { 1.0f, 1.0f, 0.8f, 1.0f };
	GLfloat diffuseLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);

	glEnable(GL_LIGHTING);	
	glEnable(GL_LIGHT0);

	glEnable(GL_NORMALIZE);


}

void drawOrigin(void) {

	glBegin(GL_LINES);
	
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3d(-2.0, 7.0, 0.0);
	glVertex3d(2.0, 7.0, 0.0);

	
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3d(0.0, 9.0, 0.0);
	glVertex3d(0.0, 5.0, 0.0);

	
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3d(0.0, 9.0, 1.0);
	glVertex3d(0.0, 5.0, 1.0);

	
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3d(0.0, 7.0, 2.0);
	glVertex3d(0.0, 7.0, -2.0);

	glEnd();
}
