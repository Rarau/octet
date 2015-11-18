
namespace octet {

	class grid  {
	private:
		grid_shader shader;

		dynarray<float> vertices;
		mat4t transform;
		vec3 color;
	public:
		void init(int num_w, int num_h, float tile_size)
		{
			shader.init();
			// Horizontal vertices
			for (int i = -num_w; i < num_w; i++)
			{
				vertices.push_back(i * tile_size);
				vertices.push_back(0.0f);
				vertices.push_back(tile_size * num_h);

				vertices.push_back(i * tile_size);
				vertices.push_back(0.0f);
				vertices.push_back(-tile_size * num_h);
			}

			// Vertical vertices
			for (int i = -num_h; i < num_h; i++)
			{
				vertices.push_back(tile_size * num_w);
				vertices.push_back(0.0f);
				vertices.push_back(i * tile_size);

				vertices.push_back(-tile_size * num_w);
				vertices.push_back(0.0f);
				vertices.push_back(i * tile_size);
			}
		}

		void render(mat4t &cameraToWorld)
		{
			mat4t modelToProjection = mat4t::build_projection_matrix(transform, cameraToWorld);
			//glBindAttribLocation(shader.program(), attribute_color, "color");
			//glBindAttribLocation(shader.program(), attribute_color, "color");

			shader.render(modelToProjection, 0);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices.data());
			glEnableVertexAttribArray(attribute_pos);

			//glVertexAttribPointer(attribute_color, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(vertices.data() + 3));
			//glEnableVertexAttribArray(attribute_color);

			glLineWidth(1.0f);
			glDrawArrays(GL_LINES, 0, vertices.size() / 3);
		}
	};
}