#ifndef KDTREE_H_
#define KDTREE_H_

#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include "ray.h"
class Geometry;
class BoundingBox;
// // Note: you can put kd-tree here



class splitPlane{
    int axis;
    glm::dvec3 position;

public:
    splitPlane(int axis, glm::dvec3 position);
    glm::dvec3 getPostition();
    void setPosition(glm::dvec3 p);
    int getAxis();
    int leftCount = 0 ;
    int rightCount = 0;
};

class Node{
    splitPlane plane;
    Node *leftNode;
    Node *rightNode;
    std::vector <Geometry*> objList;

public:
    Node(splitPlane plane, Node *leftNode, Node *rightNode, std::vector<Geometry*> objList);
    std::vector <Geometry*> getObjectList() {return objList;}
    void addToObjectList(Geometry* shape) {objList.push_back(shape);}
    bool findIntersectionSplit (ray &r, isect i, double tmin, double tmax);
    bool findintersectionLeaf (ray &r, isect i, double tmin, double tmax);
};

class kdTree{
    int axis;
    glm::dvec3 position;
    Node *root;
    std::vector <Geometry*> objList;
    int depthLimit = 5; // arbitrary limit rn

public:
    kdTree(int axis, glm::dvec3 position, Node *root, std::vector<Geometry*> objList);
};

Node buildTree(std::vector <Geometry*> objList, BoundingBox bbox, int depth, int leafSize);
splitPlane findBestSplitPlane(std::vector <Geometry*> objList, BoundingBox bbox);


#endif