
#pragma once

namespace wdk
{
    // manually define some stuff so that dependency to boost can be dropped
    struct noncopyable {
        noncopyable() {}
    private:
        noncopyable(noncopyable&);
        noncopyable& operator=(noncopyable&);

        //noncopyable(noncopyable&) = delete;
        //noncopyable& operator=(noncopyable&) = delete;
    };
    
    template<typename T, typename Deleter>
    std::unique_ptr<T, Deleter> make_unique_ptr(T* ptr, Deleter del)
    {
        return std::unique_ptr<T, Deleter>(ptr, del);
    }
    

} // wdk
