#pragma once
const char* COMPUTE_SHADER_SOURCE = R"glsl(
#version 430 core

// Define the size of the work group. 
// A size of (16, 16) is a good general purpose choice.
layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;

// Output image texture binding (must match C++ code)
// Rgba8 is suitable for color output.
layout (rgba8, binding = 0) uniform writeonly image2D u_output_image; 

// --- Constants ---
const float D_LAMBDA = 1e6;
const int MAX_STEPS = 5000;
const float ESCAPE_RADIUS = 1e12;

// --- Uniform Structures and Parameters ---

// Structure for objects in the scene
struct Object {
    vec3 position;
    float radius;
    vec3 color; // Expected to be in 0-255 range from C++ logic
};

#define MAX_OBJECTS 3
uniform Object u_objects[MAX_OBJECTS];
uniform int u_num_objects;

// Black Hole and Camera Data
uniform float u_RS;             // Schwarzschild Radius (R_S)
uniform vec3 u_bh_position;     // Black hole position (World Space)
uniform vec3 u_camera_pos;      // Camera position (World Space)
uniform vec3 u_camera_target;   // Camera target (World Space)
uniform vec2 u_resolution;      // (WIDTH, HEIGHT)

// --- Ray Structure (Matching C++ Ray.h) ---

struct Ray {
    vec3 cartesianPosition; // Position relative to BH center (0,0,0) IF BH is at origin, otherwise World Space - BH Pos
    vec3 direction;         // Ray direction in World Space

    float r, phi, theta;    // Position in spherical coord (relative to BH center)
    float dr, dphi, dtheta; // Velocity based on affine parameter (dlambda)

    float E; // Energy (Conserved)
    float L; // Angular Momentum (Conserved)
};

// --- Helper Functions ---

// Updates the cartesian position (relative to BH center) from spherical coordinates
void UpdateCartesian(inout Ray ray)
{
    ray.cartesianPosition.x = ray.r * sin(ray.theta) * cos(ray.phi);
    ray.cartesianPosition.y = ray.r * sin(ray.theta) * sin(ray.phi);
    ray.cartesianPosition.z = ray.r * cos(ray.theta);
}

// Interception check for a spherical object
bool InterceptObject(vec3 worldRayPos, const Object obj)
{
    // Checks if the world ray position is inside the object's world-space sphere
    float distSq = dot(worldRayPos - obj.position, worldRayPos - obj.position);
    return distSq <= (obj.radius * obj.radius);
}


// --- Geodesic Equations ---

// Calculates the second derivatives of spherical coordinates (r, theta, phi) w.r.t the affine parameter lambda
void GetDerivatives(const Ray ray, float R_S, out float d2r, out float d2theta, out float d2phi) {

    float r = ray.r, theta = ray.theta;
    float dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;
    float rs = R_S;

    // Schwarzschild metric function f(r) = 1 - R_S/r
    float f = 1.0 - rs / r;
    
    // Time component derivative dt/dlambda from conserved energy E
    float dt_dL;
    
    // Check for near event horizon
    if (abs(f) < 1e-6) {
        dt_dL = 0.0;
        d2r = d2theta = d2phi = 0.0;
        return;
    } else {
        dt_dL = ray.E / f;
    }

    // Geodesic Equation for r: d^2 r / d\lambda^2
    d2r = -(rs / (2.0 * r * r)) * f * dt_dL * dt_dL
        + (rs / (2.0 * r * r * f)) * dr * dr
        + r * (dtheta * dtheta + sin(theta) * sin(theta) * dphi * dphi);

    // Geodesic Equation for theta: d^2 theta / d\lambda^2
    d2theta = -2.0 * dr * dtheta / r
        + sin(theta) * cos(theta) * dphi * dphi;

    // Geodesic Equation for phi: d^2 phi / d\lambda^2
    float sin_theta = sin(theta);
    // Avoid pole singularity (theta = 0 or theta = PI)
    if (abs(sin_theta) < 1e-6) {
        d2phi = 0.0;
    } else {
        d2phi = -2.0 * dr * dphi / r
              - 2.0 * cos(theta) / sin_theta * dtheta * dphi;
    }
}

// Performs one step of Runge-Kutta 4th Order integration
void RK4STEP(inout Ray ray, float R_S)
{
    float half_dL = 0.5 * D_LAMBDA;
    float sixth_dL = D_LAMBDA / 6.0;

    // --- K1 (Derivatives at start of interval) ---
    Ray k1_ray = ray;
    float k1_d2r, k1_d2theta, k1_d2phi;
    GetDerivatives(k1_ray, R_S, k1_d2r, k1_d2theta, k1_d2phi);

    // --- K2 (Derivatives at midpoint using K1) ---
    Ray k2_ray;
    k2_ray.E = ray.E; k2_ray.L = ray.L; // Conserved quantities
    
    k2_ray.r     = ray.r     + half_dL * k1_ray.dr;
    k2_ray.theta = ray.theta + half_dL * k1_ray.dtheta;
    k2_ray.phi   = ray.phi   + half_dL * k1_ray.dphi;

    k2_ray.dr     = ray.dr     + half_dL * k1_d2r;
    k2_ray.dtheta = ray.dtheta + half_dL * k1_d2theta;
    k2_ray.dphi   = ray.dphi   + half_dL * k1_d2phi;

    float k2_d2r, k2_d2theta, k2_d2phi;
    GetDerivatives(k2_ray, R_S, k2_d2r, k2_d2theta, k2_d2phi);

    // --- K3 (Derivatives at midpoint using K2) ---
    Ray k3_ray;
    k3_ray.E = ray.E; k3_ray.L = ray.L;
    
    k3_ray.r     = ray.r     + half_dL * k2_ray.dr;
    k3_ray.theta = ray.theta + half_dL * k2_ray.dtheta;
    k3_ray.phi   = ray.phi   + half_dL * k2_ray.dphi;

    k3_ray.dr     = ray.dr     + half_dL * k2_d2r;
    k3_ray.dtheta = ray.dtheta + half_dL * k2_d2theta;
    k3_ray.dphi   = ray.dphi   + half_dL * k2_d2phi;

    float k3_d2r, k3_d2theta, k3_d2phi;
    GetDerivatives(k3_ray, R_S, k3_d2r, k3_d2theta, k3_d2phi);

    // --- K4 (Derivatives at end of interval using K3) ---
    Ray k4_ray;
    k4_ray.E = ray.E; k4_ray.L = ray.L;
    
    k4_ray.r     = ray.r     + D_LAMBDA * k3_ray.dr;
    k4_ray.theta = ray.theta + D_LAMBDA * k3_ray.dtheta;
    k4_ray.phi   = ray.phi   + D_LAMBDA * k3_ray.dphi;

    k4_ray.dr     = ray.dr     + D_LAMBDA * k3_d2r;
    k4_ray.dtheta = ray.dtheta + D_LAMBDA * k3_d2theta;
    k4_ray.dphi   = ray.dphi   + D_LAMBDA * k3_d2phi;

    float k4_d2r, k4_d2theta, k4_d2phi;
    GetDerivatives(k4_ray, R_S, k4_d2r, k4_d2theta, k4_d2phi);

    // --- Final update (Weighted average of slopes) ---

    // Update positions (r, theta, phi) using velocities (dr, dtheta, dphi)
    ray.r     += sixth_dL * (k1_ray.dr     + 2.0 * k2_ray.dr     + 2.0 * k3_ray.dr     + k4_ray.dr);
    ray.theta += sixth_dL * (k1_ray.dtheta + 2.0 * k2_ray.dtheta + 2.0 * k3_ray.dtheta + k4_ray.dtheta);
    ray.phi   += sixth_dL * (k1_ray.dphi   + 2.0 * k2_ray.dphi   + 2.0 * k3_ray.dphi   + k4_ray.dphi);

    // Update velocities (dr, dtheta, dphi) using second derivatives (d2r, d2theta, d2phi)
    ray.dr     += sixth_dL * (k1_d2r     + 2.0 * k2_d2r     + 2.0 * k3_d2r     + k4_d2r);
    ray.dtheta += sixth_dL * (k1_d2theta + 2.0 * k2_d2theta + 2.0 * k3_d2theta + k4_d2theta);
    ray.dphi   += sixth_dL * (k1_d2phi   + 2.0 * k2_d2phi   + 2.0 * k3_d2phi   + k4_d2phi);
}

// --- Main Ray Tracing Functions ---

// Calculates the initial ray properties based on camera and pixel coordinates
Ray GetInitialRay(vec2 pixelCoords)
{
    Ray initialRay;
    
    // --- 1. Calculate ray direction in world space ---
    
    // Pixel coordinates in Normalized Device Coordinates (NDC)
    float ndcX = (pixelCoords.x / u_resolution.x) * 2.0 - 1.0;
    float ndcY = (pixelCoords.y / u_resolution.y) * 2.0 - 1.0;

    float aspect = u_resolution.x / u_resolution.y;
    const float HALF_FOV = radians(30.0); // 0.5 * 60 degrees
    float tanHalfFov = tan(HALF_FOV);

    float px = ndcX * aspect * tanHalfFov;
    float py = -ndcY * tanHalfFov; // flip Y to match standard screen space

    vec3 cameraPos = u_camera_pos;

    // Standard camera space vectors
    vec3 forward = normalize(u_camera_target - cameraPos);
    vec3 worldUp = vec3(0.0, 1.0, 0.0);

    // Handle edge case: forward nearly parallel to worldUp
    vec3 right;
    if (abs(dot(forward, worldUp)) > 0.999) {
        worldUp = vec3(0.0, 0.0, 1.0);
    }
    
    right = normalize(cross(forward, worldUp));
    vec3 up = normalize(cross(right, forward));

    vec3 dir = normalize(forward + px * right + py * up);
    initialRay.direction = dir;
    initialRay.cartesianPosition = cameraPos;
    
    // --- 2. Calculate initial spherical coordinates (relative to black hole) ---
    vec3 relPos = cameraPos - u_bh_position;
    
    initialRay.r = length(relPos);
    
    if (initialRay.r < 1e-10) {
        initialRay.theta = 0.0;
        initialRay.phi = 0.0;
    } else {
        // Clamp relPos.z / initialRay.r to [-1, 1] for acos stability
        initialRay.theta = acos(clamp(relPos.z / initialRay.r, -1.0, 1.0)); 
        initialRay.phi = atan(relPos.y, relPos.x); // GLSL atan is atan2
    }
    
    // --- 3. Calculate initial derivatives (dr, dtheta, dphi) ---
    float dx = dir.x, dy = dir.y, dz = dir.z;
    float r = initialRay.r;
    float theta = initialRay.theta;
    float phi = initialRay.phi;

    // dr/d\lambda
    initialRay.dr = sin(theta) * cos(phi) * dx + sin(theta) * sin(phi) * dy + cos(theta) * dz;
    
    // d\theta/d\lambda
    initialRay.dtheta = (cos(theta) * cos(phi) * dx + cos(theta) * sin(phi) * dy - sin(theta) * dz) / r;

    // d\phi/d\lambda (avoiding pole singularity)
    float r_sin_theta = r * sin(theta);
    if (abs(r_sin_theta) < 1e-10) {
        initialRay.dphi = 0.0;
    } else {
        initialRay.dphi = (-sin(phi) * dx + cos(phi) * dy) / r_sin_theta;
    }

    // --- 4. Calculate Conserved Quantities (L and E) ---
    
    // Angular Momentum (L)
    initialRay.L = r * r * sin(theta) * initialRay.dphi;

    // Energy (E)
    float f = 1.0 - u_RS / r;
    
    float radial_term_sq = (initialRay.dr * initialRay.dr) / f;
    float angular_term_sq = r * r * (initialRay.dtheta * initialRay.dtheta + sin(theta) * sin(theta) * initialRay.dphi * initialRay.dphi);
    
    float argument = radial_term_sq + angular_term_sq;
    if (argument < 0.0) argument = 0.0; // Protect against negative square root argument

    float dt_dL = sqrt(argument);
    
    initialRay.E = f * dt_dL;

    return initialRay;
}

// Traces the ray and determines the color
vec3 TraceAndGetColor(Ray initialRay)
{
    Ray currentRay = initialRay;

    for (int i = 0; i < MAX_STEPS; i++)
    {
        // 1. Integration step
        RK4STEP(currentRay, u_RS);

        // 2. Update cartesian position (relative to BH center)
        UpdateCartesian(currentRay);
        
        // Convert to World Position for Object Intercept checks
        vec3 worldRayPos = currentRay.cartesianPosition + u_bh_position;

        // 3. Check for Escape
        if (currentRay.r > ESCAPE_RADIUS)
        {
            // Background color (black)
            return vec3(0.5, 0.5, 0.5);
        }

        // 4. Check for Black Hole Intercept (r <= R_S)
        if (currentRay.r <= u_RS)
        {
            // Black hole color (white/accretion disc)
            return vec3(1.0, 1.0, 1.0);
        }

        // 5. Check for Object Intercept
        for (int j = 0; j < u_num_objects; j++)
        {
            if (InterceptObject(worldRayPos, u_objects[j]))
            {
                // Return object's color (normalized to 0.0-1.0)
                return u_objects[j].color / 255.0; 
            }
        }
    }

    // Default return color if max steps reached (black)
    return vec3(0.5, 0.5, 0.5);
}


void main() 
{
    // The current pixel being processed is defined by the global invocation ID
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    
    // Check boundaries to avoid writing outside the image
    if (pixelCoords.x >= u_resolution.x || pixelCoords.y >= u_resolution.y) {
        return;
    }

    // 1. Get initial ray from camera/pixel
    Ray initialRay = GetInitialRay(vec2(pixelCoords));
    
    // 2. Trace and get the resulting color
    vec3 color = TraceAndGetColor(initialRay);
    
    // 3. Write the color to the output image
    imageStore(u_output_image, pixelCoords, vec4(color, 1.0));
}

)glsl";
