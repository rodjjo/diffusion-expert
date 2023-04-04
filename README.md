# Diffusion Expert


## Build and run 

Before start make sure you have 7zip, cmake, python 3.10 and Conan 1.59.0 installed.  
The build.py script will download the toolset and all dependencies.

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