////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// Single texture shader with no lighting

namespace octet { namespace shaders {
  class texture_shader : public shader {
    // indices to use with glUniform*()

    // index for model space to projection space matrix
    GLuint modelToProjectionIndex_;
	GLuint modelToWorldIndex_;

    // index for texture sampler
    GLuint samplerIndex_;

	GLuint lightPosIndex_;
  public:
    void init() {
      // this is the vertex shader.
      // it is called for each corner of each triangle
      // it inputs pos and uv from each corner
      // it outputs gl_Position and uv_ to the rasterizer
      const char vertex_shader[] = SHADER_STR(
        varying vec2 uv_;
		varying vec4 pos_;
		varying vec4 lightPos_;

        attribute vec4 pos;
        attribute vec2 uv;

        uniform mat4 modelToProjection;
		uniform mat4 modelToWorld;
		uniform vec4 lightPos;


		void main() { gl_Position = modelToProjection * pos; uv_ = uv; pos_ = modelToWorld * pos; lightPos_ = lightPos; }
      );

      // this is the fragment shader
      // after the rasterizer breaks the triangle into fragments
      // this is called for every fragment
      // it outputs gl_FragColor, the color of the pixel and inputs uv_
      const char fragment_shader[] = SHADER_STR(
        varying vec2 uv_;
		varying vec4 pos_;
		varying vec4 lightPos_;

        uniform sampler2D sampler;
		uniform vec4 ambientColor = vec4 (0.20f, 0.20f, 0.60f, 0.0f);

		uniform float k0 = 0.02f;
		uniform float k1 = 0.05f;
		uniform float k2 = 0.05f;

        void main() { 
			//vec4 lightPos = modelToProjection * lightPos;
			//float atten = 1 / (k0 + k1 * distance(lightPos.xy, gl_FragCoord.xy));
			float d = distance(lightPos_.xy, pos_.xy);
			float atten = 1 / (k0 + k1 * d + k2 * d * d);
			vec4 lightColor = vec4(1.0f, 0.9f, 0.4f, 1.0f) * atten;
			lightColor.a = 1.0f;
			vec4 diffColor = texture2D(sampler, uv_);
			vec4 fragColor = diffColor * lightColor;
			gl_FragColor = clamp(fragColor, 0.0f, 1.0f);
			/*gl_FragColor = vec4(pos_.x, pos_.y, 0.0f, 1.0f) * 0.1f;
			gl_FragColor.a = 1.0f;
			gl_FragColor.b = 1.0f;
			*/
		}
      );
    
      // use the common shader code to compile and link the shaders
      // the result is a shader program
      shader::init(vertex_shader, fragment_shader);

      // extract the indices of the uniforms to use later
      modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
	  modelToWorldIndex_ = glGetUniformLocation(program(), "modelToWorld");
      samplerIndex_ = glGetUniformLocation(program(), "sampler");
	  lightPosIndex_ = glGetUniformLocation(program(), "lightPos");
    }

	void render(const mat4t &modelToProjection, int sampler, vec3 lightPos, const mat4t &modelToWorld) {
      // tell openGL to use the program
      shader::render();

      // customize the program with uniforms
      glUniform1i(samplerIndex_, sampler);
      glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
	  glUniformMatrix4fv(modelToWorldIndex_, 1, GL_FALSE, modelToWorld.get());

	  glUniform4f(lightPosIndex_, lightPos.x(), lightPos.y(), lightPos.z(), 1.0f);
    }
  };
}}
