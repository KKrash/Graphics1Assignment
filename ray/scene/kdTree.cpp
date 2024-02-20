#include "kdTree.h"


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
        return Node(&sp, &left_node, &right_node, vector<Geometry*>());
    }
}

bool Node::findIntersectionSplit(Node n, ray &r, isect &i, double tmin, double tmax)
{
    if (left == nullptr && right == nullptr) {
        return findIntersectionLeaf(r, i, tmin, tmax);
    }
    glm::dvec3 RDirect = r.getDirection();
    int axis = n.plane->getAxis();
    double LorR = RDirect[axis];
    glm::dvec3 normal = glm::dvec3(0.0, 0.0, 0.0);
    normal[axis] = 1.0;
    double t = glm::dot((n.plane->getPosition() - r.getPosition()), normal)/glm::dot(r.getDirection(), normal);
    if (!(t >= tmin && t <= tmax)) {
        return false;
    }
    if (glm::abs(RDirect[axis] - n.plane->getPosition()[axis]) <= RAY_EPSILON) {
        RDirect[axis] += RAY_EPSILON;
    } else {

    
    // if ray is nearly parallel to the split plane, calculate as near to parallel as possible
    // basically close enough to parallel, then check if it's less than or greater than the position of the split plane axis thing
    // and the go down that 
    // else
    //{
        double t = glm::dot((n.plane->getPosition() - r.getPosition()), normal)/glm::dot(r.getDirection(), normal);
        bool lefthit = glm::dot((r.getPosition() - n.plane->getPosition()), normal) < 0.0;
        bool righthit = glm::dot((r.getPosition() - n.plane->getPosition()), normal) > 0.0;
        if (lefthit && !righthit)
        {
            
            if(findIntersectionSplit(*(n.leftNode), r, i, tmin, tmax))
            {
                return true;
            }
        }
        else if (!lefthit && righthit)
        {
            if(findIntersectionSplit(*(n.rightNode), r, i, tmin, tmax))
            {
                return true;
            }
        }
        else 
        {   
             
            // find the nearest interaction, and if that's true return true
            // find the farthest interaction, and then if this is true return true
        }
    }
    return false;
}

bool Node::findIntersectionLeaf(ray &r, isect &i, double tmin, double tmax)
{
    // peekaboo!
    bool has_inter = false;
    for (int j = 0; j < objList.size(); j++)
    {
        isect i_c_u;
        objList.at(j)->intersect(r, i_c_u);
        if (objList.at(j)->intersect(r, i_c_u) && i_c_u.getT() >= tmin && i_c_u.getT() <= tmax)
        {
            if (!has_inter || i_c_u.getT() < i.getT()){
                i.setT(i_c_u.getT());
                has_inter = true;
            }
        }
    }
    return has_inter;
}

