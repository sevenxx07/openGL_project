# Project content

* Geometric objects with normals and texturing coordinates (export from another subject or a completely new object, at least one hand-made with more than 30 triangles, the rest only freely available) hardcode in src - (attribute field -> VBO - > VAO -> drawing)
* Geometric objects loaded from a file with its own loader (without the assimp library) 4. dynamic object (change of geometry in the vertex shader - other than a raven or an inflatable kettle) 5. semi-transparent objects (correct rendering order, other than objects used for dynamic and moving texture)
* Keyboard operation - arrows (e.g. camera movement), letter characters, function keys
* Mouse operation - click (processing the position on the screen), 2nd movement (e.g. looking around the camera)
* 2x static view + free camera (look around and move) - switch from static view to free camera
* Dynamic camera along a spline curve
* Light sources hardcode in shaders (reflector with camera )
* Light sources via uniform variables (parameters like position, direction, intensity)
* Light sources types - directional, point (including attenuation), reflector
* Materials hardcode in the shader
* Materials via uniform
* Configuration constants in the source (parameters.h)
* Configuration file loaded after pressing the reload button
* Phong Lighting model
* Textures - multiplication of the diffuse component by a texture (modulate), normal maps and neighborhood map, skybox and skydome (cube around the scene fixed in the scene)
* Animation of an object (toy car, rocket, ...) following a general parametric curve - Catmull-Rom (change of position + rotation of the object)
* Fog exponential depending on the distance from the camera
* Changing parameters automatically with time - fog, skybox according to time of day
* Application interactivity - collision (does not climb from the scene), selection of object
* GUI/Menu using GLUT

# Controls

arrows - control<br />
mouse - looking around<br />
right mouse button - menu (functional outside of free cameras)<br />
R - restart<br />
L - turn on/off the wand<br />
F - fog on/off<br />
U - first static view<br />
I - second static view<br />
O - free camera<br />
ESC - shutdown the application<br />

# Photos
The Hogwarts lands, which only contain part of the castle and its surroundings. Objects from the world of Harry Potter can be found on the grounds. You can turn on the fog and light the way with a wand. On the grounds you can find a pond where you can sit on a bench.<br />
![screenshot1](https://github.com/user-attachments/assets/52888a00-9a09-40d5-bc2d-260b9c93a7a1)

![screenshot2](https://github.com/user-attachments/assets/8dd595bf-3c4c-4732-92cc-4c8ed0f449f6)

![screenshot3](https://github.com/user-attachments/assets/3a914dc2-1435-4122-8486-90881e611730)


