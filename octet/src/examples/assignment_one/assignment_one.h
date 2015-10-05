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



  /// Scene containing a box with octet.
  class assignment_one : public app {

	// Matrix to transform points in our camera space to the world.
	// This lets us move our camera
	mat4t cameraToWorld;

	tmxMap map;


	vec2 joystick_axis;
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

	  joystick_axis.x() = joystick_axis.y() = 0.0f;
    }

	void read_input() {
		joystick_axis.x() = joystick_axis.y() = 0.0f;

		if (is_key_down(key_left)) {
			joystick_axis.x() = -1.0f;
		}
		else if (is_key_down(key_right)) {
			joystick_axis.x() = 1.0f;
		}
		if (is_key_down(key_up)) {
			joystick_axis.y() = 1.0f;
		}
		else if (is_key_down(key_down)) {
			joystick_axis.y() = -1.0f;
		}
		
	}

	void simulate() {
		//test_sprite.translate(joystick_axis * 0.1f);
//		printf("x: %f   y: %f\n", test_sprite.get_pos().x(), test_sprite.get_pos().y());
		cameraToWorld.translate(vec3(joystick_axis.x() * 0.1f, joystick_axis.y() * 0.1f, 0.0f));
	}

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
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
    }
  };
}
