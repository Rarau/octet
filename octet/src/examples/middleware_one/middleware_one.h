////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
namespace octet {
	/// Scene containing a box with octet.
	class middleware_one : public app {
		// a texture for our text
		GLuint font_texture;
		ref<scene_node> sphere_node;
		// information for our text
		bitmap_font font;
		// scene for drawing box
		ref<visual_scene> app_scene;

		// helper for drawing text
		ref<text_overlay> overlay;

		ref<material> custom_mat;


		// text mesh object for overlay.
		ref<mesh_text> text;
		camera_instance* cam;

		ref<image> dif_texture;
		ref<image> light_ramp;

	public:
		/// this is called when we construct the class before everything is initialised.
		middleware_one(int argc, char **argv) : app(argc, argv) {
		}

		/// this is called once OpenGL is initialized
		void app_init() {
			app_scene = new visual_scene();
			app_scene->create_default_camera_and_lights();
			cam = app_scene->get_camera_instance(0);

			param_shader *shader = new param_shader("shaders/default.vs", "shaders/toon.fs");
			custom_mat = new material(vec4(1, 1, 1, 1), shader);


			dif_texture = new image("assets/ground.jpg");
			light_ramp = new image("assets/ramp.jpg");

			custom_mat->add_sampler(0, app_utils::get_atom("diffuse_sampler"), dif_texture, new sampler());
			custom_mat->add_sampler(1, app_utils::get_atom("light_ramp"), light_ramp, new sampler());



			// create the overlay
			overlay = new text_overlay();

			// get the default font.
			bitmap_font *font = overlay->get_default_font();

			// create a box containing text (in pixels)
			aabb bb(vec3(64.5f, -200.0f, 0.0f), vec3(256, 64, 0));
			text = new mesh_text(font, "", &bb);

			// add the mesh to the overlay.
			overlay->add_mesh_text(text);

			mesh_sphere *box = new mesh_sphere(vec3(10.0f), 3.0f);
			sphere_node = new scene_node();
			app_scene->add_child(sphere_node);


			mat4t box_location;
			box_location.translate(vec3(0, 0, 0));
			box_location.rotateX90();

			mat4t ground_location;
			ground_location.translate(vec3(0, -10.0f, 0));

			mesh_box * ground = new mesh_box(vec3(10.0f, 0.1f, 10.0f));

			mesh_instance* box_instance = app_scene->add_shape(box_location, box, custom_mat, true);


			box_instance->get_node()->get_rigid_body()->setLinearFactor(btVector3(0, 0.5f, 0));
			app_scene->add_shape(ground_location, ground, custom_mat, false);

			/*
			btVector3 cam_pos = btVector3(cam_node.get_position().x(), cam_node.get_position().y(), cam_node.get_position().z());
			btVector3 cam_dir = btVector3(cam_node.get_z().x(), cam_node.get_z().y(), cam_node.get_z().z());

			//app_scene->add_mesh_instance(new mesh_instance(node, box, red));
			btCollisionWorld::ClosestRayResultCallback RayCallback(cam_pos, cam_pos + cam_dir * 10.0f);*/
			//btCollisionWorld::rayTest(cam_pos, cam_pos + cam_dir * 10.0f, RayCallback);

		}

		/// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {
			int vx = 0, vy = 0;
			get_viewport_size(vx, vy);
			app_scene->begin_render(vx, vy);

			// update matrices. assume 30 fps.
			app_scene->update(1.0f / 30);

			// draw the scene
			app_scene->render((float)vx / vy);

			// tumble the box  (there is only one mesh instance)
			scene_node *node = app_scene->get_mesh_instance(0)->get_node();
			node->rotate(1, vec3(1, 0, 0));
			node->rotate(1, vec3(0, 1, 0));

			char score_text[32];
			int mouse_x, mouse_y;
			get_mouse_pos(mouse_x, mouse_y);

			text->clear();
			ray r = cam->get_ray((float)(mouse_x), (float)(mouse_y));

			if (is_key_going_down(' '))
			{
				printf("Ray\n");
				btVector3 start = btVector3(r.get_start().x(), r.get_start().y(), r.get_start().z());
				btVector3 end = btVector3(r.get_end().x(), r.get_end().y(), r.get_end().z());

				btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
				visual_scene::cast_result cast_result;

				app_scene->cast_ray(cast_result, r);

				if (cast_result.mi != NULL) {
					printf("HIT\n");
				}

			}

			if (is_key_down(key_down))
			{
				sphere_node->apply_central_force(vec3(0, 0, -10));
			}

			text->format("ray start: %f, %f, %f - end: %f, %f, %f\n ", r.get_start().x(), r.get_start().y(), r.get_start().z(), r.get_end().x(), r.get_end().y(), r.get_end().z());
			app_scene->set_render_debug_lines(true);
			app_scene->add_debug_line(r.get_start(), r.get_end());

			//overlay->get_node()->translate(vec3(0.10f, 0.0f, 0.0f));
			// convert it to a mesh.
			text->update();

			// draw the text overlay
			overlay->render(vx, vy);

		}



	};
}
