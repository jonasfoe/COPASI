// Copyright (C) 2013 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef QCONNECTION_GRAPHICS_ITEM
#define QCONNECTION_GRAPHICS_ITEM

#include <QtGui/QGraphicsItemGroup>
#include <QtGui/QPainterPath>
#include <QtCore/QSharedPointer>
#include <QtGui/QStyleOptionGraphicsItem>
#include <layout/CLCurve.h>
#include <qlayout/CQCopasiGraphicsItem.h>

class CLGlyphWithCurve;
class CLStyle;
class CQConnectionGraphicsItem : public QObject, public CQCopasiGraphicsItem, public QGraphicsItemGroup
{
  Q_OBJECT
public:
  CQConnectionGraphicsItem(const CLGlyphWithCurve* glyph, const CLRenderResolver* resolver = NULL);
  virtual ~CQConnectionGraphicsItem();
  static QSharedPointer<QPainterPath> getPath(const CLCurve& curve);
  virtual QPainterPath shape() const;
protected:
  virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option = new QStyleOptionGraphicsItem() , QWidget *widget = 0);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
  virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
  bool mWasMoved;
  QPainterPath mShape;
};

#endif
