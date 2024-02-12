#include "cubeMap.h"
#include "../scene/material.h"
#include "../ui/TraceUI.h"
#include "ray.h"
extern TraceUI *traceUI;

glm::dvec3 CubeMap::getColor(ray r) const {
  // YOUR CODE HERE
  // FIXME: Implement Cube Map here

  // look at each value and figure out which one is the largest out of the three
  // and then set the u and v coordinates accordingly

  glm::dvec3 posi = r.getDirection();
  CubeMap tm;
  double highest;
  double u;
  double v;
  if (glm::abs(posi.x) > glm::abs(posi.y))
  {
    highest = posi.x;
    u = posi.y;
    v = -posi.z;
    tm.setXposMap();
    if(posi.x < 0)
    {
      u = posi.y;
      v = posi.z;
      tm.setXnegMap();
    }
  }
  else
  {
    highest = posi.y;
    u = -posi.z;
    v = posi.x;
    tm.setYposMap();
    if(posi.y < 0)
    {
      u = posi.z;
      v = posi.x;
      tm.setYnegMap();
    }
  }

  if (highest < glm::abs(posi.z))
  {
    highest = posi.z;
    u = posi.y;
    v = posi.x;
    tm.setZposMap();
    if (posi.z < 0)
    {
      u = posi.y;
      v = -posi.x;
      tm.setZnegMap();
    }
  }
  double UNorm = ((u/highest) + 1)/2;
  double VNorm = ((v/highest) + 1)/2;

const glm::dvec2 coordinates = glm::dvec2(UNorm, VNorm);
glm::dvec3 colors = tm.getMappedValue(coordinates);
return colors;
}

CubeMap::CubeMap() {}

CubeMap::~CubeMap() {}

void CubeMap::setNthMap(int n, TextureMap *m) {
  if (m != tMap[n].get())
    tMap[n].reset(m);
}
