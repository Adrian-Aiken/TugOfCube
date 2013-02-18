#include <sifteo.h>
#include <sifteo/menu.h>
#include "assets.gen.h"
#include "Minigame.h"

using namespace Sifteo;

static 			int				numCubes = 3; // default 3, up to 9
static const    int				maxGameCubes = 9;
static			int				ballCube = 0;
static const	int				toWin = 3;
static			int				points;
static			Float2			ballPos;
static			Float2			ballVel;
static			Float2			ballAcc;
static			Neighborhood	neighbors;

static			float			fullprog = 1.0 / 14.0;
static			float			halfprog = 1.0 / 28.0;

static struct 	MenuItem 		menuItems[] = { {&IconTwoPlayer, NULL}, {&IconFourPlayer, NULL}, {&IconSixPlayer, NULL}, {&IconEightPlayer, NULL}, {NULL, NULL} };
static struct 	MenuAssets 		menuAssets = {&BgTile, &Footer, &LabelEmpty, {&Tip0, &Tip1, &Tip2, NULL}};


static AssetSlot MainSlot = AssetSlot::allocate()
	.bootstrap(GameAssets);

static Metadata M = Metadata()
	.title("Tug Of Cube Prototype")
	.package("edu.rit.aca6943.tilting", "0.2")
	.icon(Icon)
	.cubeRange(3,9);

// CUBE ENUMS
enum { MIDDLE, P1, P2, P3, P4, P5, P6, P7, P8 };

// Gametype Enums
enum { SHAKE, TAP, FLIP, STOP };

class MinigameCube {
public:

	void init(CubeID _cube){
		cube = _cube;
		done = false;
		timespan = 0;
		
		vid.initMode(BG0_BG1);
		vid.attach(cube);

		// Put in the background image
		int team = ((_cube-1) % 2 == 0 ? 0 : 1);
		vid.bg0.image(vec(0,0), TeamImage, team);
		
	}

	void update(TimeDelta timestep){
		if (done) return;
		
		timespan += timestep.seconds();
			
		// switch (1-n) for minigame type
		switch (minigameType) {

			case SHAKE:
			{
				Float2 accel = vid.physicalAccel().xy();
				if (abs(accel.x) > 40 && abs(accel.y) > 40) {
					shakeCount++;
					progress += 1.0/50.0;
					if (shakeCount >= 50) {
						done = true;
						vid.bg0.image(vec(0,0), DoneBack);
					}
				}
			}
			break;

			case TAP:
			{
				// check touching
				if (!touched && cube.isTouching()) {
					touched = true;
					touchesLeft--;
					progress += 0.1;
					if (touchesLeft == 0){
						done = true;
						vid.bg0.image(vec(0,0), DoneBack);
					}
				}

				else if (touched && !cube.isTouching()){
					touched = false;
				}
			}
			break;

			case FLIP:
			{
				int8_t z = vid.physicalAccel().z;
				if (z < -30) {
					done = true;
					vid.bg0.image(vec(0,0), DoneBack);
				}
			}
			break;

			case STOP:
			{
				progress += timestep.seconds() / 3.0;
				if (vid.physicalAccel().x > 1 && vid.physicalAccel().y > 1)
					progress = 0.0;
				if (progress >= 1.0) {
					done = true;
					vid.bg0.image(vec(0,0), DoneBack);
				}
			}
			break;
		}

		if (done) vid.bg1.eraseMask();

		// Draw the progress bar
		int xloc = 1;
		float progToDraw = progress;

		while (progToDraw > fullprog) {
			vid.bg1.image( vec(xloc++, 14), Fullprog);
			progToDraw -= fullprog;
		}
		if (progToDraw > halfprog)
			vid.bg1.image( vec(xloc++, 14), Halfprog);
		while (xloc <= 14)
			vid.bg1.image( vec(xloc++, 14), Noneprog);

	}
	
	void resetMinigame() {
		vid.bg0.image(vec(0,0), DoneBack);
	}
	
	void startMinigame(int type){ // take a param that sets game num

		done = false;
		timespan = 0;
		minigameType = type;
		progress = 0.0;

		vid.bg1.fillMask( vec(1, 14), vec(14, 1) );
	
		switch (minigameType) {
		case SHAKE:
			{
				vid.bg0.image(vec(0,0), Background);
				shakeCount = 0;
			} break;
		case TAP:
			{
				vid.bg0.image(vec(0,0), PressBackground);
				touchesLeft = 10;
			} break;
		case FLIP:
			{
				vid.bg0.image(vec(0,0), FlipBackground);
			} break;
		case STOP:
			{
				vid.bg0.image(vec(0,0), StopBackground);
			} break;
		}
	}
	
	
	bool isDone() {
		return done;
	}
	
	float getTimespan() {
		return timespan;
	}
	
	CubeID &getCubeID() {
		return cube;
	}

private:
	VideoBuffer vid;
	Float2		bg;
	CubeID		cube;
	int			minigameType;
	float 		timespan;
	bool		done;
	int			shakeCount;
	int			touchesLeft;
	bool		touched;
	float		progress;

	void writeText(const char *str)
    {
        // Text on BG1, in the 16x2 area we allocated
        vid.bg1.text(vec(0,14), Font, str);
    }
};


class MiddleGameCube {
public:
	int init(CubeID _cube){
		cube = _cube;
		vid.initMode(BG0_SPR_BG1);
		vid.attach(_cube);

		ropePos = LCD_width/2 - (Knot.pixelWidth() / 2);
		ropeDelta = LCD_width / (toWin * 2);

		readyToPlay = false;
		won = false;
		points = 0;

		// Background & images
        vid.bg0.erase(StripeTile);

		resetTimer();
		
		// ----------- MENU -------------
		Menu m(vid, &menuAssets, menuItems);
		m.anchor(1);
		
		struct MenuEvent e;
		uint8_t item;

		while (m.pollEvent(&e)) {
			switch (e.type) {
				case MENU_ITEM_PRESS:
					m.anchor(e.item);
					break;
				case MENU_EXIT:
					ASSERT(false);
					break;
				case MENU_NEIGHBOR_ADD:
					break;
				case MENU_NEIGHBOR_REMOVE:
					break;
				case MENU_ITEM_ARRIVE:
					item = e.item;
					break;
				case MENU_ITEM_DEPART:
					break;
				case MENU_PREPAINT:
					break;
				case MENU_UNEVENTFUL:
					ASSERT(false);
					break;
			}
			m.performDefault();
		}

		LOG("Selected Game: %d\n", e.item);
		vid.bg0.image(vec(0,0), MiddleBG);
		
		return (e.item+1)*2+1;
	}

	void update(TimeDelta timestep){
		if (won && vid.physicalAccel().z < -30) {
			won = false;
			init(cube);
			return;
		}
	
		if (won) return;

		if (!readyToPlay && cube.isTouching() && !isRunning) {
			startTimer();
			readyToPlay = true;
		}
		
		if (isRunning) timeSpan -= timestep.seconds();
		if (timeSpan < 0.0) {
			stopTimer();
			timeSpan = 0.0;
		}

		vid.sprites[6].setImage(Knot);
		vid.sprites[6].move( (toWin + points - 1) * ropeDelta, 65);


		// Testing if someone 'won'
		if ( points <= -3 ) {
			LOG("Team one wins!\n");
			vid.sprites.erase();
			vid.bg0.image(vec(0,0), Winner, 0);
			won = true; readyToPlay = false;
		}
		if ( points >= 3 ) {
			LOG("Team two wins with %d points!\n", points);
			vid.sprites.erase();
			vid.bg0.image(vec(0,0), Winner, 1);
			won = true; readyToPlay = false;
		}

		// Put in the time
		vid.sprites[1].setImage(Digits, floor(timeSpan / 10) % 10 );
		vid.sprites[2].setImage(Digits, floor(timeSpan) % 10);
		vid.sprites[3].setImage(Digits, floor(timeSpan * 10) % 10 );
		vid.sprites[4].setImage(Digits, floor(timeSpan *100) % 10 );
		vid.sprites[1].move(44, 24);
		vid.sprites[2].move(54, 24);
		vid.sprites[3].move(70, 24);
		vid.sprites[4].move(80, 24);


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
		if (player == P1 || player == P3 || player == P5 || player == P7) points--;
		if (player == P2 || player == P4 || player == P6 || player == P8) points++;
	}
	
	bool isReadyToPlay() {
		return readyToPlay;
	}
	
	bool isWon() {
		return won;
	}
	
	void setReadyToPlay(bool isReady) {
		readyToPlay = isReady;
	}
	
	VideoBuffer &getVideoBuffer() {
		return vid;
	}
	
	// for some reason I need this function so that
	// the startup doesn't have an error loading the image
	void setGameBackground() {
		vid.bg0.image(vec(0,0), MiddleBG);
	}

private:
	VideoBuffer	vid;
	bool		isRunning;
	bool		readyToPlay;
	bool		won;
	float		timeSpan;
	CubeID		cube;
	float		ropePos;
	float		ropeDelta;
};

void main(){

	static MiddleGameCube mid;
	numCubes = mid.init(0);
	mid.setGameBackground();
	static MinigameCube cubes[maxGameCubes];
	for (int i = 1; i < numCubes; i++) {
		cubes[i].init(i);
	}
	LOG("Initialized %d cubes\n", numCubes);
	
	Sifteo::AudioChannel(1).setVolume(Sifteo::AudioChannel::MAX_VOLUME);

	TimeStep ts;
	bool going = true;
	while (1) {
		TimeDelta td = ts.delta();
		
		if (going) {
			// Check for win
			
			// Team done?
			bool teamOneDone = true;
			bool teamTwoDone = true;
			
			if ( numCubes >= 3 ) {
				if (!cubes[P1].isDone()) teamOneDone = false;
				if (!cubes[P2].isDone()) teamTwoDone = false;
				if ( numCubes >= 5 ) {
					if (!cubes[P3].isDone()) teamOneDone = false;
					if (!cubes[P4].isDone()) teamTwoDone = false;
					if ( numCubes >= 7 ) {
						if (!cubes[P5].isDone()) teamOneDone = false;
						if (!cubes[P6].isDone()) teamTwoDone = false;
						if ( numCubes >= 9 ) {
							if (!cubes[P7].isDone()) teamOneDone = false;
							if (!cubes[P8].isDone()) teamTwoDone = false;
						}
					}
				}
			}
			
			// Check for chain of cubes
			bool teamOneChained = true;
			bool teamTwoChained = true;
			
			// Check team one
			if ( teamOneDone && numCubes >= 3 ) {
				neighbors = Neighborhood(MIDDLE);
				CubeID left = neighbors.cubeAt(Sifteo::LEFT);
				if ( !left.isDefined() || (int)left % 2 == 0 ) teamOneChained = false;
				else if ( numCubes >= 5 ) {
					neighbors = Neighborhood(left);
					left = neighbors.cubeAt(Sifteo::LEFT);
					if ( !left.isDefined() || (int)left % 2 == 0 ) teamOneChained = false;
					else if ( numCubes >= 7 ) {
						neighbors = Neighborhood(left);
						left = neighbors.cubeAt(Sifteo::LEFT);
						if ( !left.isDefined() || (int)left % 2 == 0 ) teamOneChained = false;
						else if ( numCubes >= 9 ) {
							neighbors = Neighborhood(left);
							left = neighbors.cubeAt(Sifteo::LEFT);
							if ( !left.isDefined() || (int)left % 2 == 0 ) teamOneChained = false;
						}
					}
				}
			}
			
			// Check team two
			if ( teamTwoDone && numCubes >= 3 ) {
				neighbors = Neighborhood(MIDDLE);
				CubeID right = neighbors.cubeAt(Sifteo::RIGHT);
				if ( !right.isDefined() || (int)right % 2 == 1 ) teamTwoChained = false;
				else if ( numCubes >= 5 ) {
					neighbors = Neighborhood(right);
					right = neighbors.cubeAt(Sifteo::RIGHT);
					if ( !right.isDefined() || (int)right % 2 == 1 ) teamTwoChained = false;
					else if ( numCubes >= 7 ) {
						neighbors = Neighborhood(right);
						right = neighbors.cubeAt(Sifteo::RIGHT);
						if ( !right.isDefined() || (int)right % 2 == 1 ) teamTwoChained = false;
						else if ( numCubes >= 9 ) {
							neighbors = Neighborhood(right);
							right = neighbors.cubeAt(Sifteo::RIGHT);
							if ( !right.isDefined() || (int)right % 2 == 1 ) teamTwoChained = false;
						}
					}
				}
			}
			
			
			if ( teamOneDone && teamOneChained ) {
				LOG("Team one wins this minigame\n");
				mid.addPoints(P1);
				mid.stopTimer();
				going = false;
			}
			if ( teamTwoDone && teamTwoChained ) {
				LOG("Team two wins this minigame\n");
				mid.addPoints(P2);
				mid.stopTimer();
				going = false;
			}
		}

		else if (mid.isReadyToPlay()){
			mid.resetTimer();
			mid.startTimer();
			going = true;
		}
		

		if (mid.isReadyToPlay()) {
			mid.setReadyToPlay(false);
			Random rand;
			int type = rand.randint(0, 3);

			// Play prompting audio
			switch(type){
			case FLIP:  {Sifteo::AudioChannel(1).play(SFlip);}  break;
			case SHAKE: {Sifteo::AudioChannel(1).play(SShake);} break;
			case STOP:  {Sifteo::AudioChannel(1).play(SStop);}	break;
			case TAP:   {Sifteo::AudioChannel(1).play(STap);}   break;
			}

			for (int i = 1; i < numCubes; i++) {
				cubes[i].startMinigame(type);
			}
		} else if (!mid.isReadyToPlay()) {
			if (cubes[0].isDone() && cubes[1].isDone()) {
				// update knot
				mid.resetTimer(10);
			}
		}

		for (unsigned i = 1; i < numCubes; i++) {
			if (!cubes[i].isDone()) {
				cubes[i].update(td);
			}
		}
		mid.update(td);


		System::paint();
		ts.next();
	}
}

