# D-Viz
A 3-D Directory Visualizer

This Qt and OpenGL based application will recursively scan your filesytem (or parts thereof) to generate a three dimensional visualization of the directory structure, where nested files and folders are represented as blocks that stack on one another to create a virtual "cityscape." Once the visualization has been generated, users will be able to fly around the scene using the familar WASD + Mouse movement scheme common to practically all modern video-games. Additionally, on the Windows platform, I also provide XBox 360 controller support. The use of the controller greatly improves the navigation experience; this is especially true when transitioning from the exploration of macro to micro features.

The project currently provides support for two different (but closely related) visualization of treemaps. The first is an implementation of the rather naive "slice and dice" technique, which results in high-aspect ratio representations of folders and files. The second technique is an adaption of the far more aestheticaly pleasing squarification technique, as outlined in the paper "Squarified Treemaps," by Mark Bruls, Kees Huizing, and Jarke van Wijk. The squarification technique aims to keep block aspect ratios as close to one as possible.

For information on how to set up and build the project, please see the Wiki.

## Notes

* For now, you'll have to locate XINPUT1_4.dll on your Windows system and copy it to the build directory in order to get XBox Controller support.
