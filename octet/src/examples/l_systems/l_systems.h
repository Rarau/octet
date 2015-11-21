////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include <fstream>
#include "tree_shader.h"
#include "grid_shader.h"

#include "l_systems_utils.h"
#include "text_helper.h"
#include "grid.h"


#define DEG_TO_RAD 0.01745329251f

namespace octet {

  class l_systems : public app {

	  struct node
	  {
		  vec2 start;
		  vec2 end;

		  vec3 color;
	  };
	  
	  string axiom;
	  dynarray<string> rules;
	  int num_iterations;

	  dynarray<vec2> pos_state;
	  dynarray<vec2> dir_state;

	  dynarray<float> angle_state;

	  vec2 cur_pos;
	  vec2 cur_dir;
	  float cur_angle;

	  vec2 start_pos;

	  float branch_len = 0.1f;
	  float leaf_len = 0.075f;
	  float angle = 45.0f;

	  dynarray<vec2> points;
	  dynarray<string> example_files;
	 
	  dynarray<node> nodes;

	  int cur_example = 0;

	  l_system tree;
	  grid grid;

	  mat4t cameraToWorld;
	  text_helper info_text, help_text;

	  // Camera configuration
	  float cam_speed = 0.250f;
	  float cam_rot_speed = 0.25f;
	  int prev_x = 0, prev_y = 0;
	  bool look_enabled = false;
	  bool rotation_enabled = true;

  public:
	  
	/// this is called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv) {
    
	}


    /// this is called once OpenGL is initialized
    void app_init() 
	{
		example_files.push_back("../../../assets/example_1.txt");
		example_files.push_back("../../../assets/example_2.txt");
		example_files.push_back("../../../assets/example_3.txt");
		example_files.push_back("../../../assets/example_4.txt");
		example_files.push_back("../../../assets/example_5.txt");
		example_files.push_back("../../../assets/example_6.txt");
		example_files.push_back("../../../assets/example_7.txt");
		example_files.push_back("../../../assets/example_8.txt");
		example_files.push_back("../../../assets/example_9.txt");
		example_files.push_back("../../../assets/example_10.txt");

		tree.load_from_file(example_files[0]);

		cameraToWorld.loadIdentity();
		cameraToWorld.translate(vec3(0.0f, 1.25f, 2.75f));
		cameraToWorld.rotateX(-5.0f);

		grid.init(20, 20, 1.0f);
		info_text.init();
		help_text.init();
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) 
	{
		int vx = 0, vy = 0;
		get_viewport_size(vx, vy);
		//glViewport(0, 0, vx, vy);
		glViewport(x, y, w, h);

		if (rotation_enabled)
		{
			// Continuously rotate the tree
			tree.transform.rotateY(0.251f);
		}

		read_input();
		update_camera();

		// clear the background to black
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDisable(GL_DEPTH_TEST);

		// allow alpha blend (transparency when alpha channel is 0)
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		

		grid.render(cameraToWorld);
		tree.render(cameraToWorld);

		char score_text[512];
		tree.get_tree_info(score_text);
		//sprintf(score_text, "score: %d   lives: %d\n", 15, 56);
		//draw_text(texture_shader_, -1.75f, 2, 1.0f / 256, score_text);

		
		info_text.draw_text(cameraToWorld, -1.75f, 1.0f, 0.75f / 256, score_text);
		help_text.draw_text(cameraToWorld, 1.75f, 1.0f, 0.5f / 256, "-WASD: Move\n-Q/E: Up/Down\n-Hold right mouse: Look\n");
		help_text.draw_text(cameraToWorld, 1.75f, 0.5f, 0.5f / 256, "-Left/Right arrows: Examples\n-Up/Down arrows: Iterate");
		help_text.draw_text(cameraToWorld, 1.75f, 0.0f, 0.5f / 256, "-Space bar: Regenerate tree\n -R: Toggle rotation");

    }


	void update_camera()
	{
		//mat4t worldToCamera;
		//cameraToWorld.invertQuick(worldToCamera);

		// Camera controls
		if (is_key_down('W'))
		{
			cameraToWorld.translate(vec4(0.0f, 0.0f, 1.0f, 0.0f) * -cam_speed);
		}
		if (is_key_down('S'))
		{
			cameraToWorld.translate(vec4(0.0f, 0.0f, 1.0f, 0.0f) * cam_speed);
		}
		if (is_key_down('A'))
		{
			cameraToWorld.translate(vec4(1.0f, 0.0f, 0.0f, 0.0f)  * -cam_speed);
		}
		if (is_key_down('D'))
		{
			cameraToWorld.translate(vec4(1.0f, 0.0f, 0.0f, 0.0f)  * cam_speed);
		}
		if (is_key_down('Q'))
		{
			cameraToWorld.translate(cameraToWorld.y() * cam_speed);
		}
		if (is_key_down('E'))
		{
			cameraToWorld.translate(cameraToWorld.y() * -cam_speed);
		}

		look_enabled = is_key_down(key_rmb);

		// Get and store the current mouse position
		int mouse_x, mouse_y;
		get_mouse_pos(mouse_x, mouse_y);


		if (look_enabled)
		{
			disable_cursor();
			//vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
			cameraToWorld.rotateX((-mouse_y + prev_y) * cam_rot_speed);
			//cameraToWorld.rotate(-mouse_x + prev_x, cameraUp.x(), cameraUp.y(), cameraUp.z());
		}
		if (is_key_going_up(key_rmb))
		{
			enable_cursor();
		}

		// Update the mouse position
		prev_x = mouse_x;
		prev_y = mouse_y;
	}

	void read_input()
	{
		if (is_key_going_down(key_up))
		{
			tree.iterate_forward();
		}

		if (is_key_going_down(key_down))
		{
			tree.iterate_backwards();
		}

		if (is_key_going_down(key_right))
		{
			cur_example++;
			if (cur_example >= example_files.size())
				cur_example = 0;

			tree.load_from_file(example_files[cur_example]);
		}

		if (is_key_going_down(key_left))
		{
			cur_example--;
			if (cur_example < 0)
				cur_example = example_files.size() - 1;

			tree.load_from_file(example_files[cur_example]);
		}

		if (is_key_down(key_space))
		{
			tree.regenerate();
		}

		if (is_key_down('X'))
		{
			tree.randomize_color();
		}

		if (is_key_going_down('R'))
		{
			rotation_enabled = !rotation_enabled;
		}
	}
  };
}
