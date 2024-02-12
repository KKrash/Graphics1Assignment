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

  int x_value = coord[0]*width;
  int y_value = coord[1]*height;

  glm::dvec3 pixelColorMain = getPixelAt(x_value, y_value);

  // by default, it's going to be 
  /*
    - x, y
    - x + 1, y
    - x, y + 1
    - x + 1, y + 1
  */
  glm::dvec3 pixelBeneath;
  glm::dvec3 pixelRight;
  glm::dvec3 pixelDiag;
  // worst case, bottom right corner
  if (x_value == width && y_value == height)
  {
    int x_value2 = x_value-1;
    int y_value2 = y_value-1;
    // technically pixelLeft but shhhhh
    pixelRight = getPixelAt(x_value2, y_value);
    pixelDiag = getPixelAt(x_value2, y_value2);
  }
  else
  {
    pixelBeneath = (y_value == height) ? getPixelAt(x_value, y_value-1) : getPixelAt(x_value, y_value+1);
    pixelRight = (x_value == width) ? getPixelAt(x_value-1, y_value) : getPixelAt(x_value+1, y_value);
    if(x_value == width)
    {
      pixelDiag = getPixelAt(x_value-1, y_value+1);
    }
    else if (y_value == height)
    {
      pixelDiag = getPixelAt(x_value+1, y_value-1);
    }
    else
    {
      pixelDiag = getPixelAt(x_value+1, y_value+1);
    }
  }

  double red = (pixelColorMain[0] + pixelBeneath[0] + pixelDiag[0] + pixelRight[0]) / 4.0;
  double green = (pixelColorMain[1] + pixelBeneath[1] + pixelDiag[1] + pixelRight[1]) / 4.0;
  double blue = (pixelColorMain[2] + pixelBeneath[2] + pixelDiag[2] + pixelRight[2]) / 4.0;

  glm::dvec3 bilinearValues = glm::dvec3(red, green, blue);

  return bilinearValues;
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const {
  // YOUR CODE HERE
  //
  // In order to add texture mapping support to the
  // raytracer, you need to implement this function.
  int index = (y*width+x)*3;
  double dataR = data[index];
  double dataG = data[index+1];
  double dataB = data[index+2];
  glm::dvec3 returnColor = glm::dvec3(dataR, dataG, dataB);
  
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
