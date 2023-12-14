# Assignment - Phong Illumination and Point Light(s)

<img align="right" src="./media/light.gif" alt="Plane" width="400px"/>

> "Adding atmosphere with lighting"

# Resources to help

Some additional resources to help you through this lab assignment

| SDL2 related links                                    | Description                       |
| --------------------------------------------------    | --------------------------------- |
| [SDL API Wiki](https://wiki.libsdl.org/APIByCategory) | Useful guide to all things SDL2   |
| [My SDL2 Youtube Playlist](https://www.youtube.com/playlist?list=PLvv0ScY6vfd-p1gSnbQhY7vMe2rng0IL0) | My Guide for using SDL2 in video form.   |
| [Lazy Foo](http://lazyfoo.net/tutorials/SDL/)         | Great page with written tutorials for learning SDL2. Helpful setup tutorials for each platform. |
| [Lazy Foo - Handling Key Presses](https://lazyfoo.net/tutorials/SDL/04_key_presses/index.php) | Useful tutorial for learning how to handle key presses | 

| OpenGL related links                                | Description                       |
| --------------------------------------------------  | --------------------------------- |
| [My OpenGL Youtube Series](https://www.youtube.com/playlist?list=PLvv0ScY6vfd9zlZkIIqGDeG5TUWswkMox) | My video series for learning OpenGL |
| [docs.gl](http://docs.gl)                           | Excellent documentation to search for OpenGL commands with examples and description of function parameters   |
| [learnopengl.com](https://learnopengl.com)          | OpenGL 3.3+ related tutorial and main free written resource for the course   |


| C++ related links                                   | Description                       |
| --------------------------------------------------  | --------------------------------- |
| [My C++ Youtube Series](https://www.youtube.com/playlist?list=PLvv0ScY6vfd8j-tlhYVPYgiIyXduu6m-L) | My video series playlist for learning C++ |
| [cppreference](https://en.cppreference.com/w/)      | Definitive, more encyclopedic guide to C++ (less beginner focused, but excellent technically) |
| [cplusplus.com](http://www.cplusplus.com)           | Nice website with examples and tutorials geared more for beginners, reference has lots of examples, and the tutorial page is a great starting point |
| [learncpp.com](https://www.learncpp.com/)           | Handy page for learning C++ in tutorial form   |


# Description

Lights! Camera! Action!

Err, well we need to add the lights -- and that is exactly our goal for this assignment. For this assignment we are going to be implementing the phong illumination model.

## Task 1 - .obj object display

The first task of this assignment is to incorporate your previous assignments .obj loader into this one. That is, we should be able to load an object(.obj) file through the command-line arguments and view it with a perspective projection.

`./prog ../../common/bunny_centered.obj`

## Task 2 - Displaying Normals

Something that will be very useful to debugging is to actually view the normals of your object. This means you will need to update your fragment shader, such that the output color is based on the normal. **You should** by default display the object with the normals.

Pragmatically:
- You need to store the normals from the .obj file per-vertex.
- You will need to pass the normals from the vertex shader to the fragment shader.
- Somewhere in your output color of the fragment shader, you'll have something that *may look like*: `out_color = vec4(v_normal.x,v_normal.y, v_normal.z,1.0f);`

Note: I do not care if you use 'smooth normals' (1 normal per vertex) or 'faceted normals' (flat shaded -- 1 normal per triangle to give a flat look). The point is that I'm able to see some shades of 'red', 'green', and 'blue' on our object, that are based on the normals.

Note: As inspiration, you can refer to the previous parts sample code. I have an example of computing the normals per triangle by taking the cross product of two edges. Keep in mind, .obj models to have a normal value already associated with each vertex (though different .obj exporters may slightly vary in the calculation).

## Task 3 - Phong Illumination - Point Lights

Your final task is to implement the phong illumination model. Recall that the phong illumination model consists of 3 components.

1. Ambient
2. Diffuse
3. Specular

The following page will be a great resource for getting started: [learnopengl.com/Lighting/Basic-Lighting](https://learnopengl.com/Lighting/Basic-Lighting). You can take as inspiration some of the constants used for the ambient, diffuse, and specular for example.

In the example provided in part 0 however, there is a bit of a problem -- in the sense that our light is a 'directional light'. This means that we are always computing the light such that we assume it's shining towards the object.

Instead, we want to implement a 'point light', such that it still shines out in all directions, but the light 'loses energy' (i.e. attenuates) over distance.

For this portion of the assignment, **you must implement** at least one point light in your scene. Your **point light** must also move -- this is how we'll know if your lighting is actually working (and I recommend to have it just circle around the origin -- again, look at the previous sample code for a sine and cosine function for creating an orbit).

Note: I would recommend when you get one light working, trying to have multiple point lights. You can assume the lights are perfectly 'white light (rgb(1.0f,1.0f,1.0f))', but it again may be nice to add the light colors as an attribute to create a more dynamic scene.

### Task 4 - Interactive graphics 

Please keep the following interactive components from the previous assignment. If you add more to your camera, that is also fine.

The tasks for interactivity in this assignment are the following:

- Pressing the <kbd>tab</kbd> key draws your object in wireframe mode (By default when you start the application it will show the model in filled).
- Pressing the <kbd>esc</kbd> key exits the application.
- Pressing the <kbd>w</kbd> and <kbd>s</kbd> key move your camera forward and backwrds

A resource for performing [keyboard input with SDL is provided here](http://lazyfoo.net/tutorials/SDL/04_key_presses/index.php)

### Assignment strategy

Some tips:

- Start slowly and add one feature at a time.
  - I would probably start with  getting the normals displayed on the .obj file as colors (i.e. so you do not have an object with one solid color, but rather with different colors depending on the normal).
  - Then I would add an abstraction for a light, so I can see a lights position in 3D space.
  - Finally, I would add in the phong illuminatino model with a point light.

## How to compile and run your program

1. Your solution should compile using the [build.py](./build.py) file. 
	- That is, we will run your program by typing [python3 build.py](./build.py) and it should just work.

# Submission/Deliverables

### Submission

- Commit all of your files to github, including any additional files you create.
- Do not commit any binary files unless told to do so.
- Do not commit any 'data' files generated when executing a binary.

### Deliverables

- Your solution should be interactive, and utilize the keypresses as described in Task 2

* You need to commit your code to this repository.
* You need to use the build.py script provided. Anything else used is at your own risk--and you should provide complete documentation. If your program does not compile and run, you get a zero!


# F.A.Q. (Instructor Anticipated Questions)

* Q: Can I add more?
  * A: Yes, of course! Just as an FYI, textures and normal mapping are coming up.
  * It may be neat to be able to render with and without normals being visualized (i.e. change between a solid color and rendering with normals)

# Found a bug?

If you found a mistake (big or small, including spelling mistakes) in this lab, kindly send me an e-mail. It is not seen as nitpicky, but appreciated! (Or rather, future generations of students will appreciate it!)

- Fun fact: The famous computer scientist Donald Knuth would pay folks one $2.56 for errors in his published works. [[source](https://en.wikipedia.org/wiki/Knuth_reward_check)]
- Unfortunately, there is no monetary reward in this course :)
