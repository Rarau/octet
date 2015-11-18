#include <math.h>

namespace octet
{
	class l_node
	{
	public:
		mat4t transform;
		vec3 color;
		vec3 end;
	};
	class l_system
	{
	private:
		tree_shader shader;

		string starting_axiom;
		dynarray<string> rules;

		string current_axiom;

		int cur_iters;

		dynarray<l_node> states;
		dynarray<l_node> nodes;
		dynarray<float> vertices;

		dynarray<float> angle_state;
		dynarray<float> angle_state_y;


		l_node cur_node;

		float angle;
		float branch_len = 0.1f;

	public:
		mat4t transform;


		l_system()
		{
			//shader = new param_shader("shaders/default.vs", "shaders/simple_color.fs");
			transform.loadIdentity();
		}

		void load_from_file(string file_name)
		{
			cur_node.transform = transform;

			shader.init();

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
			states.reset();

			int line_num = 0;
			while (!file.eof())
			{
				file.getline(buffer, sizeof(buffer));
				// Ignore "comments"
				if (buffer[0] != '/')
				{
					if (line_num == 0)
					{
						starting_axiom = buffer;
					}
					else if (line_num == 1)
					{
						cur_iters = atoi(buffer);
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

			printf("Axiom %s\n", starting_axiom);
			printf("Rules: \n");

			for (int j = 0; j < rules.size(); j++) {
				printf("%s\n", rules[j]);
			}

			generate_nodes();
		}

		void generate_nodes()
		{
			current_axiom = starting_axiom;

			for (int j = 0; j < cur_iters; j++) {
				current_axiom = apply_rules_to_axiom(current_axiom, rules);
				//printf("iteration %d: %s \n", j, current_axiom);
			}

			generate_nodes_from_axiom(current_axiom);

			mat4t t = transform;
			t.invertQuick(t);

			// Store all positions in our vertex array
			int num_nodes = nodes.size();
			vertices.reset();
			for (int j = 0; j < num_nodes; j++) {
				vertices.push_back(nodes[j].transform.row(3).x());
				vertices.push_back(nodes[j].transform.row(3).y());
				vertices.push_back(nodes[j].transform.row(3).z());

				vertices.push_back(nodes[j].color.x());
				vertices.push_back(nodes[j].color.y());
				vertices.push_back(nodes[j].color.z());
				
				vertices.push_back(nodes[j].end.x());
				vertices.push_back(nodes[j].end.y());
				vertices.push_back(nodes[j].end.z());


				vertices.push_back(nodes[j].color.x());
				vertices.push_back(nodes[j].color.y());
				vertices.push_back(nodes[j].color.z());
			}
			
			//printf("Current_axiom %s\n", current_axiom);
		}

		void regenerate()
		{
			generate_nodes();
		}

		static string apply_rules_to_axiom(string axiom, dynarray<string>& rules) {
			dynarray<char> result;

			// Go through each character of the axiom
			int axiom_size = axiom.size();
			int rules_size = rules.size();


			dynarray<int> rule_count;
			char prev_rule_char;
			for (int j = 0; j < rules_size; j++)
			{
				if (j == 0)
				{
					rule_count.push_back(1);
				}
				else
				{
					// We found the same character (left hand side) so we increase the count
					if (prev_rule_char == rules[j][0])
					{
						rule_count.back()++;
					}
					// We found a new character so we push a 1 (we only found one for now)
					else
					{
						rule_count.push_back(1);
					}
				}
				prev_rule_char = rules[j][0];
			}

			srand(time(NULL));
			//random rnd;
			//rnd.set_seed(time(NULL) / 1000);
			//printf("time %d\n", time(NULL));

			// Iterate through the axiom characters
			for (int i = 0; i < axiom_size; i++) {
				char current = axiom[i];
				int rule_count_index = 0;

				bool rule_found = 0;
				// Find the rule to apply to the current character
				int j = 0;
				for (; j < rules_size; j++) {
					/*
					if (current != prev_rule_char)
						rule_count_index++;
						*/
					//int min_random = 0;
					//float half = (float)min_random / (float)random_num;

					if (current == rules[j][0]) {

						//int random_num = rnd.get(0, rule_count[rule_count_index]);
						int random_num = rand() % rule_count[rule_count_index];
						int rule_index = j + random_num;
						//printf("Rule chosen %s\n", rules[rule_index]);

						// We push the applied rule to the result string
						for (int x = 1; x < rules[rule_index].size(); x++) {
							result.push_back(rules[rule_index][x]);
						}
						// We found the rule so we stop searching
						j += rule_count[rule_count_index];
						rule_found = true;
						rule_count_index++;
						break;
					}
				}

				// If no rule was found we just push the current character
				if (j == rules.size() && !rule_found) {
					result.push_back(current);
				}

				prev_rule_char = current;

			}

			// Add a null terminating character to the end of our result string
			result.push_back(0x00);
			return string(result.data());
		}

		void iterate_forward()
		{
			cur_iters++;
			generate_nodes();
		}

		void iterate_backwards()
		{
			cur_iters--;
			generate_nodes();
		}

		void render(mat4t &cameraToWorld)
		{
			mat4t modelToProjection = mat4t::build_projection_matrix(transform, cameraToWorld);
			//glBindAttribLocation(shader.program(), attribute_color, "color");
			glBindAttribLocation(shader.program(), attribute_color, "color");

			shader.render(modelToProjection, 0);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)vertices.data());
			glEnableVertexAttribArray(attribute_pos);
			
			glVertexAttribPointer(attribute_color, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(vertices.data() + 3));
			glEnableVertexAttribArray(attribute_color);

			glLineWidth(2.0f);
			glDrawArrays(GL_LINES, 0, nodes.size() << 1 );
		}
		
		float cur_angle, cur_angle_y;
		void generate_nodes_from_axiom(string axiom)
		{
			vec3 end_pos;

			// Reset (clear) everything
			nodes.reset();
			angle_state.reset();
			cur_angle = 0.0f;
			cur_angle_y = 0.0f;


			cur_node.transform = transform;
			cur_node.end = vec3(0, 0, 0);
			cur_node.color = vec3(0.5f);
			l_node node;

			int size = axiom.size();

			for (int j = 0; j < size; j++) {
				switch (axiom[j])
				{
				/*
				case 'L':
					cur_dir = cur_dir.normalize();
					end_pos = cur_pos + cur_dir * branch_len;


					node.color = vec3(0.0f, 250.0f, 0.0f);
					node.start = cur_pos;
					node.end = end_pos;

					nodes.push_back(node);

					break;
					*/
				case 'F':
				{
					mat4t inverseTransform;

					mat4t t = cur_node.transform;
					t.loadIdentity();
					t.translate(cur_node.end);

					t.invertQuick(inverseTransform);
					vec4 localForward = vec4(0.0f, 0.0f, 1.0f, 0.0f) * inverseTransform;
					t.rotate(cur_angle, localForward.x(), localForward.y(), localForward.z());

					vec4 up = vec4(0, 1, 0, 0);
					t.rotate(cur_angle_y, up.x(), up.y(), up.z());

					node.transform = t;

					node.color = cur_node.color + vec3(0.0f, 0.0f, 0.1f);

					
					t.invertQuick(inverseTransform);
					vec4 localUp = vec4(0, 1, 0, 0) * inverseTransform;
					//vec3 localUp2 = t.y().normalize();

					node.end = (t.row(3).xyz() + (localUp * branch_len));

					nodes.push_back(node);


					cur_node = node;
				}

					break;
				

				case '[':
				{
					// Save the current state
					//l_node n = cur_node;
					states.push_back(cur_node);
					angle_state.push_back(cur_angle);
					angle_state_y.push_back(cur_angle_y);

				}

					break;

				case ']':
				{
					// Load the saved state
					//int last = states.size();
					//cur_node = states[last - 1];
					cur_node = states.back();
					states.pop_back();


					cur_angle = angle_state.back();
					angle_state.pop_back();

					cur_angle_y = angle_state_y.back();
					angle_state_y.pop_back();
				}
					break;

				case '+':
					// Turn right
					cur_angle += angle;
					break;

				case '-':
					// Turn left 
					cur_angle -= angle;
					break;
				case '*':
					// Turn right
					cur_angle_y += angle;
					break;

				case '/':
					// Turn left 
					cur_angle_y -= angle;
					break;
				}
			}
		}
	};



	class l_system_utils
	{
	public:

		static string iterate(string axiom, dynarray<string>& rules) 
		{
			dynarray<char> result;

			// Go through each character of the axiom
			int axiom_size = axiom.size();
			int rules_size = rules.size();
			dynarray<int> rule_count;
			char prev_rule_char;
			for (int j = 0; j < rules_size; j++) 
			{
				if (j == 0)
				{
					rule_count.push_back(1);
				}
				else
				{
					// We found the same character (left hand side) so we increase the count
					if (prev_rule_char == rules[j][0])
					{
						rule_count.back()++;
					}
					// We found a new character so we push a 1 (we only found one for now)
					else
					{
						rule_count.push_back(1);
					}
				}
				prev_rule_char = rules[j][0];
			}


			for (int i = 0; i < axiom_size; i++) {
				char current = axiom[i];

				// Find the rule to apply to the current character
				int j = 0;
				for (; j < rules_size; j++) {
					if (current == rules[j][0]) {
						// We push the applied rule to the result string
						for (int x = 1; x < rules[j].size(); x++) {
							result.push_back(rules[j][x]);
						}
						// We found the rule so we stop searching
						break;
					}
				}

				// If no rule was found we just push the current character
				if (j == rules_size) {
					result.push_back(current);
				}
			}
			// Add a null terminating character to the end of our result string
			result.push_back(0x00);
			return string(result.data());
		}
	};
}