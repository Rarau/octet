
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
		GLuint normalMap_handle;

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


		void render(tilemap_shader &shader, mat4t& modelToWorld, mat4t &cameraToWorld, vec3 lightPos)
		{

			GLuint VB, IBO;
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			// set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture_handle);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, normalMap_handle);
			// use "old skool" rendering
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			shader.render(modelToProjection, 0, 1, lightPos, modelToWorld);
			float * vert_array = verts.data();
			unsigned int * ind_array = indices.data();
			float * uv_array = uvs.data();
			/*
			glGenBuffers(1, &VB);
			glBindBuffer(GL_ARRAY_BUFFER, VB);
			glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), vert_array, GL_STATIC_DRAW);
				*/

			glGenBuffers(1, &IBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), ind_array, GL_STATIC_DRAW);
			
			//glCullFace(GL_BACK);
			//ts = get_tileset(tilesets, tile_gid);
			// attribute_pos (=0) is position of each corner
			// each corner has 3 floats (x, y, z)
			// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vert_array);
			glEnableVertexAttribArray(attribute_pos);


			// attribute_uv is position in the texture of each corner
			// each corner (vertex) has 2 floats (x, y)
			// there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uv_array);
			glEnableVertexAttribArray(attribute_uv);

			//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

			//glDrawArrays(GL_TRIANGLE_FAN, 0, 60);

			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


			//glDisableVertexAttribArray(attribute_pos);
			//glDisableVertexAttribArray(attribute_uv);

		}
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
		tileLayer(string name, int w, int h, const char *raw_csv_data, dynarray<tileset> *tilesets, float z = 0.0f)
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
						ts->verts.push_back(z);

						ts->verts.push_back((i + 1) * tileset::TILE_SIZE);
						ts->verts.push_back(j * tileset::TILE_SIZE);
						ts->verts.push_back(z);

						ts->verts.push_back(i * tileset::TILE_SIZE);
						ts->verts.push_back((j + 1) * tileset::TILE_SIZE);
						ts->verts.push_back(z);

						ts->verts.push_back((i + 1) * tileset::TILE_SIZE);
						ts->verts.push_back((j + 1) * tileset::TILE_SIZE);
						ts->verts.push_back(z);

						tri_index = (ts->verts.size()) / 3;
						ts->indices.push_back(tri_index);
						ts->indices.push_back(tri_index + 2);
						ts->indices.push_back(tri_index + 1);

						ts->indices.push_back(tri_index + 1);
						ts->indices.push_back(tri_index + 3);
						ts->indices.push_back(tri_index + 2);

						tri_index += 4;

						get_uvs_for_tile(tile_gid - ts->first_gid, ts->num_rows, ts->num_cols, &(ts->uvs), uv_offset);
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


		void get_uvs_for_tile(int tile_index, int num_rows, int num_cols, dynarray<float>* dst_array, int offset)
		{
			if (num_cols == 0 && num_rows == 0)
				return;

			int tile_column = tile_index % num_cols;
			int tile_row = tile_index / num_cols;

			//0,0
			dst_array->push_back((float)tile_column / (float)num_cols);
			dst_array->push_back(1.0f - ((float)tile_row / (float)num_cols));

			//1,0
			dst_array->push_back((float)(tile_column + 1) / (float)num_cols);
			dst_array->push_back(1.0f - ((float)tile_row / (float)num_cols));

			//0,1
			dst_array->push_back((float)tile_column / (float)num_cols);
			dst_array->push_back(1.0f - ((float)(tile_row + 1) / (float)num_cols));

			//1,1
			dst_array->push_back((float)(tile_column + 1) / (float)num_cols);
			dst_array->push_back(1.0f - ((float)(tile_row + 1) / (float)num_cols));

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
		//dynarray<sprite> sprites;
		tilemap_shader tilemap_shader_;

		// Path (url) starting from octet folder
		bool load_xml(const char *url) {
			// set up the shader
			tilemap_shader_.init();

			modelToWorld.loadIdentity();
			modelToWorld.scale(1.0f, -1.0f, 1.0f);

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
					GLuint ns = resource_dict::get_texture_handle(GL_RGBA, "assets/2D_tiles/Castle2-normal.gif");


					tileset tileset(mapHeight, mapWidth);
					tileset.texture_handle = ts;
					tileset.normalMap_handle = ns;
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

		void render(mat4t &cameraToWorld, vec3 lightPos)
		{
			for (int i = 0; i < tilesets.size(); i++)
			{
				tilesets[i].render(tilemap_shader_, modelToWorld, cameraToWorld, lightPos);
			}
		}

		void load_layers(TiXmlElement *rootElement)
		{
			float z = -0.010f;
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
						tileLayer *layer = new tileLayer(elem->Attribute("name"), mapWidth, mapHeight, rawCsvData, &tilesets, z);
						printf("layer %s \n", elem->Attribute("name"));

						
						layers.push_back(*layer);
						z += 0.010f;
					}
					
					//printf(rawCsvData);
				}
			}
		}

		void dump_tilesets()
		{
			for (unsigned int i = 0; i < tilesets.size(); i++)
			{
				printf("%s\n", tilesets[i].name);
			}
		}

	};

	float tileset::TILE_SIZE = 0.24f;


}