////////////////////////////////////////////////////////////////////////////////
// Author: Raul Araujo (2015)
//
//
// Pixel and vertex shaders used to render trees. Just uses per vertex color and no lighting.
//
namespace octet {
	namespace shaders {
		class tree_shader : public shader {
			// indices to use with glUniform*()

			// index for model space to projection space matrix
			GLuint modelToProjectionIndex_;
			GLuint colorIndex_;

			// index for texture sampler
			//GLuint samplerIndex_;
		public:
			void init() {
				// this is the vertex shader.
				const char vertex_shader[] = SHADER_STR(
					varying vec3 color_;
					attribute vec4 pos;
					attribute vec3 color;

					uniform mat4 modelToProjection;

					void main() { gl_Position = modelToProjection * pos; color_ = color; }
				);

				// this is the fragment shader
				// it outputs gl_FragColor, the color of the pixel
				const char fragment_shader[] = SHADER_STR(	
					varying vec3 color_;
				
					void main() 
					{
						gl_FragColor = vec4(color_.x, color_.y, color_.z, 0.250f);
					}
				);

				// use the common shader code to compile and link the shaders
				// the result is a shader program
				shader::init(vertex_shader, fragment_shader);

				// extract the indices of the uniforms to use later
				modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
				colorIndex_ = glGetUniformLocation(program(), "color");
			}

			void render(const mat4t &modelToProjection, int sampler) {
				// tell openGL to use the program
				shader::render();

				// customize the program with uniforms
				glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
				glBindAttribLocation(program(), attribute_color, "color");
			}
		};
	}
}
