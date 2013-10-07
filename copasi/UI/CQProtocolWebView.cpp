#include "CQProtocolWebView.h"
#include <QWebPage>
#include <QMenu>
#include <QContextMenuEvent>

CQProtocolWebView::CQProtocolWebView(QWidget * parent) : QWebView(parent)
{
}

void CQProtocolWebView::contextMenuEvent(QContextMenuEvent * ev)
{
  QMenu * customContextMenu;

  customContextMenu = this->page()->createStandardContextMenu();

  customContextMenu->addAction(QWebView::pageAction(QWebPage::SelectAll));

  customContextMenu->exec(ev->globalPos());
}
