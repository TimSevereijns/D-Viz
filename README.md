# D-Viz: A 3-D Directory Visualizer

Using this **Qt** and **OpenGL** based application, users can recursively scan their filesytems (or parts thereof) to generate a three dimensional visualization of the directory structure, where nested files and folders are represented as blocks that stack on one another to create a virtual "cityscape." Once the visualization has been generated, users can fly around the scene using the familar WASD + mouse movement scheme common to practically all modern videogames. Additionally, on the Windows platform, D-Viz also provides XBox 360 controller support. The use of the controller greatly improves the navigation experience; especially when transitioning from macro to micro exploration.

The only visualization currently supported is the well known squarification technique, as outlined in the paper **Squarified Treemaps**, written by Mark Bruls, Kees Huizing, and Jarke van Wijk. The squarification technique aims to keep block aspect ratios as close to one as possible.

For information on how to set up and build the project, please see the Wiki.

## Screenshots

![Example 1]
(https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/Example1.png)

![Example 2]
(https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/Example3.png)

![Example 3]
(https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/Example5.png)

![Example 4]
(https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/Example9.png)

![Example 5]
(https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/Example11.png)

![Example 6]
(https://github.com/TimSevereijns/D-Viz/blob/master/Screenshots/Example12.png)

## Notes

* If the compiled program refuses to launch from Qt Creator, it's possible that the DLLs are missing from the build product directory. The easiest way to rectify this issue is to navigate to that directory, and to launch the generated EXE. This will obviously fail as well, but the resulting error message should tell you which DLLs are missing. Locate these missing DLLs on your system, and then copy them to the build product directory. Once this has been done for all missing DLLs, the EXE should launch correctly. When this happens, the project will also be launchable from within Qt Creator. At some point, I'll create an installer for this project.
