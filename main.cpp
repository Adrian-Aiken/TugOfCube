#include <sifteo.h>
#include "assets.gen.h"
#include "Minigame.h"

using namespace Sifteo;

static const	int				numCubes = 3;
static			int				ballCube = 0;
static			Float2			ballPos;
static			Float2			ballVel;
static			Float2			ballAcc;
static			Neighborhood	neighbors;

static AssetSlot MainSlot = AssetSlot::allocate()
	.bootstrap(GameAssets);

static Metadata M = Metadata()
	.title("Tilting Prototype")
	.package("edu.rit.aca6943.tilting", "0.1")
	.icon(Icon)
	.cubeRange(3);

class TiltGameCube {
public:

	void init(CubeID _cube){
		bg.x = 0;
		bg.y = 0;
		cube = _cube;

		vid.initMode(BG0_SPR_BG1);
		vid.attach(cube);

		// Put in the background image
		vid.bg0.image(vec(0,0), Background);
		
	}

	void update(TimeDelta timeStep){

		if (ballCube == cube){
			vid.sprites[0].setImage(Ball);
			vid.sprites[0].move(ballPos);

			ballAcc = vid.physicalAccel().xy();
		}
		
		else {
			vid.sprites[0].hide();
		}
			

	}

private:
	VideoBuffer vid;
	Float2		bg;
	int			cube;

	void writeText(const char *str)
    {
        // Text on BG1, in the 16x2 area we allocated
        vid.bg1.text(vec(0,14), Font, str);
    }
};


class MiddleGameCube {
public:
	void init(CubeID _cube){
		cube = _cube;
		vid.initMode(BG0_SPR_BG1);
		vid.attach(_cube);

		// Background
		vid.bg0.image(vec(0,0), BlackBG);
	}

	void update(TimeDelta timestep){
		if (isRunning) timeSpan += timestep.seconds();


	}

	void stopTimer(){
		isRunning = false;
	}

	void resetTimer(){
		timeSpan = 0.0;
		isRunning = true;
	}
private:
	VideoBuffer	vid;
	bool		isRunning;
	float		timeSpan;
	int			cube;
};


void main(){

	static TiltGameCube cubes[2];
	static MiddleGameCube mid;

	ballCube = 0;

	ballPos = LCD_center;
	ballVel = vec(0,0);

	cubes[0].init(0);
	mid.init(1);
	cubes[1].init(2);

	TimeStep ts;
	while (1) {
		// The game logic loop
		const Float2 minPosition = { 0, 0 };
		const Float2 maxPosition = { LCD_width - Ball.pixelWidth(), LCD_height - Ball.pixelHeight() };
        const float deadzone = 2.0f;
        const float accelScale = 0.5f;
        const float damping = 0.95f;

		neighbors = Neighborhood(ballCube);

		if (ballAcc.len2() > deadzone * deadzone)
			ballVel += ballAcc * accelScale;
		ballVel *= damping;
		ballPos += ballVel * float(ts.delta());

		if (ballPos.x <= minPosition.x) {
			if (neighbors.hasCubeAt(LEFT) && neighbors.cubeAt(LEFT) != 1){
				ballPos.x += maxPosition.x;
				ballCube = neighbors.cubeAt(LEFT);
			} else {
				ballPos.x = minPosition.x; 
				ballVel.x *= -damping;
			}			
		}
		if (ballPos.x >= maxPosition.x) {
			if (neighbors.hasCubeAt(RIGHT) && neighbors.cubeAt(RIGHT) != 1){
				ballPos.x = minPosition.x;
				ballCube = neighbors.cubeAt(RIGHT);
			} else {
				ballPos.x = maxPosition.x;
				ballVel.x *= -damping;
			}
		}
		if (ballPos.y <= minPosition.y) {
			ballPos.y = minPosition.y;
			ballVel.y *= -damping;
		}
		if (ballPos.y >= maxPosition.y) {
			ballPos.y = maxPosition.y;
			ballVel.y *= -damping;
		}


		for (unsigned i = 0; i < arraysize(cubes); i++)
			cubes[i].update(ts.delta());
		mid.update(ts.delta());


		System::paint();
		ts.next();
	}
}

