//
// C++ Template: Singleton<T>
//
#ifndef __SINGLETON_H__INCLUDED__
#define __SINGLETON_H__INCLUDED__

namespace InitializationFunctors {

    template<typename T> class DefaultConstructor
    {
    public:
        T* operator()() const { return new T(); };
    };
    
};

/**
 * Singleton template
 *
 * @remarks 
 *      The default initialization functor can handle classes with a default constructor.
 *      If you require special initialization specify a different functor.
 *      The class is NOT threadsafe.
 *
 * @remarks
 *      You must add an extern Singleton<T,INIT>::_singletonInstance in your class implementation
 *      or linking against derived classes will fail.
 */
template<typename T, typename INIT = InitializationFunctors::DefaultConstructor<T> > class Singleton 
{
protected:
    static T* _singletonInstance;
    
    static void initializeSingleton()
    {
        if (_singletonInstance != 0)
            delete _singletonInstance;
        _singletonInstance = (INIT())();
    }
    
    static void deinitializeSingleton()
    {
        _singletonInstance = 0;
    }
    
    ~Singleton()
    {
        if (_singletonInstance == this)
            deinitializeSingleton();
    }
    
public:
    static T* getSingleton()
    {
        if (_singletonInstance == 0)
            initializeSingleton();
        return _singletonInstance;
    }
};



#endif//__SINGLETON_H__INCLUDED__
