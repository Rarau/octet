//////////////////////////////////////////////////////////////////////////////////////////
//
// default frament shader for textures
//

// constant parameters
uniform vec4 lighting[17];
uniform int num_lights;
uniform sampler2D diffuse_sampler;
uniform sampler2D light_ramp;
uniform float uv_tiling;
uniform int num_spots;

// inputs
varying vec3 normal_;
varying vec3 camera_pos_;
varying vec2 uv_;
varying vec4 color_;
varying vec3 model_pos_;

void main() {
  vec4 diffuse = texture2D(diffuse_sampler, uv_ * uv_tiling);
  vec3 nnormal = normalize(normal_);
  vec3 npos = camera_pos_;
  vec3 diffuse_light = lighting[0].xyz;

  vec4 rim_color = vec4(0.97,0.88,1,0.75);
  float rim_power = 2.5;


  vec3 view_dir = normalize(model_pos_ - camera_pos_);
  vec3 rim = pow (max (0.0, dot (view_dir, nnormal) ), rim_power) * rim_color.rgb * rim_color.a;
  //rim = pow(rim, 2.5);

  for (int i = 0; i != num_lights; ++i) {
    vec3 light_pos = lighting[i * 4 + 1].xyz;
    vec3 light_direction = lighting[i * 4 + 2].xyz;
    vec3 light_color = lighting[i * 4 + 3].xyz;
    vec3 light_atten = lighting[i * 4 + 4].xyz;
    float diffuse_factor = max(dot(light_direction, nnormal), 0.0);

    float d = (dot(light_direction, nnormal) * 0.5 + 0.5);

    vec3 ramp = texture2D(light_ramp, vec2(d, d)).xyz;

    diffuse_light += ramp * light_color;
  }
  gl_FragColor = vec4(diffuse.xyz * diffuse_light , 1.0);
}

