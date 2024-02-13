#include "trimesh.h"
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <float.h>
#include <string.h>
#include "../ui/TraceUI.h"
#include <iostream>
extern TraceUI *traceUI;
extern TraceUI *traceUI;

using namespace std;

Trimesh::~Trimesh() {
  for (auto f : faces)
    delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3 &v) { vertices.emplace_back(v); }

void Trimesh::addNormal(const glm::dvec3 &n) { normals.emplace_back(n); }

void Trimesh::addColor(const glm::dvec3 &c) { vertColors.emplace_back(c); }

void Trimesh::addUV(const glm::dvec2 &uv) { uvCoords.emplace_back(uv); }

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c) {
  int vcnt = vertices.size();

  if (a >= vcnt || b >= vcnt || c >= vcnt)
    return false;

  TrimeshFace *newFace = new TrimeshFace(this, a, b, c);
  if (!newFace->degen)
    faces.push_back(newFace);
  else
    delete newFace;

  // Don't add faces to the scene's object list so we can cull by bounding
  // box
  return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char *Trimesh::doubleCheck() {
  if (!vertColors.empty() && vertColors.size() != vertices.size())
    return "Bad Trimesh: Wrong number of vertex colors.";
  if (!uvCoords.empty() && uvCoords.size() != vertices.size())
    return "Bad Trimesh: Wrong number of UV coordinates.";
  if (!normals.empty() && normals.size() != vertices.size())
    return "Bad Trimesh: Wrong number of normals.";

  return 0;
}

bool Trimesh::intersectLocal(ray &r, isect &i) const {
  bool have_one = false;
  for (auto face : faces) {
    isect cur;
    if (face->intersectLocal(r, cur)) {
      if (!have_one || (cur.getT() < i.getT())) {
        i = cur;
        have_one = true;
      }
    }
  }
  if (!have_one)
    i.setT(1000.0);
  return have_one;
}

bool TrimeshFace::intersect(ray &r, isect &i) const {
  return intersectLocal(r, i);
}


// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray &r, isect &i) const {
  // YOUR CODE HERE
  //
  // FIXME: Add ray-trimesh intersection

  /* To determine the color of an intersection, use the following rules:
     - If the parent mesh has non-empty `uvCoords`, barycentrically interpolate
       the UV coordinates of the three vertices of the face, then assign it to
       the intersection using i.setUVCoordinates().
     - Otherwise, if the parent mesh has non-empty `vertexColors`,
       barycentrically interpolate the colors from the three vertices of the
       face. Create a new material by copying the parent's material, set the
       diffuse color of this material to the interpolated color, and then 
       assign this material to the intersection.
     - If neither is true, assign the parent's material to the intersection.
  */

  // NOTE: THE DISTANCE AND NORMALIZATION OF THE A, B, C, ARE DONE ALREADY-
  //        UNDER VARIABLE "NORMAL"  
  Trimesh::UVCoords UVCoor = parent->uvCoords;
      Trimesh::VertColors VColors = parent->vertColors;

      // Compute the face normal here, not on the fly
  double t = -(glm::dot(normal, r.getPosition())-dist)/glm::dot(normal, r.getDirection());
  
  if (t < RAY_EPSILON) {
    return false;
  }
  i.setT(t);
  glm::dvec3 Q = r.getPosition() + t*r.getDirection();
  glm::dvec3 a_coords = parent->vertices[ids[0]];
  glm::dvec3 b_coords = parent->vertices[ids[1]];
  glm::dvec3 c_coords = parent->vertices[ids[2]];
  

  
  bool intersection = true;
  
  
  if (!((glm::dot(glm::cross(b_coords-a_coords, Q-a_coords), normal) > -RAY_EPSILON) 
  && (glm::dot(glm::cross(c_coords-b_coords, Q-b_coords), normal) > -RAY_EPSILON) 
  && (glm::dot(glm::cross(a_coords-c_coords, Q-c_coords), normal) > -RAY_EPSILON))) {
      return false;
    }

  double a1 = glm::length(glm::cross(c_coords-b_coords, Q-b_coords))/2.0;
  double a2 = glm::length(glm::cross(a_coords-c_coords, Q-c_coords))/2.0;
  double a3 = glm::length(glm::cross(b_coords-a_coords, Q-a_coords))/2.0;

  double total_area = a1+a2+a3;

  double alpha = a1/total_area;
  double beta = a2/total_area;
  double gamma = a3/total_area;

  assert(alpha+beta+gamma > 1 - RAY_EPSILON && alpha+beta+gamma < 1 + RAY_EPSILON);
  i.setBary(glm::dvec3(alpha, beta, gamma));
  i.setObject(parent);
  if (!parent->normals.empty()) {

    i.setN(glm::normalize(alpha*parent->normals[ids[0]]+
    beta*parent->normals[ids[1]]+
    gamma*parent->normals[ids[2]]));
  } else {
    i.setN(normal);
  }
  
  
  if(UVCoor.size() != 0)
  {
    // barycentrically interpolate UV Coordinates the 3 vertices of the face
    glm::dvec2 UV = alpha*UVCoor[ids[0]]+
    beta*UVCoor[ids[1]]+
    gamma*UVCoor[ids[2]];
    i.setUVCoordinates(UV);
  }
  else if(VColors.size() != 0)
  {
    // barycentrically interpolate the colors 
    // new material 
    // diffuse to color of new material to interpolated color
    // assign material to intersection
    glm::dvec3 kd = alpha*VColors[ids[0]]+beta*VColors[ids[1]]+gamma*VColors[ids[2]];
    Material m =  i.getMaterial();
    m.setDiffuse(kd);
    i.setMaterial(m);
  } else {
    i.setMaterial(parent->getMaterial());
  }
  //i.setObject(this->parent);
  return intersection;
}

// Once all the verts and faces are loaded, per vertex normals can bes
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals() {
  int cnt = vertices.size();
  normals.resize(cnt);
  std::vector<int> numFaces(cnt, 0);

  for (auto face : faces) {
    glm::dvec3 faceNormal = face->getNormal();

    for (int i = 0; i < 3; ++i) {
      normals[(*face)[i]] += faceNormal;
      ++numFaces[(*face)[i]];
    }
  }

  for (int i = 0; i < cnt; ++i) {
    if (numFaces[i])
      normals[i] /= numFaces[i];
  }

  vertNorms = true;
}

