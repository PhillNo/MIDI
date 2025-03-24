#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H


template <class T>
class Noncopyable
{
public: 
        Noncopyable(const Noncopyable &) = delete;
    T&  operator=(const T &) = delete;

protected:
        Noncopyable() = default;
       ~Noncopyable() = default;
};


#endif
