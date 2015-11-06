# Physics Playground and Toon Shader (for Octet)

This is a demo project for the first assignment of **Tools and Middleware** module.

Mainly it focuses on the usage of bullet physics engine and its different types of **joints** and mouse picking using **raycast**.

It also integrates a lightweight **library** (AntTweakBar) for drawing some UI elements which provide useful debug information.

Finally a custom **fragment shader** has been developed to give the whole scene a cartoonish look using a light ramp.

The user can move around the scene using the following controls:

	- Move around: WASD
	
	- Move up and down: Q and E
	
	- Look around: Hold right mouse button
	
	- Left mouse button: click 2 objects to add a spring joint between them

By modifying the **data_mv.csv** the user can add spheres, boxes or chests in the specified positions.

Clicking on the **Shoot Ball** button will shoot spheres in the camera direction.
	
Build Instructions
------------------

The project requires the AntTweakBarBinaries to run. They can be downloaded from the official website or built from the project files included in the **octet/open_source** directory.