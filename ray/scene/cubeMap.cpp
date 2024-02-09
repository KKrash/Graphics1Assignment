#include "cubeMap.h"
#include "../scene/material.h"
#include "../ui/TraceUI.h"
#include "ray.h"
extern TraceUI *traceUI;

glm::dvec3 CubeMap::getColor(ray r) const {
  // YOUR CODE HERE
  // FIXME: Implement Cube Map here

  // look at each value and figure out which one is the largest out of the three
  // (pos and neg?)
  glm::dvec3 posi = r.getPosition();
  TextureMap textMap = TextureMap("filename"); // I don't know if I'm supposed to pass in something else

  // have to check pos or negative?
  double highest;
  double u;
  double v;
  if (posi.x > posi.y)
  {
    highest = posi.x;
    u = posi.y;
    v = posi.z;
    //textMap.setXposMap(textMap);
  }
  else
  {
    highest = posi.y;
    u = posi.x;
    v = posi.z;
  }

  if (highest > posi.z)
  {
    highest = posi.z;
    u = posi.x;
    v = posi.y;
  }
  double UNorm = u/highest;
  double VNorm = v/highest;

// expand the uv coordinates to be 1, 1 max

int x = UNorm*u;
int y = VNorm*v;

int index = (y*textMap.getWidth()+x)*3;

glm::dvec3 colors = textMap.getPixelAt(x, y);
//glm::dvec2 coords = material.getMappedValue(x, y);?? 

// can't tell if I'd just return "colors" here tbh
  return glm::dvec3();
}

CubeMap::CubeMap() {}

CubeMap::~CubeMap() {}

void CubeMap::setNthMap(int n, TextureMap *m) {
  if (m != tMap[n].get())
    tMap[n].reset(m);
}
