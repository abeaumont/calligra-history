#ifndef __VPRIMITIVE_H__
#define __VPRIMITIVE_H__

/**
 * VPrimitives (VLines and VBeziers) are the most simple objects which cant
 * live alone, but form together VObjects like VPaths.
 */

class QPointArray;
class VPoint;

class VPrimitive {
public:
    VPrimitive( VPoint* fp=0L, VPoint* lp=0L )
	: m_firstPoint(fp), m_lastPoint(lp) {}
    
    VPoint* firstPoint() const { return m_firstPoint; }
    void setFirstPoint( VPoint* fp ) { m_firstPoint=fp; }
    VPoint* lastPoint() const { return m_lastPoint; }
    void setLastPoint( VPoint* lp ) { m_lastPoint=lp; }

    virtual void getQPoints( QPointArray& qpa ) = 0;

protected:
    VPoint* m_firstPoint;
    VPoint* m_lastPoint;
};


class VLine : public VPrimitive {
public:
    VLine( VPoint* fp=0L, VPoint* lp=0L )
	: VPrimitive(fp,lp) {}
	
    virtual void getQPoints( QPointArray& qpa );
};


class VBezier : public VPrimitive {
public:
    VBezier( VPoint* fp=0L, VPoint* fcp=0L, VPoint* lcp=0L, VPoint* lp=0L )
	: VPrimitive(fp,lp), m_firstCtrlPoint(fcp), m_lastCtrlPoint(lcp) {}    

    VPoint* firstCtrlPoint() const { return m_firstCtrlPoint; }
    void setFirstCtrlPoint( VPoint* fcp ) { m_firstCtrlPoint=fcp; }
    VPoint* lastCtrlPoint() const { return m_lastCtrlPoint; }
    void setLastCtrlPoint( VPoint* lcp ) { m_lastCtrlPoint=lcp; }

    virtual void getQPoints( QPointArray& qpa );
	    
private:
    VPoint* m_firstCtrlPoint;
    VPoint* m_lastCtrlPoint;
};


#endif
