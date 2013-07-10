////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2013
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// "smart" pointer for reference counting classes
//
// These should only be used in long-lived containers, never on the stack.
//

template <class item_t, class allocator_t=allocator> class ref {
  // wrapped pointer to the object
  item_t *item;

public:
  // default initializer - empty pointer
  ref() {
    item = 0;
  }

  ref(const ref &rhs) {
    item = rhs.item;
    item->add_ref();
  }

  // initialize with new item - pointer then "owns" object
  ref(item_t *new_item) {
    new_item->add_ref();
    item = new_item;
  }

  // replace item with new one - frees any old object
  item_t *operator=(item_t *new_item) {
    new_item->add_ref();
    if (item) item->release();
    item = new_item;
    return new_item;
  }

  // get a pointer to the constant item
  operator const item_t *() const { return item; }

  // get a pointer to the item. Beware! do not store or pass this pointer.
  operator item_t *() { return item; }

  // get a pointer to the item
  item_t * operator ->() const { return item; }

  // destructor - may free object
  ~ref() {
    if (item) item->release();
    item = 0;
  }
};
