# **D-Viz: A 3D Directory Visualizer**

Using this **Qt** and **OpenGL** based application, users can recursively scan filesystems (or parts thereof) to generate a three dimensional visualization of the directory structure, where nested files and folders are represented as blocks that stack on one another to create a virtual "cityscape." Once the visualization has been generated, users can fly around the scene using the familiar WASD + mouse movement scheme common to practically all modern videogames. Additionally, D-Viz also provides basic gamepad support; to date, all testing has been done with a wired Xbox 360 controller. The use of the gamepad greatly improves the navigation experience, especially when transitioning from macro to micro exploration.

The primary visualization is based on the well known "squarification" technique outlined in the paper [Squarified Treemaps](https://www.win.tue.nl/~vanwijk/stm.pdf), written by Mark Bruls, Kees Huizing, and Jarke van Wijk. This technique aims to keep block aspect ratios as close to one as possible.

For information on how to set up and build the project (on Linux), please see `.github/workflows/ccpp.yml`.

# Screenshots

![Example 1](/Screenshots/2020/D-Viz-1.png)
<p align="center">
    <em style="font-weight:bold; margin-right:10px;">Figure 1:</em>
    The yellow tile is the currently selected file, whose path and size is shown in the status bar. Only those files larger than one mebibyte are rendered, as per the settings.
</p>
<br/>

![Example 2](/Screenshots/2020/D-Viz-2.png)
<p align="center">
    <em style="font-weight:bold">Figure 2:</em>
    The green tiles represent files, while the white tiles represent directories.
</p>
<br/>

![Example 3](/Screenshots/2020/D-Viz-3.png)
<p align="center">
    <em style="font-weight:bold; margin-right:10px;">Figure 3:</em>
    The yellow tile is currently selected, and all visible DLLs are highlighted in grey.
</p>
<br/>

![Example 4](/Screenshots/2020/D-Viz-4.png)
<p align="center">
    <em style="font-weight:bold; margin-right:10px;">Figure 4:</em>
    No file is currently selected, but a particular file type is currently highlighted in grey.
</p>
<br/>

![Example 4](/Screenshots/2020/D-Viz-5.png)
<p align="center">
    <em style="font-weight:bold; margin-right:10px;">Figure 5:</em>
    The filesystem can also be actively monitored for changes. In this case, the files highlighted in pastel red have been deleted since the scan was performed.
</p>

# Key Bindings

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

# Notes

* This is a hobby project that should not be taken too seriously.
* The project currently requires Qt 5.15 or higher.
* Development and testing were done on Windows 10 and Ubuntu 18.04. An NVIDIA GeForce GTX 1070 was used for the majority of this work, and can comfortably visualize up to about one million nodes. For filesystems that exceed this file count, the sidebar can be used to filter out files smaller than a specified amount.

