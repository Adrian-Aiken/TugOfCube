#include <sifteo.h>
#include "assets.gen.h"
#include "Minigame.h"

using namespace Sifteo;

static const	int				numCubes = 3;
static			int				ballCube = 0;
static const	int				toWin = 3;
static			int				points = 0;
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

// CUBE ENUMS
enum { P1, MIDDLE, P2 };

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

		ropePos = LCD_width/2 - (Knot.pixelWidth() / 2);
		ropeDelta = LCD_width / (toWin * 2);


		// Background & images
		vid.bg0.image(vec(0,0), MiddleBG);

		resetTimer();
		startTimer();
	}

	void update(TimeDelta timestep){
		if (isRunning) timeSpan -= timestep.seconds();
		if (timeSpan < 0.0) {
			stopTimer();
			timeSpan = 0.0;
		}

		// JUST TO TEST OUT ROPE STUFF		
		vid.sprites[6].setImage(Knot);
		vid.sprites[6].move( (toWin + points - 1) * ropeDelta, 65);

		vid.sprites[0].setImage(Digits, (toWin+points+100) % 10);
		vid.sprites[0].move(LCD_width / 2, LCD_height / 2);

		// Put in the time

		vid.sprites[1].setImage(Digits, floor(timeSpan / 10) % 10 );
		vid.sprites[2].setImage(Digits, floor(timeSpan) % 10);
		vid.sprites[3].setImage(Digits, floor(timeSpan * 10) % 10 );
		vid.sprites[4].setImage(Digits, floor(timeSpan *100) % 10 );
		vid.sprites[1].move(34, 12);
		vid.sprites[2].move(44, 12);
		vid.sprites[3].move(60, 12);
		vid.sprites[4].move(70, 12);


	}

	void stopTimer(){
		isRunning = false;
	}

	void resetTimer(float resetTo = 10.0){
		timeSpan = resetTo;
	}

	void startTimer(){
		isRunning = true;
	}

	void addPoints(int player){
		if (player == P1) points--;
		if (player == P2) points++;
	}

private:
	VideoBuffer	vid;
	bool		isRunning;
	float		timeSpan;
	CubeID		cube;
	float		ropePos;
	float		ropeDelta;
};



void main(){

	static			TiltGameCube	cubes[2];
	static			MiddleGameCube	mid;

	ballCube = 0;

	ballPos = LCD_center;
	ballVel = vec(0,0);

	cubes[0].init(0);
	mid.init(1);
	cubes[1].init(2);

	

	TimeStep ts;
	float toWait;
	bool going = true;
	while (1) {
		TimeDelta td = ts.delta();

		if (going) {
			neighbors = Neighborhood(MIDDLE);
			if ( neighbors.sideOf(P1) != NO_SIDE ) {
				mid.addPoints(P1);
				toWait = 3.0;
				mid.stopTimer();
				going = false;
			}
			if ( neighbors.sideOf(P2) != NO_SIDE ) {
				mid.addPoints(P2);
				toWait = 3.0;
				mid.stopTimer();
				going = false;
			}
		}

		else {
			toWait -= td.seconds();
			if (toWait < 0.0){
				mid.resetTimer();
				mid.startTimer();
				going = true;
			}
		}

		for (unsigned i = 0; i < arraysize(cubes); i++)
			cubes[i].update(td);
		mid.update(td);


		System::paint();
		ts.next();
	}
}

