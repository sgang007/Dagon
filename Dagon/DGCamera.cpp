////////////////////////////////////////////////////////////
//
// DAGON - An Adventure Game Engine
// Copyright (c) 2011 Senscape s.r.l.
// All rights reserved.
//
// NOTICE: Senscape permits you to use, modify, and
// distribute this file in accordance with the terms of the
// license agreement accompanying it.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////

#include "DGCamera.h"
#include "DGConfig.h"

////////////////////////////////////////////////////////////
// Definitions
////////////////////////////////////////////////////////////

#define DGCamPanNone 0
#define DGCamPanLeft -1
#define DGCamPanRight 1
#define DGCamPanUp 1
#define DGCamPanDown -1

////////////////////////////////////////////////////////////
// Implementation - Constructor
////////////////////////////////////////////////////////////

DGCamera::DGCamera() {
    config = &DGConfig::getInstance();
    
    _angleH = 0.0f;
    _angleV = 0.0f;
    
    _angleHLimit = (float)M_PI * 2;
    _angleVLimit = (float)M_PI / 2;
    
    _deltaX = 0;
    _deltaY = 0;
    
    _fovCurrent = DGCamFieldOfView;
    _fovNormal = DGCamFieldOfView;
    
    _panDirH = DGCamPanNone;
    _panDirV = DGCamPanNone;
    
    _neutralZone = DGCamNeutralZone;
}

////////////////////////////////////////////////////////////
// Implementation - Destructor
////////////////////////////////////////////////////////////

DGCamera::~DGCamera() {
    // Nothing to do here
}

////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////

void DGCamera::pan(int xPosition, int yPosition) {
	if (xPosition > _panRegion.size.width) {
		_deltaX = abs(xPosition - _panRegion.size.width);
        _panDirH = DGCamPanRight;
	}
	else if (xPosition < _panRegion.origin.x) {
		_deltaX = abs(xPosition - _panRegion.origin.x);
        _panDirH = DGCamPanLeft;
	}
	else {
        _deltaX = 0;
        _panDirH = DGCamPanNone;
    }
	
	if (yPosition > _panRegion.size.height) {
		_deltaY = abs(yPosition - _panRegion.size.height);
        _panDirV = DGCamPanUp;
	}
	else if (yPosition < _panRegion.origin.y) {
		_deltaY = abs(yPosition - _panRegion.origin.y);
        _panDirV = DGCamPanDown;
	}
	else {
        _deltaY = 0;
        _panDirV = DGCamPanNone;
    }
}

void DGCamera::beginOrthoView() {
    // Switch to the projection view
    glMatrixMode(GL_PROJECTION);
    
    // Save its current state and load a new identity
    glPushMatrix();
    glLoadIdentity();
    
    // Now we prepare our orthogonal projection
    glOrtho(0, _viewport.width, 0, _viewport.height, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    
    // Note that transformations have been applied to the GL_MODELVIEW
    // so we must reload the identity matrix
    glLoadIdentity();
}

void DGCamera::endOrthoView() {
    // Go back to the projection view and its previous state
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    
    // Leave everything in model view just as it were before
    glMatrixMode(GL_MODELVIEW);
}

float DGCamera::fieldOfView() {
    return _fovNormal;
}

int DGCamera::neutralZone() {
    return _neutralZone;
}

void DGCamera::setFieldOfView(float fov) {
    _fovNormal = DGCamFieldOfView;
}

void DGCamera::setNeutralZone(int zone) {
    // We calculate the factor to stretch the neutral zone with
    // the display height
    float factor = (float)_viewport.height / (float)DGDefDisplayHeight;
    
    _neutralZone = zone;
    
    // Update panning regions with the new neutral zone
    _panRegion.origin.x = (_viewport.width / 2) - _neutralZone * factor;
    _panRegion.origin.y = (_viewport.height / 2) - _neutralZone * factor;
    _panRegion.size.width = (_viewport.width / 2) + _neutralZone * factor;
    _panRegion.size.height = (_viewport.height / 2) + _neutralZone * factor;
}

void DGCamera::setViewport(int width, int height) {
    _viewport.width = width;
    _viewport.height = height;
    
    // This forces the new display factor to be applied to the
    // current neutral zone
    setNeutralZone(_neutralZone);
    
    glViewport(0, 0, (GLint)_viewport.width, (GLint)_viewport.height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // We need a very close clipping point because the cube is rendered in a small area
    gluPerspective(_fovNormal, (GLfloat)_viewport.width / (GLfloat)_viewport.height, 0.1f, 10.0f);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void DGCamera::update() {
	float lx;
    float lz;
    
    // Implement direction
    // Delta only affects accel or speed
    // Update angles with speed
    
    if (_panDirH || _panDirV) {
        if (_panDirH) {
            _angleH += 0.025f * _panDirH;
            
            // Limit rotation
            if (_angleH > (GLfloat)_angleHLimit)
                _angleH = 0.0f;
            else if (_angleH < 0.0f)
                _angleH = (GLfloat)_angleHLimit;
        }
        
        if (_panDirV) {
            _angleV += 0.025f * _panDirV;
            
            // Limit rotation
            if (_angleV > (GLfloat)_angleVLimit)
                _angleV = (GLfloat)_angleVLimit;
            else if (_angleV < -(GLfloat)_angleVLimit)
                _angleV = -(GLfloat)_angleVLimit;
        }
    }
    
    lx = (float)sin(_angleH);
    lz = (float)-cos(_angleH);
    
    gluLookAt(0.0f, 0.0f, 0.0f,
              lx, _angleV, lz,
              0.0f, 1.0f, 0.0f);
}
