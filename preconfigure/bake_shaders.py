import sys as sus
import pathlib
import re

from tools import collect_args, print_err

UNSAFE = re.compile(r"^[\d]+|[^\w]")


def write_shaders(out, struct_name, shaders):
    out.write("    struct {\n")
    for shader_name, _, shader_path in shaders:
        name = re.sub(UNSAFE, "_", shader_name)
        out.write(f'        const char* {name} = R"(\n')
        with open(shader_path, encoding="utf-8") as shader:
            out.write(shader.read())
        out.write('\n        )";\n\n')
    out.write(f"    }} {struct_name};\n")

def collect_shaders(file_list):
    for shader in filter(lambda file: file.endswith((".frag", ".vert")), file_list):
        shader_path = pathlib.Path(shader)
        if not shader_path.exists():
            print_err("Inaccessible file: ", shader_path)
            continue

        shader_name = shader_path.name
        shader_type = shader_path.suffix.strip(".")
        yield (shader_name, shader_type, shader_path)
        # shaders.append((shader_name, shader_type, shader_path))

if __name__ == "__main__":
    args = collect_args()
    if not args: exit(1)
    infiles, outfile = args
    shaders = list(collect_shaders(infiles))

    print("Baking:", *infiles)

    vertex_shaders = filter(lambda shader: shader[1] == "vert", shaders)
    fragment_shaders = filter(lambda shader: shader[1] == "frag", shaders)

    with open(outfile, "w", encoding="utf8") as out:
        # out.write("#pragma once\n\n")
        out.write("#ifndef _BAKED_SHADERS_H\n")
        out.write("#define _BAKED_SHADERS_H\n\n")
        out.write("namespace shaders {\n")

        write_shaders(out, "vertex", vertex_shaders)
        write_shaders(out, "fragment", fragment_shaders)

        # out.write("    struct {\n")
        # for shader_name, _, shader_path in vertex_shaders:
        #     name = re.sub(UNSAFE, "_", shader_name)
        #     out.write(f'        const char* {name} = R("\n')
        #     with open(shader_path, encoding="utf-8") as shader:
        #         out.write(shader.read())
        #     out.write( '        ");\n\n')
        # out.write("    } vertex;\n")

        # out.write("    struct {\n")
        # for shader_name, _, shader_path in fragment_shaders:
        #     name = re.sub(UNSAFE, "_", shader_name)
        #     out.write(f'        const char* {name} = R("\n')
        #     with open(shader_path, encoding="utf-8") as shader:
        #         out.write(shader.read())
        #     out.write( '        ");\n\n')
        # out.write("    } fragment;\n")
        out.write("};\n\n")
        out.write("#endif")
