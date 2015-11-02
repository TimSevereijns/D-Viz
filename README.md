# D-Viz: A 3-D Directory Visualizer

Using this Qt and OpenGL based application, users can recursively scan their filesytems (or parts thereof) to generate a three dimensional visualization of the directory structure, where nested files and folders are represented as blocks that stack on one another to create a virtual "cityscape." Once the visualization has been generated, users will be able to fly around the scene using the familar WASD + Mouse movement scheme common to practically all modern video-games. Additionally, on the Windows platform, D-Viz also supports XBox 360 controller support. The use of the controller greatly improves the navigation experience; especially when transitioning from the macro to micro exploration.

The only visualization currently supported is the well known squarification technique, as outlined in the paper "Squarified Treemaps," by Mark Bruls, Kees Huizing, and Jarke van Wijk. The squarification technique aims to keep block aspect ratios as close to one as possible.

For information on how to set up and build the project, please see the Wiki.

## Screenshots

![Example 1]
(https://raw.githubusercontent.com/TimSevereijns/D-Viz/master/Screenshots/Example1.png?token=AAsAD7eC5MFEY2MddgUnp23Wy2E2UVhbks5WQAQ6wA%3D%3D)

![Example 2]
(https://raw.githubusercontent.com/TimSevereijns/D-Viz/master/Screenshots/Example3.png?token=AAsAD18uT-uSjIHwIlr-TBa3c1RbrAznks5WQAR1wA%3D%3D)

![Example 3]
(https://raw.githubusercontent.com/TimSevereijns/D-Viz/master/Screenshots/Example4.png?token=AAsAD8-yaruivPQ4eXyjNYVc7ivKNpzfks5WQASOwA%3D%3D)

## Notes

* For now, you'll have to locate XINPUT1_4.dll on your Windows system and copy it to the build directory in order to get XBox Controller support.
* If the compiled program refuses to launch from Qt Creator, it's possible that the DLLs are missing from the build product directory. The easiest way to rectify this issue is to navigate to that directory, and to launch the generated EXE. This will obviously fail as well, but the resulting error message should tell you which DLLs are missing. Locate these missing DLLs on your system, and then copy them to the build product directory. Once this has been done for all missingi DLLs, the EXE should launch correctly. When this happens, the project will also be launchable from within Qt Creator.
