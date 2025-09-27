#pragma once
const char* COMPUTE_SHADER_SOURCE = R"glsl(
#version 430 core
layout(local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D outImage;

// constants
const float c = 299792458.0;
const float G = 6.67430e-11;
const float PI = 3.141592653589793;

// raytracer uniforms
uniform float u_dLambda;
uniform int u_maxSteps;
uniform float u_escapeRadius;

// blackhole uniforms
uniform float u_RS;

// camera uniforms
uniform vec3 u_cameraPos;
uniform vec3 u_cameraTarget;

uniform float u_ScreenResolutionX;
uniform float u_ScreenResolutionY;

struct Object {
    vec4 positionRadius; 
    vec4 color;
    vec4 padding;
};

bool InterceptObject(vec3 worldRayPos, Object object) {
    float dx = worldRayPos.x - object.positionRadius.x;
    float dy = worldRayPos.y - object.positionRadius.y;
    float dz = worldRayPos.z - object.positionRadius.z;

    float distanceSquared = dx * dx + dy * dy + dz * dz;
    return distanceSquared <= (object.positionRadius.w * object.positionRadius.w);
}

layout(std430, binding = 1) buffer ObjectBuffer {
    Object u_objects[]; // Flexible size array of Objects
};

uniform int u_numObjects;

struct Ray {
    vec3 cartesianPos;
    vec3 direction;

    float r, theta, phi;
    float dr, dtheta, dphi;
    float E;
    float L;
};

// RAY HELPER FUNCTIONS

void SphericalToCartesian(inout Ray ray)
{
    ray.cartesianPos.x = ray.r * sin(ray.theta) * cos(ray.phi);
    ray.cartesianPos.y = ray.r * sin(ray.theta) * sin(ray.phi);
    ray.cartesianPos.z = ray.r * cos(ray.theta);
}

void CartesianToSpherical(inout Ray ray)
{
    ray.r = length(ray.cartesianPos);
    if (ray.r < 1e-10) { // Avoid division by zero at the origin
        ray.theta = 0.0;
        ray.phi = 0.0;
        return;
    }
    ray.theta = acos(ray.cartesianPos.z / ray.r);
    ray.phi = atan(ray.cartesianPos.y, ray.cartesianPos.x);
}

// RK4 AND CARTESIAN EQUATIONS

Ray GetInitialRay(vec2 pixelPos)
{
    Ray initialRay;

    // Normalized Device Coords
    float ndcX = ((pixelPos.x + 0.5f) / u_ScreenResolutionX) * 2.0f - 1.0f;
    float ndcY = ((pixelPos.y + 0.5f) / u_ScreenResolutionY) * 2.0f - 1.0f;

    float aspect = u_ScreenResolutionX / u_ScreenResolutionY;
    const float HALF_FOV = radians(30.0f);
    float tanHalfFov = tan(HALF_FOV);

    float px = ndcX * aspect * tanHalfFov;
    float py = -ndcY * tanHalfFov; // flip Y

    // Standard camera space vectors
    vec3 forward = normalize(u_cameraTarget - u_cameraPos);
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
    initialRay.cartesianPos = u_cameraPos;

    vec3 relPos = u_cameraPos;

    // get initial spherical coords

    initialRay.r = length(relPos);
    if (initialRay.r < 1e-10)
    {
        initialRay.theta = 0.0;
        initialRay.phi = 0.0;
    }
    else
    {
        initialRay.theta = acos(clamp(relPos.z / initialRay.r, -1.0, 1.0));
        initialRay.phi = atan(relPos.y, relPos.x);
    }

    // --- 3. Calculate initial derivatives (dr, dtheta, dphi) ---
    float dx = dir.x, dy = dir.y, dz = dir.z;
    float r = initialRay.r;
    float theta = initialRay.theta;
    float phi = initialRay.phi;

    initialRay.dr = sin(theta) * cos(phi) * dx + sin(theta) * sin(phi) * dy + cos(theta) * dz;
    initialRay.dtheta = (cos(theta) * cos(phi) * dx + cos(theta) * sin(phi) * dy - sin(theta) * dz) / r;

    float r_sin_theta = r * sin(theta);
    if (abs(r_sin_theta) < 1e-10)
    {
        initialRay.dphi = 0.0;
    }
    else
    {
        initialRay.dphi = (-sin(phi) * dx + cos(phi) * dy) / r_sin_theta;
    }

    // angular velocity/momentum
    initialRay.L = r * r * sin(theta) * initialRay.dphi;
    // Energy (E) 
    float f = 1.0 - u_RS / r;

    // This is derived from the null geodesic condition: g_mu_nu * p^mu * p^nu = 0
    float radial_term_sq = initialRay.dr * initialRay.dr;
    float angular_term_f = f * r * r * (initialRay.dtheta * initialRay.dtheta + sin(theta) * sin(theta) * initialRay.dphi * initialRay.dphi);

    float E_sq = radial_term_sq + angular_term_f;
    if (E_sq < 0.0) E_sq = 0.0;

    initialRay.E = sqrt(E_sq);

    return initialRay;
}

void GetDerivatives(const Ray ray, out float d2r, out float d2theta, out float d2phi)
{
    float r = ray.r, theta = ray.theta;
    float dr = ray.dr, dtheta = ray.dtheta, dphi = ray.dphi;
    float rs = u_RS;


    float f = 1.0 - rs / r;
    float dt_dL;

    // Check for near event horizon
    if (abs(f) < 1e-6) {
        dt_dL = 0.0;
        d2r = d2theta = d2phi = 0.0;
        return;
    }
    else {
        // dt/d\lambda = E / f(r). This is correct for the equation of motion.
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
    }
    else {
        d2phi = -2.0 * dr * dphi / r
            - 2.0 * cos(theta) / sin_theta * dtheta * dphi;
    }
}

void RK4STEP(inout Ray ray)
{
    float half_dL = 0.5 * u_dLambda;
    float sixth_dL = u_dLambda / 6.0;

    // --- K1 (Derivatives at start of interval) ---
    Ray k1_ray = ray;
    float k1_d2r, k1_d2theta, k1_d2phi;
    GetDerivatives(k1_ray, k1_d2r, k1_d2theta, k1_d2phi);

    // --- K2 (Derivatives at midpoint using K1) ---
    Ray k2_ray;
    k2_ray.E = ray.E; k2_ray.L = ray.L; // Conserved quantities

    k2_ray.r = ray.r + half_dL * k1_ray.dr;
    k2_ray.theta = ray.theta + half_dL * k1_ray.dtheta;
    k2_ray.phi = ray.phi + half_dL * k1_ray.dphi;

    k2_ray.dr = ray.dr + half_dL * k1_d2r;
    k2_ray.dtheta = ray.dtheta + half_dL * k1_d2theta;
    k2_ray.dphi = ray.dphi + half_dL * k1_d2phi;

    float k2_d2r, k2_d2theta, k2_d2phi;
    GetDerivatives(k2_ray, k2_d2r, k2_d2theta, k2_d2phi);

    // --- K3 (Derivatives at midpoint using K2) ---
    Ray k3_ray;
    k3_ray.E = ray.E; k3_ray.L = ray.L;

    k3_ray.r = ray.r + half_dL * k2_ray.dr;
    k3_ray.theta = ray.theta + half_dL * k2_ray.dtheta;
    k3_ray.phi = ray.phi + half_dL * k2_ray.dphi;

    k3_ray.dr = ray.dr + half_dL * k2_d2r;
    k3_ray.dtheta = ray.dtheta + half_dL * k2_d2theta;
    k3_ray.dphi = ray.dphi + half_dL * k2_d2phi;

    float k3_d2r, k3_d2theta, k3_d2phi;
    GetDerivatives(k3_ray, k3_d2r, k3_d2theta, k3_d2phi);

    // --- K4 (Derivatives at end of interval using K3) ---
    Ray k4_ray;
    k4_ray.E = ray.E; k4_ray.L = ray.L;

    k4_ray.r = ray.r + u_dLambda * k3_ray.dr;
    k4_ray.theta = ray.theta + u_dLambda * k3_ray.dtheta;
    k4_ray.phi = ray.phi + u_dLambda * k3_ray.dphi;

    k4_ray.dr = ray.dr + u_dLambda * k3_d2r;
    k4_ray.dtheta = ray.dtheta + u_dLambda * k3_d2theta;
    k4_ray.dphi = ray.dphi + u_dLambda * k3_d2phi;

    float k4_d2r, k4_d2theta, k4_d2phi;
    GetDerivatives(k4_ray, k4_d2r, k4_d2theta, k4_d2phi);

    // --- Final update (Weighted average of slopes) ---

    // Update positions (r, theta, phi) using velocities (dr, dtheta, dphi)
    ray.r += sixth_dL * (k1_ray.dr + 2.0 * k2_ray.dr + 2.0 * k3_ray.dr + k4_ray.dr);
    ray.theta += sixth_dL * (k1_ray.dtheta + 2.0 * k2_ray.dtheta + 2.0 * k3_ray.dtheta + k4_ray.dtheta);
    ray.phi += sixth_dL * (k1_ray.dphi + 2.0 * k2_ray.dphi + 2.0 * k3_ray.dphi + k4_ray.dphi);

    // Update velocities (dr, dtheta, dphi) using second derivatives (d2r, d2theta, d2phi)
    ray.dr += sixth_dL * (k1_d2r + 2.0 * k2_d2r + 2.0 * k3_d2r + k4_d2r);
    ray.dtheta += sixth_dL * (k1_d2theta + 2.0 * k2_d2theta + 2.0 * k3_d2theta + k4_d2theta);
    ray.dphi += sixth_dL * (k1_d2phi + 2.0 * k2_d2phi + 2.0 * k3_d2phi + k4_d2phi);
}

vec3 TraceAndGetColor(Ray initialRay)
{
    Ray currentRay = initialRay;

    for (int i = 0; i < u_maxSteps; i++)
    {
        RK4STEP(currentRay);
        SphericalToCartesian(currentRay);

        if (currentRay.r > u_escapeRadius)
        {
            return vec3(0.5, 0.5, 0.5);
        }

        if (currentRay.r <= u_RS)
        {
            return vec3(1.0, 1.0, 1.0);
        }

        for (int j = 0; j < u_numObjects; j++)
        {
            if (InterceptObject(currentRay.cartesianPos, u_objects[j]))
            {
                vec4 col = u_objects[j].color / 255.0;
                return vec3(col.x, col.y, col.z);
            }
        }
    }

    // Default return color if max steps reached (black)
    return vec3(0.0, 0.0, 0.0);
}

void main()
{
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    if (pixelCoords.x >= u_ScreenResolutionX || pixelCoords.y >= u_ScreenResolutionY)
        return;

    Ray initialRay = GetInitialRay(vec2(pixelCoords));

    vec3 color = TraceAndGetColor(initialRay);

    imageStore(outImage, pixelCoords, vec4(color, 1.0));
    return;
}
)glsl";
