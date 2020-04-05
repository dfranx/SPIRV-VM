glslangValidator -G330 -Od -S frag -e main shader.glsl -o shader.spv
spirv-dis shader.spv -o shader.spvasm
