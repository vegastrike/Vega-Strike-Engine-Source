#ifndef __ERRORS_H__
#define __ERRORS_H__

#include <ostream>
#include <fstream>
#include <exception>

class error
{
public:
};

//copied from mesh_gfx for inspiration:
class Exception : public std::exception
{
private:
    std::string _message;

public: Exception() {}
    Exception( const Exception &other ) : _message( other._message ) {}
    explicit Exception( const std::string &message ) : _message( message ) {}
    virtual ~Exception() throw () {}
    virtual const char * what() const throw ()
    {
        return _message.c_str();
    }
};

class MissingTexture : public Exception
{
public:
    explicit MissingTexture( const string &msg ) : Exception( msg ) {}
    MissingTexture() {}
};


#endif


