# 3D Black Hole Ray-Tracer

An accurate, real-time simulation built in C++ and OpenGL that visualizes the gravitational lensing and shadow of a non-rotating black hole. All raytracing is GPU-accelerated using Compute Shaders for fluid, interactive performance.



## 💡 Core Technology
This simulation models light propagation by using the Schwarzschild Metric to define the curved spacetime. Light rays (Null Geodesics) are integrated through this space using the accurate Runge-Kutta 4th Order (RK4) integrator.

- Physics: Defines the Event Horizon (RS) and the Photon Sphere (15RS).
- Implementation: The RK4 integration logic is contained within a dedicated OpenGL Compute Shader.

## 🖱️ Controls
- Left Click + Drag: Rotate the camera.
- Scroll Wheel: Zoom in and out.
