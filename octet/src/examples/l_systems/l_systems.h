////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include <fstream>
#include "l_systems_utils.h"

namespace octet {

  class l_systems : public app {

	  string axiom;
	  dynarray<string> rules;
	  int num_iterations;

  public:
	  
	/// this is called when we construct the class before everything is initialised.
    l_systems(int argc, char **argv) : app(argc, argv) {
    
	}

	void load_from_file(string file_name)
	{
		std::ifstream file(file_name);
		if (file.bad()) return;

		// store the line here
		char buffer[256];


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
				else if (line_num >= 2)
				{
					// +3 is to skip the 3 characters from 
					// the left hand operand of the rule (for example "A->")
					rules.push_back(buffer);
				}
				++line_num;
			}
		}
	}

    /// this is called once OpenGL is initialized
    void app_init() {

		load_from_file("../../../assets/example_2.txt");

		//rules.push_back("AAB");
		//rules.push_back("BA");

		printf("Axiom %s\n", axiom);
		printf("Rules: \n");

		for (int j = 0; j < rules.size(); j++) {
			printf("%s\n", rules[j]);
		}

		string result = axiom;

		for (int j = 0; j < num_iterations; j++) {
			result = l_system_utils::iterate(result, rules);
			printf("iteration %d: %s \n", j, result);
		}
    }

	void draw_iteration(string data)
	{
		for (int j = 0; j < data.size(); j++) {
			switch (data[j])
			{
			case '0':
				break;
			case '1':
				break;
			case '[':
				break;
			case ']':
				break;
			}
		}
	}

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
		
    }


	void draw_line(vec2 start, vec2 end)
	{
		glBegin(GL_LINES);
		glVertex3f(start.x(), start.y(), 0);
		glVertex3f(end.x(), end.y(), 0);
		glEnd();

		//mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);
	}


  };
}
