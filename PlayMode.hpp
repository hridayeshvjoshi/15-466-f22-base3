#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>


// Gameplay specific attributes
struct Terrain {
	int type; // 0 - Grass, 1 - forest, 2-  water
	bool hasTwigs = false; // 2% grass, 30% forest, 0% water
	// Terrain(int px_, int py_, int type_, bool hasTwigs_): px(px_), py(py_), type(type_), hasTwigs(hasTwigs_) {}
};

struct MamaDog {
	int px, py; // Position
	glm::vec2 dir;
	float speed = 10.f; // 
	// MamaDog(int px_, int py_, int vx_, int vy_, int speed_, bool isChasing_, float chaseStartTime_): px(px_), py(py_), vx(vx_), vy(vy_), speed(speed_), isChasing(isChasing_), chaseStartTime(chaseStartTime_) {}
};

struct Puppy { 
	int px, py; // Position
	bool rescued = false; 
	// Puppy(int px_, int py_, bool rescued_): px(px_), py(py_), rescued(rescued_) {}
};

struct Player {
	int px = 0;
	int py = 0; // Position
	glm::vec2 dir;
	float speed = 10.f; 
	// Player(int px_, int py_, int vx_, int vy_, int speed_): px(px_), py(py_), vx(vx_), vy(vy_), speed(speed_) {}
};


struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;

	glm::vec3 get_leg_tip_position();

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > leg_tip_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

	// Game characters
	MamaDog mama;
	Puppy puppy;
	Player player; 
	std::vector<std::vector<Terrain>> grid; // 20x20 , so 64x36 terrain valuescls
	
	// Global game conditions
	bool chaseOn = false; // Whether or not Mama is chasing you - used to set direction and increase speed 
	float chaseStartTime = 0.f; // Mama stops running after 3 seconds  - reset to random direction and speed 
	
	// Gameplay 'update' functions
	void updatePlayer(float elapsed);
	void findPuppy();
	void updateMama(float elapsed);

};
