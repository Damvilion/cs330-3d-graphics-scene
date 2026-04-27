# Final Project Design Decisions

## Scene Overview
The scene is a simple desk workspace based on my Milestone One reference
image. It contains four distinct real-world objects sitting on a tiled
brick floor: a wooden desk, a laptop, a book, and a ceramic coffee mug.
The goal was to keep the polygon count low while still clearly
communicating the layout and mood of the reference photo.

## 3D Objects and Primitive Shapes
Four distinct primitive shape types appear in the scene, satisfying the
rubric requirement of at least four shapes from the approved list:

| Primitive  | Used For                                           |
|------------|----------------------------------------------------|
| Plane      | Ground floor                                       |
| Box        | Desk top, laptop base, laptop screen, book         |
| Cylinder   | Four desk legs, coffee mug body                    |
| Torus      | Coffee mug handle                                  |

Two of the four objects are complex (made of more than one primitive):

- **Desk** — one box (tabletop) plus four cylinders (legs).
- **Coffee mug** — one cylinder (body) plus one torus (handle), rotated
  90 degrees on Y so the ring faces outward from the side of the body.

All objects are kept well under 1,000 triangles each by reusing the
framework's low-poly primitive meshes.

## Object Placement
Objects are positioned in world space so they sit naturally on top of
one another instead of overlapping at the origin:

- The desk surface sits at Y = 3.0 with the four legs dropping to the
  floor.
- The laptop base and screen are anchored to the desk top (Y = 3.3),
  with the screen tilted back 20 degrees on the X axis.
- The book sits to the right of the laptop and is rotated 15 degrees
  on Y so it is not perfectly axis-aligned.
- The coffee mug sits to the left of the laptop with the torus handle
  offset on the +X side of the cylinder body.

## Textures
Five textures are loaded and applied with accurate UV mapping:

- `brick.jpg` on the floor plane, UV-scaled 8x8 to tile the pattern
  instead of stretching a single image across the plane.
- `wood.jpg` on the desk top.
- `stainless.jpg` on all four desk legs (labeled `metal`).
- `tilesf2.jpg` on the laptop base.
- `laptop.png` on the laptop screen (labeled `screen`).
- `abstract.jpg` on the book cover (labeled `book`).

The mug intentionally uses a solid white shader color plus the
`ceramic` material instead of a texture — the two-texture minimum is
already exceeded, and a plain white ceramic looks more realistic than
a stretched image on a cylinder.

## Materials and Lighting (Phong Model)
Seven materials are defined with distinct diffuse, specular, and
shininess values so each surface responds differently to light:
`brick`, `wood`, `metal`, `tiles`, `screen`, `book`, and `ceramic`.
Shininess ranges from 4 (matte brick) to 64 (polished metal legs).

Lighting uses all three Phong components (ambient, diffuse, specular)
and includes multiple sources so no part of any object goes completely
dark as the camera orbits:

- **Directional light** — simulates a raking sidelight from the right
  with a slight downward tilt.
- **Point light 0** — main key light on the right side at mid-height.
- **Point light 1** — cooler colored fill light on the left so the left
  faces of objects are not black. This satisfies the "at least one
  colored light" requirement.
- **Point light 2** — warm accent light near the front of the desk.
- **Point lights 3 and 4** — back and side fill lights for coverage
  from all camera angles.

## Navigation Controls
Camera input is split cleanly across the keyboard and mouse:

- **W / S** — move forward / backward
- **A / D** — strafe left / right
- **Q / E** — move down / up
- **Mouse movement** — pitch and yaw (look around without moving)
- **Mouse scroll wheel** — adjust movement speed
- **P** — switch to perspective projection
- **O** — switch to orthographic projection
- **Esc** — close the window

The orthographic branch overrides the view matrix with a front-facing
`lookAt` so the ground plane goes edge-on and the 2D silhouette of the
scene reads clearly.

## Code Organization and Best Practices
The project keeps rendering, viewing, and shader concerns in separate
classes:

- `SceneManager` owns textures, materials, lights, and the geometry
  render pass. Helper methods (`LoadSceneTextures`,
  `DefineObjectMaterials`, `SetupSceneLights`, `SetTransformations`,
  `SetShaderTexture`, `SetShaderMaterial`, `SetTextureUVScale`) are
  reused for every object so `RenderScene()` stays readable.
- `ViewManager` owns the camera, window, input callbacks, and the
  perspective/orthographic projection switch.
- `ShaderManager` wraps all the shader uniform calls.

Formatting uses consistent indentation and blocks of related
transformation code are grouped per object with a comment header
explaining what the block draws and why the textures/materials were
chosen. During cleanup I also fixed a bug in `DestroyGLTextures()`
where `glGenTextures` was mistakenly called instead of
`glDeleteTextures`, which was leaking every texture at shutdown.

## Reflection

**Why these objects?** The reference photo is a desk workspace, so a
desk, laptop, book, and mug recreate its mood with the fewest possible
primitives. Each object also teaches a different skill: the desk shows
a multi-primitive complex object, the laptop shows a tilted transform
and multi-texturing, the book shows a rotated standalone textured box,
and the mug shows combining a cylinder and torus into a recognizable
shape.

**How does a user navigate?** WASD plus QE gives full six-direction
translation, the mouse rotates the view in place, the scroll wheel
tunes movement speed so users can move fast across the scene and then
slow down for close inspection, and P / O toggle between perspective
and orthographic projection without moving the camera.

**Custom functions and reusability.** `SetTransformations` is the
backbone of the render loop — every draw call goes through it, so the
same function scales, rotates, and translates every primitive without
duplicating matrix math. `SetShaderTexture`, `SetShaderMaterial`, and
`SetTextureUVScale` let each object pick its own look in two lines of
code. `DefineObjectMaterials` and `LoadSceneTextures` isolate all the
asset setup in one place so adding a new object later is just a matter
of loading a texture, defining a material, and writing one render
block. This modular structure is what makes adding the book and the
coffee mug to the scene trivial, and it is directly reusable for any
future scenes on other projects.
