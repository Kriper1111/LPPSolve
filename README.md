# LPP - Show
*for when Blender isn't enough*

## About
This program aims to visualise three-dimensional linear programming problems: their allowed value ranges, solutions and limiting planes.
It *should* work with two-dimensional problems as well.

## Installation guide
In the (near) future, you'll be able to find latest binaries in releases, auto-built by GitHub CI. For now, you'll have to compile it manually.
Only Makefile workflow is suported for now.

Note:
When building on Windows, use Cygwin enviroment. Although the `mingw64`, `git-bash`, `GnuWin32` enviroment is supported by the Makefile, it don't have access to `cddlib`. In that case, you'll have to build it yourself. **Good luck.**

This project requires the following libraries for a release build:
 * [GLFW3](https://github.com/glfw/glfw): `libglfw3-dev`/`glfw3`
 * [GLM](https://github.com/g-truc/glm): `libglm-dev`/`glm`
 * [GLAD](https://github.com/Dav1dde/glad) OpenGL 3.3 Core headers and objects. They are included with the project. For manual acquiring, follow https://glad.dav1d.de/#profile=core&language=c&specification=gl&loader=on&api=gl%3D3.3.
 * [ImGui](https://github.com/ocornut/imgui/): `libimgui-dev`/`imgui`
 * QuickHull by akuukka: [sources](https://github.com/akuukka/quickhull), included as submodule.
 * cddlib: `libcdd-dev`/[sources](https://github.com/cddlib/cddlib)

These libraries are available in `vcpkg` repository on Windows. Their names are listed after the slash. If possible, acquire them from there.
If not, please put all required (`glfw3` and `cddlib`) static libraries into `libraries/` subfolder and third-party includes into `thirdparty/` subfolder.

Additionally, `OBJ_Loader.h` by [Bly7](https://github.com/Bly7/OBJ-Loader) is required for debug builds<sup>[1]</sup>.

## User guide
To enter the linear constraint system, use [Add Plane]
button and enter its coefficients into the newly added fields. Coefficients are as follows: `a1 a2 a3 b1`, in form of `Ax <= b`.
The objective function and its direction could be set by the respective field above.

Use [Solve] button to solve the given system. The result will be automatically visualized, if possible. Upon editing the system, the visualization will get erased.

You can control the visibility of axis guides and grids via corresponding checkboxes.

Keyboard controls:
| key  |  action   |
|------|-----------|
|[KeyA]|Orbit left |
|[KeyD]|Orbit right|
|[KeyW]|Orbit up   |
|[KeyS]|Orbit down |
|[KeyR]|Zoom in<sup>[1]</sup>    |
|[KeyF]|Zoom out<sup>[1]</sup>   |

## Credit<sup>[1]</sup>s, Licenses, Afterthoughts
This program uses and relies on other open-source libraries. you can check their respective licenses in their repositories, linked in the dependency list.
This program uses generated GLAD OpenGL bindings (at least I think that's the term). They fall under CC0 license, except for certain parts of Khronos EGL specs, which fall under the MIT License.

I've yet to decide on the license to use myself, but I'll probably stick to the MIT License.

For features pending implementation refer to [`TODO`](./TODO) and probably GitHub issues.

The `assets.cpp` and `camera.cpp` are borrowed from a not-yet-and-probably-wont-be-ever released GLFW/OpenGL game/wrapper/engine of mine. If I ever to release MCTE, you'll find some similarities there.

<sup>1</sup> - Not really relevant as of writing this README for multiple reasons:
* While we do have different build configurations, for the time of being the "debug" one is enforced.
* The solution itself -- the vector and quite possibly the plane aren't rendered just yet. We do print the numbers though.
* The zooming functionality is not implemented on the input level just yet. You could manually tweak it via a respective slider on the Camera Controls debug menu, but nothing more than that.

