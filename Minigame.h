#include <sifteo.h>

class Minigame {
public:
	// Initilization stuff should happen in here
	Minigame(VideoBuffer vb);
	~Minigame();
	
	// Will be called constantly - game logic should be here
	virtual void update(TimeDelta timeStep) = 0;
	
	// Called after every update until true is returned
	virtual bool isDone() = 0;
	
private:
	VideoBuffer vid;
	unsigned frame;
	float fpsTimeSpan;
}