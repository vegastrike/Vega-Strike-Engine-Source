#ifndef __AUDIO_UTILS_H__INCLUDED__
#define __AUDIO_UTILS_H__INCLUDED__

#include "Types.h"
#include "Exceptions.h"
#include <map>

namespace Audio {

    // Forward declarations
    class Listener;
    class Source;

    /** Get the game time stamp - ie, the time as it elapses in the game's universe */
    Timestamp getGameTime() throw();
    
    /** Get the current real time stamp */
    Timestamp getRealTime() throw();

    /** Estimate a distant source's gain
     * @remarks Computes source attenuation relative to a listener.
     *      The computation is an estimated (a rather accurate estimate though).
     *      Advanced factors like atmospheric absorption and environment filtering
     *      may make the final attenuation differ significantly, so this should only
     *      be used for culling purposes.
     */
    Scalar estimateGain(const Source &src, const Listener &listener) throw();
    
    /** Make the thread sleep for at least 'ms' milliseconds.
     * @remarks sleep(0) is a very common way to implement a waiting loop:
     *      @code while (condition) sleep(0);
     *      @par Interval precision is highly dependant on the platform. Usually,
     *      it lays close to 15ms.
     */
    void sleep(unsigned int ms);
    
    /** Parse an enumerated string into its actual value 
     * @param s The string to be parsed
     * @param enumMap A map associating strings to values
     * @remarks Returns enumMap[s] raising an exception on error.
     */
    template<typename T> T parseEnum(const std::string &s, const std::map<std::string, T> &enumMap)
    {
        typename std::map<std::string, T>::const_iterator it = enumMap.find(s);
        if (it != enumMap.end())
            return it->second; else
            throw InvalidParametersException("Enumerated value \"" + s + "\" not recognized");
    }
    
    /** Parse an enumerated string into its actual value 
     * @param s The string to be parsed
     * @param enumMap A map associating strings to values
     * @param deflt A default value to return when s is found in the enumeration map.
     * @remarks Returns enumMap[s] or the default, no exceptions raised.
     */
    template<typename T> T parseEnum(const std::string &s, const std::map<std::string, T> &enumMap, T deflt)
    {
        typename std::map<std::string, T>::const_iterator it = enumMap.find(s);
        if (it != enumMap.end())
            return it->second; else
            return deflt;
    }
    

}

#endif//__AUDIO_UTILS_H__INCLUDED__

