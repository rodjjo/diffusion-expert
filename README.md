# Diffusion Expert

## Description
This project is on early status of Development. Expect more updates.  
It will be a fast interface for drawing with help of the stable diffusion.  
It uses a interface written in C++ with a embedded Python interpreter.  

*Prompt*   
![Ubuntu screenshot](https://github.com/rodjjo/diffusion-expert/raw/main/docs/images/difusion-expert-pre-alpha.jpg)

*Images x Variations*   
![Ubuntu screenshot](https://github.com/rodjjo/diffusion-expert/raw/main/docs/images/difusion-expert-pre-alpha-2.jpg)

*Progress with image preview and cancel button*   
![Ubuntu screenshot](https://github.com/rodjjo/diffusion-expert/raw/main/docs/images/difusion-expert-pre-alpha-3.jpg)

## Build and run 

Before start make sure you have 7zip, cmake, python 3.10 and Conan 1.59.0 installed.  
The build.py script will download the toolset and all the dependencies.

Building the project
```bash
python devtools/build.py
```

Clear the build files and build it again
```bash
python devtools/rebuild.py
```

Run the project
```bash
# run:
python devtools/run.py --copy-stuff
# build and run:
python devtools/run.py --build --copy-stuff
```