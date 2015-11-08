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

		ref<material> custom_mat;
		ref<material> ground_mat;
		ref<material> color_mat;


		// text mesh object for overlay.
		ref<mesh_text> text;
		camera_instance* cam;

		ref<image> dif_texture;
		ref<image> light_ramp;
		ref<image> ground_diff;
		ref<image> obj_texture;


		mouse_look mouse_look_helper;

		collada_builder loader;
		TwBar *myBar, *selection_bar;

		vec3 pos;
		int mouse_pos[2];
		vec2 mouse_scr_pos;

		scene_node * selected_node = 0;
		vec3 sel_pos;

	public:
		/// this is called when we construct the class before everything is initialised.
		middleware_one(int argc, char **argv) : app(argc, argv) {
		}



		static void TW_CALL shoot_ball(void *data) {
			middleware_one* app = (middleware_one *)data;
			scene_node * cam_node = app->cam->get_node();
			mesh_instance* ball = app->add_sphere(cam_node->get_position() + cam_node->get_z() * -10.0f);
			ball->get_node()->apply_central_force(cam_node->get_z() * -1000.0f);
		}


		/// this is called once OpenGL is initialized
		void app_init() 
		{
			app_scene = new visual_scene();
			app_scene->create_default_camera_and_lights();
			
			cam = app_scene->get_camera_instance(0);
			cam->set_far_plane(750.0f);

			mouse_look_helper.init(this, 200.0f / 360.0f, false);

			// Load textures
			obj_texture = new image("assets/grass.jpg");
			dif_texture = new image("assets/chest.gif");
			ground_diff = new image("assets/ground_2.gif");
			light_ramp = new image("assets/ramp_2.gif");

			// Init shader and materials
			float uv_tiling = 1.0f;

			param_shader *shader = new param_shader("shaders/default.vs", "shaders/toon.fs");

			custom_mat = new material(vec4(1, 1, 1, 1), shader);
			custom_mat->add_sampler(0, app_utils::get_atom("diffuse_sampler"), dif_texture, new sampler());
			custom_mat->add_sampler(1, app_utils::get_atom("light_ramp"), light_ramp, new sampler());
			custom_mat->add_uniform(&uv_tiling, app_utils::get_atom("uv_tiling"), GL_FLOAT, 1);

			uv_tiling = 5.0f;
			ground_mat = new material(vec4(1, 1, 1, 1), shader);
			ground_mat->add_sampler(0, app_utils::get_atom("diffuse_sampler"), ground_diff, new sampler());
			ground_mat->add_sampler(1, app_utils::get_atom("light_ramp"), light_ramp, new sampler());
			ground_mat->add_uniform(&uv_tiling, app_utils::get_atom("uv_tiling"), GL_FLOAT, 1);

			color_mat = new material(vec4(1, 1, 1, 1), shader);
			color_mat->add_sampler(0, app_utils::get_atom("diffuse_sampler"), obj_texture, new sampler());
			color_mat->add_sampler(1, app_utils::get_atom("light_ramp"), light_ramp, new sampler());



			add_walls();

			init_tweakbars();

			init_from_csv();
			
			/*
			mesh_instance* ceiling_ball = add_sphere(vec3(0.0f, 30.0f, 0.0f));
			ceiling_ball->get_node()->get_rigid_body()->setLinearFactor(btVector3(0.0f,0.0f,0.0f));

			mesh_instance* ceiling_ball_2 = add_sphere(vec3(0.0f, 10.0f, 0.0f));
			
			add_spring_joint(ceiling_ball->get_scene_node(), ceiling_ball_2->get_scene_node());
			*/

			mat4t location;

			location.translate(vec3(-20.0f, 1.0f, 0.0f));
			scene_node* cube_1 = app_scene->add_shape_node(location, new mesh_box(vec3(1.0f)), color_mat, false);

			location.loadIdentity();
			location.translate(vec3(-10.0f, 1.0f, 0.0f));
			scene_node* cube_2 = app_scene->add_shape_node(location, new mesh_box(vec3(1.0f, 2.0f, 1.0f)), color_mat, true);

			add_spring_joint(cube_1, cube_2, true);
			//add_hinge_joint(cube_1, cube_2, btVector3(0.0f, -1.0f, 0.0f), btVector3(0.0f, 2.0f, 0.0f), btVector3(1.0f, 0.0f, 0.0f));
			//add_point2point_joint(cube_1, cube_2, btVector3(0.0f, -1.0f, 0.0f));
		}

		void add_walls()
		{
			mat4t ground_location;
			ground_location.translate(vec3(0, -10.0f, 0));

			mesh_box * ground = new mesh_box(vec3(100.0f, 0.1f, 100.0f));
			app_scene->add_shape(ground_location, ground, ground_mat, false);
			ground_location.loadIdentity();
			ground_location.translate(vec3(0.0f, 0.0f, -100.0f));
			ground_location.rotateX90();
			app_scene->add_shape(ground_location, ground, ground_mat, false);

			ground_location.loadIdentity();
			ground_location.translate(vec3(+100.0f, 0.0f, 0.0f));
			ground_location.rotateX90();
			ground_location.rotateY90();
			app_scene->add_shape(ground_location, ground, ground_mat, false);

			ground_location.loadIdentity();
			ground_location.translate(vec3(0.0f, 0.0f, 100.0f));
			ground_location.rotateX90();
			app_scene->add_shape(ground_location, ground, ground_mat, false);

			ground_location.loadIdentity();
			ground_location.translate(vec3(-100.0f, 0.0f, 0.0f));
			ground_location.rotateX90();
			ground_location.rotateY90();
			app_scene->add_shape(ground_location, ground, ground_mat, false);
		}

		void init_tweakbars()
		{
			TwInit(TW_OPENGL, NULL);
			myBar = TwNewBar("Camera");
			selection_bar = TwNewBar("Selection");

			TwAddVarRO(myBar, "Cam_X", TW_TYPE_FLOAT, &(pos.x()), " label='X' ");
			TwAddVarRO(myBar, "Cam_Y", TW_TYPE_FLOAT, &(pos.y()), " label='Y' ");
			TwAddVarRO(myBar, "Cam_Z", TW_TYPE_FLOAT, &(pos.z()), " label='Z' ");
			TwAddVarRO(myBar, "Mouse_X", TW_TYPE_INT32, &(mouse_pos[0]), " label='Mouse X' ");
			TwAddVarRO(myBar, "Mouse_Y", TW_TYPE_INT32, &(mouse_pos[1]), " label='Mouse Y' ");

			TwAddVarRO(myBar, "Mouse_scr_X", TW_TYPE_FLOAT, &(mouse_scr_pos.x()), " label='M_scr X' ");
			TwAddVarRO(myBar, "Mouse_scr_Y", TW_TYPE_FLOAT, &(mouse_scr_pos.y()), " label='M_scr Y' ");

			TwAddButton(selection_bar, "comment1", NULL, NULL, " label='Position' ");
			TwAddVarRO(selection_bar, "Item_X", TW_TYPE_FLOAT, &(sel_pos.x()), " label='X' ");
			TwAddVarRO(selection_bar, "Item_Y", TW_TYPE_FLOAT, &(sel_pos.y()), " label='Y' ");
			TwAddVarRO(selection_bar, "Item_Z", TW_TYPE_FLOAT, &(sel_pos.z()), " label='Z' ");

			TwAddButton(selection_bar, "Button", shoot_ball, this, " label='Shoot ball' ");
		}

		void add_hinge_joint(scene_node* node_a, scene_node* node_b, btVector3 anchor_a, btVector3 anchor_b, btVector3 axis)
		{
			btRigidBody *rigidbody_a = node_a->get_rigid_body();
			btRigidBody *rigidbody_b = node_b->get_rigid_body();

			btHingeConstraint *hinge = new btHingeConstraint(*rigidbody_a, *rigidbody_b, anchor_a, anchor_b, axis, -axis, true);
			hinge->setDbgDrawSize(btScalar(5.f));
			
			app_scene->add_hinge(hinge);
		}

		void add_spring_joint(scene_node* node_a, scene_node* node_b, bool allow_static = false)
		{
			btRigidBody *rigidbody_a = node_a->get_rigid_body();
			btRigidBody *rigidbody_b = node_b->get_rigid_body();
			
			if (!allow_static && (rigidbody_a->isStaticOrKinematicObject() || rigidbody_b->isStaticOrKinematicObject()))
				return;

			btTransform frameInA, frameInB;
			frameInA = btTransform::getIdentity();
			frameInB = btTransform::getIdentity();

			frameInA.setOrigin(btVector3(10., 0., 0.));
			frameInB.setOrigin(btVector3(0., 0., 0.));


			btGeneric6DofSpringConstraint *spring = new btGeneric6DofSpringConstraint(*rigidbody_a, *rigidbody_b, frameInA, frameInB, true);
			//spring->setDamping(0, 1.0f);
			btVector3 low_limit = btVector3(0.0f, 1.0f, 0.0f);
			spring->setLinearUpperLimit(btVector3(5., 0., 0.));
			spring->setLinearLowerLimit(btVector3(-5., 0., 0.));

			spring->setAngularLowerLimit(btVector3(0.f, 0.f, -1.5f));
			spring->setAngularUpperLimit(btVector3(0.f, 0.f, 1.5f));

			spring->enableSpring(0, true);
			spring->setStiffness(0, 39.478f);
			spring->setDamping(0, 0.5f);
			spring->enableSpring(5, true);
			spring->setStiffness(5, 39.478f);
			spring->setDamping(0, 0.3f);
			spring->setEquilibriumPoint();
			//spring->setLinearLowerLimit(low_limit);
			spring->setDbgDrawSize(btScalar(5.f));
			app_scene->add_spring(spring);	
		}

		void add_point2point_joint(scene_node* node_a, scene_node* node_b, btVector3& point_in_a, bool allow_static = false)
		{
			btRigidBody *rigidbody_a = node_a->get_rigid_body();
			btRigidBody *rigidbody_b = node_b->get_rigid_body();

			btPoint2PointConstraint *p2pc = new btPoint2PointConstraint(*rigidbody_a, *rigidbody_b, point_in_a, point_in_a);
		}

		void init_from_csv()
		{
			std::ifstream is("../../../assets/data_mw.csv");
			if (is.bad())
			{
				printf("Error opening csv file\n");
				return;
			}
			// store the line here
			char buffer[2048];

			// loop over lines
			while (!is.eof()) {
				is.getline(buffer, sizeof(buffer));
				vec3 pos;
				char object_type = 0;
				
				// loop over columns
				char *b = buffer;
				for (int col = 0;; ++col) {
					char *e = b;
					while (*e != 0 && *e != ',') ++e;

					if (col == 0) {
						object_type = b[0];
					}
					else if (col == 1) {
						pos.x() = atof(b);
					}
					else if (col == 2) {
						pos.y() = atof(b);
					}
					else if (col == 3) {
						pos.z() = atof(b);
					}


					if (*e != ',') break;
					b = e + 1;
				}
				// line ended
				add_object(object_type, pos);
			}
		}
		
		public:



		mesh_instance* add_object(char type, vec3 pos)
		{
			if (type == 'c')
			{
				return add_chest(pos);
			}
			else if (type == 's')
			{
				return add_sphere(pos);
			}
			else if (type == 'b')
			{
				return add_cube(pos);
			}
		}

		mesh_instance* add_chest(vec3 pos)
		{
			if (!loader.load_xml("assets/chest.dae")) {
				printf("failed to load file!\n");
				exit(1);
			}

			resource_dict dict;
			loader.get_resources(dict);

			// note that this call will dump the code below to log.txt
			dict.dump_assets(log(""));
			mesh *Chest_mesh = dict.get_mesh("Chest-mesh+ChestMaterial-material");
			//image *chest_jpg = dict.get_image("chest_jpg");

			mat4t location;
			location.translate(pos);
			//location.rotateX90();
			mesh_instance* chest_instance = app_scene->add_shape(location, Chest_mesh, custom_mat, false);

			return chest_instance;

		}

		mesh_instance* add_sphere(vec3 pos)
		{
			mesh_sphere *sphere = new mesh_sphere(vec3(0), 1.0f);

			mat4t location;
			location.translate(pos);


			mesh_instance* sphere_instance = app_scene->add_shape(location, sphere, color_mat, true);

			return sphere_instance;
		}
		
		mesh_instance* add_cube(vec3 pos)
		{
			mat4t location;
			location.translate(pos);

			mesh_box *box = new mesh_box(vec3(1.0f));

			mesh_instance* box_instance = app_scene->add_shape(location, box, color_mat, true);
			return box_instance;
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
		void draw_world(int x, int y, int w, int h)
		{
			if (selected_node != NULL)
			{
				sel_pos = selected_node->get_position();
			}

			get_mouse_pos(mouse_pos[0], mouse_pos[1]);
			mouse_scr_pos = mouse_to_screen(mouse_pos[0], mouse_pos[1], w, h);

			pos = cam->get_node()->get_position();
			int vx = 0, vy = 0;
			get_viewport_size(vx, vy);
			app_scene->begin_render(vx, vy);

			// update matrices. assume 30 fps.
			app_scene->update(1.0f / 30);

			// draw the scene
			app_scene->render((float)vx / vy);
			
			int mouse_x, mouse_y;
			get_mouse_pos(mouse_x, mouse_y);

			ray r = cam->get_ray(mouse_scr_pos.x(), mouse_scr_pos.y());
			app_scene->set_render_debug_lines(true);

			// Do mouse raycast when click
			if (is_key_going_down(key_lmb))
			{
				btVector3 start = btVector3(r.get_start().x(), r.get_start().y(), r.get_start().z());
				btVector3 end = btVector3(r.get_end().x(), r.get_end().y(), r.get_end().z());
				app_scene->add_debug_line(r.get_start(), r.get_end());

				btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
				visual_scene::cast_result cast_result;

				app_scene->cast_ray_with_bullet(cast_result, r);

				if (cast_result.node != NULL) 
				{
					printf("HIT depth: %f\n", cast_result.depth);
					cast_result.node->activate();
					cast_result.node->apply_central_force((r.get_end() - r.get_start()).normalize() * 600.0f);
					/*
					
					if (selected_node != NULL && selected_node != cast_result.node)
					{
						add_spring_joint(cast_result.node, selected_node);
					}
					
					*/
					selected_node = cast_result.node;
				}
			}



			/*
			if (is_key_going_down(key_rmb))
			{
				if (selected_node != NULL)
				{
					selected_node->activate();
					selected_node->apply_central_force((r.get_end() - r.get_start()).normalize() * 10.0f);
				}
			}
			*/
			float cam_speed = 0.65f;
			mouse_look_helper.set_enabled(is_key_down(key_rmb));

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



			// Update the camera controller
			mat4t &camera_to_world = cam->get_node()->access_nodeToParent();
			mouse_look_helper.update(camera_to_world);
			//camera_to_world.rotateX(10.0f);
			update_tweakbar(w, h);
		}

		vec2 mouse_to_screen(int x, int y, int w, int h)
		{
			vec2 result;
			result.x() = ((float)x / (float)w) * 2.0f - 1.0f;
			result.y() = -(((float)y / (float)h) * 2.0f - 1.0f);

			return result;
		}

		void update_tweakbar(int w, int h)
		{
			TwWindowSize(w, h);

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

			TwDraw();  // Draw the tweak bar(s)
		}


	};
}
