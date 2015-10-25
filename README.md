# D-Viz
A 3-D Directory Visualizer

This Qt and OpenGL based application will recursively scan your filesytem (or parts thereof) to generate a three dimensional visualization of the directory structure, where nested files and folders are represented as blocks that stack on one another to create a virtual "cityscape." Once the visualization has been generated, users will be able to fly around the scene using the familar WASD + Mouse movement scheme common to practically all modern video-games. Additionally, on the Windows platform, I also provide XBox 360 controller support. The use of the controller greatly improves the navigation experience; this is especially true when transitioning from the exploration of macro to micro features.

The project currently provides support for two different (but closely related) visualization of treemaps. The first is an implementation of the rather naive "slice and dice" technique, which results in high-aspect ratio representations of folders and files. The second technique is an adaption of the far more aesthetically pleasing squarification technique, as outlined in the paper "Squarified Treemaps," by Mark Bruls, Kees Huizing, and Jarke van Wijk. The squarification technique aims to keep block aspect ratios as close to one as possible.

For information on how to set up and build the project, please see the Wiki.

## Screenshots

![Example 1]
(https://raw.githubusercontent.com/TimSevereijns/D-Viz/master/Screenshots/Example1.png?token=AAsADyYMtO45ilWzi898t0zG6dmMmrb2ks5WNrE1wA%3D%3D)

![Example 2]
(https://raw.githubusercontent.com/TimSevereijns/D-Viz/master/Screenshots/Example3.png?token=AAsAD3YA9PKJvce2RzjcjD4veWUMDbwGks5WNrGWwA%3D%3D)

![Example 3]
(https://raw.githubusercontent.com/TimSevereijns/D-Viz/master/Screenshots/Example4.png?token=AAsADyX8DJhFWJ2UXMOCV1-flThEKi-dks5WNrGywA%3D%3D)

## Notes

* For now, you'll have to locate XINPUT1_4.dll on your Windows system and copy it to the build directory in order to get XBox Controller support.
* If the compiled program refuses to launch from Qt Creator, it's possible that the DLLs are missing from the build product directory. The easiest way to rectify this issue is to navigate to that directory, and to launch the generated EXE. This will obviously fail as well, but the resulting error message should tell you which DLLs are missing. Locate these missing DLLs on your system, and then copy them to the build product directory. Once this has been done for all missingi DLLs, the EXE should launch correctly. When this happens, the project will also be launchable from within Qt Creator.
