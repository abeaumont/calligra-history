/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KOTEXTONSHAPECONTAINER_H
#define KOTEXTONSHAPECONTAINER_H

#include "KoShapeContainer.h"

#include "flake_export.h"

class KoTextOnShapeContainerPrivate;
class KoResourceManager;

/**
 * Container that is used to wrap a shape with a text on top.
 * Adding this container as a parent to any shape will allow you to add text
 * on top of that shape in the form of the decorator (design) pattern.
 */
class FLAKE_EXPORT KoTextOnShapeContainer : public KoShapeContainer
{
public:
    /**
     * Constructor to make a decorator container that adds a text on top of another shape.
     * @param childShape this is the original shape that should get the text placed on top of.
     * @param documentResources a resource manager holding properties for the text shape to
     *      integrate better into the document. Not passing this will mean that for example
     *      the text styles will not be shared with other shapes.
     */
    explicit KoTextOnShapeContainer(KoShape *childShape, KoResourceManager *documentResources = 0);
    virtual ~KoTextOnShapeContainer();

    // reimplemented
    virtual void paintComponent(QPainter &painter, const KoViewConverter &converter);
    // reimplemented
    virtual bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context);
    // reimplemented
    virtual void saveOdf(KoShapeSavingContext &context) const;

    /// different kinds of resizing behavior to determine how to treat text overflow
    enum ResizeBehavior {
        TextFollowsContentSize, ///< Text area is same size as content, extra text will be clipped
        ContentFollowsTextSize, ///< Content shape will get resized if text grows/shrinks
        IndependendSizes,       ///< The text can get bigger than the content
        TextFollowsPreferredTextRect ///< The size/position of the text area will follow the preferredTextRect property
    };

    /**
     * Set the behavior that is used to resize the text or content.
     * In order to determine what to do when there is too much text to fit or suddenly less
     * text the user can define the wanted behavior using the ResizeBehavior
     * @param resizeBehavior the new ResizeBehavior
     */
    void setResizeBehavior(ResizeBehavior resizeBehavior);

    /**
     * Returns the current ResizeBehavior.
     */
    ResizeBehavior resizeBehavior() const;

    /**
     * Set the current prefered text rectangle. This rect contains the coordinates of
     * the embedded text shape relative to the content shape. This value is ignored if
     * resizeBehavior is not TextFollowsPreferredTextRect.
     * @param rect the new preferred text rectangle
     */
    void setPreferredTextRect(const QRectF &rect);

    /**
     * Returns the current prefered text rectangle.
     */
    QRectF preferredTextRect() const;

    /** Sets the alignment of the text. */
    void setTextAlignment(Qt::Alignment alignment);
    /** Returns the alignment of all text */
    Qt::Alignment textAlignment() const;

    /**
     * Set some plain text to be displayed on the shape.
     * @param text the full text.
     */
    void setPlainText(const QString &text);

    /**
     * This method is called by shapes owned by this container to allow the container to add
     * ODf content as child elements of the same element that the shape itself saved into.
     * This method is typically called in the shapes saveOdf() method before closing the
     * xml element it saved into.
     */
    virtual void saveOdfChildElements(KoShapeSavingContext &context) const;

    /**
     * Try to load text-on-shape from \a element and wrap \a shape with it.
     * In ODF certain shape-types can have a "text:p" child-element which
     * translates in KOffice design to it ending up with a
     * KoTextOnShapeContainer as parent.
     * What this method does is check if the argument element has a text:p
     * child and if it does (and is valid) then it adds a parent to the
     * shape (see constructor) and loads the text data.
     * If no text data was found the shape is not touched.
     * Nothing is returned as there typically is no need to check. If you want
     * to know that the text was added you can call parent() on the shape you
     * passed in.
     */
    static void tryWrapShape(KoShape *shape, const KoXmlElement &element,
            KoShapeLoadingContext &context);

private:
    Q_DECLARE_PRIVATE(KoTextOnShapeContainer)
};

#endif
