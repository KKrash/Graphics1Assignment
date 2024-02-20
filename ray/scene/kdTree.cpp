#include "kdTree.h"
#include <iostream>
using namespace std;

using namespace std;

glm::dvec3 splitPlane::getPosition() {
    return position;
}

double splitPlane::getSpecificPart(int axis)
{
    return position[axis];
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

    //TODO SORTING
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
        int leftcount = 0;
        int rightcount = 0;
        for (Geometry* obj : objList) {
            if (obj->getBoundingBox().getMin()[c.getAxis()] < c.getPosition()[c.getAxis()]) {
                leftcount++;
            }
            if (obj->getBoundingBox().getMax()[c.getAxis()] > c.getPosition()[c.getAxis()]) {
                rightcount++;
            }
            if (c.getPosition()[c.getAxis()] == obj->getBoundingBox().getMax()[c.getAxis()]
            && c.getPosition()[c.getAxis()] == obj->getBoundingBox().getMin()[c.getAxis()]) {
                if (obj->getNormal()[c.getAxis()] >= 0.0) {
                    rightcount++;
                } else {
                    leftcount++;
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
        double sam = ((leftcount*leftbbox.area())+(rightcount*rightbbox.area()))/bbox.area();
        if (minsam == -1 || sam < minsam) {
            minsam = sam;
            ret = c;
        }
        
        c.leftbbox = leftbbox;
        c.rightbbox = rightbbox;
    }
    ret.left = vector<Geometry*>();
    ret.right = vector<Geometry*>();
    for (Geometry* obj : objList) {
        if (obj->getBoundingBox().getMin()[ret.getAxis()] < ret.getPosition()[ret.getAxis()]) {
                ret.left.push_back(obj);
            }
            if (obj->getBoundingBox().getMax()[ret.getAxis()] > ret.getPosition()[ret.getAxis()]) {
                ret.right.push_back(obj);
            }
            if (ret.getPosition()[ret.getAxis()] == obj->getBoundingBox().getMax()[ret.getAxis()]
            && ret.getPosition()[ret.getAxis()] == obj->getBoundingBox().getMin()[ret.getAxis()]) {
                if (obj->getNormal()[ret.getAxis()] >= 0.0) {
                    ret.right.push_back(obj);
                } else {
                    ret.left.push_back(obj);
                }
        }
    }
    return ret;

}

Node buildTree(std::vector <Geometry*> objList, BoundingBox bbox, int depth) {
    cout << "Recursive Build Tree" << endl;
    if (objList.size() <= traceUI->getLeafSize() || ++depth == traceUI->getMaxDepth()) {
        return Node(nullptr, nullptr, nullptr, objList);
    }
    splitPlane sp = findBestSplitPlane(objList, bbox);
    vector<Geometry*> left = sp.left;
    vector<Geometry*> right = sp.right;

    if (right.empty() || left.empty()) {
        return Node(nullptr, nullptr, nullptr, objList);
    } else {
        Node left_node = buildTree(left, sp.leftbbox, depth+1);
        Node right_node = buildTree(right, sp.rightbbox, depth+1); 
        return Node(&sp, &left_node, &right_node, objList);
    }
}

bool Node::findIntersectionSplit(Node *n, ray &r, isect &i, double tmin, double tmax)
{
    if (n->leftNode == NULL && n->rightNode == NULL)
    {
        findIntersectionLeaf(n, r, i, tmin, tmax);
        return true;
    }
    double rayPosition = r.getPosition()[n->plane->getAxis()];
    double planePosition = n->plane->getPosition()[n->plane->getAxis()];
    // are they parallel to each other?
    if (rayPosition <= planePosition+RAY_EPSILON && rayPosition >= planePosition-RAY_EPSILON)
    {
        if (rayPosition < planePosition)
        {
            return findIntersectionSplit(n->leftNode, r, i, tmin, tmax);
        }
        else if (rayPosition > planePosition)
        {
            return findIntersectionSplit(n->rightNode, r, i, tmin, tmax);
        }
        else
        {
            bool leftSide = findIntersectionSplit(n->leftNode, r, i, tmin, tmax);
            bool rightSide = findIntersectionSplit(n->rightNode, r, i, tmin, tmax);
            if (leftSide || rightSide)
            {
                return true;
            }
        }
    }
    else
    {
        // they are not near-parallel, check each side
        bool intersectsLeft = (tmin > n->plane->leftbbox.getMin()[n->plane->getAxis()] && (tmax < n->plane->leftbbox.getMax()[n->plane->getAxis()]));
        bool intersectRight = (tmin > n->plane->rightbbox.getMin()[n->plane->getAxis()] && (tmax <= n->plane->rightbbox.getMax()[n->plane->getAxis()]));
        if (rayPosition < 0.0)
        {
            // heading in the "negative" direction, so check right then left
            if (intersectRight)
            {
                if (findIntersectionSplit(n->rightNode, r, i, tmin, tmax)) {return true;};
            }
            if (intersectsLeft)
            {
                if (findIntersectionSplit(n->leftNode, r, i, tmin, tmax)) {return true;};
            }
        }
        else if (rayPosition> 0.0)
        {
            // heading in the "positive" direction, so check left then right
            if (intersectsLeft)
            {
                if (findIntersectionSplit(n->leftNode, r, i, tmin, tmax)) {return true;};
            }
            if (intersectRight)
            {
                if (findIntersectionSplit(n->rightNode, r, i, tmin, tmax)) {return true;};
            }
        }
        else
        {
            // if they don't perfectly fit into the boxes? make sure to find how they intersect in the other stuff
            if (rayPosition < 0.0)
            {
                bool checkRight = findIntersectionSplit(n->rightNode, r, i, tmin, n->plane->rightbbox.getMin()[n->plane->getAxis()]);
                bool checkLeft = findIntersectionSplit(n->leftNode, r, i, n->plane->leftbbox.getMax()[n->plane->getAxis()], tmax);
                if (checkRight || checkLeft)
                {
                    return true;
                }
            }
            else if (rayPosition> 0.0)
            {
                bool checkLeft = findIntersectionSplit(n->leftNode, r, i, tmin, n->plane->leftbbox.getMax()[n->plane->getAxis()]);
                bool checkRight = findIntersectionSplit(n->rightNode, r, i, n->plane->rightbbox.getMin()[n->plane->getAxis()], tmax);
                if (checkLeft || checkRight)
                {
                    return true;
                }
            }
            
        }
    }
    // none fo these apply? At all? Then no intersection
    return false;
}

void Node::findIntersectionLeaf(Node *n, ray &r, isect &i, double tmin, double tmax)
{
    
    // peekaboo!
    for (int j = 0; j < n->getObjectList().size(); j++)
    {
        isect i_c_u; 
        if (objList.at(j)->intersect(r, i_c_u) && i_c_u.getT() >= tmin && i_c_u.getT() <= tmax)
        {
            i = i_c_u;
        }
    }
}

