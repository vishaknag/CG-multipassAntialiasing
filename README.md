CG-multipassAntialiasing
========================

Performs Multipass Antialiasing.  The algorithm computes multiple samples distributed around each display pixel and weights (filters) these into one final value

* A screen space offset is added to the transformed vertex coordinates of each triangle to produce the sample distribution.

* A 6x3 table specifies the filter offsets and weights. Offsets are fractional pixel shifts and the sum of all the weights = 1.0. 

* Application sets up multiple Renderers and Displays -- one pair for each sample in the table.  

* Each Renderer is initialized with a different offset and display to write into.  

* All triangles are sent to all renderers.  

* After rendering all the triangles, the application combines the multiple display images by weighting their pixel values.  

* The filtered (result) image is stored in one of the displays (or a new one) and then written to disk.  

* Also the filtered image is stored in the frame buffer to window draw.