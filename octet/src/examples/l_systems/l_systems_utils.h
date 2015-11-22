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
		string file_;

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
		float cur_angle, cur_angle_y;
		float line_width = 1.0f;

		vec3 start_color;
	public:
		mat4t transform;

		l_system()
		{
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

			generate_tree();
			//getFileName(file_name);

			printf("fnpos: %d\n", file_name.filename_pos());

			file_ = string(file_name.data() + file_name.filename_pos());
		}


		void generate_tree()
		{
			// Reset the axiom to the initial value
			current_axiom = starting_axiom;

			// Depending on the number of iterations, run the rule engine recursively on the axiom
			for (int j = 0; j < cur_iters; j++) 
			{
				current_axiom = apply_rules_to_axiom(current_axiom, rules);
			}

			// Generate all the required data (nodes) using the turtle algorithm
			parse_text_tree(current_axiom);

			// Using the previously generated data nodes, generate our geometry (vertices)
			generate_geometry();
		}

		vec3 max, min;
		float scale;
		void generate_geometry()
		{
			transform.loadIdentity();
			scale = 1.0f;
			max = vec3(0);
			min = vec3(0);

			int num_nodes = nodes.size();
			vertices.reset();
			for (int j = 0; j < num_nodes; j++) 
			{
				// Update the tree boundaries
				max.x() = (nodes[j].transform.row(3).x() > max.x()) ? nodes[j].transform.row(3).x() : max.x();
				max.y() = (nodes[j].transform.row(3).y() > max.y()) ? nodes[j].transform.row(3).y() : max.y();
				max.z() = (nodes[j].transform.row(3).z() > max.z()) ? nodes[j].transform.row(3).z() : max.z();

				min.x() = (nodes[j].transform.row(3).x() < min.x()) ? nodes[j].transform.row(3).x() : min.x();
				min.y() = (nodes[j].transform.row(3).y() < min.y()) ? nodes[j].transform.row(3).y() : min.y();
				min.z() = (nodes[j].transform.row(3).z() < min.z()) ? nodes[j].transform.row(3).z() : min.z();

				// Update the tree boundaries
				max.x() = (nodes[j].end.x() > max.x()) ? nodes[j].end.x() : max.x();
				max.y() = (nodes[j].end.y() > max.y()) ? nodes[j].end.y() : max.y();
				max.z() = (nodes[j].end.z() > max.z()) ? nodes[j].end.z() : max.z();

				min.x() = (nodes[j].end.x() < min.x()) ? nodes[j].end.x() : min.x();
				min.y() = (nodes[j].end.y() < min.y()) ? nodes[j].end.y() : min.y();
				min.z() = (nodes[j].end.z() < min.z()) ? nodes[j].end.z() : min.z();

				// Push start vertex coordinates
				vertices.push_back(nodes[j].transform.row(3).x());
				vertices.push_back(nodes[j].transform.row(3).y());
				vertices.push_back(nodes[j].transform.row(3).z());

				// Push start vertex color
				vertices.push_back(nodes[j].color.x());
				vertices.push_back(nodes[j].color.y());
				vertices.push_back(nodes[j].color.z());

				// Push end vertex coordinates
				vertices.push_back(nodes[j].end.x());
				vertices.push_back(nodes[j].end.y());
				vertices.push_back(nodes[j].end.z());

				// Push end vertex color
				vertices.push_back(nodes[j].color.x());
				vertices.push_back(nodes[j].color.y());
				vertices.push_back(nodes[j].color.z());
			}

			// Calculate the tree scale according to the boundaries we found
			scale = (max - min).length();
			// Check division by small numbers
			if (scale > 0.0001f)
			{
				// Scale the tree transform so that it always has the same size
				transform.scale(3.0f / scale, 3.0f / scale, 3.0f / scale);
			}
		}

		// Recreate the whole tree (useful to see the stochastics rules in action)
		void regenerate()
		{
			generate_tree();
		}

		static string apply_rules_to_axiom(string axiom, dynarray<string>& rules) 
		{
			dynarray<char> result;

			// Go through each character of the axiom
			int axiom_size = axiom.size();
			int rules_size = rules.size();

			// This array stores the number of rules found for each character
			dynarray<int> rule_count;
			char prev_rule_char;
			for (int j = 0; j < rules_size; j++)
			{
				// For the first rule, just push a 1 to the array
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

			// Set the random seed to use in the stochastic rules
			srand(time(NULL));

			// Iterate through the axiom characters
			for (int i = 0; i < axiom_size; i++) 
			{
				char current = axiom[i];
				int rule_count_index = 0;

				bool rule_found = false;
				// Find the rule to apply to the current character
				int j = 0;
				for (; j < rules_size; j++) 
				{
					if (current == rules[j][0]) 
					{
						int rule_index = j;
						// If there's more than one rule for this character
						if (rule_count[rule_count_index] > 1)
						{
							// Generate a random number to choose our rule
							int random_num = rand() % rule_count[rule_count_index];
							rule_index = j + random_num;
							//printf("Rule chosen %s\n", rules[rule_index]);
						}
		
						// We push the applied rule to the result string
						for (int x = 2; x < rules[rule_index].size(); x++) 
						{
							result.push_back(rules[rule_index][x]);
						}
						// We found the rule so we stop searching
						// And skip the rest of the rules for this character
						j += rule_count[rule_count_index];
						rule_found = true;
						//rule_count_index++;
						break;
					}
					rule_count_index++;
				}

				// If no rule was found we just push the current character
				if (j == rules.size() && !rule_found) 
				{
					result.push_back(current);
				}

			}

			// Add a null terminating character to the end of our result string
			result.push_back(0x00);
			return string(result.data());
		}

		void iterate_forward()
		{
			cur_iters++;
			generate_tree();
		}

		void iterate_backwards()
		{
			cur_iters--;
			generate_tree();
		}

		void step_line_width(float step)
		{
			line_width += step;
		}

		void randomize_color()
		{
			float r = ((double)rand() / (RAND_MAX) /  2.0f + 0.5);
			float g = ((double)rand() / (RAND_MAX) / 2.0f + 0.5);
			float b = ((double)rand() / (RAND_MAX) / 2.0f + 0.5);

			start_color = vec3(r, g, b);
			regenerate();
		}

		// Render should be called every frame to draw the tree
		void render(mat4t &cameraToWorld)
		{
			mat4t modelToProjection = mat4t::build_projection_matrix(transform, cameraToWorld);

			shader.render(modelToProjection, 0);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)vertices.data());
			glEnableVertexAttribArray(attribute_pos);
			
			glVertexAttribPointer(attribute_color, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(vertices.data() + 3));
			glEnableVertexAttribArray(attribute_color);

			glLineWidth(line_width);
			glDrawArrays(GL_LINES, 0, nodes.size() << 1 );
		}
		
		// Returns a short summary to display on the screen
		void get_tree_info(char *dst_buffer)
		{
			sprintf(dst_buffer, "File: %s\nIterations: %d\nAngle: %.2f\n", file_, cur_iters, angle);
		}

		void parse_text_tree(string axiom)
		{
			vec3 end_pos;

			// Reset (clear) everything
			nodes.reset();
			angle_state.reset();
			cur_angle = 0.0f;
			cur_angle_y = 0.0f;


			cur_node.transform = transform;
			cur_node.end = vec3(0, 0, 0);
			cur_node.color = start_color;
			l_node node;

			int size = axiom.size();

			for (int j = 0; j < size; j++)
			{
				switch (axiom[j])
				{

				case 'L':
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

					node.color = vec3(0.0f, 255.0f, 0.0f);


					t.invertQuick(inverseTransform);
					vec4 localUp = vec4(0, 1, 0, 0) * inverseTransform;
					//vec3 localUp2 = t.y().normalize();

					node.end = (t.row(3).xyz() + (localUp * branch_len ));

					nodes.push_back(node);

					cur_node = node;
				}
				break;

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

					float r = ((double)rand() / (RAND_MAX)) + 0.1f;
					float g = ((double)rand() / (RAND_MAX)) + 0.1f;
					float b = ((double)rand() / (RAND_MAX)) + 0.1f;

					node.color = cur_node.color;// +vec3(r, g, b);

					
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
					states.push_back(cur_node);
					angle_state.push_back(cur_angle);
					angle_state_y.push_back(cur_angle_y);
				}
				break;

				case ']':
				{
					// Load the saved state
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
}