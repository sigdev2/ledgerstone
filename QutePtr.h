//
//
//////////////////////////////////////////////////////////////////////////
// It's a simple wrapper on pointer. 
// We can't use auto_ptr or shared_ptr because 
// 1. They own the internal object, but it objects belongs to QWidget
// 2. We could use several qt_ptr that contain pointer to the same object

#ifndef __QUTE_PTR_H__
#define __QUTE_PTR_H__

class QObject;

template<typename T>
class _qt_ptr_accessor;

#ifndef _ass
#    include <assert.h>
#    define _ass(x) assert( x );
#endif // _ass

//////////////////////////////////////////////////////////////////////////
template<typename T>
class qt_ptr
{
    friend class _qt_ptr_accessor<T>;

public:
    qt_ptr() : m_ptr(0) { /* qt_guard(m_ptr); */ }
    template<typename Y> explicit qt_ptr(Y* pvalue) : m_ptr(pvalue) {}
    template<typename Y> explicit qt_ptr(const qt_ptr<Y>& rvalue) { _qt_ptr_accessor<Y> _a(rvalue); m_ptr=_a.ptr(); }

    template<typename Y> qt_ptr<T>& operator=(Y* pvalue) { m_ptr=pvalue; return *this; }
    qt_ptr<T>& operator=(T* pvalue) { m_ptr=pvalue; return *this; }

    template<typename Y> qt_ptr<T>& operator=(const qt_ptr<Y>& other) { _qt_ptr_accessor<Y> _a(other); m_ptr=_a.ptr(); return *this; }
    //qt_ptr<T>& operator=(const qt_ptr<T>& other) { m_ptr=other.m_ptr; return *this; } // Uses when Y==T

    T* ptr()             { return m_ptr; }
    const T* ptr() const { return m_ptr; }

    void deleteLater() { if (m_ptr) { m_ptr->deleteLater(); m_ptr = 0; } }

    operator T*()               { /*_ass(m_ptr);*/ return m_ptr; }
    operator const T*() const   { _ass(m_ptr); return m_ptr; }
    T& operator*()              { _ass(m_ptr); return *m_ptr; }
    const T& operator*() const  { _ass(m_ptr); return *m_ptr; }
    T* operator->()             { _ass(m_ptr); return m_ptr; }
    const T* operator->() const { _ass(m_ptr); return m_ptr; }

    bool operator!() const { return m_ptr == 0; }
    operator bool()  const { return m_ptr != 0; }

    bool isEqual(const T *p)    const { return m_ptr == p; }
//    bool isEqual(T *p)    const { return m_ptr == p; }

    bool operator==(const T *p) const { return m_ptr == p; }
    bool operator!=(const T *p) const { return m_ptr != p; }

    template<typename Y> bool operator==(const Y *yp) const { return m_ptr == yp; }
    template<typename Y> bool operator!=(const Y *yp) const { return m_ptr != yp; }    

    bool operator==(const qt_ptr<T>& p) const { return m_ptr == p.m_ptr; }
    bool operator!=(const qt_ptr<T>& p) const { return m_ptr != p.m_ptr; }

    template<typename Y> bool operator==(const qt_ptr<Y>& p) const { return m_ptr == p.ptr(); }
    template<typename Y> bool operator!=(const qt_ptr<Y>& p) const { return m_ptr != p.ptr(); }

    // for comparisions like  "qt_ptr_inst != 0"
    // clashes with X64 build
    //bool operator==(int p) const { _ass(0==p); return m_ptr == 0; }
    //bool operator!=(int p) const { _ass(0==p); return m_ptr != 0; }

private:
    static void qt_guard(QObject*){}
    T*  m_ptr;
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
class _qt_ptr_accessor
{
public:
    _qt_ptr_accessor(const qt_ptr<T>& val) : m_val(val) {}
    T* ptr() { return m_val.m_ptr; }

private:
    const qt_ptr<T>& m_val;
};

#endif //__QUTE_PTR_H__
