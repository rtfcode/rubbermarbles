attribute vec3 coord3d;
attribute vec3 v_color;

uniform mat4 mvp;
uniform mat4 model;
uniform int colset;

varying vec3 f_color;

void main(void) {

  gl_Position = mvp * vec4(coord3d, 1.0);

  if (colset == 0) {
    f_color = v_color;
  } else if (colset == 1) {
    f_color = vec3(v_color.r, v_color.r, v_color.r);
  } else {
    f_color = vec3(0.0, 0.0, 1.0);
  }

}



