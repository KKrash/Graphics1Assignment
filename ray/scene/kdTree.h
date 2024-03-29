#ifndef KDTREE_H_
#define KDTREE_H_

#pragma once
#include <vector>
#include <glm/vec3.hpp>
#include "../ui/TraceUI.h"
#include "scene.h"

class Geometry;
class BoundingBox;
extern TraceUI *traceUI;

// // Note: you can put kd-tree here

class splitPlane{
    int axis;
    glm::dvec3 position;
    
public:
    splitPlane(int a, glm::dvec3 p);
    glm::dvec3 getPosition();
    double getSpecificPart(int axis);
    void setPosition(glm::dvec3 p);
    int getAxis();
    int leftCount = 0;
    int rightCount = 0;
    BoundingBox leftbbox;
    BoundingBox rightbbox;
    std::vector <Geometry*> left;
    std::vector <Geometry*> right;
};

class Node{
    splitPlane* plane;
    Node *leftNode;
    Node *rightNode;
    std::vector <Geometry*> objList;

public:
    Node(splitPlane* p, Node *l, Node *r, std::vector<Geometry*> ol);
    std::vector <Geometry*> getObjectList() {return objList;}
    void addToObjectList(Geometry* shape) {objList.push_back(shape);}
    bool findIntersectionSplit(Node *n, ray &r, isect &i, double tmin, double tmax);
    void findIntersectionLeaf (Node *n, ray &r, isect &i, double tmin, double tmax);
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

Node buildTree(std::vector <Geometry*> objList, BoundingBox bbox, int depth);
splitPlane findBestSplitPlane(std::vector <Geometry*> objList, BoundingBox bbox);




#endif