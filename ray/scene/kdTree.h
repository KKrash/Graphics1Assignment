#ifndef KDTREE_H_
#define KDTREE_H_

#pragma once
#include <vector>
#include <glm/vec3.hpp>
class Geometry;
class BoundingBox;
// // Note: you can put kd-tree here


class splitNode{
    
    splitNode *leftNode;
    splitNode *rightNode;
public:
    splitNode(splitNode *leftNode, splitNode *rightNode);
};

class leafNode{
    std::vector <Geometry*> objList;

public:
    leafNode(std::vector <Geometry*> objList);
    std::vector <Geometry*> getObjectList() { return objList;}
    void addToObjectList(Geometry* shape) {objList.push_back(shape);}
    
};

class splitPlane{
public:
    

};

class kdTree{
    int axis;
    glm::dvec3 position;
    splitNode *root;
    std::vector <Geometry*> objList;

    int depthLimit = 5; // arbitrary limit rn

public:
    kdTree(int axis, glm::dvec3 position, splitNode *root, std::vector<Geometry*> objList);

    leafNode buildTree(std::vector <Geometry*> objList, BoundingBox bbox, int depth, int leafSize);
};


#endif