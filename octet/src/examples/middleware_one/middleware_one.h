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
		ref<mesh_instance> sphere_instance;
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

		mouse_look mouse_look_helper;

		collada_builder loader;
		TwBar *myBar;

		vec3 pos;
		int mouse_pos[2];

	public:
		/// this is called when we construct the class before everything is initialised.
		middleware_one(int argc, char **argv) : app(argc, argv) {
		}

		/// this is called once OpenGL is initialized
		void app_init() {
			TwInit(TW_OPENGL, NULL);
			myBar = TwNewBar("Camera");

			if (!loader.load_xml("assets/chest.dae")) {
				printf("failed to load file!\n");
				exit(1);
			}
			resource_dict dict;
			loader.get_resources(dict);

			// note that this call will dump the code below to log.txt
			dict.dump_assets(log(""));
			mesh *Chest_mesh = dict.get_mesh("Chest-mesh");
			//image *chest_jpg = dict.get_image("chest_jpg");

			mat4t location;
			location.translate(vec3(0, 0.0f, 0));
			location.rotateX90();



			mouse_look_helper.init(this, 200.0f / 360.0f, false);

			app_scene = new visual_scene();
			app_scene->create_default_camera_and_lights();
			cam = app_scene->get_camera_instance(0);

			param_shader *shader = new param_shader("shaders/default.vs", "shaders/toon.fs");
			custom_mat = new material(vec4(1, 1, 1, 1), shader);


			dif_texture = new image("assets/chest.gif");
			light_ramp = new image("assets/ramp_2.gif");

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


			mat4t ground_location;
			ground_location.translate(vec3(0, -10.0f, 0));

			mesh_box * ground = new mesh_box(vec3(10.0f, 0.1f, 10.0f));
			app_scene->add_shape(ground_location, ground, custom_mat, false);


			//mesh_instance* chest_instance = app_scene->add_shape(location, Chest_mesh, custom_mat, true);
			//chest_instance->get_node()->rotate(-90.0f, vec3(1, 0, 0));

			/*
			
			btVector3 cam_pos = btVector3(cam_node.get_position().x(), cam_node.get_position().y(), cam_node.get_position().z());
			btVector3 cam_dir = btVector3(cam_node.get_z().x(), cam_node.get_z().y(), cam_node.get_z().z());

			//app_scene->add_mesh_instance(new mesh_instance(node, box, red));
			btCollisionWorld::ClosestRayResultCallback RayCallback(cam_pos, cam_pos + cam_dir * 10.0f);
			
			*/
			
			//btCollisionWorld::rayTest(cam_pos, cam_pos + cam_dir * 10.0f, RayCallback);
			

			sphere_instance = add_sphere(vec3(0.0f, 4.5f, 0.0f));
			mesh_instance* sphere_instance_2 = add_sphere(vec3(0.0f, -2.5f, 0.0f));
			sphere_instance_2->get_node()->get_rigid_body()->setLinearFactor(btVector3(0, 0, 0));

			TwAddVarRO(myBar, "Cam_X", TW_TYPE_FLOAT, &(pos.x()), " label='X' ");
			TwAddVarRO(myBar, "Cam_Y", TW_TYPE_FLOAT, &(pos.y()), " label='Y' ");
			TwAddVarRO(myBar, "Cam_Z", TW_TYPE_FLOAT, &(pos.z()), " label='Z' ");
			TwAddVarRO(myBar, "Mouse_X", TW_TYPE_INT32, &(mouse_pos[0]), " label='Mouse X' ");
			TwAddVarRO(myBar, "Mouse_Y", TW_TYPE_INT32, &(mouse_pos[1]), " label='Mouse Y' ");


			add_hinge_joint(sphere_instance->get_node(), sphere_instance_2->get_node(), btVector3(0.0f, 1.5f, 0.0f), btVector3(0.0f, 1.50f, 0.0f), btVector3(0.0f, 0.0f, 1.0f));
		}

		void add_hinge_joint(scene_node* node_a, scene_node* node_b, btVector3 anchor_a, btVector3 anchor_b, btVector3 axis)
		{
			btRigidBody *rigidbody_a = node_a->get_rigid_body();
			btRigidBody *rigidbody_b = node_b->get_rigid_body();

			btHingeConstraint *hinge = new btHingeConstraint(*rigidbody_a, *rigidbody_b, anchor_a, anchor_b, axis, axis, true);
			hinge->setDbgDrawSize(btScalar(5.f));
			app_scene->add_hinge(hinge);
		}

		mesh_instance* add_sphere(vec3 pos)
		{
			mesh_sphere *sphere = new mesh_sphere(pos, 1.0f);

			mat4t location;
			location.translate(pos);
			location.rotateX90();

			mesh_instance* sphere_instance = app_scene->add_shape(location, sphere, custom_mat, true);
			//sphere_instance->get_node()->get_rigid_body()->setLinearFactor(btVector3(0, 0.5f, 0));

			return sphere_instance;
		}
		
		
		int prev_mouse_x, prev_mouse_y;
		void get_mouse_delta(int& delta_x, int& delta_y)
		{
			int mouse_x, mouse_y;
			get_mouse_pos(mouse_x, mouse_y);
			delta_x = mouse_x - prev_mouse_x;
			delta_y = mouse_y - prev_mouse_y;
		}


		bool enable_mouselook;
		/// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {
			TwWindowSize(w, h);

			get_mouse_pos(mouse_pos[0], mouse_pos[1]);
			pos = cam->get_node()->get_position();
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

			float cam_speed = 0.65f;


			/*
			if (is_key_going_down('ESC'))
			{
				enable_mouselook = !enable_mouselook;
			}*/
			if (is_key_down('W'))
			{
				cam->get_node()->translate(-vec3(0,0,1) * cam_speed);
			}
			if (is_key_down('S'))
			{
				cam->get_node()->translate(vec3(0, 0, 1) * cam_speed);
			}
			if (is_key_down('A'))
			{
				cam->get_node()->translate(-vec3(1, 0, 0) * cam_speed);
			}
			if (is_key_down('D'))
			{
				cam->get_node()->translate(vec3(1, 0, 0) * cam_speed);
			}
			if (is_key_down('E'))
			{
				cam->get_node()->translate(-vec3(0, 1, 0) * cam_speed);
			}
			if (is_key_down('Q'))
			{
				cam->get_node()->translate(vec3(0, 1, 0) * cam_speed);
			}


			if (is_key_down(key_down))
			{
				sphere_instance->get_node()->activate();
				//sphere_instance->get_node()->apply_central_force(vec3(0, -1, 0) * 100.0f);
				sphere_instance->get_node()->get_rigid_body()->applyCentralForce(btVector3(0, -1, 0) * 100.0f);
			}
			if (is_key_down(key_up))
			{
				sphere_instance->get_node()->activate();
				sphere_instance->get_node()->apply_central_force(vec3(0,1,0) * 100.0f);
			}

			text->format("ray start: %f, %f, %f - end: %f, %f, %f\n ", r.get_start().x(), r.get_start().y(), r.get_start().z(), r.get_end().x(), r.get_end().y(), r.get_end().z());
			app_scene->set_render_debug_lines(true);
			app_scene->add_debug_line(r.get_start(), r.get_end());

			//overlay->get_node()->translate(vec3(0.10f, 0.0f, 0.0f));
			// convert it to a mesh.
			text->update();

			// draw the text overlay
			overlay->render(vx, vy);

			update_tweakbar();
			TwDraw();  // draw the tweak bar(s)


			mat4t &camera_to_world = cam->get_node()->access_nodeToParent();
			if (enable_mouselook)
				mouse_look_helper.update(camera_to_world);
		}


		void update_tweakbar()
		{
			int mX, mY;
			get_mouse_pos(mX, mY);

			if (is_key_down(key_lmb))
			{
				TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_LEFT);
			}
			//if (is_key_going_up(key_lmb))
			else
			{
				TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_LEFT);
			}
			/*
			if (is_key_down(key_rmb))
			{
				TwMouseButton(TW_MOUSE_PRESSED, TW_MOUSE_RIGHT);
			}
			//if (is_key_going_up(key_rmb))
			else
			{
				TwMouseButton(TW_MOUSE_RELEASED, TW_MOUSE_RIGHT);
			}
			**/
			TwMouseWheel(get_mouse_wheel());
			TwMouseMotion(mX, mY);

			
		}


	};
}
