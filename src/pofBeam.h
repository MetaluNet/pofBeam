/*
 * Copyright (c) 2023 Antoine Rousseau <antoine@metalu.net>
 * BSD Simplified License, see the file "LICENSE.txt" in this distribution.
 * See https://github.com/MetaluNet/pofBeam for documentation and updates.
 */
#pragma once

#include "pofBase.h"

class pofBeam: public pofBase {
	public:
		pofBeam(t_class *Class):
			pofBase(Class)
		{
		}

		void addPoint(ofVec2f p, float w, int maxRearrange = 1);
		void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c);
		void addFace(ofMesh& mesh, ofVec3f a, ofVec3f b, ofVec3f c, ofVec3f d);
		void addTexCoords(ofMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c);
		void addTexCoords(ofMesh& mesh, ofVec2f a, ofVec2f b, ofVec2f c, ofVec2f d);
		void removeLastFace(ofMesh& mesh);
		
		ofMesh mesh;
		vector<ofVec2f> points;
		float width = 30;
		float divider = 5;
		ofVec2f imSize = ofVec2f(1,1);
		float widthFilter1 = 0.0, widthFilter2 = 0.0;
		float widthFilter = 0.1;
		ofVec2f lastPoint;

		virtual void draw();
		virtual void message(int  arc, t_atom *argv);
		static void setup(void);
};

extern "C" {
	void pofbeam_setup(void);
}

