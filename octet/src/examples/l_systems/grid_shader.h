////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Single texture shader with no lighting

namespace octet {
	namespace shaders {
		class grid_shader : public shader {

			// index for model space to projection space matrix
			GLuint modelToProjectionIndex_;

		public:
			void init() {
				// this is the vertex shader.
				const char vertex_shader[] = SHADER_STR(
					varying vec3 color_;

					attribute vec4 pos;
					attribute vec3 color;

					uniform mat4 modelToProjection;

					void main() { gl_Position = modelToProjection * pos; }
				);

				// this is the fragment shader
				const char fragment_shader[] = SHADER_STR(


				void main()
				{
					gl_FragColor = vec4(0.75f, 0.75f, 0.75f, 0.2125f);
				}
				);

				// use the common shader code to compile and link the shaders
				// the result is a shader program
				shader::init(vertex_shader, fragment_shader);

				// extract the indices of the uniforms to use later
				modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
			}

			void render(const mat4t &modelToProjection, int sampler) {
				// tell openGL to use the program
				shader::render();

				// customize the program with uniforms
				glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
			}
		};
	}
}
