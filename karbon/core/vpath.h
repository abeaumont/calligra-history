/* This file is part of the KDE project
   Copyright (C) 2001, The Karbon Developers
*/

#ifndef __VPATH_H__
#define __VPATH_H__

#include <qlist.h>
#include <qpointarray.h>

#include "vobject.h"
#include "vrect.h"

class VAffineMap;
class VPoint;


// VSegment is the abstract base class for VLine and VBezier. Segments are a
// help-class for VPath (see below). Segments share exactly one point: the
// lastPoint of the previos segment.

// TODO: - make VSegment _really_ abstract.
class VSegment
{
public:
	VSegment( const double lp_x = 0.0, const double lp_y = 0.0 );

	// Beziers need the previous segment's lastpoint to calculate all their QPoints
	virtual const QPointArray& getQPointArray( const VSegment& prevSeg,
		const double& zoomFactor ) const { return QPointArray(); }

	virtual const VPoint* firstCtrlPoint( const VSegment& prevSeg ) const
		{ return 0L; }
 	virtual const VPoint* lastCtrlPoint( const VSegment& prevSeg ) const
		{ return 0L; }
 	const VPoint* lastPoint() const { return &m_lastPoint; }

/* gxpath2.c */
	virtual const VSegment revert( const VSegment& prevSeg ) {};

	virtual void transform( const VAffineMap& affmap );

protected:
	// m_firstPoint is a waste, that's why we skip it
	VPoint m_lastPoint;

	mutable bool m_isDirty;				// need to recreate QPointArray?
	mutable QPointArray m_QPointArray;	// for painting
};


class VLine : public VSegment
{
public:
	VLine( const double lp_x = 0.0, const double lp_y = 0.0 );

	virtual const QPointArray& getQPointArray( const VSegment& prevSeg,
		const double& zoomFactor ) const;

	virtual const VSegment revert( const VSegment& prevSeg );
};


// VCurve is a ordinary bezier. VCurve1 has "no" first control-point, VCurve2
// has "no" last control-point (have a look at VPath::curveTo(),
// VPath::curve1To(), VPath::curve2To() ).

class VCurve : public VSegment
{
public:
	VCurve(
		const double fcp_x = 0.0, const double fcp_y = 0.0,
		const double lcp_x = 0.0, const double lcp_y = 0.0,
		const double lp_x = 0.0, const double lp_y = 0.0 );

	virtual const QPointArray& getQPointArray( const VSegment& prevSeg,
		const double& zoomFactor ) const {};

	virtual const VPoint* firstCtrlPoint( const VSegment& prevSeg ) const
		{ return &m_firstCtrlPoint; }
 	virtual const VPoint* lastCtrlPoint( const VSegment& prevSeg ) const
		{ return &m_lastCtrlPoint; }

	virtual const VSegment revert( const VSegment& prevSeg );

	virtual void transform( const VAffineMap& affmap );

private:
	VPoint m_firstCtrlPoint;
	VPoint m_lastCtrlPoint;

	VRect m_boundingBox;
};


class VCurve1 : public VSegment
{
public:
	VCurve1(
		const double lcp_x = 0.0, const double lcp_y = 0.0,
		const double lp_x = 0.0, const double lp_y = 0.0 );

	virtual const QPointArray& getQPointArray( const VSegment& prevSeg,
		const double& zoomFactor ) const {};

	virtual const VPoint* firstCtrlPoint( const VSegment& prevSeg ) const
		{ return prevSeg.lastPoint(); }
 	virtual const VPoint* lastCtrlPoint( const VSegment& prevSeg ) const
		{ return &m_lastCtrlPoint; }

	virtual const VSegment revert( const VSegment& prevSeg );

	virtual void transform( const VAffineMap& affmap );

private:
	VPoint m_lastCtrlPoint;

	VRect m_boundingBox;
};


class VCurve2 : public VSegment
{
public:
	VCurve2(
		const double fcp_x = 0.0, const double fcp_y = 0.0,
		const double lp_x = 0.0, const double lp_y = 0.0 );

	virtual const QPointArray& getQPointArray( const VSegment& prevSeg,
		const double& zoomFactor ) const {};

	virtual const VPoint* firstCtrlPoint( const VSegment& prevSeg ) const
		{ return &m_firstCtrlPoint; }
 	virtual const VPoint* lastCtrlPoint( const VSegment& prevSeg ) const
		{ return &m_lastPoint; }

	virtual const VSegment revert( const VSegment& prevSeg );

	virtual void transform( const VAffineMap& affmap );

private:
	VPoint m_firstCtrlPoint;

	VRect m_boundingBox;
};


// VPaths are the most common high-level objects. They consist of
// VSegments, which are VLines or VBeziers.

class VPath : public VObject
{
public:
	VPath();
	VPath( const VPath& path );
	virtual ~VPath();

	virtual void draw( QPainter& painter, const QRect& rect,
		const double zoomFactor = 1.0 );

	const VPoint* currentPoint() const;
	const VSegment* lastSegment() const { return m_segments.getLast(); };

	// postscript-like commands:
	VPath& moveTo( const double x, const double y );
	VPath& lineTo( const double x, const double y );

	// curveTo():
	//
	//   p1          p2
	//    O   ____   O
	//    | _/    \_ |
	//    |/        \|
	//    x          x
	// currP         p3
	//
	VPath& curveTo(
		const double x1, const double y1,
		const double x2, const double y2,
		const double x3, const double y3 );

	// curve1To():
	//
	//               p2
	//         ____  O
	//      __/    \ |
	//     /        \|
	//    x          x
	// currP         p3
	//
	VPath& curve1To(
		const double x2, const double y2,
		const double x3, const double y3 );

	// curve2To():
	//
	//   p1
	//    O  ____
	//    | /    \__
	//    |/        \
	//    x          x
	// currP         p3
	//
	VPath& curve2To(
		const double x1, const double y1,
		const double x3, const double y3 );

	// this is a convenience function to approximate circular arcs with
	// beziers
	VPath& arcTo(
		const double x1, const double y1,
		const double x2, const double y2, const double r );

	VPath& close();
	const bool isClosed() { return( m_isClosed ); }

	const VPath& revert();

	virtual VObject& transform( const VAffineMap& affmap );

private:
	// m_segments store all segemnts ( lines or beziers)
	QList<VSegment>	m_segments;

	bool m_isClosed;
};

#endif
