# To-do

## Basic functions

* [ ] **Basic voxel rendering**
  * [ ] Color-less rendering
  * [ ] Materials
  * [ ] Basic file format
  * [ ] Cleanup OpenCL code
  * [ ] Cleanup C code
  * [ ]  Re-do lua interface
* [ ] **Basic GUI**
  * [ ] Render to screen
  * [ ] Movement
* [ ] **Basic editor**
  * [ ] Place/remove blocks
  * [ ] Material selector
* [ ] **Better file format**
  * [ ] Chunks
  * [ ] Convert from .vox

## Better editor

...

# Project structure

* **src**
  * **gui**
    * `gui.c` / `gui.h`
  * **renderer**
    * **kernel**
      * `main.cl`
      * `materials.cl`
      * `math.cl`
      * `rng.cl`
    * `renderer.h`
    * `kernel_manger.c` / `kernel_manager.h`
    * `scene_manger.c` / `scene_manager.h`
    * `material_manger.c` / `material_manager.h`
    * `camera_manger.c` / `camera_manager.h`
  * **lua**
    * `lua_runner.c` / `lua_runner.h`
    * `lua_wrapper.c` / `lua_wrapper.h`
  * `main.c`