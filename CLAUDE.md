# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Rewind Viewer is a fast, OpenGL-based visualization tool for Russian AI Cup championship matches with rewinding support. It receives drawing primitives via JSON over TCP sockets and renders them in real-time with frame navigation capabilities.

## Build Commands

**Build the project:**
```bash
mkdir build && cd build
cmake --CMAKE_BUILD_TYPE=Release ..
cmake --build .
```

**Debug build:**
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
```

## Architecture Overview

### Core System Flow
```
Network Data → JsonHandler → FrameEditor → Frame → Scene → Renderer → OpenGL
```

### Key Subsystems

**Networking** (`src/net/`):
- `NetListener`: TCP server on `127.0.0.1:9111`
- `JsonHandler`: Parses JSON protocol into primitives
- Runs in separate thread with spinlock synchronization

**Scene Management** (`src/viewer/`):
- `Scene`: Central hub managing frames and coordinating rendering
- `Frame`: Immutable frame data with 10 rendering layers (0-9, default layer 2)
- `FrameEditor`: Mutable builder for constructing frames from network data
- Supports frame rewinding and playback controls

**Rendering** (`src/viewer/` + `src/cgutils/`):
- `Renderer`: OpenGL rendering implementation
- `RenderContext`: Accumulates primitives for batched drawing
- `ShaderCollection`: Manages OpenGL shaders (color_pos, circle, uniform_color)
- `Camera`: 2D camera with configurable coordinate system
- `ResourceManager`: RAII-based OpenGL resource management

**UI** (`src/viewer/UIController.*`):
- ImGui-based interface with developer mode (Ctrl+D / Cmd+D)
- Frame playback controls, FPS overlay, configuration widgets
- Font system updated for ImGui 1.92+ with Retina support

### Threading Model
- **Main thread**: Rendering and UI
- **Network thread**: TCP socket handling and frame building
- **Synchronization**: Spinlock between threads for frame data access

### Protocol
JSON-based protocol supporting primitives: circle, rectangle, triangle, polyline, message, popup, options, end. See `clients/README.md` for protocol documentation.

## Development Notes

### ImGui Integration
- Project uses ImGui 1.92+ with modern key handling (`ImGuiKey_*` enums)
- Font merging configured for FontAwesome icons with proper exclusion ranges
- Retina display support via `RasterizerDensity` instead of legacy scaling

### OpenGL Usage
- OpenGL 3.3 Core Profile with GLAD loader
- Custom shaders in `resources/shaders/`
- Geometry shaders for efficient circle rendering

### Configuration System
- Centralized `Config` class with ImGui integration
- INI file persistence (`rewindviewer.ini`)
- Categories: UI, Scene, Network, Camera settings

### Layer System
- 10 independent rendering layers (0-9)
- Layer visibility controlled via UI (1-0 keys)
- Permanent frames for persistent overlays

## Common Patterns

**Adding New Primitives:**
1. Add enum to `PrimitiveType.h`
2. Implement parsing in `JsonHandler`
3. Add rendering in `RenderContext`
4. Update protocol documentation

**Shader Management:**
- Shaders embedded at compile time from `resources/shaders/` into generated headers
- CMake automatically embeds all shaders using `embed_shaders()` function
- Runtime loading via `Shader::EmbeddedShaders` struct instead of file paths
- Uniform management via `ShaderCollection`

**Resource Cleanup:**
- RAII pattern enforced via `ResourceManager`
- Automatic OpenGL resource cleanup
- Exception-safe resource handling

## Dependencies

Key 3rd party libraries in `3rdparty/`:
- **GLFW**: Window management and input
- **ImGui**: Immediate mode UI framework
- **GLM**: Mathematics library for graphics
- **RapidJSON**: JSON parsing
- **loguru**: Structured logging
- **CSimpleSocket**: TCP networking
- **GLAD**: OpenGL function loader
- **stb_image**: Image loading

## File Structure

- `src/main.cpp`: Application entry point and main loop
- `src/viewer/`: Core rendering and UI components
- `src/net/`: Networking and protocol handling
- `src/cgutils/`: Computer graphics utilities
- `src/common/`: Shared utilities (spinlock, logging)
- `src/imgui_impl/`: ImGui platform integration
- `resources/`: Source assets (embedded at build time)
- `cmake/`: CMake modules for resource embedding (`EmbedResources.cmake`, `embed_shader.cmake`, `embed_binary.cmake`)
- `clients/`: Protocol clients for various languages