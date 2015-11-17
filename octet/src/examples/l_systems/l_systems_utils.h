
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
		mat4t transform;
		tree_shader shader;

		string starting_axiom;
		dynarray<string> rules;

		string current_axiom;

		int cur_iters;

		dynarray<l_node> states;
		dynarray<l_node> nodes;
		dynarray<float> vertices;

		l_node cur_node;

		float angle;
		float branch_len = 0.1f;

	public:
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

				
				vertices.push_back(nodes[j].end.x());
				vertices.push_back(nodes[j].end.y());
				vertices.push_back(nodes[j].end.z());
				
				//j++;
				/*
				vertices.push_back(nodes[j].transform.row(3).x());
				vertices.push_back(nodes[j].transform.row(3).y());
				vertices.push_back(nodes[j].transform.row(3).z());
				*/
			}
			
			printf("Current_axiom %s\n", current_axiom);
		}

		static string apply_rules_to_axiom(string axiom, dynarray<string>& rules) {
			dynarray<char> result;

			// Go through each character of the axiom
			int axiom_size = axiom.size();
			for (int i = 0; i < axiom_size; i++) {
				char current = axiom[i];

				// Find the rule to apply to the current character
				int j = 0;
				int rules_size = rules.size();
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
				if (j == rules.size()) {
					result.push_back(current);
				}
			}

			// Add a null terminating character to the end of our result string
			result.push_back(0x00);
			return string(result.data());
		}

		void iterate_forward()
		{

		}

		void iterate_backwards()
		{

		}

		void render(mat4t &cameraToWorld)
		{
			mat4t modelToProjection = mat4t::build_projection_matrix(transform, cameraToWorld);
			shader.render(modelToProjection, 0);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices.data());
			glEnableVertexAttribArray(attribute_pos);

			glLineWidth(1.0f);
			glDrawArrays(GL_POINTS, 0, nodes.size());
		}
		
		void generate_nodes_from_axiom(string axiom)
		{
			vec3 end_pos;

			// Reset (clear) everything
			nodes.reset();

			l_node node;
			cur_node.transform.loadIdentity();

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
					node.transform = cur_node.transform;

					node.color = vec3(0.0f, 250.0f, 0.0f);
					node.transform.translate(node.transform.y() * branch_len);
					node.end = (cur_node.transform.y() * branch_len + cur_node.transform.row(3));

					nodes.push_back(node);
					cur_node = node;

					break;

				case '[':
					// Save the current state
					states.push_back(cur_node);
					
					break;

				case ']':
					// Load the saved state
					cur_node = states.back();
					states.pop_back();

					break;

				case '+':
					// Turn right
					/*
					mat4t mat;
					mat.loadIdentity();
					mat.rotateZ(angle);
					
					rotate_vec2(cur_dir, -angle);
					*/
					cur_node.transform.rotateZ(-angle);
					break;

				case '-':
					// Turn left 
					//rotate_vec2(cur_dir, angle);
					cur_node.transform.rotateZ(angle);

					break;
				}
			}
		}
	};

	class l_system_utils
	{
	public:

		static string iterate(string axiom, dynarray<string>& rules) {
			dynarray<char> result;

			// Go through each character of the axiom
			int axiom_size = axiom.size();
			for (int i = 0; i < axiom_size; i++) {
				char current = axiom[i];

				// Find the rule to apply to the current character
				int j = 0;
				int rules_size = rules.size();
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
				if (j == rules.size()) {
					result.push_back(current);
				}
			}

			// Add a null terminating character to the end of our result string
			result.push_back(0x00);
			return string(result.data());
		}
	};
}