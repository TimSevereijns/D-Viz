# D-Viz: A 3-D Directory Visualizer

Using this **Qt** and **OpenGL** based application, users can recursively scan their filesytems (or parts thereof) to generate a three dimensional visualization of the directory structure, where nested files and folders are represented as blocks that stack on one another to create a virtual "cityscape." Once the visualization has been generated, users can fly around the scene using the familiar WASD + mouse movement scheme common to practically all modern videogames. Additionally, D-Viz also provides basic gamepad support. Note that all testing has been done with a wired Xbox 360 controller. The use of the controller greatly improves the navigation experience; especially when transitioning from macro to micro exploration.

The primary visualization is based on the well known "squarification" technique outlined in the paper **Squarified Treemaps**, written by Mark Bruls, Kees Huizing, and Jarke van Wijk. This technique aims to keep block aspect ratios as close to one as possible.

For information on how to set up and build the project (on Linux), please see `.github/workflows/ccpp.yml`.

## Screenshots

![Example 1](https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/2020/D-Viz-1.png)

![Example 2](https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/2020/D-Viz-2.png)

![Example 3](https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/2020/D-Viz-3.png)

![Example 4](https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/2020/D-Viz-4.png)

## Key Bindings

| Key(s)                                     | Action                   |
|--------------------------------------------|--------------------------|
| <kbd>W</kbd>                               | Move forward             |
| <kbd>A</kbd>                               | Move left                |
| <kbd>S</kbd>                               | Move backward            |
| <kbd>D</kbd>                               | Move right               |
| <kbd>Left-Click</kbd>                      | Hold down to pan         |
| <kbd>Right-Click</kbd>                     | Select a node            |
| <kbd>Ctrl</kbd> + <kbd>Right-Click</kbd>   | Show context menu        |
| <kbd>Scroll Up</kbd>                       | Increase camera speed    |
| <kbd>Scroll Down</kbd>                     | Decrease camera speed    |
| <kbd>Shift</kbd> + <kbd>Scroll Up</kbd>    | Zoom in                  |
| <kbd>Shift</kbd> + <kbd>Scroll Down</kbd>  | Zoom out                 |
