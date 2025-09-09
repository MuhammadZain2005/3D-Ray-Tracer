#include "vector.h"
#include <math.h>
#include <stdio.h>

// Vector Addition
Vec3 add(Vec3 v1, Vec3 v2) {
    Vec3 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    return result;
}

// Vector Subtraction
Vec3 subtract(Vec3 v1, Vec3 v2) {
    Vec3 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    return result;
}

// Vector Scalar Multiplication
Vec3 scalarMultiply(float s, Vec3 v) {
    Vec3 result;
    result.x = s * v.x;
    result.y = s * v.y;
    result.z = s * v.z;
    return result;
}

// Vector Scalar Division
Vec3 scalarDivide(Vec3 v, float d) {
    Vec3 result = {0, 0, 0};
    if (d == 0) {
        fprintf(stderr, "Error: Division by zero in scalarDivide\n");
        return result;
    }
    result.x = v.x / d;
    result.y = v.y / d;
    result.z = v.z / d;
    return result;
}

// Vector Normalization
Vec3 normalize(Vec3 v) {
    float len = length(v);
    if (len == 0) {
        fprintf(stderr, "Warning: Attempting to normalize a zero-length vector\n");
        return v; // Returning the zero vector as is
    }
    return scalarDivide(v, len);
}

// Vector Dot Product
float dot(Vec3 v1, Vec3 v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

// Vector Length Squared
float length2(Vec3 v) {
    return dot(v, v);
}

// Vector Length
float length(Vec3 v) {
    return sqrt(length2(v));
}

// Vector Distance Squared
float distance2(Vec3 v1, Vec3 v2) {
    return length2(subtract(v1, v2));
}

// Vector Distance
float distance(Vec3 v1, Vec3 v2) {
    return sqrt(distance2(v1, v2));
}
