# **D-Viz: A 3-D Directory Visualizer**

Using this **Qt** and **OpenGL** based application, users can recursively scan their filesytems (or parts thereof) to generate a three dimensional visualization of the directory structure, where nested files and folders are represented as blocks that stack on one another to create a virtual "cityscape." Once the visualization has been generated, users can fly around the scene using the familiar WASD + mouse movement scheme common to practically all modern videogames. Additionally, D-Viz also provides basic gamepad support. Note that all testing has been done with a wired Xbox 360 controller. The use of the controller greatly improves the navigation experience; especially when transitioning from macro to micro exploration.

The primary visualization is based on the well known "squarification" technique outlined in the paper **Squarified Treemaps**, written by Mark Bruls, Kees Huizing, and Jarke van Wijk. This technique aims to keep block aspect ratios as close to one as possible.

For information on how to set up and build the project (on Linux), please see `.github/workflows/ccpp.yml`.

# Screenshots

![Example 1](/Screenshots/2020/D-Viz-1.png)
<p style="text-align:center; margin:-5px 50px 32px 50px;">
    <span style="font-weight:bold; margin-right:10px;">Figure 1:</span>
    The yellow tile is the currently selected file, whose path is shown in the status bar. Only those files that smaller than 1 mebibyte.
</p>

![Example 2](/Screenshots/2020/D-Viz-2.png)
<p style="text-align:center; margin:-5px 50px 32px 50px;">
    <span style="font-weight:bold; margin-right:10px;">Figure 2:</span>
    The green tiles represent files, while the white nodes represent directories.
</p>

![Example 3](/Screenshots/2020/D-Viz-3.png)
<p style="text-align:center; margin:-5px 50px 32px 50px;">
    <span style="font-weight:bold; margin-right:10px;">Figure 3:</span>
    The yellow tile is the currently selected file, whose path is shown in the status bar. Only those files that smaller than 1 mebibyte.
</p>

![Example 4](/Screenshots/2020/D-Viz-4.png)
<p style="text-align:center; margin:-5px 50px 32px 50px;">
    <span style="font-weight:bold; margin-right:10px;">Figure 4:</span>
    No file is current selected, but all files of a particular type (i.e., with a identical file extension) are shows as grey highlights.
</p>

![Example 4](/Screenshots/2020/D-Viz-5.png)
<p style="text-align:center; margin:-5px 50px 32px 50px;">
    <span style="font-weight:bold; margin-right:10px;">Figure 5:</span>
    The file system can also be actively monitored for changes. In this case, the files highlighted in pastel red have been deleted since the scan was performed.
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

* Development and testing were done on Windows 10 and Ubuntu 18.04. An NVIDIA GeForce GTX 1070 was used for the majority of this work, and this is sufficient to visualize up to about one million files. For filesystems that exceed this file count, the pruning menu can be used to filter out files smaller than a given amount.
