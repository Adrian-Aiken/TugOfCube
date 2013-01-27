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
	.title("Tug Of Cube Prototype")
	.package("edu.rit.aca6943.tilting", "0.2")
	.icon(Icon)
	.cubeRange(3);

// CUBE ENUMS
enum { P1, MIDDLE, P2 };

class MinigameCube {
public:

	void init(CubeID _cube){
		cube = _cube;
		done = false;
		timespan = 0;
		
		vid.initMode(BG0_SPR_BG1);
		vid.attach(cube);

		// Put in the background image
		vid.bg0.image(vec(0,0), Blank);
		
	}

	void update(TimeDelta timestep){
		if (!done) {
			timespan += timestep.seconds();
			
			// switch (1-n) for minigame type
			if (minigameType == 0) {
				Float2 accel = vid.physicalAccel().xy();
				if (abs(accel.x) > 20 && abs(accel.y) > 20) {
					shakeCount++;
					if (shakeCount >= 3) {
						done = true;
					}
				}
			} else if (minigameType == 1) {
				// check touch
				if (cube.isTouching()) {
					done = true;
				}

			}
		}
	}
	
	void resetMinigame() {
		vid.bg0.image(vec(0,0), Blank);
	}
	
	void startMinigame(int type){ // take a param that sets game num

		done = false;
		timespan = 0;
		minigameType = type;
	
		if (minigameType == 0) {
			vid.bg0.image(vec(0,0), Background);
			shakeCount = 0;
		} else if (minigameType == 1) {
			vid.bg1.image(vec(0,0), PressBackground);
		}
	}
	
	
	bool isDone() {
		return done;
	}
	
	float getTimespan() {
		return timespan;
	}

private:
	VideoBuffer vid;
	Float2		bg;
	CubeID		cube;
	int			minigameType;
	float 		timespan;
	bool		done;
	int			shakeCount;

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

		readyToPlay = false;

		// Background & images
		vid.bg0.image(vec(0,0), MiddleBG);

		resetTimer();
		//startTimer();
        Events::cubeTouch.set(&MiddleGameCube::onTouch, this);
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
	
	void onTouch(unsigned id)
    {
		startTimer();
		readyToPlay = true;
    }
	
	bool isReadyToPlay() {
		return readyToPlay;
	}
	
	void setReadyToPlay(bool isReady) {
		readyToPlay = isReady;
	}

private:
	VideoBuffer	vid;
	bool		isRunning;
	bool		readyToPlay;
	float		timeSpan;
	CubeID		cube;
	float		ropePos;
	float		ropeDelta;
};



void main(){

	static MinigameCube cubes[2];
	static MiddleGameCube mid;

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

		if (mid.isReadyToPlay()) {
			Random rand;
			int type = rand.randint(0, 0);
			cubes[0].startMinigame(type);
			cubes[1].startMinigame(type);
			mid.setReadyToPlay(false);
		} else if (!mid.isReadyToPlay()) {
			if (cubes[0].isDone()) {
				cubes[0].resetMinigame();
			}
			if (cubes[1].isDone()) {
				cubes[1].resetMinigame();
			}
			if (cubes[0].isDone() && cubes[1].isDone()) {
				// update knot
				mid.resetTimer(10);
			}
		}

		for (unsigned i = 0; i < arraysize(cubes); i++)
			cubes[i].update(td);
		mid.update(td);


		System::paint();
		ts.next();
	}
}

