# Charcoal — ALPHA 0.30

![[update-snapshot-0_30.png]]
**Project:** Charcoal  
**Version:** ALPHA 0.30 — Foundational Build  
**Application Type:** Real-Time Computer Graphics Desktop Application  
**Primary Platform:** Windows  
**Graphics API:** DirectX 12  
**Primary Languages:** C++, HLSL  
**Development Status:** Alpha

---
### 1.1 Project Description

**Charcoal** is a Windows desktop application designed for experimenting with real-time computer graphics, procedural visual effects, motion data, physical sensors, audio, video, and spatial interaction.

The application provides a dedicated environment for receiving digital or real-world input, processing that information, and transforming it into interactive graphical output. Its long-term purpose is to support graphics experimentation, visualization, and hardware-integrated experiences within a unified desktop environment.

Unlike general-purpose game engines such as Unity or Unreal Engine, Charcoal is not intended to provide a complete game development framework. Instead, it focuses on graphics rendering, data-driven visualization, spatial interaction, and direct integration with external systems.

Charcoal is being developed as an open-source project, with a public release planned as development progresses toward version 1.00.

### 1.2 Purpose of ALPHA 0.30

ALPHA 0.30, designated **Foundational Build**, focuses on establishing the core rendering capabilities necessary for future development.

Previous iterations primarily concentrated on initializing the application, establishing communication with the graphics API, and introducing basic rendering functionality.

ALPHA 0.30 expands upon this foundation by implementing and integrating several systems required to render more complex three-dimensional scenes.

__The primary objectives of this version include:__
- Rendering three-dimensional geometry using DirectX 12.
- Supporting world, view, and projection transformations.
- Implementing depth testing for correct spatial rendering.
- Introducing directional, point, and spot lighting.
- Supporting material properties and surface textures.
- Exploring normal, bump, and height-based surface detailing.
- Introducing emissive materials and bloom post-processing.
- Improving debugging, logging, and application stability.
- Preparing the project for documentation, demonstration, and public distribution.

The objective is not to deliver a complete graphics production environment, but rather to establish a functional rendering foundation that can support more advanced systems in subsequent versions.

### 1.3 Application Architecture Summary

Charcoal is organized into two principal Visual Studio projects:

**ShaderApp**

The Windows desktop application responsible for initializing and managing the application environment, providing user-facing controls, and coordinating rendering operations.

**dx12lib**

A C++ dynamic-link library containing reusable DirectX 12 rendering functionality, including graphics resources, command execution, and supporting rendering infrastructure.

This separation distinguishes application-specific behavior from lower-level graphics operations and provides a foundation for future architectural improvements.

The rendering implementation uses DirectX 12 to communicate with compatible graphics hardware, while shaders written in HLSL perform GPU-based graphical computations.

More detailed information about component responsibilities, dependencies, and communication is provided in the _Architecture_ documentation.

### 1.4 Rendering Capabilities

ALPHA 0.30 introduces several graphical capabilities that collectively support the rendering of three-dimensional environments.

##### Geometry and Spatial Transformations

Three-dimensional objects are represented using vertex and index data. Transformation matrices control object placement, orientation, and scale, while camera transformations determine the perspective from which a scene is rendered.

Depth testing ensures that visible surfaces are correctly resolved according to their spatial relationship with the camera.

![[figure1-4-1.png]]
Figure 1.4.1: Initial Hex Field rendering featuring repeated three-dimensional geometry, dark surface, materials, and high-contrast red illumination.

##### Lighting

The rendering system incorporates multiple lighting models to simulate illumination within a scene.

Directional lights provide illumination originating from a specified direction, point lights emit illumination from a position in three-dimensional space, and spot lights produce directional illumination within a defined region.

These systems allow the renderer to combine different lighting sources to produce more complex visual results.

![[figure1-4-2.png]]
Figure 1.4.2: Lighting Demonstration highlighting variations in surface illumination and the interaction between light and surrounding hexagonal geometry,

##### Materials and Texture

Materials control visual properties such as surface coloration and interactions with lighting.

Texture sampling expands these capabilities through image-based surface information, including albedo, normal, and height-related textures.

These features allow geometric surfaces to display visual detail beyond their underlying mesh structure.

![[figure1-4-3.png]]
Figure 1.4.3: Early surface-detail demonstration showing textured hexagonal geometry under localized illumination in a dark environment.

![[figure1-4-4.png]]
Figure 1.4.4: Material rendering test demonstrating subtle surface texture variations across the hexagonal structures, in this case, only the Normal Mapping is enabled.

![[figure1-4-5.png]]
Figure 1.4.5: The same scene as Figure 1.4.4 but instead of Normal Mapping, the Bump Mapping is solely enabled.

![[figure1-4-6.png]]
Figure 1.4.6: Normal Mapping and Bump Mapping both enabled in this render.

**Emissive Materials and Post-Processing**

Emissive material properties allow objects to visually represent light-emitting surfaces.

Bloom post-processing further enhances bright regions by introducing a glow effect around areas of high intensity.

Together, these techniques allow Charcoal to produce stylized lighting effects and more visually distinctive environments.

![[figure1-4-7.png]]
Figure 1.4.7: Low-intensity illumination test demonstrating localized surface highlights and the contrast between dark materials and orange emissive regions.

![[figure1-4-8.png]]
Figure 1.4.8: Refined Lighting configuration (Directional + Point Lights) demonstrating increased surface visibility and the interaction between illumination and material detail.

### 1.5 Hex Field — Rendering Demonstration

The **Hex Field** serves as the primary graphical demonstration for ALPHA 0.30.

The scene consists of repeated hexagonal geometric structures arranged within a three-dimensional environment. It provides a controlled setting for testing geometry, camera positioning, lighting, surface materials, texture sampling, and post-processing effects.

Throughout development, the Hex Field has undergone multiple visual revisions corresponding to the implementation and refinement of the rendering systems.

These iterations include initial geometry rendering, lighting experiments, emissive surface development, texture integration, and bloom adjustments.

The Hex Field also serves as a visual record of Charcoal's technical progression. Screenshots and recorded demonstrations provide evidence of individual system implementations and their combined graphical results.

Rather than serving solely as a visual showcase, the scene functions as a practical testing environment for evaluating the rendering pipeline.

![[figure1-5-1.png]]
Figure 1.5.1: Final render of the Hex Field environment scene; featuring textured walls, hexagonal platforms, and warm emissive lighting within an enclosed three-dimensional scene.

![[figure1-5-2.png]]
Figure 1.5.2: An alternate render demonstrating reduced illumination, pronounced material contrast, and spatial depth within the environment. (And also, a more defined version of the emissive material that uses gradients to demonstrate the glow's intensity)

### 1.6 Development Scope and Limitations

ALPHA 0.30 remains an early-stage development build.

Although the application demonstrates foundational three-dimensional rendering functionality, several capabilities remain incomplete or are reserved for future development.

These include more advanced scene management, generalized asset importing, expanded runtime controls, improved user interface integration, and additional rendering optimizations.

Scene loading currently combines hardcoded C++ content with JSON-based configuration and loading functionality. The scene architecture and associated resource lifecycle management require further refinement.

The application is therefore best understood as a functional graphics development environment under active construction rather than a completed end-user product.

These limitations are expected to guide the development of subsequent releases.

### 1.7 Future Development

ALPHA 0.30 establishes the technical foundation for the next stages of Charcoal's development.

ALPHA 0.40 will focus on improving scene and asset architecture and integrating Dear ImGui for development-oriented runtime controls.

Subsequent versions will expand asset loading, runtime interaction, visualization capabilities, and application stability, progressing toward the initial public release.

Long-term development will extend beyond foundational rendering to include external hardware interaction, audiovisual visualization, spatial input, and worldbuilding capabilities.

These objectives reflect Charcoal's broader purpose as a specialized graphics application rather than a traditional game engine.

### 1.8 Documentation Organization

The ALPHA 0.30 technical documentation is organized into five principal sections.

|Section|Description|
|---|---|
|**1. Overview**|Introduces Charcoal, the objectives of ALPHA 0.30, and its primary capabilities.|
|**2. Architecture**|Describes the application's structural organization, components, and dependencies.|
|**3. Execution Flow**|Explains initialization, scene preparation, rendering operations, and application execution.|
|**4. Major Systems**|Documents the individual graphics systems and their technical responsibilities.|
|**5. Important Source Files**|Identifies the primary source files and their roles within the application.|

Supporting diagrams, screenshots, and development recordings complement these sections by illustrating the internal organization of the application and demonstrating its rendering capabilities.

---

**Charcoal ALPHA 0.30 — Foundational Build**

_Technical Documentation — Section 1: Overview_