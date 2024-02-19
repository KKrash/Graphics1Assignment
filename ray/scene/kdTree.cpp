#include "kdTree.h"
#include "scene.h"

using namespace std;

glm::dvec3 splitPlane::getPostition() {
    return position;
}

glm::dvec3 splitPlane::getAxis() {
    return axis;
}

void splitPlane::setPosition(glm::dvec3 p ){
    position = p;
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
        int leftCount = 0;
        int rightCount = 0;
        double leftbbox = 0;
        double rightbbox = 0;
        for (Geometry* obj : objList) {
            if (obj->getBoundingBox().getMax()[c.getAxis()] <= c.getPostition()[c.getAxis()]) {
                leftCount++;
            }
            if (obj->getBoundingBox().getMin()[c.getAxis()] >= c.getPostition()[c.getAxis()]) {
                rightCount++;
            }
        }
        glm::dvec3 p = c.getPostition();
        for (double val = 0; val < 3; val++) {
            if (val != c.getAxis()) {
                p[val] = bbox.getMax()[val];
            }
        }
        p = p - bbox.getMin();
        leftbbox = glm::abs(p[0]*p[1]*p[2]);
        p = c.getPostition();
        for (double val = 0; val < 3; val++) {
            if (val != c.getAxis()) {
                p[val] = bbox.getMin()[val];
            }
        }
        p = bbox.getMax() - p;
        rightbbox = glm::abs(p[0]*p[1]*p[2]);

        double sam = ((leftCount*leftbbox)+(rightCount*rightbbox))/bbox.volume();
        if (minsam == -1 || sam < minsam) {
            minsam = sam;
            ret = c;
        }
        return ret;

    }

}


