# TMX Map loader with normal mapping for Octet

This is a project for the Intro to Programming module in Goldsmiths's MsC in Computer Games and Entertainment.

It consists on a simple demo for rendering TMX map files (http://doc.mapeditor.org/reference/tmx-map-format/) which can be authored with an external editor called Tiled Editor (http://www.mapeditor.org/)

Essentially TMX files are an XML file defining 2D map properties based in tiles. These tiles are sorted in layers and can be drawn from different textures called tilesets.

In this project, I aimed to make the rendering as efficient as possible by saving as many drawcalls as possible.

	-Initially I drawed a quad for each tile, thus making one draw call for each tile which quickly degraded performance as the map increased size.
	
	-For the second iteration I took a different approach: A planar grid mesh is generated in runtime for each tileset defined in the tmx file. Then the UV coordinates for each vertex of this mesh are assigned depending on the tile.
	By doing this the program can render a map with just one draw call per tileset, however there are depth rendering issues.
	
	-As a future third iteration I will aim for a draw call per layer, using a multitexture shader which will take the different tilesets for that layer as parameters and this will fix the depth rendering issues while keeping the drawcall count low.


Although the project runs inside the Octet framework, the rendering is done directly in OpenGL.

For loading and parsing the TMX files I used the TinyXML library which is integrated in the Octet framework.

Currently the program supports only one point light, however support for more could be added easily.


