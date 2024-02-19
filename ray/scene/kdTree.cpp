#include "kdTree.h"


using namespace std;

glm::dvec3 splitPlane::getPosition() {
    return position;
}

int splitPlane::getAxis() {
    return axis;
}

void splitPlane::setPosition(glm::dvec3 p ){
    position = p;
}

splitPlane::splitPlane(int a, glm::dvec3 p) {
    axis = a; 
    position = p;
}

Node::Node(splitPlane* p, Node *l, Node *r, std::vector<Geometry*> ol) {
    plane = p; 
    leftNode = l;
    rightNode = r;
    objList = ol;
}

splitPlane findBestSplitPlane(std::vector <Geometry*> objList, BoundingBox bbox){
    vector<splitPlane> candidates = vector<splitPlane>();
    for (int axis = 0; axis < 3; axis++) {
        for (Geometry* obj : objList) {
            glm::dvec3 temp = glm::dvec3((0.0, 0.0, 0.0));
            splitPlane sp1 = splitPlane(axis, temp);
            splitPlane sp2 = splitPlane(axis, temp);
            temp[axis] = obj->getBoundingBox().getMin()[axis];
            sp1.setPosition(temp);
            temp[axis] = obj->getBoundingBox().getMax()[axis];
            sp2.setPosition(temp);

            candidates.push_back(sp1);
            candidates.push_back(sp2);
            
        }
    }
    double minsam = -1;
    splitPlane ret = candidates[0];
    for (splitPlane c : candidates) {


        for (Geometry* obj : objList) {
            if (obj->getBoundingBox().getMin()[c.getAxis()] < c.getPosition()[c.getAxis()]) {
                c.left.push_back(obj);
            }
            if (obj->getBoundingBox().getMax()[c.getAxis()] > c.getPosition()[c.getAxis()]) {
                c.right.push_back(obj);
            }
            if (c.getPosition()[c.getAxis()] == obj->getBoundingBox().getMax()[c.getAxis()]
            && c.getPosition()[c.getAxis()] == obj->getBoundingBox().getMin()[c.getAxis()]) {
                if (obj->getNormal()[c.getAxis()] >= 0.0) {
                    c.right.push_back(obj);
                } else {
                    c.left.push_back(obj);
                }
            }
        }
        glm::dvec3 p = c.getPosition();
        for (double val = 0; val < 3; val++) {
            if (val != c.getAxis()) {
                p[val] = bbox.getMax()[val];
            }
        }
        BoundingBox leftbbox = BoundingBox(bbox.getMin(), p);
        for (double val = 0; val < 3; val++) {
            if (val != c.getAxis()) {
                p[val] = bbox.getMin()[val];
            }
        }
        BoundingBox rightbbox = BoundingBox(p, bbox.getMax());
        double sam = ((c.left.size()*leftbbox.volume())+(c.right.size()*rightbbox.volume()))/bbox.volume();
        if (minsam == -1 || sam < minsam) {
            minsam = sam;
            ret = c;
        }
        
        c.leftbbox = leftbbox;
        c.rightbbox = rightbbox;
    }
    return ret;

}

Node buildTree(std::vector <Geometry*> objList, BoundingBox bbox, int depth, int leafSize) {
    //change later have to get from traceui
    int depthLimit = 0;
    if (objList.size() <= leafSize || ++depth == depthLimit) {
        return Node(nullptr, nullptr, nullptr, objList);
    }
    splitPlane sp = findBestSplitPlane(objList, bbox);
    vector<Geometry*> left = sp.left;

    vector<Geometry*> right = sp.right;

    if (right.empty() || left.empty()) {
        return Node(nullptr, nullptr, nullptr, objList);
    } else {
        Node left_node = buildTree(left, sp.leftbbox, depth, leafSize);
        Node right_node = buildTree(right, sp.rightbbox, depth, leafSize); 
        return Node(&sp, &left_node, &right_node, vector<Geometry*>());
    }
}

