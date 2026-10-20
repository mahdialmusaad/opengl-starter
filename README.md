OpenGL triangle project for [almu.uk](https://almu.uk).
See the full article explaining all the code [here](https://almu.uk/blogs/creating-programs-with-opengl).

If you encounter issues with reading the vertex and fragment shader files, make sure they are in the current working directory, usually where the executable is located.

## Building
This repository uses submodules to retrieve GLFW and vmlib:
```bash
git clone https://github.com/mahdialmusaad/opengl-starter --recurse-submodules
cd opengl-starter
cmake -B build
cmake --build build
```
The program executable can be found in the `build` directory.
