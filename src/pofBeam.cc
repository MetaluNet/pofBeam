/*
 * Copyright (c) 2023 Antoine Rousseau <antoine@metalu.net>
 * BSD Simplified License, see the file "LICENSE.txt" in this distribution.
 * See https://github.com/MetaluNet/pofBeam for documentation and updates.
 */
#include "pofBeam.h"

static t_class *pofbeam_class;
static t_symbol *s_clear, *s_add; 

#define NEXT_FLOAT_ARG(var) if((argc>0) && (argv->a_type == A_FLOAT)) { var = atom_getfloat(argv); argv++; argc--; }

static void *pofbeam_new(t_symbol *s, int argc, t_atom *argv)
{
    pofBeam* obj = new pofBeam(pofbeam_class);
    NEXT_FLOAT_ARG(obj->imSize.x);
    NEXT_FLOAT_ARG(obj->imSize.y);

    return (void*) (obj->pdobj);
}

static void pofbeam_free(void *x)
{
	delete (pofBeam*)(((PdObject*)x)->parent);
}

static void pofbeam_width(void *x, float width)
{
	pofBeam* px = (pofBeam*)(((PdObject*)x)->parent);
	px->width = width;
}

static void pofbeam_divider(void *x, float d)
{
	pofBeam* px = (pofBeam*)(((PdObject*)x)->parent);
	px->divider = d;
}


static void pofbeam_widthFilter(void *x, float d)
{
	pofBeam* px = (pofBeam*)(((PdObject*)x)->parent);
	px->widthFilter = d;
}


static void pofbeam_imsize(void *x, float w, float h)
{
	pofBeam* px = (pofBeam*)(((PdObject*)x)->parent);
	px->imSize = ofVec2f(w, h);
}


void pofBeam::setup(void)
{
	s_clear = gensym("clear");
	s_add = gensym("add");

	pofbeam_class = class_new(gensym("pofbeam"), (t_newmethod)pofbeam_new, (t_method)pofbeam_free,
		sizeof(PdObject), 0, A_GIMME, A_NULL);
	
	class_addmethod(pofbeam_class, (t_method)tellGUI, s_clear,  A_GIMME, A_NULL);
	class_addmethod(pofbeam_class, (t_method)tellGUI, s_add,  A_GIMME, A_NULL);
	class_addmethod(pofbeam_class, (t_method)pofbeam_width, gensym("width"), A_FLOAT, A_NULL);
	class_addmethod(pofbeam_class, (t_method)pofbeam_divider, gensym("divider"), A_FLOAT, A_NULL);
	class_addmethod(pofbeam_class, (t_method)pofbeam_widthFilter, gensym("widthFilter"), A_FLOAT, A_NULL);
	class_addmethod(pofbeam_class, (t_method)pofbeam_imsize, gensym("imsize"), A_FLOAT, A_FLOAT, A_NULL);

	POF_SETUP(pofbeam_class);
}

//--------------------------------------------------------------
void pofBeam::addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c) {
	mesh.addVertex(a);
	mesh.addVertex(b);
	mesh.addVertex(c);
}

//--------------------------------------------------------------
void pofBeam::addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d) {
	addFace(mesh, a, b, c);
	addFace(mesh, a, c, d);
}

//--------------------------------------------------------------
void pofBeam::addTexCoords(ofMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c) {
	mesh.addTexCoord(a);
	mesh.addTexCoord(b);
	mesh.addTexCoord(c);
}

//--------------------------------------------------------------
void pofBeam::addTexCoords(ofMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d) {
	addTexCoords(mesh, a, b, c);
	addTexCoords(mesh, a, c, d);
}

//--------------------------------------------------------------
void pofBeam::removeLastFace(ofMesh& mesh) {
	if(mesh.getNumVertices() < 6) return;
	for(int i= 0; i < 6; i++) {
		mesh.getVertices().pop_back();
		mesh.getTexCoords().pop_back();
	}
}
//----------------- Line Segment Intersection test ---------------------------------------------
// Given three collinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
static bool onSegment(ofVec2f p, ofVec2f q, ofVec2f r)
{
	if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) &&
		q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
	return true;

	return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are collinear
// 1 --> Clockwise
// 2 --> Counterclockwise
static int orientation(ofVec2f p, ofVec2f q, ofVec2f r)
{
	// See https://www.geeksforgeeks.org/orientation-3-ordered-points/
	// for details of below formula.
	int val = (q.y - p.y) * (r.x - q.x) -
			(q.x - p.x) * (r.y - q.y);

	if (val == 0) return 0; // collinear

	return (val > 0)? 1: 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
static bool doIntersect(ofVec2f p1, ofVec2f q1, ofVec2f p2, ofVec2f q2)
{
	// Find the four orientations needed for general and
	// special cases
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	// General case
	if (o1 != o2 && o3 != o4)
		return true;

	// Special Cases
	// p1, q1 and p2 are collinear and p2 lies on segment p1q1
	if (o1 == 0 && onSegment(p1, p2, q1)) return true;

	// p1, q1 and q2 are collinear and q2 lies on segment p1q1
	if (o2 == 0 && onSegment(p1, q2, q1)) return true;

	// p2, q2 and p1 are collinear and p1 lies on segment p2q2
	if (o3 == 0 && onSegment(p2, p1, q2)) return true;

	// p2, q2 and q1 are collinear and q1 lies on segment p2q2
	if (o4 == 0 && onSegment(p2, q1, q2)) return true;

	return false; // Doesn't fall in any of the above cases
}

//--------------------------------------------------------------
void pofBeam::addPoint(ofVec2f p, float w, int maxRearrange) {
	lastPoint = p;
	points.push_back(p);
	int n = points.size();
	if(n < 4) return;
	ofVec2f p0 = points[n - 4];
	ofVec2f p1 = points[n - 3];
	ofVec2f p2 = points[n - 2];
	ofVec2f p3 = points[n - 1];
	// the "thick segment" quad will be created between p1 and p2.

	// compute p0p1p2 and p1p2p3 bisection:
	ofVec2f v02 = (p2 - p1).normalize() - (p0 - p1).normalize();
	ofVec2f v13 = (p3 - p2).normalize() - (p1 - p2).normalize();
	ofVec2f ov02 = v02.rotate(-90).normalize();
	ofVec2f ov13 = v13.rotate(-90).normalize();

	float w1 = widthFilter2; //width / (1 + (p1 - p0).length() / divider);
	float w2 = w * width / (1 + (p2 - p1).length() / divider);
	float wF1 = widthFilter1 * (1 - widthFilter) + w2 * widthFilter;
	float wF2 = widthFilter2 * (1 - widthFilter) + wF1 * widthFilter;
	w2 = wF2;

	ofVec2f q1 = p1 - ov02 * w1;
	ofVec2f q2 = p2 - ov13 * w2;
	ofVec2f q3 = p2 + ov13 * w2;
	ofVec2f q4 = p1 + ov02 * w1;

	if(maxRearrange > 0 && doIntersect(q4, q1, q2, q3)) {
		// remove p2 point:
		points.pop_back(); // remove p3
		points.pop_back(); // remove p2
		removeLastFace(mesh);
		addPoint(p3, w, maxRearrange - 1); // re-add p3
		return;
	}

	widthFilter1 = wF1;
	widthFilter2 = wF2;
	addFace(mesh, q1, q2, q3, q4);

#ifdef TARGET_OPENGLES
	addTexCoords(mesh, ofVec2f(0, 0), ofVec2f(1, 0), ofVec2f(1, 1), ofVec2f(0, 1));
#else
	addTexCoords(mesh, ofVec2f(0, 0), ofVec2f(imSize.x, 0), ofVec2f(imSize.x, imSize.y), ofVec2f(0, imSize.y));
#endif
}

void pofBeam::draw()
{
	mesh.draw();
}

void pofBeam::message(int argc, t_atom *argv)
{
	float x = 0, y = 0, w = 1.0;	

	t_symbol *key = atom_getsymbol(argv); 
	argv++; argc--;
  
	if(key == s_clear) {
		mesh.clear();
		points.clear();
		widthFilter1 = widthFilter2 = 0.0;
	}
	else if(key == s_add) {
		NEXT_FLOAT_ARG(x);
		NEXT_FLOAT_ARG(y);
		NEXT_FLOAT_ARG(w);
		addPoint(ofVec2f(x, y), w);
	}
}

extern "C" {
	void pofbeam_setup(void)
	{
		pofBeam::setup();
	}
}

