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
    * `gui.c`
    * `gui.h`
  * **renderer**
    * **host**
      * `render_manager.c`
      * `render_manager.h`
      * `materials.c`
      * `materials.h`
    * **device**
      * `renderer.cl`
      * `materials.cl`
      * `math.cl`
      * `rng.cl`
    * `renderer.h`
  * **renderer_device**
  * **lua**
    * `lua_runner.c`
    * `lua_runner.h`
    * `lua_wrapper.c`
    * `lua_wrapper.h`
  * `main.c`