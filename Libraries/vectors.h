#pragma once

#include <iostream>
#include <math.h>
#include <cmath>


class vec3{

private:
    double x;
    double y;
    double z;

public:

    vec3(double x, double y, double z);

    double get_x() const;
    double get_y() const;
    double get_z() const;
    double norma() const;

    vec3 operator+(const vec3& v) const;
    vec3 operator-(const vec3& v) const;
    vec3 operator*(const double& s) const;

    vec3 inverse() const;
    double dot(const vec3& v) const;
    vec3 cross(const vec3& v) const;

    vec3 projection(const vec3& v) const; // projeção do vetor em v
    vec3 reflect(const char& c) const;

    void normalize();
};

std::ostream& operator<<(std::ostream& os, const vec3& v);