//////////////////////////////////////////////////////////////////////////////////////////
//
// default frament shader for textures
//

// constant parameters
uniform vec4 lighting[17];
uniform int num_lights;
uniform sampler2D diffuse_sampler;
uniform sampler2D light_ramp;

uniform int num_spots;

// inputs
varying vec3 normal_;
varying vec3 camera_pos_;
varying vec2 uv_;
varying vec4 color_;
varying vec3 model_pos_;

void main() {
  vec4 diffuse = texture2D(diffuse_sampler, uv_);
  vec3 nnormal = normalize(normal_);
  vec3 npos = camera_pos_;
  vec3 diffuse_light = lighting[0].xyz;


  for (int i = 0; i != num_lights; ++i) {
    vec3 light_pos = lighting[i * 4 + 1].xyz;
    vec3 light_direction = lighting[i * 4 + 2].xyz;
    vec3 light_color = lighting[i * 4 + 3].xyz;
    vec3 light_atten = lighting[i * 4 + 4].xyz;
    float diffuse_factor = max(dot(light_direction, nnormal), 0.0);
    vec3 ramp = texture2D(light_ramp, vec2(diffuse_factor, 0.5)).xyz;

    diffuse_light += ramp * light_color;
  }
  gl_FragColor = vec4(diffuse.xyz * diffuse_light, 1.0);
}

