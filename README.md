# D-Viz: A 3-D Directory Visualizer

Using this **Qt** and **OpenGL** based application, users can recursively scan their filesytems (or parts thereof) to generate a three dimensional visualization of the directory structure, where nested files and folders are represented as blocks that stack on one another to create a virtual "cityscape." Once the visualization has been generated, users can fly around the scene using the familiar WASD + mouse movement scheme common to practically all modern videogames. Additionally, D-Viz also provides basic gamepad support; all testing has been done with a wired Xbox 360 controller. The use of the controller greatly improves the navigation experience; especially when transitioning from macro to micro exploration.

The only visualization currently supported is the well known squarification technique, as outlined in the paper **Squarified Treemaps**, written by Mark Bruls, Kees Huizing, and Jarke van Wijk. The squarification technique aims to keep block aspect ratios as close to one as possible.

For information on how to set up and build the project, please see the Wiki.

## Screenshots

![Example 1](https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/D-Viz-1.png)

![Example 2](https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/D-Viz-2.png)

![Example 3](https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/D-Viz-3.png)

![Example 4](https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/D-Viz-4.png)

## Technical Notes

* This project is being actively developed against `MSVC++ 14.1` on Windows, and `GCC 7.2` on Mint Linux. The version of Qt being used is 5.9. Since this is purely a hobby project, I liberally use the latest C++ features as they become available on both platforms, and as such this project is likely not very backwards compatible---sorry.
