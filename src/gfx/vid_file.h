//
// C++ Interface: vid_file
//
#ifndef __VID_FILE_H__INCLUDED__
#define __VID_FILE_H__INCLUDED__

#include <string>
#include <vsfilesystem.h>
#include <exception>

class VideoFileImpl;

class VideoFile
{
public:
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
    
    class UnsupportedCodecException : public Exception {
    public:
        UnsupportedCodecException() {}
        UnsupportedCodecException(const UnsupportedCodecException &other) : Exception(other) {}
        explicit UnsupportedCodecException(const std::string &message) : Exception(message) {}
    };
    class FileOpenException : public Exception {
    public:
        FileOpenException() {}
        FileOpenException(const FileOpenException &other) : Exception(other) {}
        explicit FileOpenException(const std::string &message) : Exception(message) {}
    };
    class FrameDecodeException : public Exception {
    public:
        FrameDecodeException() {}
        FrameDecodeException(const FrameDecodeException &other) : Exception(other) {}
        explicit FrameDecodeException(const std::string &message) : Exception(message) {}
    };
    class EndOfStreamException : public Exception {
    public:
        EndOfStreamException() {}
        EndOfStreamException(const EndOfStreamException &other) : Exception(other) {}
        explicit EndOfStreamException(const std::string &message) : Exception(message) {}
    };

public:
    VideoFile() throw();
    ~VideoFile();
    
    bool isOpen() const throw();
    
    void open(const std::string& path, size_t maxDimension = 65535) throw(Exception);
    void close() throw();
    
    /** Seeks to the specified time
      * @Returns true if frame changed, false otherwise.
      * @Throws EndOfStreamException when time lays past the end.
      * @Throws FrameDecodeException when an error occurs during frame decode.
      */
    bool seek(float time) throw(Exception);

    float getFrameRate() const throw();
    float getDuration() const throw();
    int getWidth() const throw();
    int getHeight() const throw();
    void* getFrameBuffer() const throw();
    int getFrameBufferStride() const throw();
    
private:
    VideoFileImpl *impl;
};

#endif//__VID_FILE_H__INCLUDED__
