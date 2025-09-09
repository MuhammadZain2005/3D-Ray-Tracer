#include "vector.h"
#include "spheres.h"
#include "color.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define HEX_LENGTH 10 // Maximum Length of HEX code
#define SHADOW_FACTOR 0.1f // Shadow Intensity

typedef struct {
    Vec3 position;
    float focal_length;
} Camera;   

typedef struct {
    float height;
    float width;
    Vec3 position;
} Viewport;

typedef struct {
    Vec3 position;
    float brightness;
} Light;

typedef struct {
    Vec3 color;
} Background;



void SetCamera(Camera *camera, float focal_length) {
    camera->position = (Vec3){0.0f, 0.0f, 0.0f}; // Position of camera set to (0,0,0)
    camera->focal_length = focal_length;
}

void setViewport(Viewport *viewport, float height, float aspect_ratio, float focal_length) {
    viewport->height = height;
    viewport->width = height * aspect_ratio;
    viewport->position = (Vec3){0.0f, 0.0f, -focal_length};
}

// Milestone 1
Vec3 addBGLight(Vec3 background, Vec3 light) {
    Vec3 result = {
        background.x + light.x,
        background.y + light.y,
        background.z + light.z
    };
    return result;
}

Vec3 subLightBG(Vec3 background, Vec3 light) {
    Vec3 result = {
        background.x - light.x,
        background.y - light.y,
        background.z - light.z
    };
    return result;
}

Vec3 mulVPLight(float viewport_width, Vec3 light) {
    Vec3 result = {
        viewport_width * light.x,
        viewport_width * light.y,
        viewport_width * light.z
    };
    return result;
}

Vec3 normLight(Vec3 light) {
    float magnitude = sqrt(light.x * light.x + light.y * light.y + light.z * light.z);
    Vec3 result = { 
        light.x / magnitude, 
        light.y / magnitude, 
        light.z / magnitude
    };
    return result;
}

void performSphereCalculations(Sphere *sphere, Vec3 light, int sphereIndex, FILE *OutputFile) {
    
    if (sphere == NULL) {
        fprintf(stderr, "Error: Null sphere pointer for sphere %d\n", sphereIndex);
        return;
    }
    
    // Scalar division (sphere.color / sphere.radius)
    Vec3 result1 = scalarDivide(sphere->color, sphere->r);
    fprintf(OutputFile, "(%.1f, %.1f, %.1f) / %.1f = (%.1f, %.1f, %.1f)\n", 
            sphere->color.x, sphere->color.y, sphere->color.z, 
            sphere->r, result1.x, result1.y, result1.z);

    // Dot product (light.position dot sphere.position)
    float result2 = dot(light, sphere->pos);
    fprintf(OutputFile, "dot((%.1f, %.1f, %.1f), (%.1f, %.1f, %.1f)) = %.1f\n", 
            light.x, light.y, light.z, sphere->pos.x, sphere->pos.y, sphere->pos.z, result2);

    // Distance (distance between light.position and sphere.position)
    float result3 = distance(light, sphere->pos);
    fprintf(OutputFile, "distance((%.1f, %.1f, %.1f), (%.1f, %.1f, %.1f)) = %.1f\n", 
            light.x, light.y, light.z, sphere->pos.x, sphere->pos.y, sphere->pos.z, result3);

    // Length (length of sphere.position)
    float result4 = length(sphere->pos);
    fprintf(OutputFile, "length(%.1f, %.1f, %.1f) = %.1f\n", 
            sphere->pos.x, sphere->pos.y, sphere->pos.z, result4);
}
// MS1 END

// Milestone 2
typedef struct {
    Vec3 origin;     // Starting point of the ray
    Vec3 direction;  // Direction the ray is traveling
} Ray;

Ray generateRay(Camera *camera, Viewport *viewport, int x, int y, int image_width, int image_height, float sample_x, float sample_y) {
    Ray ray;
    ray.origin = camera->position; // Set to (0,0,0)

    // Step 1: Calculate pixel center in viewport with anti-aliasing offsets (FS)
    float pixel_x = (x + 0.5f + sample_x) / image_width * viewport->width - viewport->width / 2.0f; // Maps x in (x,y) to viewport coords
    float pixel_y = (y + 0.5f + sample_y) / image_height * viewport->height - viewport->height / 2.0f; // Maps y in (x,y) to viewport coords
    
    // Create pixel position in world
    Vec3 pixel_position = {
        pixel_x,
        pixel_y,
        viewport->position.z  // Use viewport's z-coordinate (focal length)
    };
    
    // Step 2: Compute the ray's direction by subtracting the origin from the pixel position. Use normalize to ensure unit length.
    ray.direction = normalize(subtract(pixel_position, ray.origin));
    
    return ray;
}


Vec3 PixelColorandShadow(Vec3 sphereColor, Light light, Vec3 intersectionPoint, Vec3 normal, const Sphere *spheres, int numSpheres) {
    
    // Step 1: Lighting and Calculating Color Part (from assignment)
    Vec3 LightDirection = normalize(subtract(light.position, intersectionPoint)); // Normalized Light Direction
    float dotprod = fmax(dot(LightDirection, normal), 0.0f); // Dot Product max(d*n,0)
    float distanceSquared = distance2(light.position, intersectionPoint); // dist(l*p)^2
    float intensity = light.brightness * (dotprod / distanceSquared); // Intensity
    intensity = fmin(intensity, 1.0f); // Cap intensity at 1
    Vec3 surfaceLightingColor = scalarMultiply(intensity, sphereColor); // Final Color P(x,y)
    
    // Step 2: Casting Shadows
    Vec3 shadowRayOrigin = add(intersectionPoint, scalarMultiply(0.001f, normal)); // Offset for precision
    Vec3 shadowRayDirection = normalize(LightDirection); // Unit length

    // Step 3: Check for intersections with other spheres along the shadow ray.
    for (int i = 0; i < numSpheres; i++) {
        float t;
        if (doesIntersect(&spheres[i], shadowRayOrigin, shadowRayDirection, &t)) {
            if (t <= distanceToLight) {  // Only apply shadow if intersection is between point and light
                surfaceLightingColor = scalarMultiply(SHADOW_FACTOR, surfaceLightingColor);
                break;
            }
        }
    }

    return surfaceLightingColor; // Final colored with shds
}

// Rendering Pipeline : Step 2, 3, 4
Vec3 traceRay(Ray ray, const Sphere *spheres, int numSpheres, Vec3 BackgroundColor, Light light) {
    float closest_t = INFINITY; // Ensure that any actual intersection distance will be smaller and replace it
    int hit_sphere = -1;
    float t;
    
    // Step 2: Check for any sphere intersections with that ray
    for (int i = 0; i < numSpheres; i++) {
        if (doesIntersect(&spheres[i], ray.origin, ray.direction, &t)) {
            if (t < closest_t) {
                closest_t = t;
                hit_sphere = i;
            }
        }
    }

    // Step 3: If no intersection found, return background color
    if (hit_sphere == -1) {
        return BackgroundColor;
    }

    // Step 4: If the ray hits a sphere, calculate the surface color at that point affected by lighting
    const Sphere *closestSphere = &spheres[hit_sphere]; // Get closest sphere
    Vec3 intersectionPoint = add(ray.origin, scalarMultiply(closest_t, ray.direction)); // Intersection Point
    Vec3 normal = normalize(subtract(intersectionPoint, spheres[hit_sphere].pos)); // Normal //closestSphere->pos

    // Call PixelColorandShadow to compute lighting and shadow effects
    Vec3 SurfaceColor = PixelColorandShadow(
        closestSphere->color,   // Sphere's surface color
        light,                  // Light source
        intersectionPoint,      // Intersection point
        normal,                 // Normal at the intersection
        spheres,                // Array of spheres (to check for shadows)
        numSpheres             // Total number of spheres
    );

    return SurfaceColor;
}

// MS2 END

// FS

Vec3 traceRayWithAA(Camera *camera, Viewport *viewport, const Sphere *spheres, int numSpheres, Vec3 BackgroundColor, Light light, int x, int y, int image_width, int image_height) {
    Vec3 final_color = {0.0f, 0.0f, 0.0f}; // Initialize the final color to black (zero)

    // Define the offsets for 1/6, 3/6, 5/6 sampling
    float offsets[3] = {1.0f / 6.0f, 3.0f / 6.0f, 5.0f / 6.0f};

    int sample_count = 0; // To keep track of the number of samples

    // Loop through the 3x3 sampling grid
    for (int sample_x = 0; sample_x < 3; sample_x++) {
        for (int sample_y = 0; sample_y < 3; sample_y++) {
            
            // Compute offsets for this sample
            float offset_x = offsets[sample_x];
            float offset_y = offsets[sample_y];
            
            // Generate a ray for the current sample
            Ray ray = generateRay(camera, viewport, x, y, image_width, image_height, offset_x, offset_y);
            
            // Trace the ray and get the color
            Vec3 color = traceRay(ray, spheres, numSpheres, BackgroundColor, light);

            // Accumulate the color
            final_color = add(final_color, color);
            sample_count++;
        }
    }

    // Compute the final averaged color
    final_color = scalarMultiply(1.0f / (float)sample_count, final_color);

    return final_color;
}

// FS END

int main(int argc, char *argv[]){
    // Input file open
    if (argc != 3) {
        fprintf(stderr, "Files in use: %s <Input file path> <Output file path>\n", argv[0]);
        return 1;
    }

    FILE *InputFile = fopen(argv[1], "r");
    if(InputFile == NULL) {
        fprintf(stderr, "%s can not be accessed\n", argv[1]);
        return 1;
    }

    FILE *OutputFile = fopen(argv[2], "w");
    if (OutputFile == NULL) {
        fprintf(stderr, "%s can not be accessed\n", argv[2]);
        fclose(InputFile);
        return 1;
    }

    // Inputs Taken & Initializing
    Camera camera;
    int ImageWidth, ImageHeight, NumColors, BGindex, NumSpheres, SphereCI;
    float aspect_ratio, focal_length;
    Light light;
    Viewport viewport;
    World world;
    worldInit(&world);

    // Background Color
    Background background;

    // Read the image dimensions, viewport height, and focal length
    fscanf(InputFile, "%d %d", &ImageWidth, &ImageHeight);
    fscanf(InputFile, "%f", &viewport.height);

    // Aspect Ratio and Focal Length
    aspect_ratio = (float)ImageWidth / (float)ImageHeight;
    fscanf(InputFile, "%f", &focal_length);
    SetCamera(&camera, focal_length);

    // Light Position and Brightness
    fscanf(InputFile, "%f %f %f %f", &light.position.x, &light.position.y, &light.position.z, &light.brightness);

    // Color input, Color array, BG Index
    fscanf(InputFile, "%d", &NumColors);
    unsigned int *colors = malloc(NumColors * sizeof(unsigned int));
    
    for (int i = 0; i < NumColors; i++) {
        fscanf(InputFile, "%x", &colors[i]);  // Read the color as a hex value
    }

    qsort(colors, NumColors, sizeof(unsigned int), compareColor);
    fscanf(InputFile, "%d", &BGindex);  // BG Index

    // Spheres: Read the number of spheres
    fscanf(InputFile, "%d", &NumSpheres);
    if (NumSpheres <= 0) {
        fprintf(stderr, "Error: Invalid number of spheres %d\n", NumSpheres);
        return 1;
    }

    // Allocate memory for spheres
    Sphere *spheres = malloc(NumSpheres * sizeof(Sphere));
    if (spheres == NULL) {
        fprintf(stderr, "Error: Memory allocation failed for spheres\n");
        return 1;
    }

    // Read sphere data
    for (int i = 0; i < NumSpheres; i++) {
        fscanf(InputFile, "%f %f %f %f %d", 
            &spheres[i].pos.x, 
            &spheres[i].pos.y, 
            &spheres[i].pos.z, 
            &spheres[i].r, 
            &SphereCI);

        // Directly access the color from the 'colors' array as an unsigned int
        unsigned int packedColor = colors[SphereCI];
        spheres[i].color = unpackRGB(packedColor);
    }

    // Read background color from the 'colors' array
    unsigned int packedColor = colors[BGindex];
    background.color = unpackRGB(packedColor);


    // Set up viewport
    setViewport(&viewport, viewport.height, aspect_ratio, focal_length);
    
    #ifdef MS1

    Vec3 resultAdd = addBGLight(background.color, light.position);
    Vec3 resultSub = subLightBG(background.color, light.position);
    Vec3 resultMul = mulVPLight(viewport.width, light.position);
    Vec3 resultNorm = normLight(light.position);

    // Output results
    fprintf(OutputFile, "(%.1f, %.1f, %.1f) + (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n", 
            background.color.x, background.color.y, background.color.z, 
            light.position.x, light.position.y, light.position.z, 
            resultAdd.x, resultAdd.y, resultAdd.z);

    fprintf(OutputFile, "(%.1f, %.1f, %.1f) - (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n", 
            background.color.x, background.color.y, background.color.z, 
            light.position.x, light.position.y, light.position.z, 
            resultSub.x, resultSub.y, resultSub.z);

    fprintf(OutputFile, "%.1f * (%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n", 
            viewport.width, light.position.x, light.position.y, light.position.z, 
            resultMul.x, resultMul.y, resultMul.z);

    fprintf(OutputFile, "normalize(%.1f, %.1f, %.1f) = (%.1f, %.1f, %.1f)\n", 
            light.position.x, light.position.y, light.position.z, 
            resultNorm.x, resultNorm.y, resultNorm.z);
    
    fprintf(OutputFile, "\n");

    for (int i = 0; i < NumSpheres; i++) {
        // Perform sphere calculations
        performSphereCalculations(&spheres[i], light.position, i + 1, OutputFile);
        
        if (i < NumSpheres - 1) {
        fprintf(OutputFile, "\n");
        }
    }
    #endif
    
    #ifdef MS2
    fprintf(OutputFile, "P3\n");
    fprintf(OutputFile, "%d %d\n", ImageWidth, ImageHeight);
    fprintf(OutputFile, "255\n");

    // Ray tracing and color calculation for each pixel
    for (int y = ImageHeight - 1; y >= 0; y--) {  // Start from the bottom row
        for (int x = 0; x < ImageWidth; x++) {
            Ray ray = generateRay(&camera, &viewport, x, y, ImageWidth, ImageHeight, 0.0f, 0.0f); // Generate Ray
            Vec3 color = traceRay(ray, spheres, NumSpheres, background.color, light); // Trace Ray to get colorr
            writeColour(OutputFile, color); // Write color
        }
    }
    #endif

    #ifdef FS
    fprintf(OutputFile, "P3\n");
    fprintf(OutputFile, "%d %d\n", ImageWidth, ImageHeight);
    fprintf(OutputFile, "255\n");

    // Ray tracing and anti-aliasing for each pixel
    for (int y = ImageHeight - 1; y >= 0; y--) {  // Start from the bottom row
        for (int x = 0; x < ImageWidth; x++) {
            Vec3 color = traceRayWithAA(&camera, &viewport, spheres, NumSpheres, background.color, light, x, y, ImageWidth, ImageHeight);
            writeColour(OutputFile, color);
        }
    }
    #endif


    // Clean up
    fclose(InputFile);
    fclose(OutputFile);
    free(colors);
    free(spheres);
    freeWorld(&world);

    return 0;
}
    