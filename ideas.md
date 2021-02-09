# Renderer usage and information

## Usage

### Main functions

1. **Initialisation**

   * `Renderer *renderer = create_renderer(fileName, kernelName);`

     Initialise OpenCL platform, device, context, command queue, program and kernel.

2. **Image setup**

   * `set_image_properties(renderer, width, height, maxRayDepth);`

     Allocate memory for image and setup a OpenCL buffer for image exchange with the kernel.

3. **Main loop**

   1. **Scene setup**

      * `set_background_color(renderer, color);`

        Set the background color of the scene.

      * `Material material = create_material(type, r, g, b, tint, fuzzyness, refIdx);`

        Create material structure.

      * `add_sphere(renderer, x, y, z, radius, material);`

        Add spheres to the scene.

      * `set_camera_properties(renderer, pos, rot, sensorWidth, focalLength, aperture, exposure);`

        Set camera properties.

   2. **Render scene**

      * `Image *image = render(renderer, samples);`

        Render requested number of samples and return a image. An OpenCL buffer for spheres is  setup and destroyed after rendering.

4. **Cleanup**

   * `destroy_renderer(renderer);`

     Destroy image buffer, free image and sphereList, and release OpenCL components.

### Other functions

* `write_image(image, fileName);`

  Write image returned from `render`, `stb_image_write` wrapper.

* `render_to_file(renderer, samples, fileName);`

  `render` replacement.

  ## Structures

* `CLProgram`

  ```c
  struct CLProgram {
      // initialise at the beginning of the program (once)
  	cl_platform_id platform;
  	cl_device_id device;
  	cl_context context;
  	cl_command_queue queue;
  	cl_program program;
  	cl_kernel kernel;
  	
      // initialise with image setup
  	cl_mem imageBuff;
      
      // initialise when rendering if size has changed
  	cl_mem sphereBuff;
  }
  ```

* `ImageInfo`

  ```c
  struct ImageInfo {
      cl_int2 size;
      cl_int maxRayDepth;
  }
  ```

* `Scene`

  ```c
  struct SceneInfo {
      cl_float3 color;
      cl_int sphereCount;
  }
  ```

* `Sphere`

   ```c
   struct Sphere {
   	cl_float3 center;
   	cl_float radius;
   	Material material;
   }
   ```

* `Material`

   ```c
   struct Material {
   	cl_int type;
   	cl_float3 color;
   	cl_float tint;
   	cl_float fuzzyness;
   	cl_float refIdx;
   }
   ```

* `Camera`

   ```c
   struct Camera {
       cl_float3 pos;
   	cl_float3 rot;
   	cl_float sensorWidth;
   	cl_float focalLength;
   	cl_float aperture;
   	cl_float exposure;
   }
   ```

* `Renderer`

   ```c
  struct Renderer {
  	CLProgram clProgram;
  	ImageInfo imageInfo;
      cl_float3 *image;
      SceneInfo sceneInfo;
      Sphere *sphereList;
      Camera camera;
  }
  ```
  
* `Image`

   ```c
   // for use with stb_image_write
   struct Image {
       int width;
       int height;
       
       // r1, g1, b1, r2, g2, b2, ...
       unsigned char *data;
   }
   ```

   