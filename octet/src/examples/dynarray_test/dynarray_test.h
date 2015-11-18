////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {

	class l_node
	{
	public:
		mat4t transform;
		vec3 color;

		l_node()
		{
			//printf("COPY \n");
		}
		l_node(l_node& node)
		{
			printf("COPY \n");
		}
	};
  /// Scene containing a box with octet.
  class dynarray_test : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

	dynarray<l_node> nodes;

  public:
    /// this is called when we construct the class before everything is initialised.
    dynarray_test(int argc, char **argv) : app(argc, argv) {
    }
	
    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      material *red = new material(vec4(1, 0, 0, 1));
      mesh_box *box = new mesh_box(vec3(4));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));


	  l_node my_node;
	  for (int i = 0; i < 100; i++)
	  {
		  my_node.color = vec3(i, i, i);
		  my_node.transform.translate(vec3(i, i, i));

		  nodes.push_back(my_node);
	  }

	  printf("Nodes: \n");

	 /* for (int i = 0; i < 100; i++)
	  {
		  printf("----\n");
		  printf("Color %f\n", nodes[i].color.x());
		  printf("X %f\n", nodes[i].transform.row(3).x());

	  }*/
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {
      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);

      // tumble the box  (there is only one mesh instance)
      scene_node *node = app_scene->get_mesh_instance(0)->get_node();
      node->rotate(1, vec3(1, 0, 0));
      node->rotate(1, vec3(0, 1, 0));
    }
  };
}
