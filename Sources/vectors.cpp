#include "../Libraries/vectors.h"

vec3::vec3(double x, double y, double z){
    this->x = x;
    this->y = y;
    this->z = z;
}

double vec3::get_x() const{
    return this->x;
}

double vec3::get_y() const{
    return this->y;
}

double vec3::get_z() const{
    return this->z;
}

double vec3::norma() const{
    return std::sqrt(std::pow(this->get_x(), 2) + std::pow(this->get_y(), 2) + std::pow(this->get_z(), 2));
}

vec3 vec3::operator+(const vec3& v) const{
    
    double x = this->get_x() + v.get_x();
    double y = this->get_y() + v.get_y();
    double z = this->get_z() + v.get_z();

    return vec3(x,y,z);
}

vec3 vec3::operator-(const vec3& v) const{
    
    double x = this->get_x() - v.get_x();
    double y = this->get_y() - v.get_y();
    double z = this->get_z() - v.get_z();

    return vec3(x,y,z);
}
    
vec3 vec3::operator*(const double& s) const{
    return vec3((this->get_x() * s), (this->get_y() * s), (this->get_z() * s));
}

std::ostream& operator<<(std::ostream& os, const vec3& v){
    return os << "vec3(" << v.get_x() << ", " << v.get_y() << ", " << v.get_z() << ")";
}

vec3 vec3::inverse() const{
    return (*this * -1);
}

double vec3::dot(const vec3& v) const{
    // Somatório de i até n (1 até 3 --> Vetores 3D) de Ai * Bi
    return (this->get_x() * v.get_x()) + (this->get_y() * v.get_y()) + (this->get_z() * v.get_z());
}

vec3 vec3::cross(const vec3& v) const{
    //Cálculo via Determinante.

    double x = (this->get_y() * v.get_z()) - (this->get_z() * v.get_y());
    double y = (this->get_z() * v.get_x()) - (this->get_x() * v.get_z());
    double z = (this->get_x() * v.get_y()) - (this->get_y() * v.get_x());

    return vec3(x,y,z);
}

vec3 vec3::projection(const vec3& v) const{
    // projeção do vetor atual (this) em v
    double a = this->dot(v);
    double b = v.dot(v);

    double l = a / b;

    return (v * l);
}

vec3 vec3::reflect(const char& c) const{
    /* Reflexão relativa à coordenada z == Reflexão relativa ao plano xy  
       --> Preserva as coordenadas xy e inverte z.
    */

    /* |1 0 0 |   |x|    |x |
       |0 1 0 | . |y| =  |y |   MATRIZ DE TRANSFORMAÇÃO -> reflect_z
       |0 0 -1|   |z|    |-z| 
    */

    if(c == 'z'){
        return vec3(this->get_x(), this->get_y(), this->get_z() * -1);
    }else if(c == 'x'){
        return vec3(this->get_x() * -1, this->get_y(), this->get_z());
    }else if (c == 'y') {
        return vec3(this->get_x(), this->get_y() * -1, this->get_z());
    }else {
        std::cout << "Reflexão inválida" << std::endl;
        return vec3(0.0, 0.0, 0.0);
    }
}

void vec3::normalize(){
    double d = std::sqrt(std::pow(this->get_x(),2) + std::pow(this->get_y(),2));

    this->x = x / d;
    this->y = y / d;
    this->z = z / d;
}