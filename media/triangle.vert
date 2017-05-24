#version 100

uniform float   time;
attribute float vertexId;

varying vec4 color;

void
main()
{
  vec4 vertices[3];
  vertices[0] = vec4(-1.0, -1.0, 0.0, 1.0);
  vertices[1] = vec4(1.0, -1.0, 0.0, 1.0);
  vertices[2] = vec4(0.0, 1.0, 0.0, 1.0);

  vec4 colors[3];
  colors[0] = vec4(1.0, 0.0, 0.0, 1.0);
  colors[1] = vec4(0.0, 1.0, 0.0, 1.0);
  colors[2] = vec4(0.0, 0.0, 1.0, 1.0);

  int i       = int(vertexId);
  gl_Position = vertices[i] * vec4(time, time, time, 1.0);
  color       = colors[i] * vec4(time, time, time, 1.0);
}
