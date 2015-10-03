////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include "sprite.h"
#include "xmldump.h"
#include <iostream>
#include <fstream>
#include <string>
namespace octet {

	class tileset {
	public:
		int first_gid;
		int tileW, tileH;
		string name;
		string image_path;
		int imageW, imageH;
		GLuint texture_handle;

		tileset(){};
	};

	typedef dynarray<int> tileLayer;
  /// Scene containing a box with octet.
  class assignment_one : public app {

	// Matrix to transform points in our camera space to the world.
	// This lets us move our camera
	mat4t cameraToWorld;

	// shader to draw a textured triangle
	texture_shader texture_shader_;
	dynarray<sprite> sprites;
	dynarray<tileset> tilesets;
	dynarray<tileLayer> layers;

	vec2 joystick_axis;
  public:
    /// this is called when we construct the class before everything is initialised.
    assignment_one(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
		
	  // set up the shader
	  texture_shader_.init();
	  string tmxPath = "C:/Users/Nevak/Documents/GitHub/octet/octet/assets/2D_tiles/Examples";
	  string tmxFileName = "Dungeon.tmx";
	  string myPath;
	  TiXmlDocument doc(myPath.format("%s/%s", tmxPath, tmxFileName));
	  printf(myPath);
	  if (doc.LoadFile())
	  {
		  for (TiXmlElement* elem = doc.RootElement()->FirstChildElement(); elem != NULL; elem = elem->NextSiblingElement())
		  {
			  string elemName = elem->Value();

			  const char* attr;
			  if (elemName == "tileset")
			  {
				  //printf("%s\n", elemName);
				  const char* tilesetPath = elem->FirstChildElement()->Attribute("source");
				  attr = elem->Attribute("name");
				  myPath.format("%s/%s", tmxPath, tilesetPath);
				  //printf("tspath %s\n", myPath);

				  //if (attr != NULL)
					 // printf("name %s\n", tilesetPath);

				  GLuint ts = resource_dict::get_texture_handle(GL_RGBA, myPath);

				  tileset tileset;
				  tileset.texture_handle = ts;
				  tileset.first_gid = atoi(elem->Attribute("firstgid"));
				  tileset.tileH = atoi(elem->Attribute("tileheight"));
				  tileset.tileW = atoi(elem->Attribute("tilewidth"));
				  tileset.imageH = atoi(elem->FirstChildElement()->Attribute("height"));
				  tileset.imageW = atoi(elem->FirstChildElement()->Attribute("width"));

				  tilesets.push_back(tileset);
			  }
			  else if (elemName == "tileset")
			  {
				  printf("layer");
			  }
		  }
	  }
	  else
		  printf("error loading tmx\n");

	  int i = 0;
	  for each (tileset ts in tilesets)
	  {
		  //printf("gid: %d\n", ts.first_gid);
		  //ts.init(ship, (0.25f * i) * tileScale, -(0.25f * k) * tileScale, 0.25f * tileScale, 0.25f * tileScale, tileIndex, 16, 16, 320, 816);
		  sprite test_sprite;

		  test_sprite.init(ts.texture_handle, i * 0.520f, 0.0f, 0.520f, 0.520f);

		  sprites.push_back(test_sprite);
		  ++i;
	  }

	  //dump_to_stdout("C:\\Users\\Nevak\\Documents\\GitHub\\octet\\octet\\assets\\2D_tiles\\Examples\\Dungeon.tmx");
	  /*
	  std::ifstream file("C:\\\Users\\Nevak\\Documents\\GitHub\\octet\\octet\\assets\\2D_tiles\\Dungeon_Tiles.csv"); // declare file stream: http://www.cplusplus.com/reference/iostream/ifstream/
	  std::string value;

	  GLuint ship = resource_dict::get_texture_handle(GL_RGBA, "assets/2D_tiles/Wall.gif");
	  int i = 0, k = 0;
	  int mapWidth = 20;
	  float tileScale = 0.85f;
	  while (file.good())
	  {
		  getline(file, value, ','); // read a string until next comma: http://www.cplusplus.com/reference/string/getline/
		  //std::cout << std::string(value, 1, value.length() - 2); // display value removing the first and the last character from it
		  if (value[0] == 0x0d)
			  printf("line!");

		  int tileIndex = atoi(value.c_str());
		  if (tileIndex != -1)
		  {
			  //printf("%d\n", tileIndex);
			  sprite test_sprite;

			  test_sprite.init(ship, (0.25f * i) * tileScale, -(0.25f * k) * tileScale, 0.25f * tileScale, 0.25f * tileScale, tileIndex, 16, 16, 320, 816);

			  sprites.push_back(test_sprite);
		  }
		  if (i >= mapWidth - 1)
		  {
			  i = 0;
			  k++;
		  }
		  i++;

	  }
	  */
	  // set up the matrices with a camera 5 units from the origin
	  cameraToWorld.loadIdentity();
	  cameraToWorld.translate(0, 0, 3);

	  joystick_axis.x() = joystick_axis.y() = 0.0f;
    }

	void read_input() {
		joystick_axis.x() = joystick_axis.y() = 0.0f;

		if (is_key_down(key_left)) {
			joystick_axis.x() = -1.0f;
		}
		else if (is_key_down(key_right)) {
			joystick_axis.x() = 1.0f;
		}
		if (is_key_down(key_up)) {
			joystick_axis.y() = 1.0f;
		}
		else if (is_key_down(key_down)) {
			joystick_axis.y() = -1.0f;
		}
		
	}

	void simulate() {
		//test_sprite.translate(joystick_axis * 0.1f);
//		printf("x: %f   y: %f\n", test_sprite.get_pos().x(), test_sprite.get_pos().y());

	}

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);

	  // set a viewport - includes whole window area
	  glViewport(x, y, w, h);

	  // clear the background to black
	  glClearColor(0, 0, 0, 1);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	  // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
	  glDisable(GL_DEPTH_TEST);

	  // allow alpha blend (transparency when alpha channel is 0)
	  glEnable(GL_BLEND);
	  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	  read_input();
	  simulate();

	  //test_sprite.render(texture_shader_, cameraToWorld);

	  for each (sprite s in sprites)
	  {
		  s.render(texture_shader_, cameraToWorld);
	  }
    }
  };
}
