// Copyright (C) 2019 - 2020 by Pedro Mendes, Rector and Visitors of the
// University of Virginia, University of Heidelberg, and University
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2017 - 2018 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2005 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#ifndef CUPDOWNSUBWIDGET_H
#define CUPDOWNSUBWIDGET_H

#include <QtCore/QVariant>
#include "ui_CUpDownSubwidget.h"

class CUpDownSubwidget : public QWidget, public Ui::CUpDownSubwidget
{
  Q_OBJECT

public:
  CUpDownSubwidget(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::WindowFlags());
  ~CUpDownSubwidget();

  virtual int getIndex() const;
  void enableCopy(const bool &);

public slots:
  virtual void setIndex(int, bool, bool);

signals:
  void copy(int);
  void up(int);
  void down(int);
  void del(int);

protected:
  int mIndex;

protected slots:

  void slotUp();
  void slotDown();
  void slotDel();
  void slotCopy();

private:
  void init();

protected:
  enum IconID
  {
    image0_ID,
    image1_ID,
    image2_ID,
    image3_ID,
    unknown_ID
  };
  static QPixmap qt_get_icon(IconID id)
  {
    static const unsigned char image0_data[] =
    {
      0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
      0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f,
      0x08, 0x06, 0x00, 0x00, 0x00, 0x3b, 0xd6, 0x95, 0x4a, 0x00, 0x00, 0x00,
      0xbf, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0xad, 0x92, 0x41, 0x0a, 0xc2,
      0x30, 0x14, 0x44, 0xdf, 0x88, 0x7b, 0x11, 0x4f, 0x20, 0xb8, 0xa8, 0xb7,
      0x70, 0xe3, 0x41, 0x3c, 0x47, 0x8f, 0x96, 0x8d, 0x27, 0x70, 0xd9, 0xb4,
      0x10, 0xe8, 0x09, 0x8a, 0x78, 0x01, 0xbf, 0x1b, 0x53, 0x6c, 0x6c, 0xaa,
      0x54, 0x3f, 0x04, 0xc2, 0x64, 0x26, 0xc3, 0x4c, 0x22, 0x33, 0x63, 0xee,
      0x2c, 0x66, 0x2b, 0xff, 0x2a, 0xae, 0x25, 0xe7, 0xa5, 0x30, 0x25, 0xf0,
      0x52, 0xe8, 0x39, 0x66, 0x36, 0x58, 0x1e, 0xba, 0x0a, 0x42, 0x8a, 0xd7,
      0x50, 0x56, 0x10, 0x6a, 0x28, 0x23, 0xa6, 0xb4, 0xb0, 0x46, 0x2a, 0xef,
      0x70, 0x02, 0xd8, 0x9b, 0xed, 0xa2, 0x1b, 0x80, 0xa0, 0x2d, 0xcc, 0x8e,
      0x3d, 0x39, 0x75, 0x88, 0x2e, 0x1e, 0xba, 0xe8, 0x16, 0xf7, 0x29, 0xef,
      0xcd, 0xf9, 0x25, 0x7f, 0x07, 0xac, 0x80, 0x5b, 0x61, 0xb6, 0x19, 0xe3,
      0x2c, 0x33, 0x42, 0x67, 0x70, 0xd5, 0x54, 0x73, 0x30, 0x74, 0x8e, 0x79,
      0x05, 0x6b, 0xe0, 0x22, 0x38, 0x03, 0x3c, 0xb1, 0x61, 0xde, 0x54, 0xec,
      0xa5, 0x20, 0x68, 0x01, 0x22, 0xb1, 0x91, 0x4a, 0x83, 0x83, 0xc1, 0x36,
      0x5b, 0x98, 0x07, 0xe7, 0xc1, 0x8d, 0x15, 0x98, 0x7b, 0xaa, 0xfe, 0x60,
      0xac, 0xcd, 0xdc, 0x25, 0x1f, 0xdb, 0xfe, 0x66, 0x7e, 0xfa, 0xdb, 0x0f,
      0x18, 0xe8, 0xd7, 0x34, 0x08, 0xf9, 0x77, 0xe5, 0x00, 0x00, 0x00, 0x00,
      0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
    };

    static const unsigned char image1_data[] =
    {
      0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
      0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f,
      0x08, 0x06, 0x00, 0x00, 0x00, 0x3b, 0xd6, 0x95, 0x4a, 0x00, 0x00, 0x00,
      0x57, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0xcd, 0x92, 0x41, 0x0a, 0xc0,
      0x20, 0x10, 0x03, 0x1d, 0xbf, 0xdd, 0xc7, 0xa7, 0x27, 0x8b, 0xd8, 0x44,
      0x84, 0x8a, 0x34, 0x17, 0xc1, 0x35, 0xcc, 0xa0, 0x22, 0xa9, 0xa4, 0x00,
      0x92, 0x44, 0x9a, 0xd7, 0xd8, 0x5c, 0x48, 0x2c, 0x03, 0x57, 0xbf, 0xda,
      0x33, 0x49, 0x1b, 0x78, 0x06, 0x49, 0xdd, 0x92, 0x47, 0x5a, 0xa2, 0x5b,
      0x72, 0x4f, 0x9d, 0xd1, 0xf7, 0x5e, 0x58, 0x54, 0x34, 0xfb, 0x2f, 0x6d,
      0xa7, 0xdc, 0x32, 0xaa, 0x7f, 0xd2, 0x8e, 0x4f, 0xd5, 0x2c, 0xce, 0xff,
      0xb0, 0x7f, 0x97, 0x6f, 0x05, 0xe3, 0x25, 0xff, 0x82, 0xbd, 0xc4, 0x5e,
      0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
    };

    static const unsigned char image2_data[] =
    {
      0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
      0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
      0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x02,
      0x18, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x7d, 0x92, 0xcb, 0x4a, 0x5c,
      0x41, 0x10, 0x86, 0xbf, 0xea, 0xee, 0x39, 0x73, 0xc6, 0x11, 0x41, 0x72,
      0x11, 0x11, 0x02, 0x51, 0xb3, 0x90, 0x90, 0x59, 0x8a, 0xb8, 0xc8, 0x22,
      0x2f, 0x12, 0x12, 0x92, 0x07, 0x08, 0x24, 0x8b, 0x11, 0x02, 0x82, 0x1b,
      0x4d, 0xde, 0x20, 0x31, 0x97, 0x95, 0x1b, 0x1f, 0x20, 0xcc, 0xd2, 0x80,
      0x8a, 0x06, 0xc4, 0x45, 0x44, 0x54, 0x22, 0x51, 0x88, 0x59, 0xa8, 0xa8,
      0x73, 0xc6, 0xb9, 0x9e, 0xd3, 0x95, 0xc5, 0x78, 0xd7, 0x49, 0x41, 0x6d,
      0xaa, 0xe8, 0xbf, 0xff, 0xff, 0xeb, 0x96, 0xd9, 0xd9, 0xf5, 0x83, 0xce,
      0xce, 0x76, 0xa7, 0xea, 0x89, 0x63, 0x8f, 0xf7, 0x8a, 0x31, 0x96, 0x95,
      0x95, 0xcd, 0xf6, 0xe5, 0xe5, 0xb5, 0xe7, 0x13, 0x13, 0x2f, 0xbe, 0xf0,
      0xbf, 0x5a, 0x5a, 0xda, 0x6c, 0x78, 0xef, 0xf5, 0xb4, 0x6b, 0x35, 0xaf,
      0xfb, 0xfb, 0x15, 0x5d, 0x5d, 0xfd, 0xab, 0x53, 0x53, 0xdf, 0x75, 0x64,
      0xe4, 0xd3, 0x1b, 0x55, 0xa5, 0x55, 0x9b, 0xab, 0x82, 0xd6, 0x42, 0x10,
      0x04, 0x6c, 0x6c, 0xfc, 0x61, 0x6f, 0xef, 0x80, 0x30, 0x0c, 0xde, 0xe5,
      0xf3, 0x93, 0x6f, 0x5b, 0x19, 0xb8, 0x26, 0x20, 0x02, 0x49, 0x12, 0x93,
      0xcb, 0xf5, 0x33, 0x38, 0xf8, 0x90, 0x81, 0x81, 0x5e, 0x44, 0xcc, 0xfd,
      0x56, 0x02, 0xee, 0x26, 0x01, 0x6b, 0x2d, 0xc5, 0x62, 0x99, 0x4a, 0xa5,
      0x4e, 0x47, 0x47, 0x96, 0xe1, 0xe1, 0x47, 0x8f, 0x17, 0x16, 0x7e, 0x7d,
      0x88, 0xe3, 0x38, 0x53, 0x2c, 0x56, 0xb2, 0x33, 0x33, 0x3f, 0xbe, 0x8d,
      0x8f, 0xbf, 0xfc, 0x78, 0x23, 0x83, 0x28, 0xf2, 0xba, 0xbd, 0x7d, 0xa8,
      0x73, 0x73, 0xeb, 0x7a, 0x74, 0x54, 0xbd, 0x34, 0xdf, 0xd9, 0x89, 0x74,
      0x6b, 0xeb, 0x50, 0xa7, 0xa7, 0xe7, 0x74, 0x74, 0xf4, 0xeb, 0x6b, 0x55,
      0xbd, 0xec, 0xa0, 0x5a, 0x85, 0x72, 0xb9, 0x8a, 0x31, 0x16, 0x6b, 0x2d,
      0xe9, 0xf4, 0xf9, 0x3a, 0x9b, 0x05, 0xef, 0x33, 0xec, 0xee, 0x1e, 0x12,
      0x45, 0x11, 0xe9, 0x74, 0xf8, 0x3e, 0x9f, 0x9f, 0xbc, 0x7b, 0xc6, 0x40,
      0x15, 0xea, 0xf5, 0x06, 0xce, 0x19, 0xc2, 0xd0, 0x21, 0x22, 0xa8, 0x5e,
      0xc9, 0xeb, 0x84, 0x20, 0x48, 0xd1, 0xdd, 0x7d, 0x87, 0xfe, 0xfe, 0x7b,
      0xb4, 0xb5, 0x85, 0xaf, 0xce, 0x04, 0xea, 0x75, 0x10, 0xb1, 0x88, 0xb4,
      0xc2, 0x05, 0xc6, 0x08, 0xe9, 0x74, 0x8a, 0xae, 0xae, 0x5b, 0xf4, 0xf4,
      0xdc, 0xc6, 0xb9, 0x14, 0xce, 0x98, 0xf3, 0x87, 0x70, 0x4e, 0x30, 0xc6,
      0x91, 0x24, 0x70, 0x71, 0x7e, 0x5a, 0xe5, 0x72, 0x9d, 0x52, 0xa9, 0x8c,
      0xf7, 0x4a, 0x26, 0x93, 0x46, 0x44, 0x70, 0x51, 0x54, 0x21, 0x8a, 0x1a,
      0x44, 0x51, 0x15, 0xef, 0x15, 0xef, 0x3d, 0x49, 0xa2, 0xd4, 0xeb, 0x31,
      0x72, 0xc1, 0x4e, 0x14, 0x79, 0x8e, 0x8f, 0x6b, 0xa8, 0x0a, 0xd9, 0x6c,
      0x06, 0x6b, 0x2d, 0xce, 0x81, 0x2b, 0x14, 0xe6, 0xa7, 0x0a, 0x85, 0x79,
      0x73, 0xf9, 0xa6, 0x5a, 0x25, 0x97, 0x7b, 0xf0, 0x64, 0x68, 0xa8, 0xb7,
      0xef, 0x14, 0x6e, 0x1c, 0x37, 0x08, 0x02, 0x87, 0x31, 0x06, 0x63, 0xe4,
      0xc4, 0xa1, 0xe0, 0xc6, 0xc6, 0x9e, 0x3d, 0xbd, 0x29, 0x6f, 0xa1, 0xb0,
      0xfc, 0xd9, 0x7b, 0xfa, 0x8c, 0x81, 0x24, 0xf1, 0x58, 0x6b, 0x88, 0x63,
      0xc5, 0x5a, 0x83, 0xb5, 0x06, 0x11, 0xdb, 0xe4, 0xd2, 0x0a, 0x58, 0x18,
      0x86, 0x27, 0x51, 0x00, 0x9a, 0x51, 0xac, 0xbd, 0x4e, 0xf8, 0xda, 0x4f,
      0x3c, 0x27, 0xde, 0x3c, 0x98, 0x4a, 0x81, 0x88, 0xe0, 0x7d, 0x0a, 0x11,
      0x8f, 0x48, 0x0c, 0x80, 0x88, 0x69, 0x42, 0x6c, 0x25, 0xb0, 0xb8, 0xf8,
      0xb3, 0xb6, 0xb6, 0xf6, 0x1b, 0xa0, 0x04, 0xa0, 0xaa, 0x24, 0x49, 0x73,
      0xa7, 0x9a, 0x20, 0x02, 0xa5, 0x52, 0x25, 0xfe, 0x07, 0xd9, 0xbb, 0x1d,
      0xb5, 0x0b, 0x97, 0x87, 0x95, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e,
      0x44, 0xae, 0x42, 0x60, 0x82
    };

    static const unsigned char image3_data[] =
    {
      0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
      0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f,
      0x08, 0x06, 0x00, 0x00, 0x00, 0x3b, 0xd6, 0x95, 0x4a, 0x00, 0x00, 0x00,
      0x59, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0xd5, 0x93, 0x3b, 0x0e, 0xc0,
      0x30, 0x08, 0x43, 0xfd, 0x7a, 0xee, 0xdc, 0xdd, 0x9d, 0x3a, 0x94, 0xe0,
      0x2a, 0x43, 0x33, 0x84, 0x25, 0x12, 0x1f, 0xf1, 0x30, 0x01, 0xdb, 0x4a,
      0x06, 0xd8, 0x36, 0x29, 0x7e, 0xc5, 0xca, 0x05, 0x3b, 0xb4, 0x98, 0x2a,
      0x18, 0x10, 0x15, 0xac, 0xe2, 0xfd, 0x8e, 0x3d, 0x42, 0xee, 0xe4, 0x9f,
      0xb0, 0xa5, 0x1e, 0xbd, 0xdb, 0xf7, 0x16, 0xb5, 0x2b, 0x62, 0x3b, 0x4a,
      0x8b, 0x2d, 0xbd, 0xd1, 0xd3, 0x17, 0xfd, 0xc2, 0x1e, 0xe5, 0x5d, 0xef,
      0xfc, 0x74, 0xdf, 0x76, 0x18, 0x37, 0xe7, 0x62, 0x25, 0xf3, 0x8a, 0x3d,
      0x31, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42,
      0x60, 0x82
    };

    switch (id)
      {
        case image0_ID:  {QImage img; img.loadFromData(image0_data, sizeof(image0_data), "PNG"); return QPixmap::fromImage(img);}

        case image1_ID:  {QImage img; img.loadFromData(image1_data, sizeof(image1_data), "PNG"); return QPixmap::fromImage(img);}

        case image2_ID:  {QImage img; img.loadFromData(image2_data, sizeof(image2_data), "PNG"); return QPixmap::fromImage(img);}

        case image3_ID:  {QImage img; img.loadFromData(image3_data, sizeof(image3_data), "PNG"); return QPixmap::fromImage(img);}

        default: return QPixmap();
      } // switch
  } // icon
};

#endif // CUPDOWNSUBWIDGET_H
