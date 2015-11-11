////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include <fstream>
#include "l_systems_utils.h"

#define DEG_TO_RAD 0.01745329251f

namespace octet {

  class l_systems : public app {

	  struct node
	  {
		  vec3 pos;
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
	 

	  int cur_example = 0;
  public:
	  
	/// this is called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv) {
    
	}


    /// this is called once OpenGL is initialized
    void app_init() {

		cur_angle = 0.0f;

		cur_dir = vec2(0.0f, 1.0f);

		start_pos = vec2(0.0f, -1.0f);
		cur_pos = start_pos;

		example_files.push_back("../../../assets/example_1.txt");
		example_files.push_back("../../../assets/example_2.txt");
		example_files.push_back("../../../assets/example_3.txt");
		example_files.push_back("../../../assets/example_4.txt");

		load_from_file(example_files[0]);
		cur_iters = num_iterations;
		generate_points(axiom, rules, cur_iters);
		center_points(points);
		/*
		printf("Axiom %s\n", axiom);
		printf("Rules: \n");

		for (int j = 0; j < rules.size(); j++) {
			printf("%s\n", rules[j]);
		}

		string result = axiom;

		for (int j = 0; j < num_iterations; j++) {
			result = l_system_utils::iterate(result, rules);
			//printf("iteration %d: %s \n", j, result);
		}

		parse_iteration(result);
		*/
    }
	int cur_iters = 1;

	void generate_points(string axiom, dynarray<string>& rules, int num_iterations)
	{
		string result = axiom;

		for (int j = 0; j < num_iterations; j++) {
			result = l_system_utils::iterate(result, rules);
			printf("iteration %d: %s \n", j, result);
		}

		parse_iteration(result);
	}

	void parse_iteration(string data)
	{

		vec2 end_pos;
		cur_angle = 0.0f;

		cur_dir = vec2(0.0f, 1.0f);

		start_pos = vec2(0.0f, -1.0f);
		cur_pos = start_pos;
		points.reset();

		for (int j = 0; j < data.size(); j++) {
			switch (data[j])
			{
			case 'L':

				break;
			case 'F':
				// Draw a line segment
				cur_dir = cur_dir.normalize();
				end_pos = cur_pos + cur_dir * branch_len;
				//draw_line(cur_pos, end_pos);
				points.push_back(cur_pos);
				points.push_back(end_pos);
				cur_pos = end_pos;
				break;

			case '[':
				// Push position and angle
				pos_state.push_back(cur_pos);
				dir_state.push_back(cur_dir);
				break;

			case ']':
				// Pop position and angle
				cur_pos = pos_state.back();
				cur_dir = dir_state.back();
				pos_state.pop_back();
				dir_state.pop_back();
				break;

			case '+':
				// Turn right
				rotate_vec2(cur_dir, -angle);
				break;

			case '-':
				// Turn left 
				rotate_vec2(cur_dir, angle);
				break;
			}
		}
	}

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

		draw_points();

		if (is_key_going_down(key_up))
		{
			cur_iters++;
			generate_points(axiom, rules, cur_iters);
			center_points(points);
		}

		if (is_key_going_down(key_down))
		{
			cur_iters--;
			cur_iters = cur_iters < 1 ? 1 : cur_iters;

			generate_points(axiom, rules, cur_iters);
			center_points(points);
		}

		if (is_key_going_down(key_right))
		{
			//cur_example = (cur_example + 1) & 0xFFFF;
			//cur_example %= (example_files.size());
			cur_example++;
			if (cur_example >= example_files.size())
				cur_example = 0;

			load_from_file(example_files[cur_example]);

			cur_iters = num_iterations;
			generate_points(axiom, rules, cur_iters);
			center_points(points);
		}

		if (is_key_going_down(key_left))
		{
			//cur_example = (cur_example - 1) & 0xFFFF;
			cur_example--;
			if (cur_example < 0)
				cur_example = example_files.size() - 1;
			//cur_example = abs(cur_example);
			//cur_example %= (example_files.size());
			load_from_file(example_files[cur_example]);

			cur_iters = num_iterations;
			generate_points(axiom, rules, cur_iters);
			center_points(points);
		}

		//printf("cur_iters %d\n", cur_iters);

		//cur_iters = cur_iters > 6 ? 6 : cur_iters;

    }

	void rotate_vec2(vec2& vec, float degrees)
	{
		float cs = cos(degrees * DEG_TO_RAD);
		float sn = sin(degrees * DEG_TO_RAD);
		float px = vec.x() * cs - vec.y() * sn;
		float py = vec.x() * sn + vec.y() * cs;

		vec.x() = px;
		vec.y() = py;
	}

	void get_scale_and_midpoint(dynarray<vec2>& points, float& scale, vec2& midpoint)
	{
		scale = 0.0f;
		midpoint = vec2(0.0f);

		vec2 top_right = vec2(0.0f);
		vec2 bottom_left = vec2(0.0f);

		for (int i = 0; i < points.size(); i++)
		{
			top_right.x() = (points[i].x() < top_right.x()) ? top_right.x() : points[i].x();
			top_right.y() = (points[i].y() < top_right.y()) ? top_right.y() : points[i].y();

			bottom_left.x() = (points[i].x() > bottom_left.x()) ? bottom_left.x() : points[i].x();
			bottom_left.y() = (points[i].y() > bottom_left.y()) ? bottom_left.y() : points[i].y();
		}

		scale = (top_right - bottom_left).length();

		printf("top_right %f, %f\n", top_right.x(), top_right.y());
		printf("bottom_left %f, %f\n", bottom_left.x(), bottom_left.y());

		printf("Scale of points: %f\n", scale);

		midpoint = (top_right + bottom_left) * 0.5f;
		//return scale;
	}

	void draw_points()
	{
		// clear the background to black
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBegin(GL_LINES);
		glColor3f(250.0f, 0.0f, 0.0f);
		//glColor3b(0x00, 0xff, 0x00);
		for (int i = 0; i < points.size(); i++)
		{
			glVertex3f(points[i].x(), points[i].y(), 0);
			++i;
			glVertex3f(points[i].x(), points[i].y(), 0);
		}

		glEnd();

		//mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
	}

	// Resize the tree so that it fits the screen (normalize it) and center it
	void center_points(dynarray<vec2>& points)
	{
		float scale;
		vec2 center;
		get_scale_and_midpoint(points, scale, center);

		if (scale != 0.0f)
			scale = 2.0f / scale;
		else return;

		for (int i = 0; i < points.size(); i++)
		{
			points[i] -= center;
			points[i] *= scale;
		}
	}

	void load_from_file(string file_name)
	{
		std::ifstream file(file_name);
		if (file.bad())
		{
			printf("Error loading file: %s\n", file_name);
			return;
		}
		else
			printf("Success loading file: %s\n", file_name);

		// store the line here
		char buffer[256];

		rules.reset();
		pos_state.reset();
		dir_state.reset();

		int line_num = 0;
		while (!file.eof())
		{
			file.getline(buffer, sizeof(buffer));
			// Ignore "comments"
			if (buffer[0] != '/')
			{
				if (line_num == 0)
				{
					axiom = buffer;
				}
				else if (line_num == 1)
				{
					num_iterations = atoi(buffer);
				}
				else if (line_num == 2)
				{
					angle = atof(buffer);
				}
				else if (line_num >= 3)
				{
					// +3 is to skip the 3 characters from 
					// the left hand operand of the rule (for example "A->")
					rules.push_back(buffer);
				}
				++line_num;
			}
		}

		printf("Axiom %s\n", axiom);
		printf("Rules: \n");

		for (int j = 0; j < rules.size(); j++) {
			printf("%s\n", rules[j]);
		}
	}

  };
}
