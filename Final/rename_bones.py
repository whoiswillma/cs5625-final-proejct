from pygltflib import GLTF2
INPUT_FILE = "../resources/scenes/final_test.glb"
OUTPUT_FILE = "../resources/scenes/final_final.glb"
gltf = GLTF2.load(INPUT_FILE)
count = 1
for node in gltf.nodes:
    if node.name[:4] == "Bone":
        node.name = node.name + str(count)
        count += 1
gltf.save(OUTPUT_FILE)
