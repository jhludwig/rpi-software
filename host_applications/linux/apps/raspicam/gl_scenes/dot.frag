#extension GL_OES_EGL_image_external : require 
uniform samplerExternalOES tex; 
varying vec2 texcoord; 
const float angle = 1.57; 
const float scale = 1.0; 
const float center = 0.5;
const float tSize = 256.0;
float pattern() {
  float s = sin( angle ), c = cos( angle );
  vec2 tex = texcoord * tSize - center;
  vec2 point = vec2( c * tex.x - s * tex.y, s * tex.x + c * tex.y ) * scale;
  return ( sin( point.x ) * sin( point.y ) ) * 4.0;
}
void main() {
  vec4 color = texture2D( tex, texcoord );
  float average = ( color.r + color.g + color.b ) / 3.0;
  gl_FragColor = vec4( vec3( average * 10.0 - 5.0 + pattern() ), color.a );
}