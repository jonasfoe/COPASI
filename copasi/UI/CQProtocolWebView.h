#ifndef CQPROTOCOLWEBVIEW_H
#define CQPROTOCOLWEBVIEW_H

#include <QWebView>

class CQProtocolWebView : public QWebView
{
  Q_OBJECT

public:
  CQProtocolWebView(QWidget * parent = 0);
protected:
  virtual void contextMenuEvent(QContextMenuEvent * ev);
};

#endif // CQPROTOCOLWEBVIEW_H
