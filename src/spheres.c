#include "spheres.h"
#include <stdlib.h>
#include "vector.h"
#include <stdio.h>
#include <math.h>  // Ensure math.h is included

void worldInit(World *world) {
    world->size = 0;
    world->capacity = 1;
    world->spheres = malloc(sizeof(Sphere *) * world->capacity);

    if (world->spheres == NULL) {
        fprintf(stderr, "Memory allocation failed in worldInit\n");
        exit(1);  // Exit if allocation fails, or handle it appropriately
    }
}

void freeWorld(World *world) {
    if (world->spheres != NULL) {
        for (int i = 0; i < world->size; i++) {
            if (world->spheres[i] != NULL) {
                free(world->spheres[i]);
                world->spheres[i] = NULL;  // Set to NULL after freeing
            }
        }
        free(world->spheres);
        world->spheres = NULL;  // Set to NULL to avoid dangling pointer
    }
    world->size = 0;
    world->capacity = 0;
}

void addSphere(World *world, Sphere *sphere) {
    if (world->size >= world->capacity) {
        world->capacity *= 2;
        Sphere **temp = realloc(world->spheres, sizeof(Sphere *) * world->capacity);
        if (temp == NULL) {
            fprintf(stderr, "Memory reallocation failed in addSphere\n");
            freeWorld(world);  // Free any allocated memory to prevent leaks
            exit(1);
        }
        world->spheres = temp;
    }
    world->spheres[world->size++] = sphere;
}

Sphere *createSphere(float radius, Vec3 position, Vec3 color) {
    Sphere *sphere = malloc(sizeof(Sphere));
    sphere->r = radius;
    sphere->pos = position;
    sphere->color = color;
    return sphere;
}

int doesIntersect(const Sphere *sphere, Vec3 rayPos, Vec3 rayDir, float *t) {
    Vec3 spherepos = sphere->pos;
    float sphererad = sphere->r;

    // Ensure the line below ends with a semicolon
    Vec3 V = subtract(rayPos, spherepos);  

    float a = dot(rayDir, rayDir);
    float b = 2.0f * dot(rayDir, V);
    float c = dot(V, V) - sphererad * sphererad;  // Corrected variable name
    
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0.0f) {
        return 0;   
    }

    float sqrtDiscriminant = sqrtf(discriminant);
    float t1 = (-b - sqrtDiscriminant) / (2.0f * a); // Closest intersection point
    float t2 = (-b + sqrtDiscriminant) / (2.0f * a); // Far intersection point
    
    if (t1 < 0.0f && t2 < 0.0f) {
        return 0; // No intersection
    }

    if (t1 > 0) {
        *t = t1; // Use the nearest valid intersection
        return 1;
    } else if (t2 > 0) {
        *t = t2; // Use the far intersection if the nearest is behind
        return 1;
    }

    return 0;
}
