
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

		tileset(){};

		/*tileset(const tileset &copy) {
			first_gid = copy.first_gid;
			tileW = copy.tileW;

			printf("Copy constructor called\n");
		}*/
	};
	
	class tileLayer {
	public:
		string name;
	
		dynarray<unsigned int> data;
		int mapH, mapW;

	public:
		tileLayer(int w, int h)
		{
			mapH = h;
			mapW = w;
		}

		tileLayer(){ mapH = mapW = 0; }

		~tileLayer()
		{
			printf("Destructor tileLayer\n");
		}
		
		unsigned int get_tile_at(int x, int y)
		{
			return (x < mapW) && (x >= 0) && (y < mapH) && (y >= 0) ?
				data[y * mapW + x] :
				0;
		}
	};
	
	class tmxMap {
		// Bits on the far end of the 32-bit global tile ID are used for tile flags
		const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
		const unsigned FLIPPED_VERTICALLY_FLAG = 0x40000000;
		const unsigned FLIPPED_DIAGONALLY_FLAG = 0x20000000;

		dynarray<int> tiles;
		dynarray<tileset> tilesets;
		dynarray<tileLayer> layers;


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

			load_sprites();
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

					tileset tileset;
					tileset.texture_handle = ts;
					tileset.name = string(elem->Attribute("name"));
					tileset.first_gid = atoi(elem->Attribute("firstgid"));
					tileset.tileH = atoi(elem->Attribute("tileheight"));
					tileset.tileW = atoi(elem->Attribute("tilewidth"));
					tileset.imageH = atoi(elem->FirstChildElement()->Attribute("height"));
					tileset.imageW = atoi(elem->FirstChildElement()->Attribute("width"));

					tilesets.push_back(tileset);
				}
			}
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
						tileLayer *layer = new tileLayer(mapWidth, mapHeight);
						printf("layer\n");

						const char* rawCsvData = elem->FirstChildElement()->GetText();
						atoiv_csv(layer->data, rawCsvData);

						
						layers.push_back(*layer);
					}
					//printf(rawCsvData);
				}
			}
		}

		// create a sprite for each tile, might be an overkill...
		void load_sprites()
		{
			for each (tileLayer layer in layers)
			{
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
							printf("%d - %d, %d - %s\n", tile_gid, i, j, ts->name);

							test_sprite.init(ts->texture_handle, i * tileset::TILE_SIZE, -j * tileset::TILE_SIZE, tileset::TILE_SIZE, tileset::TILE_SIZE, tile_gid - ts->first_gid, ts->tileW, ts->tileH, ts->imageW, ts->imageH);
							

							float scale_x = flipped_horizontally || flipped_diagonally ? -1.0f : 1.0f;
							float scale_y = flipped_vertically || flipped_diagonally ? -1.0f : 1.0f;
							test_sprite.modelToWorld.scale(scale_x, scale_y, 1.0f);

							sprites.push_back(test_sprite);
						}
					}
				}
			}
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


	};
	float tileset::TILE_SIZE = 0.2f;


}