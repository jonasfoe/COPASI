#ifndef OBJECT_BROWSER_ITEM_H
#define OBJECT_BROWSER_ITEM_H

#include "qlistview.h"

class QListViewItem;
class QListView;
class CCopasiObject;

#define NOCHECKED -1
#define ALLCHECKED 1
#define PARTCHECKED 0

enum objectType {FIELDATTR = 0, OBJECTATTR, CONTAINERATTR};

class objectList;

struct browserObject
  {
    CCopasiObject* pCopasiObject;
    bool mChecked;
    objectList* referenceList;
    browserObject();
    ~browserObject();
  };

class ObjectBrowserItem : public QListViewItem
  {
  private:
    static long KeySpace;
    browserObject* pBrowserObject;
    ObjectBrowserItem* pParent;
    ObjectBrowserItem* pChild;
    ObjectBrowserItem* pSibling;
    objectType mType;
    QString mKey;

  public:
    browserObject* getObject()
    {
      return pBrowserObject;
    }

    virtual QString key (int column, bool ascending) const
      {
        return mKey;
      }

    // inline const QString & getKey() const {return mKey;}

    ObjectBrowserItem (QListView * parent, ObjectBrowserItem * after, CCopasiObject* mObject, objectList* pList);
    ObjectBrowserItem (ObjectBrowserItem * parent, ObjectBrowserItem * after, CCopasiObject* mObject, objectList* pList);
    ~ObjectBrowserItem()
    {
      delete pBrowserObject;
    }

    void attachKey();
    void setParent(ObjectBrowserItem* parent);
    void setChild(ObjectBrowserItem* child);
    void setSibling(ObjectBrowserItem* sibling);

    ObjectBrowserItem* parent() const;
    ObjectBrowserItem* child() const;
    ObjectBrowserItem* sibling() const;

    void reverseChecked();
    bool isChecked() const;

    void setObjectType(objectType newType)
    {
      mType = newType;
    }

    objectType getType()
    {
      return mType;
    }

    //-1 if this is no user checked
    //0 if this is only partly checked
    //1 if this is full checked
    int nUserChecked();
  };

struct objectListItem
  {
    objectListItem(ObjectBrowserItem* item, objectListItem* next)
    {
      pItem = item;
      pNext = next;
    }
    ObjectBrowserItem* pItem;
    objectListItem* pNext;
  };

class objectList
  {
  private:
    objectListItem* root;
    int length;
  public:
    objectList();
    ~objectList()
    {
      while (length > 0)
        pop();
    }
    void insert(ObjectBrowserItem* pItem);
    objectListItem* getRoot();
    ObjectBrowserItem* pop();
  inline int len() {return length;};
  };

#endif
