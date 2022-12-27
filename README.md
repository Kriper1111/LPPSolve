# LPP - Show
*for when Blender isn't enough*

## About
This program aims to visualise three-dimensional linear programming problems: their feasible ranges, optimal plans and limiting planes.
It *should* work with two-dimensional problems as well.

## Installation guide
You should find latest release in the "Releases" section on the right, as well as in recent "Actions" artifacts.
If you don't trust the runners (and frankly I don't either), you could build it from source.

### Building
Makefile and CMake workflows are supported now.

Note:
When building on Windows, use Cygwin enviroment. Although the `mingw64`, `git-bash`, `GnuWin32` enviroment is supported by the Makefile, it don't have access to `cddlib`. In that case, you'll have to build it yourself. **Good luck.**

This project requires the following libraries for a release build:
 * [GLFW3](https://github.com/glfw/glfw): `libglfw3-dev`/`glfw3`
 * [GLM](https://github.com/g-truc/glm): `libglm-dev`/`glm`
 * [GLAD](https://github.com/Dav1dde/glad) OpenGL 3.3 Core headers and objects. They are included with the project. For manual acquiring, follow https://glad.dav1d.de/#profile=core&language=c&specification=gl&loader=on&api=gl%3D3.3.
 * [ImGui](https://github.com/ocornut/imgui/): `libimgui-dev`/`imgui`
 * QuickHull by akuukka: [sources](https://github.com/akuukka/quickhull), included as submodule.
 * moFileReader by AnoterFoxGuy: [sources](https://github.com/AnotherFoxGuy/MofileReader), only the header from releases is required.
 * cddlib: `libcdd-dev`/[sources](https://github.com/cddlib/cddlib)

~~These libraries are available in `vcpkg` repository on Windows. Their names are listed after the slash. If you really want so, acquire them from there. Keep in mind, you will have to specify vcpkg's toolchain file for CMake in this case, or scout its files for downloaded headers and libraries to put into `thirdparty/` and `libraries/` respectively.~~
The CMake recipe will now fetch required libraries from source, if building on Windows (but not ImGui on Linux). This is the preferred way. Otherwise, you can locate and put all required (`glfw3` and `cddlib`) static libraries into `libraries/` subfolder and third-party includes (`glfw3`, `glm`, `OBJ_Loader.h` and `cddlib`) into `thirdparty/` subfolder. Please make sure to keep some sort of folder structure and leave each dependency, except for `OBJ_Loader.h`, its own folder.

Additionally, `OBJ_Loader.h` by [Bly7](https://github.com/Bly7/OBJ-Loader) is required for debug builds<sup>[1]</sup>.

## User guide
To add a new linear constraint system, use [Add Plane] or [+] buttons on the left.
Then, fill out freshly appeared row in the constraint table. Note that coefficients are summed by default, so only include minus sign.
To change inequality type, click the "<=" field and choose from drop down.
To remove the (in)equality, use the [x] button in the leftmost column.
To toggle its visibility, use the checkbox in the rightmost column.
The objective function and its direction could be set by the respective field on the top.

Use [Solve] button to solve the given system. The result will be automatically visualized, if possible.
Note, that by default the optimal plan vector will be occluded by the feasible range. To disable its display (and configure visibility of other elements), open preferences with "File" -> "Preferences" and navigate to "Visibility" tab.

Keyboard controls:
| key  |  action   |
|------|-----------|
|[KeyLeftArrow]|Orbit left |
|[KeyRightArrow]|Orbit right|
|[KeyUpArrow]|Orbit up   |
|[KeyDownArrow]|Orbit down |
|[KeyW]|Move forwards without changing height|
|[KeyS]|Move backwards without changing height|
|[KeyA]|Move to the left without changing height|
|[KeyS]|Move to the right without changing height|
|[KeyR]|Zoom in|
|[KeyF]|Zoom out|

## Credits, Licenses, Afterthoughts
This program uses and relies on other open-source libraries. you can check their respective licenses in their repositories, linked in the dependency list.
This program uses generated GLAD OpenGL bindings (at least I think that's the term). They fall under CC0 license, except for certain parts of Khronos EGL specs, which fall under the MIT License.
This program uses (and at some point will embed) DejaVu fonts. You can find their license at https://dejavu-fonts.github.io/License.html.

I've yet to decide on the license to use myself, but I'll probably stick to the MIT License.

For features pending implementation refer to [`TODO`](./TODO) and probably GitHub issues.

The `assets.cpp` and `camera.cpp` are borrowed from a not-yet-and-probably-wont-be-ever released GLFW/OpenGL game/wrapper/engine of mine. If I ever to release MCTE, you'll find some similarities there.

<sup>1</sup> - Not really relevant as of writing this README for multiple reasons:
* While we do have different build configurations, for the time of being the "debug" one is enforced in Makefile script. Additionally, the runners use "RelWithDebInfo" configuration, which may require some extra libraries.
Once I'm confident enough, I'll switch those to Release mode. CMake could be configured to use any build configuration.
