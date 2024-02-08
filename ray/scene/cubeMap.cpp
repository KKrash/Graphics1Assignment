#include "cubeMap.h"
#include "../scene/material.h"
#include "../ui/TraceUI.h"
#include "ray.h"
extern TraceUI *traceUI;

glm::dvec3 CubeMap::getColor(ray r) const {
  // YOUR CODE HERE
  // FIXME: Implement Cube Map here

  // look at each value and figure out which one is the largest out of the three
  glm::dvec3 posi = r.getPosition();
  double highest = posi.x > posi.y ? posi.x : posi.y;
  bool xy = posi.x > posi.y ? true : false;
  highest = highest > posi.z ? highest : posi.z;
  bool higher = highest > posi.z ? true : false;

  
  // barycentric coordinates and also renormalize it to the longest one
  // now you get u,v, and then getPixelAt?? Stride through the 1D array?
  // and then yeah return the color (there might be a getColor array somewhere
  // but hell if I know where it is WE ARE THE GET COLOR)

  return glm::dvec3();
}

CubeMap::CubeMap() {}

CubeMap::~CubeMap() {}

void CubeMap::setNthMap(int n, TextureMap *m) {
  if (m != tMap[n].get())
    tMap[n].reset(m);
}
