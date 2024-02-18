#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI *traceUI;

#include "../fileio/images.h"
#include <glm/gtx/io.hpp>
#include <iostream>

using namespace std;
extern bool debugMode;

Material::~Material() {}



// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene *scene, const ray &r, const isect &i) const {
  // YOUR CODE HERE
 
  glm::dvec3 Q = r.at(i.getT());
  glm::dvec3 IBase = ke(i) + ka(i)*scene->ambient();
  double dotted;
  double dotted2;

  for (const auto &pLight: scene->getAllLights())
  {
    glm::dvec3 dir = pLight->getDirection(Q);
    glm::dvec3 R_direction = glm::reflect(-dir, i.getN());
    glm::dvec3 V_direction = -r.getDirection();

    dotted = glm::dot(R_direction, V_direction);
    dotted2 = glm::dot(dir, i.getN());

    glm::dvec3 specular = ks(i)*pLight->getColor()*std::pow((max(dotted, 0.0)), shininess(i));
    glm::dvec3 diffuse = kd(i)*pLight->getColor()*max(dotted2, 0.0);

    glm::dvec3 atten = pLight->distanceAttenuation(Q)*pLight->shadowAttenuation(r, Q);
    
    IBase = IBase + atten*(diffuse + specular);
  }
  return IBase;
}

TextureMap::TextureMap(string filename) {
  data = readImage(filename.c_str(), width, height);
  if (data.empty()) {
    width = 0;
    height = 0;
    string error("Unable to load texture map '");
    error.append(filename);
    error.append("'.");
    throw TextureMapException(error);
  }
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2 &coord) const {
  // YOUR CODE HERE
  //
  // In order to add texture mapping support to the
  // raytracer, you need to implement this function.
  // What this function should do is convert from
  // parametric space which is the unit square
  // [0, 1] x [0, 1] in 2-space to bitmap coordinates,
  // and use these to perform bilinear interpolation
  // of the values.

  double x = coord[0] * width;
  double y = coord[1] * height;

  int x_value = std::floor(x);
  int y_value = std::floor(y);

  y_value = (y_value >= height) ? y_value-1 : y_value;
  int x_value2 = (x_value < width-1) ? x_value+1 : x_value;
  int y_value2 = (y_value < height-1) ? y_value+1 : y_value;

  // by default, it's going to be 
  /*
    - x, y
    - x + 1, y
    - x, y + 1
    - x + 1, y + 1
  */

  // worst case, bottom right corner
  double q1 = (x_value2 - x) / (x_value2 - x_value);
  double q2 = (x - x_value) / (x_value2 - x_value);
  
  // bilinear interpolation! Thanks! I hate it!
  glm::dvec3 Q12 = getPixelAt(x_value, y_value); // top left
  glm::dvec3 Q11 = getPixelAt(x_value, y_value2); // bottom left
  glm::dvec3 Q22 = getPixelAt(x_value2, y_value); // top right
  glm::dvec3 Q21 = getPixelAt(x_value2, y_value2); // bottom right

  glm::dvec3 R1 = (Q11*q1) + (Q21*q2);
  glm::dvec3 R2 = (Q12*q1) + (Q22*q2);

  double r1 = (y_value2 - y) / (y_value2 - y_value);
  double r2 = (y - y_value) / (y_value2 - y_value);

  glm::dvec3 P = (R1*r1) + (R2*r2);
  return P;
}


glm::dvec3 TextureMap::getPixelAt(int x, int y) const {
  // YOUR CODE HERE
  //
  // In order to add texture mapping support to the
  // raytracer, you need to implement this function.
  int index = ((y*width)+x)*3;

  //cout<<"Width: " << width << "\t Height: " << height << "\n";
  //cout << "X: " << x << "\t Y: " << y << "\n\n";
  double dataR = data.at(index);
  double dataG = data.at(index+1);
  double dataB = data.at(index+2);

  glm::dvec3 returnColor = glm::dvec3(dataR/255, dataG/255, dataB/255);

  
  return returnColor;
}

glm::dvec3 MaterialParameter::value(const isect &is) const {
  if (0 != _textureMap)
    return _textureMap->getMappedValue(is.getUVCoordinates());
  else
    return _value;
}

double MaterialParameter::intensityValue(const isect &is) const {
  if (0 != _textureMap) {
    glm::dvec3 value(_textureMap->getMappedValue(is.getUVCoordinates()));
    return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
  } else
    return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}
