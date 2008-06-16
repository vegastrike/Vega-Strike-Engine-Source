//
// C++ Interface: Audio::Codec
//
#ifndef __AUDIO_EXCEPTIONS_H__INCLUDED__
#define __AUDIO_EXCEPTIONS_H__INCLUDED__

#include <string>
#include <exception>

namespace Audio {

    /**
     * Base exception class
     *
     * @see std::exception
     * 
     */
    class Exception : public std::exception
    {
    private:
        std::string _message;
        
    public:
        Exception() {};
        Exception(const Exception &other) : _message(other._message) {}
        explicit Exception(const std::string &message) : _message(message) {}
        virtual ~Exception() throw() {}
        virtual const char* what() const throw() { return _message.c_str(); }
    };
    
    /**
     * File Open exception
     * @remarks thrown when an unrecoverable attempt to open a file fails.
     */
    class FileOpenException : public Exception {
    public:
        FileOpenException() {}
        FileOpenException(const FileOpenException &other) : Exception(other) {}
        explicit FileOpenException(const std::string &message) : Exception(message) {}
    };
    
    /**
     * Codec not found exception
     * @remarks thrown when an attempt to open a file with a certain codec fails,
     *      or when such codec cannot be found.
     */
    class CodecNotFoundException : public Exception {
    public:
        CodecNotFoundException() {}
        CodecNotFoundException(const CodecNotFoundException &other) : Exception(other) {}
        explicit CodecNotFoundException(const std::string &message) : Exception(message) {}
    };
    
    /**
     * File Format exception
     * @remarks thrown when an unrecoverable attempt to parse a file fails.
     */
    class FileFormatException : public Exception {
    public:
        FileFormatException() {}
        FileFormatException(const FileFormatException &other) : Exception(other) {}
        explicit FileFormatException(const std::string &message) : Exception(message) {}
    };
    
    /**
     * End-Of-Stream exception
     * @remarks thrown when an attempt to seek past the end of a stream is made.
     */
    class EndOfStreamException : public Exception {
    public:
        EndOfStreamException() {}
        EndOfStreamException(const EndOfStreamException &other) : Exception(other) {}
        explicit EndOfStreamException(const std::string &message) : Exception(message) {}
    };
    
    /**
     * Corrupt-Stream exception
     * @remarks thrown when a corruption in an already open and seemingly correct stream is found.
     *      @par It is possible that implementations may recover from this. If so, a second attempt
     *      at reading may or may not fail. Recoverability is reported by isFatal(). Notice that
     *      recoverable corruption means only that an attempt to recover is possible, not that the
     *      attempt will succeed.
     */
    class CorruptStreamException : public Exception {
        bool fatal;
    public:
        CorruptStreamException(const CorruptStreamException &other) : Exception(other) {}
        explicit CorruptStreamException(bool _fatal) 
            : Exception(fatal ? "Fatal corruption on stream" : "Recoverable corruption on stream"),
              fatal(_fatal) 
        {}
        
        bool isFatal() const { return fatal; }
    };
    
    /**
     * Resource not loaded exception
     * @remarks thrown when an attempt to access a resource that hasn't been loaded yet is made,
     *      within a context where automatic loading is not possible.
     */
    class ResourceNotLoadedException : public Exception {
    public:
        ResourceNotLoadedException() {}
        ResourceNotLoadedException(const ResourceNotLoadedException &other) : Exception(other) {}
        explicit ResourceNotLoadedException(const std::string &message) : Exception(message) {}
    };
    
    /**
     * Attempted to create an object that already existed
     */
    class DuplicateObjectException : public Exception {
    public:
        explicit DuplicateObjectException(const std::string &name) : 
            Exception(std::string("Object with name \"") + name + "\" already existed") {}
    };
    
    /**
     * Seeked object did not exist
     */
    class NotFoundException : public Exception {
    public:
        explicit NotFoundException(const std::string &name) : 
            Exception(std::string("Object with name \"") + name + "\" does not exist") {}
    };
    
};

#endif//__AUDIO_EXCEPTIONS_H__INCLUDED__
