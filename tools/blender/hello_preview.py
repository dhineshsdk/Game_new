"""Bridge smoke test: builds a tiny scene in headless Blender and renders a preview PNG.

Run with tools/blender/run_blender.ps1. Output: build/previews/blender_hello.png
"""
import os
import sys

import bpy

root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
out_path = os.path.join(root, "build", "previews", "blender_hello.png")

bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene

bpy.ops.mesh.primitive_cube_add(size=2, location=(0, 0, 1))
bpy.ops.mesh.primitive_plane_add(size=10, location=(0, 0, 0))

cam_data = bpy.data.cameras.new("Cam")
cam = bpy.data.objects.new("Cam", cam_data)
scene.collection.objects.link(cam)
cam.location = (6, -6, 4)
cam.rotation_euler = (1.1, 0, 0.785)
scene.camera = cam

light_data = bpy.data.lights.new("Sun", type="SUN")
light = bpy.data.objects.new("Sun", light_data)
scene.collection.objects.link(light)
light.rotation_euler = (0.8, 0.2, 0.6)

scene.render.engine = "BLENDER_WORKBENCH"
scene.render.resolution_x = 640
scene.render.resolution_y = 360
scene.render.filepath = out_path
bpy.ops.render.render(write_still=True)

print("CODEX: blender_version=" + bpy.app.version_string)
print("CODEX: objects=" + ",".join(o.name for o in scene.objects))
print("CODEX: preview=" + out_path + " exists=" + str(os.path.exists(out_path)))
