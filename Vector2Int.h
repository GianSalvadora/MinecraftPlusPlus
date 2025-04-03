#ifndef VECTOR2INT_H
#define VECTOR2INT_H

#include <iostream>

struct Vector2Int {
    int x, y;

    // Default constructor (0, 0)
    Vector2Int() : x(0), y(0) {}

    // Constructor for (int, int)
    Vector2Int(int xVal, int yVal) : x(xVal), y(yVal) {}

    // Display function
    void display() const {
        std::cout << "x: " << x << ", y: " << y << std::endl;
    }

    // Add another vector
    Vector2Int operator+(const Vector2Int& other) {
        return Vector2Int(x + other.x, y + other.y);
    }

    // Subtract another vector
    Vector2Int operator-(const Vector2Int& other) {
        return Vector2Int(x - other.x, y - other.y);
    }

    // Scalar multiplication
    Vector2Int operator*(int scalar) {
        return Vector2Int(x * scalar, y * scalar);
    }

    // Scalar division
    Vector2Int operator/(int scalar) {
        if (scalar != 0) {
            return Vector2Int(x / scalar, y / scalar);
        }
        std::cerr << "Error: Division by zero!" << std::endl;
        return *this; // Return the current object to avoid undefined behavior
    }

    bool operator==(const Vector2Int& other) {
        return (other.x == x && other.y == y);
    }
};

#endif // VECTOR2INT_H
