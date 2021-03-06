////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include "sprite.h"
#include "xmldump.h"
#include "TmxMap.h"
#include <iostream>
#include <fstream>
#include <string>

namespace octet {

	class character {

	private:
		sprite _sprite;
		texture_shader texture_shader_;

		vec2 prevPos, targetPos;
		bool is_moving;
	public:
		character(){}
		character(sprite s)
		{
			_sprite = s;
			texture_shader_.init();
			rot = 0;
			prevPos = _sprite.get_pos();
			targetPos = prevPos;
			is_moving = false;
		}

		void move(int dir)
		{
			if (is_moving)
			{
				return;
			}
			//_sprite.set_sprite_index(dir);
			switch (dir)
			{

			case 0:
				rot = 4;
				//_sprite.translate(-tileset::TILE_SIZE, 0.0f);
				targetPos = prevPos + vec2(-tileset::TILE_SIZE, 0.0f);
				break;

			case 1:
				rot = 12;
				//_sprite.translate(0.0f, tileset::TILE_SIZE);
				targetPos = prevPos + vec2(0.0f, tileset::TILE_SIZE);

				break;

			case 2:
				rot = 8;
				//_sprite.translate(tileset::TILE_SIZE, 0.0f);
				targetPos = prevPos + vec2(tileset::TILE_SIZE, 0.0f);

				break;

			case 3:
				rot = 0;
				//_sprite.translate(0.0f, -tileset::TILE_SIZE);
				targetPos = prevPos + vec2(0.0f, -tileset::TILE_SIZE);

				break;
			}
			//_sprite.translate(tileset::TILE_SIZE, tileset::TILE_SIZE);
		}

		int rot;
		unsigned int anim_fr;
		void update(float delta_time)
		{

			++anim_fr;
			_sprite.set_sprite_index(rot + ((anim_fr / 4) % 4));


			if ((targetPos - _sprite.get_pos()).length() > 0.01f)
			{
				//printf("%f, %f \n", targetPos.x(), targetPos.y());
				is_moving = true;
				_sprite.translate((targetPos - _sprite.get_pos()).normalize() * delta_time * 0.8f);
			}
			else
			{
				//printf("arrived\n");
				is_moving = false;
				prevPos = targetPos;
				_sprite.set_pos(prevPos);
			}
		}

		void render(mat4t camera)
		{
			_sprite.render(texture_shader_, camera);
		}
	};

  /// Scene containing a box with octet.
  class assignment_one : public app {

	// Matrix to transform points in our camera space to the world.
	// This lets us move our camera
	mat4t cameraToWorld;

	tmxMap map;


	vec2 joystick_axis;

	sprite player_sprite;
	character player;
  public:
    /// this is called when we construct the class before everything is initialised.
    assignment_one(int argc, char **argv) : app(argc, argv) {
    }


    /// this is called once OpenGL is initialized
    void app_init() {

		map.load_xml("assets/2D_tiles/Examples/Dungeon.tmx");
		map.dump_tilesets();
		// set up the matrices with a camera 5 units from the origin
		cameraToWorld.loadIdentity();
		cameraToWorld.translate(0, 0, 3);
		GLuint player_tex = resource_dict::get_texture_handle(GL_RGBA, "assets/2D_tiles/Commissions/Template.gif");
		player_sprite.init(player_tex, 0.2f, 0.2f, 0.2f, 0.2f, 0, 16, 16, 64, 64);
		player = character(player_sprite);

		joystick_axis.x() = joystick_axis.y() = 0.0f;
    }

	void read_input() {
		joystick_axis.x() = joystick_axis.y() = 0.0f;

		if (is_key_down(key_left)) {
			joystick_axis.x() = -1.0f;
			player.move(0);

		}
		else if (is_key_down(key_right)) {
			joystick_axis.x() = 1.0f;
			player.move(2);

		}
		if (is_key_down(key_up)) {
			joystick_axis.y() = 1.0f;
			player.move(1);

		}
		else if (is_key_down(key_down)) {
			joystick_axis.y() = -1.0f;
			player.move(3);
		}
		
	}



	void simulate() {
		//test_sprite.translate(joystick_axis * 0.1f);
//		printf("x: %f   y: %f\n", test_sprite.get_pos().x(), test_sprite.get_pos().y());
		//cameraToWorld.translate(vec3(joystick_axis.x() * 0.2f, joystick_axis.y() * 0.2f, 0.0f));
		player.update(1/33.0f);
	}

	int framecount;
    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
		//printf(app_utils::get_atom_name(atom_rotateX));


		++framecount;
		//player.move((framecount / 4) % 4);

		int vx = 0, vy = 0;
		get_viewport_size(vx, vy);

		// set a viewport - includes whole window area
		glViewport(x, y, w, h);

		// clear the background to black
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
		glDisable(GL_DEPTH_TEST);

		// allow alpha blend (transparency when alpha channel is 0)
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		read_input();
		simulate();

		//test_sprite.render(texture_shader_, cameraToWorld);

		for each (sprite s in map.sprites)
		{
			s.render(map.texture_shader_, cameraToWorld);
		}

		player.render(cameraToWorld);

		//player_sprite.tra
    }
  };
}
