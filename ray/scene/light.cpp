#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3 &) const {
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}

glm::dvec3 DirectionalLight::shadowAttenuation(const ray &r,
                                               const glm::dvec3 &p) const {
  // YOUR CODE HERE: 
  
    isect i;
    glm::dvec3 d = getDirection(p);
    ray r2 = ray(p, d, glm::dvec3(1, 1, 1));
    bool intersectQ = scene->intersect(r2, i);
    glm::dvec3 Q = r2.at(i.getT());
    if (intersectQ) {
        return glm::dvec3(0, 0, 0);
    } 
  
  return glm::dvec3(1, 1, 1);
}

glm::dvec3 DirectionalLight::getColor() const { return color; }

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3 &) const {
  return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3 &P) const {
  // YOUR CODE HERE

  double att;
  double d = glm::distance(position, P);
  att = 1/(constantTerm + linearTerm*d + quadraticTerm*(std::pow(d, 2)));
  att = glm::min(1.0, att);
  return att;
}

glm::dvec3 PointLight::getColor() const { return color; }

glm::dvec3 PointLight::getDirection(const glm::dvec3 &P) const {
  return glm::normalize(position - P);
}

glm::dvec3 PointLight::shadowAttenuation(const ray &r,
                                         const glm::dvec3 &p) const {
  // YOUR CODE HERE:

    isect i;
    glm::dvec3 d = getDirection(p);
    ray r2 = ray(p, d, glm::dvec3(1, 1, 1));
    bool intersectQ = scene->intersect(r2, i);
    glm::dvec3 Q = r2.at(i.getT());
    if (intersectQ) {
      glm::dvec3 b = glm::lessThan(p, Q);
      if (b[0] == 1.0) {
        return glm::dvec3(0, 0, 0);
      }
    } 
  
  return glm::dvec3(1, 1, 1);
}

#define VERBOSE 0

