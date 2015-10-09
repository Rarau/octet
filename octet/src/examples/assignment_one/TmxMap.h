
namespace octet {


	//typedef dynarray<int> tileLayer;

	class tileset {

	public:
		static float TILE_SIZE;
		int first_gid;
		int tileW, tileH;
		string name;
		int imageW, imageH;
		GLuint texture_handle;

		int num_cols;
		int num_rows;

		// For each tile we'll have 8 floats (2 for each corner)
		dynarray<float> uvs;
		dynarray<float> verts;
		dynarray<unsigned int> indices;


		tileset(){}
		tileset(int mapH, int mapW){
		
			/*
			// For each tile we'll have 8 floats (2 for each corner)
			uvs.reserve(mapH * mapW * 8);
			// 3 floats for each vertex (x, y, z) and 4 verts for each tile
			verts.reserve(3 * mapH * mapW * 4);
			// 2 triangles for each tile
			indices.reserve(6 * mapH * mapW);
			*/
		};


		void render(mat4t& modelToWorld, mat4t &cameraToWorld)
		{
			GLuint IBO;
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			// set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture_handle);

			///shader.render(modelToProjection, 0);
			
			glGenBuffers(1, &IBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size(), indices.data(), GL_STATIC_DRAW);
			// attribute_pos (=0) is position of each corner
			// each corner has 3 floats (x, y, z)
			// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
			//glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)verts.data());
			glEnableVertexAttribArray(attribute_pos);
			//ts = get_tileset(tilesets, tile_gid);

			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

			// attribute_uv is position in the texture of each corner
			// each corner (vertex) has 2 floats (x, y)
			// there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs.data());
			glEnableVertexAttribArray(attribute_uv);


		}

		/*tileset(const tileset &copy) {
			first_gid = copy.first_gid;
			tileW = copy.tileW;

			printf("Copy constructor called\n");
		}*/
	};
	
	class tileLayer {

	public:
		string name;

		struct uv {
			float u;
			float v;
		};

		dynarray<unsigned int> data;
		
		dynarray<tileset> *tilesets_ref;

		int mapHeight, mapWidth;

	public:
		tileLayer(int w, int h, const char *raw_csv_data, dynarray<tileset> *tilesets)
		{
			mapHeight = h;
			mapWidth = w;
			tilesets_ref = tilesets;

			atoiv_csv(data, raw_csv_data);

			int tri_index = 0, uv_offset = 0;
			for (int j = 0; j < mapHeight; j++)
			{
				for (int i = 0; i < mapWidth; i++)
				{
					unsigned int tile_gid = get_tile_at(i, j);
					if (tile_gid > 0)
					{
						tileset* ts;
						ts = get_tileset(*tilesets_ref, tile_gid);

						ts->verts.push_back(i * tileset::TILE_SIZE);
						ts->verts.push_back(j * tileset::TILE_SIZE);
						ts->verts.push_back(0.0f);

						ts->verts.push_back((i + 1) * tileset::TILE_SIZE);
						ts->verts.push_back(j * tileset::TILE_SIZE);
						ts->verts.push_back(0.0f);

						ts->verts.push_back(i * tileset::TILE_SIZE);
						ts->verts.push_back((j + 1) * tileset::TILE_SIZE);
						ts->verts.push_back(0.0f);

						ts->verts.push_back((i + 1) * tileset::TILE_SIZE);
						ts->verts.push_back((j + 1) * tileset::TILE_SIZE);
						ts->verts.push_back(0.0f);


						ts->indices.push_back(tri_index);
						ts->indices.push_back(tri_index + 1);
						ts->indices.push_back(tri_index + 2);

						ts->indices.push_back(tri_index + 1);
						ts->indices.push_back(tri_index + 3);
						ts->indices.push_back(tri_index + 2);

						tri_index += 4;

						get_uvs_for_tile(tile_gid - ts->first_gid, ts->num_rows, ts->num_cols, ts->uvs, uv_offset);
						uv_offset += 8;
					}
				}
			}
		}

		tileLayer(){ mapHeight = mapWidth = 0; }

		~tileLayer()
		{
			printf("Destructor tileLayer\n");
		}
		
		unsigned int get_tile_at(int x, int y)
		{
			return (x < mapWidth) && (x >= 0) && (y < mapHeight) && (y >= 0) ?
				data[y * mapWidth + x] :
				0;
		}


		// convert an ascii sequence of comma separated integers like "1,3,9,12,34" to an array of integers
		void atoiv_csv(dynarray<unsigned int> &values, const char *src) {
			//values.resize(0);
			if (!src) return;

			while (*src > 0 && *src <= ',') ++src;
			while (*src != 0) {
				unsigned int whole = 0;//, msign = 1;
				//if (*src == '-') { msign = -1; src++; }
				while (*src >= '0' && *src <= '9') whole = whole * 10 + (*src++ - '0');
				values.push_back(whole);// *msign);
				while (*src > 0 && *src <= ',') ++src;
			}
		}


		void get_uvs_for_tile(int tile_index, int num_rows, int num_cols, dynarray<float> dst_array, int offset)
		{
			if (num_cols == 0 && num_rows == 0)
				return;

			int tile_column = tile_index % num_cols;
			int tile_row = tile_index / num_cols;


			dst_array.push_back((float)tile_column / (float)num_cols);
			dst_array.push_back((float)tile_row / (float)num_cols);
			dst_array.push_back((float)(tile_column + 1) / (float)num_cols);
			dst_array.push_back((float)tile_row / (float)num_cols);
			dst_array.push_back((float)tile_column / (float)num_cols);
			dst_array.push_back((float)(tile_row + 1) / (float)num_cols);
			dst_array.push_back((float)(tile_column + 1) / (float)num_cols);
			dst_array.push_back((float)(tile_row + 1) / (float)num_cols);

			/*
			dst_array[0 + offset] = (float)tile_column / (float)num_cols;
			dst_array[1 + offset] = (float)tile_row / (float)num_cols;

			dst_array[2 + offset] = (float)(tile_column + 1) / (float)num_cols;
			dst_array[3 + offset] = (float)tile_row / (float)num_cols;

			dst_array[4 + offset] = (float)tile_column / (float)num_cols;
			dst_array[5 + offset] = (float)(tile_row + 1) / (float)num_cols;

			dst_array[6 + offset] = (float)(tile_column + 1) / (float)num_cols;
			dst_array[7 + offset] = (float)(tile_row + 1) / (float)num_cols;
			*/
		}

		tileset* get_tileset(dynarray<tileset>& tilesets, int tile_gid) {
			tileset* ts = NULL;
			for (unsigned int i = 0; i < tilesets.size(); i++)
			{
				if (tilesets[i].first_gid <= tile_gid) {
					if (ts == NULL || tilesets[i].first_gid > ts->first_gid) {
						ts = &tilesets[i];
					}
				}
			}
			return ts;
		}
	};



	
	class tmxMap {
		// Bits on the far end of the 32-bit global tile ID are used for tile flags
		const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
		const unsigned FLIPPED_VERTICALLY_FLAG = 0x40000000;
		const unsigned FLIPPED_DIAGONALLY_FLAG = 0x20000000;

		mat4t modelToWorld;

		dynarray<int> tiles;
		dynarray<tileset> tilesets;
		dynarray<tileLayer> layers;


		dynarray<float> vertices;
		dynarray<int> indices;

		TiXmlDocument doc;
		string doc_path;
		string tmxFileName = "Dungeon.tmx";
		string myPath;

		int mapHeight, mapWidth;

	public:
		dynarray<sprite> sprites;
		texture_shader texture_shader_;

		// Path (url) starting from octet folder
		bool load_xml(const char *url) {
			// set up the shader
			texture_shader_.init();

			modelToWorld.loadIdentity();

			doc_path = url;
			doc_path.truncate(doc_path.filename_pos());
			const char *path = app_utils::get_path(url);
			char buf[256];
			getcwd(buf, sizeof(buf));
			doc.LoadFile(path);

			TiXmlElement *top = doc.RootElement();
			if (!top) {
				printf("file %s not found\n", path);
				return false;
			}

			if (strcmp(top->Value(), "map")) {
				printf("warning: not a tmx file");
				return false;
			}

			mapHeight = atoi(top->Attribute("height"));
			mapWidth = atoi(top->Attribute("width"));

			load_tilesets(top);
			load_layers(top);

//			load_sprites();
			return true;
		}

		void generate_mesh_data()
		{
			// 3 floats for each vertex (x, y, z) and 4 verts for each tile
			vertices.reserve(3 * mapHeight * mapWidth * 4);

			for (int j = 0; j < mapHeight; j++)
			{
				for (int i = 0; i < mapWidth; i++)
				{
					vertices.push_back(i * tileset::TILE_SIZE);
					vertices.push_back(j * tileset::TILE_SIZE);
					vertices.push_back(0.0f);

					vertices.push_back((i + 1) * tileset::TILE_SIZE);
					vertices.push_back(j * tileset::TILE_SIZE);
					vertices.push_back(0.0f);

					vertices.push_back(i * tileset::TILE_SIZE);
					vertices.push_back((j + 1) * tileset::TILE_SIZE);
					vertices.push_back(0.0f);

					vertices.push_back((i + 1) * tileset::TILE_SIZE);
					vertices.push_back((j + 1) * tileset::TILE_SIZE);
					vertices.push_back(0.0f);
				}
			}

			// 2 triangles for each tile
			indices.reserve(6 * mapHeight * mapWidth);

			for (int j = 0; j < mapHeight * mapWidth; j++)
			{
				indices.push_back(j);
				indices.push_back(j + 1);
				indices.push_back(j + 2);

				indices.push_back(j + 1);
				indices.push_back(j + 3);
				indices.push_back(j + 2);
			}
		}

		void load_tilesets(TiXmlElement *rootElement)
		{

			for (TiXmlElement *elem = rootElement->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
			{
				string elemName = elem->Value();

				const char* attr;
				if (elemName == "tileset")
				{
					//printf("%s\n", elemName);
					const char* tilesetPath = elem->FirstChildElement()->Attribute("source");
					attr = elem->Attribute("name");
					myPath.format("%s/%s", doc_path, tilesetPath);
					//printf("tspath %s\n", myPath);

					//if (attr != NULL)
					// printf("name %s\n", tilesetPath);

					GLuint ts = resource_dict::get_texture_handle(GL_RGBA, myPath);

					tileset tileset(mapHeight, mapWidth);
					tileset.texture_handle = ts;
					tileset.name = string(elem->Attribute("name"));
					tileset.first_gid = atoi(elem->Attribute("firstgid"));
					tileset.tileH = atoi(elem->Attribute("tileheight"));
					tileset.tileW = atoi(elem->Attribute("tilewidth"));
					tileset.imageH = atoi(elem->FirstChildElement()->Attribute("height"));
					tileset.imageW = atoi(elem->FirstChildElement()->Attribute("width"));
					tileset.num_cols = tileset.imageW / tileset.tileW;
					tileset.num_rows = tileset.imageH / tileset.tileH;

					tilesets.push_back(tileset);
				}
			}
		}

		void render(mat4t &cameraToWorld)
		{
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

		}

		void load_layers(TiXmlElement *rootElement)
		{
			for (TiXmlElement *elem = rootElement->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
			{
				string elemName = elem->Value();

				if (elemName == "layer")
				{
					string encoding = elem->FirstChildElement()->Attribute("encoding");
					printf("%s\n", encoding);
					if (encoding == "csv")
					{


						const char* rawCsvData = elem->FirstChildElement()->GetText();
						tileLayer *layer = new tileLayer(mapWidth, mapHeight, rawCsvData, &tilesets);
						printf("layer\n");

						
						layers.push_back(*layer);
					}
					//printf(rawCsvData);
				}
			}
		}

		// create a sprite for each tile, might be an overkill...
		/*void load_sprites()
		{
			for each (tileLayer layer in layers)
			{
				//float *uvs_ptr = layer.uvs.data();
				int uv_offset = 0;
				for (int i = 0; i < mapWidth; i++)
				{
					for (int j = 0; j < mapHeight; j++)
					{
						unsigned int tile_gid = layer.get_tile_at(i, j);
						if (tile_gid > 0)
						{
							// Read out the flags
							bool flipped_horizontally = (tile_gid & FLIPPED_HORIZONTALLY_FLAG);
							bool flipped_vertically = (tile_gid & FLIPPED_VERTICALLY_FLAG);
							bool flipped_diagonally = (tile_gid & FLIPPED_DIAGONALLY_FLAG);

							// Clear the flags
							tile_gid &= ~(FLIPPED_HORIZONTALLY_FLAG |
								FLIPPED_VERTICALLY_FLAG |
								FLIPPED_DIAGONALLY_FLAG);

							sprite test_sprite;
							tileset* ts;
							ts = get_tileset(tilesets, tile_gid);
							//printf("%d - %d, %d - %s\n", tile_gid, i, j, ts->name);

							test_sprite.init(ts->texture_handle, i * tileset::TILE_SIZE, -j * tileset::TILE_SIZE, tileset::TILE_SIZE, tileset::TILE_SIZE, tile_gid - ts->first_gid, ts->tileW, ts->tileH, ts->imageW, ts->imageH);
							get_uvs_for_tile(tile_gid - ts->first_gid, ts->num_rows, ts->num_cols, layer.uvs, uv_offset);
							uv_offset += 8;

							
							float scale_x = flipped_horizontally || flipped_diagonally ? -1.0f : 1.0f;
							float scale_y = flipped_vertically || flipped_diagonally ? -1.0f : 1.0f;
							test_sprite.modelToWorld.scale(scale_x, scale_y, 1.0f);

							sprites.push_back(test_sprite);
						}
					}
				}
			}
		}
		*/


		void dump_tilesets()
		{
			for (unsigned int i = 0; i < tilesets.size(); i++)
			{
				printf("%s\n", tilesets[i].name);
			}
		}
		
		void render(mat4t camera)
		{
			for each (sprite s in sprites)
			{
				s.render(texture_shader_, camera);
			}
		}

	};

	float tileset::TILE_SIZE = 0.24f;


}