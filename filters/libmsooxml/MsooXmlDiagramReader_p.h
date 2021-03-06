/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 * Copyright (c) 2010 Carlos Licea <carlos@kdab.com>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef MSOOXMLXMLDIAGRAMREADER_P_H
#define MSOOXMLXMLDIAGRAMREADER_P_H

#include <cmath>
#include <QString>
#include <QList>
#include <QVector>
#include <QMap>
#include <QPair>
#include <QExplicitlySharedDataPointer>

namespace MSOOXML {
    class MsooXmlDiagramReader;
}
class KoXmlWriter;
class KoGenStyles;

namespace MSOOXML { namespace Diagram {

/****************************************************************************************************
 * The following classes where designed after the way the dmg-namespace is described in the
 * MSOOXML-specs and how it was done in oo.org.
 *
 * Note that we cannot just translate the drawing1.xml cause there are cases where those file doesn't
 * contain the content or all of the content. A typical example where it's needed to eval the whole
 * data1.xml datamodel and the layout1.xml layout-definition are Venn diagrams. But seems it's also
 * possible to just turn any drawing1.xml into a "redirect" to data1.xml+layout1.xml. So, all in all
 * we cannot trust drawing1.xml to contain anything useful :-/
 *
 * See also;
 * - http://wiki.services.openoffice.org/wiki/SmartArt
 * - http://msdn.microsoft.com/en-us/magazine/cc163470.aspx
 * - http://msdn.microsoft.com/en-us/library/dd439435(v=office.12).aspx
 * - http://msdn.microsoft.com/en-us/library/dd439443(v=office.12).aspx
 * - http://msdn.microsoft.com/en-us/library/dd439454(v=office.12).aspx
 * - http://blogs.code-counsel.net/Wouter/Lists/Posts/Post.aspx?ID=36
 */

class AbstractNode;
class PointNode;
class PointListNode;
class ConnectionListNode;
class AbstractAtom;
class LayoutNodeAtom;
class ConstraintAtom;
class AlgorithmAtom;

/// The evaluation context that is passed around and contains all kind of state-informations.
class Context
{
    public:
        /// The "doc" root node.
        AbstractNode* m_rootPoint;
        /// A list of connections between nodes.
        ConnectionListNode* m_connections;
        /// The root layout node.
        QExplicitlySharedDataPointer<LayoutNodeAtom> m_rootLayout;
        /// The current parent layout node. This will change during walking through the layout nodes.
        QExplicitlySharedDataPointer<LayoutNodeAtom> m_parentLayout;
        /// A identifier=>LayoutNodeAtom map used to access the layouts by there unique identifiers.
        QMap<QString, QExplicitlySharedDataPointer<LayoutNodeAtom> > m_layoutMap;
        /// A PointNode=>LayoutNodeAtom map used to know which datapoint maps to which layoutnode.
        QMap<QString, QExplicitlySharedDataPointer<LayoutNodeAtom> > m_pointLayoutMap;

        explicit Context();
        ~Context();
        AbstractNode* currentNode() const;
        void setCurrentNode(AbstractNode* node);
    private:
        /// the moving context node
        AbstractNode* m_currentNode;
};

/****************************************************************************************************
 * It follws the classes used within the data-model to build up a tree of data-nodes.
 */

/// The AbstractNode is the base class to handle the diagram data-model (content of data1.xml).
class AbstractNode
{
    public:
        const QString m_tagName;
        explicit AbstractNode(const QString &tagName);
        virtual ~AbstractNode();
        virtual void dump(Context* context, int level);
        virtual void readElement(Context*, MsooXmlDiagramReader*);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        AbstractNode* parent() const;
        QList<AbstractNode*> children() const;
        void insertChild(int index, AbstractNode* node);
        void addChild(AbstractNode* node);
        void removeChild(AbstractNode* node);
        QList<AbstractNode*> descendant() const;
        QList<AbstractNode*> peers() const;
    private:
        AbstractNode* m_parent;
        mutable QList<AbstractNode*> m_cachedChildren;
        QMap<int,QList<AbstractNode*> > m_orderedChildren;
        QMap<AbstractNode*,int> m_orderedChildrenReverse;
        QList<AbstractNode*> m_appendedChildren;
};

/// A point in the data-model.
class PointNode : public AbstractNode
{
    public:
        QString m_modelId;
        QString m_type;
        QString m_cxnId;
        QString m_text;
        explicit PointNode() : AbstractNode("dgm:pt") {}
        virtual ~PointNode() {}
        virtual void dump(Context* context, int level);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
    private:
        void readTextBody(Context*, MsooXmlDiagramReader* reader);
};

/// A list of points in the data-model.
class PointListNode : public AbstractNode
{
    public:
        explicit PointListNode() : AbstractNode("dgm:ptLst") {}
        virtual ~PointListNode() {}
        virtual void dump(Context* context, int level);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
};

/// A connection between two nodes in the data-model.
class ConnectionNode : public AbstractNode
{
    public:
        QString m_modelId;
        QString m_type;
        QString m_srcId;
        QString m_destId;
        QString m_presId;
        QString m_parTransId;
        QString m_sibTransId;
        int m_srcOrd;
        int m_destOrd;
        explicit ConnectionNode() : AbstractNode("dgm:cxn"), m_srcOrd(0), m_destOrd(0) {}
        virtual ~ConnectionNode() {}
        virtual void dump(Context*, int level);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
};

/// A list of connections in the data-model.
class ConnectionListNode : public AbstractNode
{
    public:
        explicit ConnectionListNode() : AbstractNode("dgm:cxnLst") {}
        virtual ~ConnectionListNode() {}
        virtual void dump(Context* context, int level);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
};

/****************************************************************************************************
 * So much for the nodes. Now the atoms are following which are used to add some logic to the
 * data-model and they do provide the functionality to build up a hierarchical layout tree.
 */

/// Base class for layout-operations (content of layout1.xml)
class AbstractAtom : public QSharedData
{
    public:
        const QString m_tagName;
        explicit AbstractAtom(const QString &tagName);
        virtual ~AbstractAtom();
        virtual AbstractAtom* clone() = 0;
        virtual void dump(Context* context, int level);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        virtual void build(Context* context);
        virtual void layoutAtom(Context* context);
        virtual void writeAtom(Context* context, KoXmlWriter* xmlWriter, KoGenStyles* styles);
        QExplicitlySharedDataPointer<AbstractAtom> parent() const;
        QVector< QExplicitlySharedDataPointer<AbstractAtom> > children() const;
        void addChild(AbstractAtom* node);
        void addChild(QExplicitlySharedDataPointer<AbstractAtom> node);
        void removeChild(QExplicitlySharedDataPointer<AbstractAtom> node);
    protected:
        QExplicitlySharedDataPointer<AbstractAtom> m_parent;
        QVector< QExplicitlySharedDataPointer<AbstractAtom> > m_children;
        QList<AbstractNode*> fetchAxis(Context* context, const QString& _axis, const QString &_ptType, const QString& _start, const QString& _count, const QString& _step) const;
    private:
        QList<AbstractNode*> fetchAxis(Context* context, QList<AbstractNode*> list, const QString& axis, const QString &ptType, const QString& start, const QString& count, const QString& step) const;
        QList<AbstractNode*> foreachAxis(Context*, const QList<AbstractNode*> &list, int start, int count, int step) const;
};

/// The algorithm used by the containing layout node. The algorithm defines the behavior of the layout node along with the behavior and layout of the nested layout nodes.
class AlgorithmAtom : public AbstractAtom
{
    public:
        /// The used layout-algorithm.
        enum Algorithm {
            UnknownAlg, ///< Unknown algorithm. This should happen...
            CompositeAlg, ///< The composite algorithm specifies the size and position for all child layout nodes. You can use it to create graphics with a predetermined layout or in combination with other algorithms to create more complex shapes.
            ConnectorAlg, ///< The connector algorithm lays out and routes connecting lines, arrows, and shapes between layout nodes.
            CycleAlg, ///< The cycle algorithm lays out child layout nodes around a circle or portion of a circle using equal angle spacing.
            HierChildAlg, ///< The hierarchy child algorithm works with the hierRoot algorithm to create hierarchical tree layouts. This algorithm aligns and positions its child layout nodes in a linear path under the hierRoot layout node.
            HierRootAlg, ///< The hierarchy root algorithm works with the hierChild algorithm to create hierarchical tree layouts. The hierRoot algorithm aligns and positions the hierRoot layout node in relation to the hierChild layout nodes.
            LinearAlg, ///< The linear algorithm lays out child layout nodes along a linear path.
            PyramidAlg, ///< The pyramid algorithm lays out child layout nodes along a vertical path and works with the trapezoid shape to create a pyramid.
            SnakeAlg, ///< The snake algorithm lays out child layout nodes along a linear path in two dimensions, allowing the linear flow to continue across multiple rows or columns.
            SpaceAlg, ///< The space algorithm is used to specify a minimum space between other layout nodes or as an indication to do nothing with the layout node’s size and position.
            TextAlg ///< The text algorithm sizes text to fit inside a shape and controls its margins and alignment.
        };
        Algorithm m_type;
        QMap<QString, QString> m_params; // list of type=value parameters that modify the default behavior of the algorithm.
        explicit AlgorithmAtom() : AbstractAtom("dgm:alg"), m_type(UnknownAlg) {}
        virtual ~AlgorithmAtom() {}
        virtual AlgorithmAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        virtual void readElement(Context*, MsooXmlDiagramReader* reader);
};

/// The layout node is the basic building block of diagrams. The layout node is responsible for defining how shapes are arranged in a diagram and how the data maps to a particular shape in a diagram.
class LayoutNodeAtom : public AbstractAtom
{
    public:
        QString m_name;
        QMap<QString, qreal> m_values; // map that contains values like l,t,w,h,ctrX and ctrY for positioning the layout
        QMap<QString, qreal> m_factors;
        QMap<QString, int> m_countFactors;
        int m_rotateAngle;
        bool m_needsReinit, m_needsRelayout, m_childNeedsRelayout;
        explicit LayoutNodeAtom() : AbstractAtom("dgm:layoutNode"), m_rotateAngle(0), m_needsReinit(true), m_needsRelayout(true), m_childNeedsRelayout(true), m_firstLayout(true) {}
        virtual ~LayoutNodeAtom() {}
        virtual LayoutNodeAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        virtual void build(Context* context);
        virtual void layoutAtom(Context* context);
        virtual void writeAtom(Context* context, KoXmlWriter* xmlWriter, KoGenStyles* styles);

        QList< QExplicitlySharedDataPointer<ConstraintAtom> > constraints() const;
        void addConstraint(QExplicitlySharedDataPointer<ConstraintAtom> constraint);

        QExplicitlySharedDataPointer<AlgorithmAtom> algorithm() const;
        void setAlgorithm(QExplicitlySharedDataPointer<AlgorithmAtom> algorithm);

        QList<AbstractNode*> axis() const;
        void setAxis(Context* context, const QList<AbstractNode*> &axis);

        void setNeedsReinit(bool needsReinit);
        void setNeedsRelayout(bool needsRelayout);

        AlgorithmAtom::Algorithm algorithmType() const;
        QMap<QString,QString> algorithmParams() const;
        QString algorithmParam(const QString &name, const QString &defaultValue = QString()) const;

        QString variable(const QString &name, bool checkParents = false) const;
        QMap<QString, QString> variables() const;
        void setVariable(const QString &name, const QString &value);
        QMap<QString, qreal> finalValues() const;
        
        QExplicitlySharedDataPointer<LayoutNodeAtom> parentLayout() const;
        QList< QExplicitlySharedDataPointer<LayoutNodeAtom> > childrenLayouts() const;
        QList< QExplicitlySharedDataPointer<LayoutNodeAtom> > descendantLayouts() const;
        QPair<LayoutNodeAtom*,LayoutNodeAtom*> neighbors() const;

        qreal distanceTo(LayoutNodeAtom* otherAtom) const;

    private:
        QList< QExplicitlySharedDataPointer<ConstraintAtom> > m_constraints;
        QList<AbstractNode*> m_axis;
        QMap<QString, QString> m_variables;
        bool m_firstLayout;
};

/// Specify size and position of nodes, text values, and layout dependencies between nodes in a layout definition.
class ConstraintAtom : public AbstractAtom
{
    public:
        /// Factor used in a reference constraint or a rule in order to modify a referenced value by the factor defined.
        qreal m_fact;
        /// Specifies the axis of layout nodes to apply a constraint or rule to.
        QString m_for;
        /// Specifies the name of the layout node to apply a constraint or rule to.
        QString m_forName;
        /// The operator constraint used to evaluate the condition.
        QString m_op;
        /// Specifies the type of data point to select.
        QString m_ptType;
        /// The point type used int he referenced constraint.
        QString m_refPtType;
        /// Specifies the type of a reference constraint.
        QString m_refType;
        /// The for value of the referenced constraint.
        QString m_refFor;
        /// The name of the layout node referenced by a reference constraint.
        QString m_refForName;
        /// Specifies the constraint to apply to this layout node.
        QString m_type;
        /// Specifies an absolute value instead of reference another constraint.
        QString m_value;
        explicit ConstraintAtom() : AbstractAtom("dgm:constr"), m_fact(1.0) {}
        virtual ~ConstraintAtom() {}
        virtual ConstraintAtom* clone();
        virtual void dump(Context*, int level);
        virtual void readAll(Context*, MsooXmlDiagramReader* reader);
        virtual void build(Context* context);
};

/// Rules indicate the ranges of values that a layout algorithm can use to modify the constraint values if it cannot lay out the graphic by using the constraints.
class RuleAtom : public AbstractAtom
{
    public:
        QString m_fact;
        QString m_for;
        QString m_forName;
        QString m_max;
        QString m_ptType;
        QString m_type;
        QString m_value;
        explicit RuleAtom() : AbstractAtom("dgm:rule") {}
        virtual ~RuleAtom() {}
        virtual RuleAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
};

/// Shape adjust value. These can be used to modify the adjust handles supported on various auto shapes. It is only possible to set the initial value, not to modify it using constraints and rules.
class AdjustAtom : public AbstractAtom
{
    public:
        int m_index;
        qreal m_value;
        explicit AdjustAtom() : AbstractAtom("dgm:adj"), m_index(-1) {}
        virtual ~AdjustAtom() {}
        virtual AdjustAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
};

/// List of atoms.
class ListAtom : public AbstractAtom
{
    public:
        explicit ListAtom(const QString &tagName) : AbstractAtom(tagName) {}
        explicit ListAtom(const QStringRef &tagName) : AbstractAtom(tagName.toString()) {}
        virtual ~ListAtom() {}
        virtual ListAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
};

/// The shape displayed by the containing layout node. Not all layout nodes display shapes.
class ShapeAtom : public AbstractAtom
{
    public:
        QString m_type;
        QString m_blip;
        bool m_hideGeom;
        explicit ShapeAtom() : AbstractAtom("dgm:shape"), m_hideGeom(false) {}
        virtual ~ShapeAtom() {}
        virtual ShapeAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        virtual void writeAtom(Context* context, KoXmlWriter* xmlWriter, KoGenStyles* styles);
};

/// This element specifies a particular data model point which is to be mapped to the containing layout node.
class PresentationOfAtom : public AbstractAtom
{
    public:
        QString m_axis; // This determines how to navigate through the data model, setting the context node as it moves. 
        QString m_ptType; // dataPointType
        QString m_count;
        QString m_hideLastTrans;
        QString m_start;
        QString m_step;
        explicit PresentationOfAtom() : AbstractAtom("dgm:presOf") {}
        virtual ~PresentationOfAtom() {}
        virtual PresentationOfAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        virtual void build(Context* context);
};

/// The if element represents a condition that applies to all it's children.
class IfAtom : public AbstractAtom
{
    public:
        QString m_argument;
        QString m_axis;
        QString m_function;
        QString m_hideLastTrans;
        QString m_name;
        QString m_operator;
        QString m_ptType;
        QString m_start;
        QString m_step;
        QString m_count;
        QString m_value;
        explicit IfAtom(bool isTrue) : AbstractAtom(isTrue ? "dgm:if" : "dgm:else"), m_isTrue(isTrue) {}
        virtual ~IfAtom() {}
        virtual IfAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        bool isTrue() const;
        bool testAtom(Context* context);
    private:
        bool m_isTrue;
};

/// The choose element wraps if/else blocks into a choose block.
class ChooseAtom : public AbstractAtom
{
    public:
        QString m_name;
        explicit ChooseAtom() : AbstractAtom("dgm:choose") {}
        virtual ~ChooseAtom() {}
        virtual ChooseAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        virtual void readElement(Context* context, MsooXmlDiagramReader* reader);
        virtual void build(Context* context);
};

/// A looping structure, similar to a for loop in a programming language, which defines what data model points will use this layout node.
class ForEachAtom : public AbstractAtom
{
    public:
        QString m_axis;
        QString m_hideLastTrans;
        QString m_name;
        QString m_ptType;
        QString m_reference;
        QString m_start;
        QString m_step;
        QString m_count;
        explicit ForEachAtom() : AbstractAtom("dgm:forEach") {}
        virtual ~ForEachAtom() {}
        virtual ForEachAtom* clone();
        virtual void dump(Context* context, int level);
        virtual void readAll(Context* context, MsooXmlDiagramReader* reader);
        virtual void build(Context* context);
};

/// The base class for layout-algorithms.
class AbstractAlgorithm {
    public:
        explicit AbstractAlgorithm();
        virtual ~AbstractAlgorithm();
        Context* context() const;
        LayoutNodeAtom* layout() const;
        LayoutNodeAtom* parentLayout() const;
        QList<LayoutNodeAtom*> childLayouts() const;
        qreal defaultValue(const QString& type, const QMap<QString, qreal>& values);
        void doInit(Context* context, QExplicitlySharedDataPointer<LayoutNodeAtom> layout);
        void doLayout();
        void doLayoutChildren();
    protected:
        void setNodePosition(LayoutNodeAtom* l, qreal x, qreal y, qreal w, qreal h);
        virtual qreal virtualGetDefaultValue(const QString& type, const QMap<QString, qreal>& values);
        virtual void virtualDoInit();
        virtual void virtualDoLayout();
        virtual void virtualDoLayoutChildren();
    private:
        Context* m_context;
        QExplicitlySharedDataPointer<LayoutNodeAtom> m_layout;
        QExplicitlySharedDataPointer<LayoutNodeAtom> m_parentLayout;
        AbstractNode* m_oldCurrentNode;
};

/// The composite algorithm specifies the size and position for all child layout nodes. You can use it to create graphics with a predetermined layout or in combination with other algorithms to create more complex shapes.
class CompositeAlgorithm : public AbstractAlgorithm {
    public:
        explicit CompositeAlgorithm() : AbstractAlgorithm() {}
        virtual ~CompositeAlgorithm() {}
};

/// The connector algorithm lays out and routes connecting lines, arrows, and shapes between layout nodes.
class ConnectorAlgorithm : public AbstractAlgorithm {
    public:
        explicit ConnectorAlgorithm() : AbstractAlgorithm() {}
        virtual ~ConnectorAlgorithm() {}
    protected:
        virtual qreal virtualGetDefaultValue(const QString& type, const QMap<QString, qreal>& values);
        virtual void virtualDoLayoutChildren();
};

/// The cycle algorithm lays out child layout nodes around a circle or portion of a circle using equal angle spacing.
class CycleAlgorithm : public AbstractAlgorithm {
    public:
        explicit CycleAlgorithm() : AbstractAlgorithm() {}
        virtual ~CycleAlgorithm() {}
    protected:
        virtual qreal virtualGetDefaultValue(const QString& type, const QMap<QString, qreal>& values);
        virtual void virtualDoLayout();
};

/// The linear algorithm lays out child layout nodes along a horizontal or vertical linear path.
class LinearAlgorithm : public AbstractAlgorithm {
    public:
        explicit LinearAlgorithm() : AbstractAlgorithm() {}
        virtual ~LinearAlgorithm() {}
    protected:
        virtual void virtualDoLayout();
};

/// The snake algorithm lays out child layout nodes along a linear path in two dimensions, allowing the linear flow to continue across multiple rows or columns.
class SnakeAlgorithm : public AbstractAlgorithm {
    public:
        explicit SnakeAlgorithm() : AbstractAlgorithm() {}
        virtual ~SnakeAlgorithm() {}
    protected:
        virtual void virtualDoLayout();
};

/// The hierarchy root algorithm works with the hierChild algorithm to create hierarchical tree layouts.
class HierarchyAlgorithm : public AbstractAlgorithm {
    public:
        explicit HierarchyAlgorithm(bool isRoot) : AbstractAlgorithm(), m_isRoot(isRoot) {}
        virtual ~HierarchyAlgorithm() {}
    protected:
        virtual void virtualDoLayout();
    private:
        bool m_isRoot; // root or child?
};

/// The pyramid algorithm lays out child layout nodes along a vertical path and works with the trapezoid shape to create a pyramid.
class PyramidAlgorithm : public AbstractAlgorithm {
    public:
        explicit PyramidAlgorithm() : AbstractAlgorithm() {}
        virtual ~PyramidAlgorithm() {}
    protected:
        virtual qreal virtualGetDefaultValue(const QString& type, const QMap<QString, qreal>& values);
        virtual void virtualDoLayout();
};

/// The space algorithm is used to specify a minimum space between other layout nodes or as an indication to do nothing with the layout node’s size and position.
class SpaceAlg : public AbstractAlgorithm {
    public:
        explicit SpaceAlg() : AbstractAlgorithm() {}
        virtual ~SpaceAlg() {}
        virtual void virtualDoLayout();
};

/// The text algorithm sizes text to fit inside a shape and controls its margins and alignment.
class TextAlgorithm : public AbstractAlgorithm {
    public:
        explicit TextAlgorithm() : AbstractAlgorithm() {}
        virtual ~TextAlgorithm() {}
    protected:
        virtual qreal virtualGetDefaultValue(const QString& type, const QMap<QString, qreal>& values);
        virtual void virtualDoLayout();
};

}} // namespace MSOOXML::Diagram

#endif
