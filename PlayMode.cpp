#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <math.h>
#include <random>

#define WIDTH 1280
#define HEIGHT 720
#define CHARACTER_SIZE 5 
#define CHASE_DURATION 3.0
#define GRASS 0.05
#define FOREST 0.75
#define MAMA_CHASE 70.0

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("hexapod.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("hexapod.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

// Load< Sound::Sample > dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const * {
// 	return new Sound::Sample(data_path("dusty-floor.opus"));
// });

Load< Sound::Sample > mama_bark(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("mama-bark-final.opus"));
});

Load< Sound::Sample > water_splash(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("water-splash-final.opus"));
});

Load< Sound::Sample > puppy_cry(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("puppy-cry-final.opus"));
});

Load< Sound::Sample > twig_snapping(LoadTagDefault, []() -> Sound::Sample const * {
	return new Sound::Sample(data_path("twig-snapping-final.opus"));
});

// PlayMode::PlayMode() : scene(*hexapod_scene) {
PlayMode::PlayMode()  {
	// Initialize game attributes 
	{
		// Source: https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_int_distribution<std::mt19937::result_type> posx(800, 1276); // Generate random px
		std::uniform_int_distribution<std::mt19937::result_type> posy(300, 716); // Generate random py
		std::uniform_real_distribution<> dist(0.0,1.0);
		// Mama
		mama.px = (int) posx(rng);
		mama.py = (int) posy(rng);
		mama.dir = glm::normalize(glm::vec2((float) dist(rng), (float) dist(rng)));
		// Puppy
		puppy.px = posx(rng);
		puppy.py = posy(rng);
		// Terrain
		for(uint32_t r = 0; r < 36; r++) {
			std::vector<Terrain> row;
			row.reserve(64);
			for(uint32_t c = 0; c < 64; c++) {
				Terrain terr; 
				std::uniform_real_distribution<> prob(0.0,1.0);
				// Choosing terrain type probabilistically
				float terrType = (float) prob(rng);
				if (terrType < 0.65f) terr.type = 0;
				else if (0.65f <= terrType && terrType < 0.9f) terr.type = 1;
				else terr.type = 2;

				// Adding twigs probabilisticaly depending on terrain type
				float addTwigs = (float) prob(rng);
				if (terr.type == 0 && addTwigs < 0.05f) terr.hasTwigs = true;
				else if (terr.type == 1 && addTwigs < 0.5f) terr.hasTwigs = true;

				row.push_back(terr);
			}
			grid.push_back(row);
		}	
	}
	


	// //get pointers to leg for convenience:
	// for (auto &transform : scene.transforms) {
	// 	if (transform.name == "Hip.FL") hip = &transform;
	// 	else if (transform.name == "UpperLeg.FL") upper_leg = &transform;
	// 	else if (transform.name == "LowerLeg.FL") lower_leg = &transform;
	// }
	// if (hip == nullptr) throw std::runtime_error("Hip not found.");
	// if (upper_leg == nullptr) throw std::runtime_error("Upper leg not found.");
	// if (lower_leg == nullptr) throw std::runtime_error("Lower leg not found.");

	// hip_base_rotation = hip->rotation;
	// upper_leg_base_rotation = upper_leg->rotation;
	// lower_leg_base_rotation = lower_leg->rotation;

	// //get pointer to camera for convenience:
	// if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	// camera = &scene.cameras.front();

	// //start music loop playing:
	// // (note: position will be over-ridden in update())
	// leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	}
	// } else if (evt.type == SDL_MOUSEBUTTONDOWN) {
	// 	if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
	// 		SDL_SetRelativeMouseMode(SDL_TRUE);
	// 		return true;
	// 	}
	// } else if (evt.type == SDL_MOUSEMOTION) {
	// 	if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
	// 		glm::vec2 motion = glm::vec2(
	// 			evt.motion.xrel / float(window_size.y),
	// 			-evt.motion.yrel / float(window_size.y)
	// 		);
	// 		camera->transform->rotation = glm::normalize(
	// 			camera->transform->rotation
	// 			* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
	// 			* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
	// 		);
	// 		return true;
	// 	}
	// }

	return false;
}

void PlayMode::update(float elapsed) {
	// Move player
	this->updatePlayer(elapsed);

	// Find puppy? 
	this->findPuppy();

	// Move mama 
	this->updateMama(elapsed);	 

	// Reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	// //slowly rotates through [0,1):
	// wobble += elapsed / 10.0f;
	// wobble -= std::floor(wobble);

	// hip->rotation = hip_base_rotation * glm::angleAxis(
	// 	glm::radians(5.0f * std::sin(wobble * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 1.0f, 0.0f)
	// );
	// upper_leg->rotation = upper_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(7.0f * std::sin(wobble * 2.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );
	// lower_leg->rotation = lower_leg_base_rotation * glm::angleAxis(
	// 	glm::radians(10.0f * std::sin(wobble * 3.0f * 2.0f * float(M_PI))),
	// 	glm::vec3(0.0f, 0.0f, 1.0f)
	// );

	// //move sound to follow leg tip position:
	// leg_tip_loop->set_position(get_leg_tip_position(), 1.0f / 60.0f);

	// //move camera:
	// {

	// 	//combine inputs into a move:
	// 	constexpr float PlayerSpeed = 30.0f;
	// 	glm::vec2 move = glm::vec2(0.0f);
	// 	if (left.pressed && !right.pressed) move.x =-1.0f;
	// 	if (!left.pressed && right.pressed) move.x = 1.0f;
	// 	if (down.pressed && !up.pressed) move.y =-1.0f;
	// 	if (!down.pressed && up.pressed) move.y = 1.0f;

	// 	//make it so that moving diagonally doesn't go faster:
	// 	if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

	// 	glm::mat4x3 frame = camera->transform->make_local_to_parent();
	// 	glm::vec3 frame_right = frame[0];
	// 	//glm::vec3 up = frame[1];
	// 	glm::vec3 frame_forward = -frame[2];

	// 	camera->transform->position += move.x * frame_right + move.y * frame_forward;
	// }

	// { //update listener to camera position:
	// 	glm::mat4x3 frame = camera->transform->make_local_to_parent();
	// 	glm::vec3 frame_right = frame[0];
	// 	glm::vec3 frame_at = frame[3];
	// 	Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	// }

	// //reset button press counters:
	// left.downs = 0;
	// right.downs = 0;
	// up.downs = 0;
	// down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		constexpr float H = 0.09f;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text("Mouse motion rotates camera; WASD moves; escape ungrabs mouse",
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

glm::vec3 PlayMode::get_leg_tip_position() {
	//the vertex position here was read from the model in blender:
	return lower_leg->make_local_to_world() * glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
}


void PlayMode::updatePlayer(float elapsed) {
	// Move player
	if (left.pressed && !right.pressed) {
		if (player.px > 0 + player.speed) player.dir.x = -1.f;
	}
	if (!left.pressed && right.pressed) {
		if (player.px < WIDTH) player.dir.x = 1.f;
	}
	if (down.pressed && !up.pressed) {
		if (player.py > 0) player.dir.y = -1.f;
	}
	if (!down.pressed && up.pressed) {
		if (player.py < HEIGHT) player.dir.y = 1.f;
	}
	glm::vec2 move = glm::normalize(player.dir) * player.speed * elapsed;
	// std::cout << "Player move: " << move.x << " " << move.y << "\n";
	// player.px += (int) move.x;
	// player.py += (int) move.y;

	// Check terrain type to check for chase
	uint32_t tRow = player.px / 20;
	uint32_t tCol = player.py / 20;
	Terrain terr = grid[tRow][tCol];
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<> dist(0.0,1.0);
	bool chaseBefore = chaseOn; 
	if (terr.type == 0) {
		if (dist(rng) < GRASS) {
			chaseOn = true; 
			chaseStartTime = elapsed;
			std::cout << "YOU STEPPED ON A TWIG AND THE MOTHER HAS SPOTTED YOU. THE CHASE IS ON\n";
		}
	} else if (terr.type == 1) {
		if (dist(rng) < FOREST) {
			chaseOn = true; 
			chaseStartTime = elapsed;
			std::cout << "YOU STEPPED ON A TWIG AND THE MOTHER HAS SPOTTED YOU. THE CHASE IS ON\n";
		}
	}

	if (!chaseBefore && chaseOn) {
		// Play branch breaking
		auto twig_sound = Sound::play(*twig_snapping, 1.f, 0.f);

		// Play mama barking 
		auto mama_bark_sound = Sound::play(*mama_bark, 1.f, 0.f);
	}

	// Player makes it to water
	if (chaseOn && terr.type == 2) {
		chaseOn = false;
		auto water_splash_sound = Sound::play(*water_splash, 1.f, 0.f);
	}
	// std::cout << "Player position: " << player.px << " " << player.py << "\n";
}

void PlayMode::findPuppy() {
	// Play puppy wailing if close
	float xdist = (float) abs(puppy.px - player.px);
	float ydist = (float) abs(puppy.py - player.py);
	float dist = (float) sqrt(pow(xdist, 2) + pow(ydist, 2));
	float scaleDist = dist / 30.f;
	float vol = 0.f;
	float panDir = 0.f;
	if (scaleDist <= 1.f) {
		vol = 1.f - scaleDist; 
		if (puppy.px <= player.px) { // puppy to left
			panDir = -1.f * scaleDist;
		} else {                     // puppy to right 
			panDir = scaleDist; 
		}
	}

	// Puppy found
	if(abs(player.px - puppy.px) < CHARACTER_SIZE &&
		abs(player.py - puppy.py) < CHARACTER_SIZE) {
			auto puppy_cry_sound = Sound::play(*puppy_cry, 1.f, 0.f);
			std::cout << "PUP FOUND! YOU WIN!\n";
			exit(0);
	} else {
		auto puppy_cry_sound = Sound::play(*puppy_cry, vol, panDir);
	}
}

void PlayMode::updateMama(float elapsed) {
	// CHASE IS ON 
	if (chaseOn) {
		mama.speed = MAMA_CHASE;
		float vecx = (float) player.px - mama.px; 
		float vecy = (float) player.py - mama.py;
		mama.dir = glm::vec2(vecx, vecy);

	} else { // Move mama randomly if no chase
		std::random_device dev;
		std::mt19937 rng(dev());
		std::uniform_real_distribution<> dist(0.0,1.0);
		mama.dir = glm::vec2((float) dist(rng), (float) dist(rng));
	}

	glm::vec2 move = glm::normalize(mama.dir) * mama.speed * elapsed;
	mama.px += (int) move.x;
	mama.py += (int) move.y;

	// Check if mama catches player while on hunt
	if (chaseOn &&
		(abs(mama.px - player.px) < CHARACTER_SIZE &&
		abs(mama.py - player.py) < CHARACTER_SIZE)) {
			std::cout << "MAMA CAUGHT YOU WHILE ON THE HUNT. YOU LOST - GAME OVER\n";
			exit(0); 
	}

	// End chase criteria
	if (chaseOn && (elapsed - chaseStartTime >= CHASE_DURATION)) {
		chaseOn = false;
		mama.speed = 4.f;
		std::cout << "CHASE IS OVER MAMA IS TIRED\n";
	}
}