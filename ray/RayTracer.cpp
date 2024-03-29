// The main ray tracer.

#pragma warning(disable : 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"
#include "scene/scene.h"
#include "scene/kdTree.h"

#include "parser/JsonParser.h"
#include "parser/Parser.h"
#include "parser/Tokenizer.h"
#include <json.hpp>

#include "ui/TraceUI.h"
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <string.h> // for memset

#include <fstream>
#include <iostream>

using namespace std;
extern TraceUI *traceUI;

// Use this variable to decide if you want to print out debugging messages. Gets
// set in the "trace single ray" mode in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates
// (x,y), through the projection plane, and out into the scene. All we do is
// enter the main ray-tracing method, getting things started by plugging in an
// initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

glm::dvec3 RayTracer::trace(double x, double y) {
  // Clear out the ray cache in the scene for debugging purposes,
  if (TraceUI::m_debug) {
    scene->clearIntersectCache();
  }

  ray r(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0), glm::dvec3(1, 1, 1),
        ray::VISIBILITY);
  scene->getCamera().rayThrough(x, y, r);
  double dummy;
  glm::dvec3 ret =
      traceRay(r, glm::dvec3(1.0, 1.0, 1.0), traceUI->getDepth(), dummy);
  ret = glm::clamp(ret, 0.0, 1.0);
  return ret;
}

glm::dvec3 RayTracer::tracePixel(int i, int j) {
  glm::dvec3 col(0, 0, 0);

  if (!sceneLoaded())
    return col;

  double x = double(i) / double(buffer_width);
  double y = double(j) / double(buffer_height);

  unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;

  if(traceUI->aaSwitch())
  {
    double stride = 1.0/double(samples);
    double x_stride = stride / double (buffer_width);
    double y_stride = stride / double (buffer_height);

    double xDup = x;
    double yDup = y;

    for (int ycross = 0; ycross < samples; ycross++)
    {
      for (int xcross = 0; xcross < samples; xcross++)
      {
        col += trace(xDup, yDup) / double(samples*samples);
        xDup += x_stride;
      }
      xDup = x;
      yDup += y_stride;
    }

    pixel[0] = (int)(255.0 * (col[0]));
    pixel[1] = (int)(255.0 * (col[1]));
    pixel[2] = (int)(255.0 * (col[2]));
  }
  else
  {
    col = trace(x, y);

    pixel[0] = (int)(255.0 * (col[0]));
    pixel[1] = (int)(255.0 * (col[1]));
    pixel[2] = (int)(255.0 * (col[2]));
  }

  return col;
}

#define VERBOSE 0

// Do recursive ray tracing! You'll want to insert a lot of code here (or places
// called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray &r, const glm::dvec3 &thresh, int depth,
                               double &t) {
  if (depth < 0) {
    return glm::dvec3(0.0, 0.0, 0.0);
  }
  isect i;
  glm::dvec3 I;
#if VERBOSE
  std::cerr << "== current depth: " << depth << std::endl;
#endif
  bool intersection = scene->intersect(r, i);
  if (intersection) { 
    // YOUR CODE HERE

    glm::dvec3 Q = r.at(i.getT());
    const Material &m = i.getMaterial();
    I = m.shade(scene.get(), r, i);
    
    if (((I[0]+I[1]+I[2])) > (thresh[0] + thresh[1] + thresh[2])) {
      return I;
    }
    double n_i;
    double n_t;
    
    glm::dvec3 R_direction = glm::reflect(r.getDirection(), i.getN());
    ray R = ray(r);
    R.setDirection(R_direction);
    R.setPosition(Q);
    I = I + m.kr(i) * traceRay(R, thresh, depth-1, t);
    
 // Determining TIR⃗
    double dotty = glm::dot(i.getN(), -r.getDirection());
    if(dotty > 0)
    {
      n_i = 1; // air index
      n_t = m.index(i);
    }
    else if (dotty < 0)
    {
      n_t = 1; // air index
      n_i = m.index(i);
    }
    double nSquare = std::pow((n_i/n_t), 2);
    double cosine = std::sqrt(1-nSquare*(1-(std::pow(dotty, 2))));
    if ((m.kt(i)[0] > 0 || m.kt(i)[1] > 0 || m.kt(i)[2] > 0) && cosine > 0)
    {
      // now we refract
      glm::dvec3 T;
      if (dotty > 0) // entering
      {
        T = glm::refract(r.getDirection(), i.getN(), (n_i/n_t));
      }
      else if (dotty < 0) // exitting
      {
        T = glm::refract(r.getDirection(), -i.getN(), (n_i/n_t));
      }
      ray R_refract (Q, T, glm::dvec3(0.0), ray::REFRACTION);
      I = I + m.kt(i) * traceRay(R_refract, thresh, depth-1, t);
    }
    else // TIR
    {
      glm::dvec3 Ref = glm::reflect(r.getDirection(), i.getN());
      ray R_reflect (Q, Ref, glm::dvec3(0.0), ray::REFRACTION);
      I = I + m.kr(i) * traceRay(R_reflect, thresh, depth-1, t);
    }

  } else {
    // No intersection. This ray travels to infinity, so we color
    // it according to the background color, which in this (simple)
    // case is just black.
    bool cubeMapExists = traceUI->cubeMap();
    if (cubeMapExists)
    {
      I = traceUI->getCubeMap()->getColor(r);
    }
    else
    {
      I = glm::dvec3(0.0, 0.0, 0.0);
    }
  }
#if VERBOSE
  std::cerr << "== depth: " << depth + 1 << " done, returning: " << colorC
            << std::endl;
#endif
  return I;
}

RayTracer::RayTracer()
    : scene(nullptr), buffer(0), thresh(0), buffer_width(0), buffer_height(0),
      m_bBufferReady(false) {
}

RayTracer::~RayTracer() {}

void RayTracer::getBuffer(unsigned char *&buf, int &w, int &h) {
  buf = buffer.data();
  w = buffer_width;
  h = buffer_height;
}

double RayTracer::aspectRatio() {
  return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene(const char *fn) {
  ifstream ifs(fn);
  if (!ifs) {
    string msg("Error: couldn't read scene file ");
    msg.append(fn);
    traceUI->alert(msg);
    return false;
  }

  // Check if fn ends in '.ray'
  bool isRay = false;
  const char *ext = strrchr(fn, '.');
  if (ext && !strcmp(ext, ".ray"))
    isRay = true;

  // Strip off filename, leaving only the path:
  string path(fn);
  if (path.find_last_of("\\/") == string::npos)
    path = ".";
  else
    path = path.substr(0, path.find_last_of("\\/"));

  if (isRay) {
    // .ray Parsing Path
    // Call this with 'true' for debug output from the tokenizer
    Tokenizer tokenizer(ifs, false);
    Parser parser(tokenizer, path);
    try {
      scene.reset(parser.parseScene());
    } catch (SyntaxErrorException &pe) {
      traceUI->alert(pe.formattedMessage());
      return false;
    } catch (ParserException &pe) {
      string msg("Parser: fatal exception ");
      msg.append(pe.message());
      traceUI->alert(msg);
      return false;
    } catch (TextureMapException e) {
      string msg("Texture mapping exception: ");
      msg.append(e.message());
      traceUI->alert(msg);
      return false;
    }
  } else {
    // JSON Parsing Path
    try {
      JsonParser parser(path, ifs);
      scene.reset(parser.parseScene());
    } catch (ParserException &pe) {
      string msg("Parser: fatal exception ");
      msg.append(pe.message());
      traceUI->alert(msg);
      return false;
    } catch (const json::exception &je) {
      string msg("Invalid JSON encountered ");
      msg.append(je.what());
      traceUI->alert(msg);
      return false;
    }
  }

  if (!sceneLoaded())
    return false;

  return true;
}

void RayTracer::traceSetup(int w, int h) {
  size_t newBufferSize = w * h * 3;
  if (newBufferSize != buffer.size()) {
    bufferSize = newBufferSize;
    buffer.resize(bufferSize);
  }
  buffer_width = w;
  buffer_height = h;
  std::fill(buffer.begin(), buffer.end(), 0);
  m_bBufferReady = true;

  /*
   * Sync with TraceUI
   */

  threads = traceUI->getThreads();
  block_size = traceUI->getBlockSize();
  thresh = traceUI->getThreshold();
  samples = traceUI->getSuperSamples();
  aaThresh = traceUI->getAaThreshold();

  // YOUR CODE HERE
  // FIXME: Additional initializations

  scene->build(scene->getAllObjects(), scene->bounds(), 0);
  
}
// Node buildTree(std::vector <Geometry*> objList, BoundingBox bbox, int depth, int leafSize);

/*
 * RayTracer::traceImage
 *
 *	Trace the image and store the pixel data in RayTracer::buffer.
 *
 *	Arguments:
 *		w:	width of the image buffer
 *		h:	height of the image buffer
 *
 */
void RayTracer::traceImage(int w, int h) {
  // Always call traceSetup before rendering anything.
  traceSetup(w, h);

  // YOUR CODE HERE
  // FIXME: Start one or more threads for ray tracing
  glm::dvec3 Trace;
  for (int i = 0; i < h; i++)
  {
    for (int j = 0; j < w; j++)
    {
      Trace = tracePixel(i, j);
      setPixel(i, j, Trace);
    }
  }
  // if (traceUI->aaSwitch())
  // {
  //   aaImage(w, h);
  // }
}

void RayTracer :: aaImage(int w, int h) {
  // YOUR CODE HERE
  // FIXME: Implement Anti-aliasing here
  //
  // TIP: samples and aaThresh have been synchronized with TraceUI by
  //      RayTracer::traceSetup() function

  // glm::dvec3 averageColors;
  // for (int i = 0; i < h; i++)
  // {
  //   for (int j = 0; j < w; j++)
  //   {
  //     averageColors = tracePixel(i, j);
  //     setPixel(i, j, averageColors);
  //   }
  // }
}

// where I overcomplicate things by making an entirely separate helper function that's almost
// the exact same as tracePixel but because of one specification I decide that this was a better
// route
// right now this is entirely pointless as I moved the calculations to tracePixel, but
// more of just a testament to my work I guess?
glm::dvec3 RayTracer :: aaHelper(int i, int j)
{
  //cout<<"I'm doing my part! :3 \n";
  // glm::dvec3 col(0, 0, 0);

  //   if (!sceneLoaded())
  //     return col;
  //   //cout<<samples<<"\n\n";
  //   //int divvy = sqrt(samples);
  //   double starting = (1.0/double(samples));
  //   double pixelX = double(i) / double(buffer_width);
  //   double pixelY = double(j) / double(buffer_height);
  //   double tbAddX = starting;
  //   double tbAddY = starting;

  //   unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;

  //   for (int i = 0; i < samples; i++) // y position
  //   {
  //     for (int j = 0; j < samples; j++) // x position
  //     {
  //       double x = ((tbAddX/double(buffer_width))+pixelX);
  //       double y = ((tbAddY/double(buffer_height))+pixelY);
  //       col += trace(x, y);
  //       cout<<"X: " << x << " Y: " << y << "\n";
  //       cout<<col<<"\n\n";
  //       tbAddX += starting;
  //     }
  //     tbAddX = starting;
  //     tbAddY += starting;
  //   }
  //   col[0] = col[0]/samples;
  //   col[1] = col[1]/samples;
  //   col[2] = col[2]/samples;

  //   pixel[0] = (int)(255.0 * ((col[0])));
  //   pixel[1] = (int)(255.0 * ((col[1])));
  //   pixel[2] = (int)(255.0 * ((col[2])));

  //   return col;
}

bool RayTracer::checkRender() {
  // YOUR CODE HERE
  // FIXME: Return true if tracing is done.
  //        This is a helper routine for GUI.
  //
  // TIPS: Introduce an array to track the status of each worker thread.
  //       This array is maintained by the worker threads.
  return true;
}

void RayTracer::waitRender() {
  // YOUR CODE HERE
  // FIXME: Wait until the rendering process is done.
  //        This function is essential if you are using an asynchronous
  //        traceImage implementation.
  //
  // TIPS: Join all worker threads here.
}


glm::dvec3 RayTracer::getPixel(int i, int j) {
  unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;
  return glm::dvec3((double)pixel[0] / 255.0, (double)pixel[1] / 255.0,
                    (double)pixel[2] / 255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color) {
  unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;

  pixel[0] = (int)(255.0 * color[0]);
  pixel[1] = (int)(255.0 * color[1]);
  pixel[2] = (int)(255.0 * color[2]);
}
