# Diffusion Expert

## Description
This project is on early status of Development. Expect more updates.  
It will be a fast interface for drawing with help of the stable diffusion.  
It uses a interface written in C++ with a embedded Python interpreter.  

*Input image*   
![Ubuntu screenshot](https://github.com/rodjjo/diffusion-expert/raw/main/docs/images/difusion-expert-pre-alpha.jpg)

*Result*   
![Ubuntu screenshot](https://github.com/rodjjo/diffusion-expert/raw/main/docs/images/difusion-expert-pre-alpha-2.jpg)

*Outpaint example*   
![Ubuntu screenshot](https://github.com/rodjjo/diffusion-expert/raw/main/docs/images/difusion-expert-pre-alpha-3.jpg)


*Input image for inpainting, image2image and controlnet*   
![Ubuntu screenshot](https://github.com/rodjjo/diffusion-expert/raw/main/docs/images/difusion-expert-pre-alpha-4.jpg)


## Installation video tutorial

Click on the video bellow:   
[![Installation Tutorial](http://img.youtube.com/vi/cv_RmXgOrjQ/0.jpg)](https://www.youtube.com/watch?v=cv_RmXgOrjQ)

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

## Credits
Licenses for borrowed code can be found in docs/licenses.html file.   
  
**Stable diffusion pipelines**  
Hugging Face 🤗
* [Diffusers](https://github.com/huggingface/diffusers)   
* [Diffusers-Docs](https://huggingface.co/docs/diffusers/index)   

**Model Conversion**  
* [ratwithacompiler](https://github.com/ratwithacompiler/diffusers_stablediff_conversion)

**Automatic11**  
* [stable-diffusion-webui](https://github.com/AUTOMATIC1111/stable-diffusion-webui)

**FLTK**
* [User-Interface](https://www.fltk.org/doc-1.3/)
