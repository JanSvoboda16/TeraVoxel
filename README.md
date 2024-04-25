# TeraVoxel

 Author: Jan Svoboda

 Email: jan.svoboda16@seznam.cz

## What TeraVoxel is?

TeraVoxel is an open-source software designed for real-time displaying of large volumetric datasets on CPU using client-server architecture. 

The server is used as a remote storage of volumetric datasets that are split into small blocks. The client application controls the server and retrieves volumetric data from it according to the current need. The client application displays this data on CPU.

TeraVoxel currently supports simple ray-casting visualization with optional shading and interpolation.

![Screenshot](/doc/images/windows.png)

## Architecture 
### Server (ASP .NET Core 7.0)
### Client (C++ 20)

## Minimum Requirements to Run
### Client
* Operating System: Windows 10/11
* Processor architecture: x86 or x64 (not ARM)

### Server
* Installed .NET 7.0 runtime and ASP.NET Core 7.0 runtime, I recommend installing the .NET 7.0 SDK, which includes everything you need from: https://dotnet.microsoft.com/en-us/download, but you can also install both separately from: https://dotnet.microsoft.com/en-us/download/dotnet/7.0. 

## Supported Formats
* NIFTI file
* TIFF sequence (zipped in .zip archive without any subfolders - 3D TIFF not supported)

## Startup and Configuration 
### Server configuration 
In "appsettings.json" fill in the "StoragePath" which represents the directory where data will be stored. You can also edit "SegmentSize" which represents the size of one voxel block.
The default value is 256 and this value is optimal. The "CompressionLevel" parameter affects the lossless compression level of individual block files. Too high a compression level can slow down processing considerably. 

### Server Compiling
Depend on your deployment. For local usage just build on the release profile.

### Client Compiling
Build on the release profile (IMPORTANT, debug is veeery slow) and run .exe file.

## Application controls
### Connecting to the server
In the window "Projects" fill in the URL of the server (for example: "localhost:5000") and click on the button "Connect". You should see the panel for project creation and all projects on the server if exist.

### Creating project
Fill in the "Project name" field and click on the "Create" button. You should see your project in the "Projects" table. Insert the source file path into the "File to upload" field and click on the "Upload" button. When the file is uploaded the "Build" button should appear. Click on this button and project building will start. After the project is built you should see the "Load" button. A project can be deleted by clicking on the "Delete" button. 

### Project view
Click on the "Load" button and in the window "Visualizer Settings" select a visualizer. The default visualizer is the "Fast RC" visualizer. When using this visualizer you have to create a mapping table or load some existing one. To select an existing mapping table just click the "Load" button in the "Existing Tables" table. You can also delete this table. 
When the table is loaded you should see table parameters in the "Mapping Table" table in "Color Mapping Editor". 

Mapping tables contain rows that represent linear interpolation of the two colors between values of the dataset. For example, values 200 - 500 can represent bone which will be represented by shades of grey. You 

If you want to create a new table just add rows or edit values in the empty or existing Mapping table, fill in the "File name" field, and click on the "Save" button.

For good-quality visualization, you can select the "Interpolate + shading" option which uses the Phong lighting model. This visualization is much slower. You can edit all parameters for this lighting model.

## Scene controls
* Left mouse button + move: rotate
* Right mouse button + vertical movement: shift the center of rotation in the camera axis
* Wheel - click + movement: shift the center of rotation in the plane of view
* Wheel - rotation: zoom in/out
* Button "V" - change view

### Settings 
In the "Settings" window you can edit all rendering parameters according to the parameters of your computer. 

