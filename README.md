# What is Charcoal?
**Charcoal** is an open-source Windows Desktop Application designed for experimenting with real-time computer graphics, procedural visual effects, motion data, physical sensors, audio, video, and spatial interaction within a focused desktop application and hardware-facing environment.

Rather than attempting to replace general-purpose engines such as Unity or Unreal Engine, Charcoal concentrates on a narrower workflow; receiving digital or real-world input, transforming that data into interactive graphics, and capturing the resulting visual performance.

__Main Features__
- Rendering 2D and 3D Graphics through DirectX 12.
- Loading scenes, meshes, textures, materials, and externally created assets.
- Providing basic scene controls through a desktop editor interface.
- Visualizing tracked points, skeletons, motion-capture data, and sensor information.
- Providing procedural effects such as motion trails, heat signatures, particles, point clouds, and audio-reactive visuals.
- Receiving data from devices such as Arduino Boards, Raspberry Pi systems, depth sensors, infrared cameras, and XR trackers.
- Playing and capturing audio and video.
- Exporting screenshots, image sequences, recordings, and eventually synchronized media files.
- Recording and replaying motion or sensor sessions for testing and creative use.

External applications remain responsible for specialized content creation. Maya can be used for modeling and animation, Krita for textures and interface artwork, and dedicated media software for advanced editing and conversion. Charcoal imports and combines those assets rather than recreating every professional production tool internally.

The project is both a graphics-engine learning environment and a usable creative laboratory. Its development provides practical experience with rendering architecture, GPU programming, linear algebra, asset pipelines, performance optimization, hardware communication, real-time media, and application development.

## PIPELINE
Input → Processing → Real-Time Visualization → Capture or Export

Input may come from either a scene file, audio track, pre-recorded motion session, live sensor, XR tracker, or another connected device. Charcoal processes that information and represents it through configurable spatial and procedural graphics.

The intended result is not a universal game engine. It is a specialized tool for creating, testing, displaying, and recording sensor-driven and audiovisual graphics experiments.

```
Programming Languages: C++, HLSL, JSON
Graphics & Systems APIs: DirectX 12, DXGI, Win32 API, COM
Frameworks: MFC
Libraries: DirectXMath, Assimp, Dear ImGui, JSON Parser, Microsoft WRL
SDK & Toolchain: Windows SDK, MSVC C++ Toolchain, Visual Studio C++ Desktop, FXC/DXC
```

# Summary: Charcoal turns motion, media, and sensor data into real-time spatial graphics.
