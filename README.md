# CS-330
CS-330-15594-M01 Comp Graphic and Visualization

I learned how to manipulate 3D shapes to create static scenes in this course. I first had to look at the rubric requirements and then the announcement posts and emails to make sure what I created fit the rubric to the best of my ability. I had no prior experience with graphic rendering so this course taught me how to work with OPENGL and tools like that to render 3D scenes. I even briefly learned how to make a brick-shooter style game for my module 8 assignment.

I struggled the most with object transforming and making sure elements were properly rotated so they did not float. At some angles my setup appeared fine, but when I moved the camera, it looked strange. I do not want to pursue a career in something like game design or anything heavily involving 3D rendering, but I remembered looking at a demo of a street map before, so I think I could use these skills if I ever needed to create a navigation-like view that is similar to a GPS but more interactive so you can actually visualize a street at a top-level/less detailed view.


Github did not allow me to upload any files above 25MB, so I was not able to upload my final project zip. So I simply just uploaded the individual files for my project. I was not able to upload my docx file at all for my final Project docx for some reason, so I tried to upload PDF version that also didnt work, so I am just pasting what I had in the doc below:






Faith Agepogu 

CS 330 

Apr 20, 2026 

7-1 Final Project Reflection 

### Justify development choices for your 3D scene. Think about why you chose your selected objects. Also consider how you were able to program for the required functionality. 

To confirm, this was the picture I chose to replicate: 


For this assignment, I was instructed to pick an image to choose specific objects to replicate. I had to replicate them as closely as possible, but they didn't have to be completely identical (so I could pick whatever textures I wanted, for example). I chose to replicate the laptop, both vases, and both book stacks beside the laptop. Since I was required to make a certain number of objects, I decided to condense the right book stack to just a single notebook. To rehash how I constructed them: 

Laptop: 

Flattened vertical box can be used for the top part. 

Flattened horizontal box for the bottom part. 

Two cylinders for the hinges. 

Plane applied image for the screen. 

Flattened box with complex texturing for the keyboard surface. 

Textures and plastic materials applied to the laptop lid. 

 

Left Vase: 

Upside down thin tapered cylinder for the bottom taper. 

Tall cylinder for the main part. 

Tapered cylinder for the top taper. 

Flattened torus for the top ridge. 

 

Right Vase: 

Thin upside-down tapered cylinder for the bottom taper. 

Regular cylinder for the main part. 

Elongated prism for the spout. 

Torus for the top ridge. 

Half-torus for the handle. 

 

Left Book Stack: 

Advice I was given was: 

Three thin boxes to replicate the covers (top, bottom, spine) 

Fatter box for the inside pages. 

But the final result looks more like two separate objects/defined books. 

 

Right Notebook: 

Thin tori aligned in a row to replicate the spiral binding. 

Fat box for paper stack. 

Thin box for notebook cover at the bottom. 

One adjustment I made was to rotate the notebook so it was facing the viewer, because it made more sense to me if the book was facing straight, not the other way around.  

 

### Explain how a user can navigate your 3D scene. Explain how you set up to control the virtual camera for your 3D scene using different input devices. 

Navigation is done with WASD, Q and E. 

W = forward 

A = move left 

S = move back 

D = move right 

Q = go up 

E = go down 

OPENGL has a feature to enable movement controls. I simply had to refer to the sample project to see how it was implemented to replicate in my project, HOWEVER the original sample had a different navigation method. It used number based shortcuts to pan to specific angles of the scene, but the new orthographic view I had to implement provided less specific angles (simply 2 view modes and then WASD, Q & E to move). 

 

### Explain the custom functions in your program that you are using to make your code more modular and organized. Ask yourself, what does the function you developed do and how is it reusable?

I am unsure what this part means, but I did try to make my code modular in some ways. For example, for the left book stack, I left it under the same function so it was organized instead of making it render separately. I also used similar bases for the vases but tweaked the right one as needed since it did not require a tapered top, plus it had extras like a handle and spout. 
