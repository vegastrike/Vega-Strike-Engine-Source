#ifndef VSNET_DLOADITEM_H
#define VSNET_DLOADITEM_H

#include <config.h>

#include <string>
#include <vector>
#include <list>
#include <map>
#include <queue>
#include <iostream>

#include "vsnet_dloadenum.h"

#include "boost/shared_ptr.hpp"
#include "boost/shared_array.hpp"

#include "vsnet_socket.h"
#include <fstream>
namespace VsnetDownload
{

namespace Client
{

// forward declaration
class Manager;

/** The virtual base class of all class that are notified by the client download
 *  manager when an event occurs during a download. Such an event my be
 *  a name resolution, an data packet that has arrived, the completion of
 *  the download, and many kinds of errors.
 *  The virtual base class does not maintain any of the information. It's child
 *  class have to do all the work for actually notifying someone.
 */
class Notify;

/** The virtual base class of things that are downloaded to the client.
 *  The base class implements the functions that are called by the download
 *  manager when the status of the item to download changes, such as an error
 *  that occurs, or data that arrives. The virtual base class doesn't do anything
 *  with data that arrives, but it calls functions that must be implemented by
 *  it's child classes.
 *  If a Notify pointer has been provided, the virtual base class takes care of
 *  informing the Notify object of all changes.
 */
class Item;

/** A child class of Notify. You can call the functions done() to test whether
 *  a download has been completed, ok() to test whether an error has occurred,
 *  total() to get the total number of bytes that will be downloaded, and
 *  offset() to get the total number of bytes that have been downloaded. If
 *  ok() returns false, the download has also been stopped. total() does not
 *  return a sensible value before the download has actually started.
 *
 *  If your class inherits Item and NotifyMe, you can check easily whether your
 *  item is done!
 */
class NotifyMe;

/** A child class of Notify. This one takes a global C++ function that must match
 *  the prototype Notify::NotifyFunction. Whenever something changes during
 *  download, this NotifyFunction is called with the appropriate filename and
 *  update information.
 */
class Notify_f;

/** A shared pointer with a special constructor for Notify_f. The nice thing about
 *  the class Notify_f is that cleaning it up is noncritical. So, just use the
 *  constructor of Notify_fp to pass your notification function to the Item that
 *  your want to watch, and when the item is downloaded, the Notify object vanishes
 *  all by itself after telling you that the download is complete. No hassle at all
 *  with NotifyPtrs.
 */
class Notify_fp;

/** A child class of Item that downloads a file to a given location.
 *  You must pass a Notify object as fourth constructor parameter if you want
 *  to stay informed about the ongoing download.
 */
class File;

/** A child class of Item that downloads a file into memory.
 *  This may eventually come handy. The filled buffer can be accessed through
 *  a boost::shared_array, which is a specific smart pointer class that works
 *  well for arrays of all kind.
 *  Like File, it needs a Notifier as constructor parameter, so that you can
 *  stay up to date on the download.
 */
class Buffer;

/** A child class of Item that was only used for testing the download procedure.
 *  Useless now. Adapt to your needs if you want.
 */
class TestItem;

/** A child class of both File and NotifyMe. Although the download manager is
 *  supposed to run in the background, a client may run into a situation where
 *  waiting for a download to complete is necessary. In that case, you could
 *  create a local NoteFile object, add it to the download manager using addItem,
 *  and than loop until NoteFile::done() returns true.
 * @Note Please keep in mind that the download is currently processed in
 *       NetClient::recvMsg and NetServer::processPacket, so you have to make
 *       sure those get called in your loop!
 */
class NoteFile;

/** This is a class that takes a set of filenames in its constructor. The
 *  FileSet::isDone() function returns true when the download of all the files
 *  has been finished, either successfully or unsuccessfully.
 */
class FileSet;

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify
 *------------------------------------------------------------*/

class Notify
{
public:
    virtual void notify( State s, VSError e ) = 0;
    virtual void setTotalBytes( int sz ) { }
    virtual void addBytes( int sz ) { }
};

typedef boost::shared_ptr<Notify> NotifyPtr;

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::NotifyMe
 *------------------------------------------------------------*/

class NotifyMe : public Notify
{
public:
    NotifyMe( );
    virtual ~NotifyMe( ) { }

    virtual void notify( State s, VSError e );
    virtual void setTotalBytes( int sz );
    virtual void addBytes( int sz );

    inline bool done( ) const {
        return ( _state == Completed );
    }

    inline bool ok( ) const {
        return ( _error == Ok );
    }

    inline int total( ) const {
        return _total;
    }

    inline int offset( ) const {
        return _offset;
    }

private:
    State _state;
    VSError _error;
    int   _total;
    int   _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Item
 *------------------------------------------------------------*/

class Item
{
public:
    Item( SOCKETALT sock, const std::string& filename, NotifyPtr notify = NotifyPtr() );
    virtual ~Item( );

    State state( ) const;
    VSError error( ) const;

    void changeState( State s );
    void changeState( State s, VSError e );

    void setSize( int len );
    void append( unsigned char* buffer, int bufsize );

    const std::string& getFilename( ) const;
    SOCKETALT          getSock() const;
    int                get_fd() const;

protected:
    virtual void childSetSize( int len ) = 0;
    virtual void childAppend( unsigned char* buffer, int bufsize ) = 0;

    void    protected_replace_notifier( NotifyPtr ptr );

private:
    SOCKETALT         _sock;
    const std::string _filename;

    mutable VSMutex _mx;
    State           _state;
    VSError           _error;
    NotifyPtr       _notify;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::File
 *------------------------------------------------------------*/

class File : public Item
{
public:
    File( SOCKETALT          sock,
          const std::string& filename,
          std::string        localbasepath,
          NotifyPtr          notify = NotifyPtr() );

    virtual ~File( );

protected:
    virtual void childSetSize( int len );
    virtual void childAppend( unsigned char* buffer, int bufsize );

private:
    std::string    _localbasepath;
    std::ofstream* _of;
    int            _len;
    int            _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::NoteFile
 *------------------------------------------------------------*/

class NoteFile : public File
{
private:
    NotifyPtr _me;

    inline const NotifyMe* me() const {
    	return (NotifyMe*)_me.get();
    }

public:
    NoteFile( SOCKETALT          sock,
              const std::string& filename,
              std::string        localbasepath );
    NoteFile( SOCKETALT          sock,
              const std::string& filename );

    virtual ~NoteFile( );

    inline bool done( ) const {
        return me()->done();
    }

    inline bool ok( ) const {
        return me()->ok();
    }

    inline int total( ) const {
        return me()->total();
    }

    inline int offset( ) const {
        return me()->offset();
    }
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Buffer
 *------------------------------------------------------------*/

class Buffer : public Item, public NotifyMe
{
    typedef unsigned char uchar;

public:
    Buffer( SOCKETALT          sock,
            const std::string& filename,
            NotifyPtr          notify = NotifyPtr() );

    virtual ~Buffer( );

    boost::shared_array<uchar> getBuffer( ) const;

protected:
    virtual void childSetSize( int len );
    virtual void childAppend( unsigned char* buffer, int bufsize );

private:
    boost::shared_array<uchar> _buf;
    int                        _len;
    int                        _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::TestItem
 *------------------------------------------------------------*/

class TestItem : public Item, public NotifyMe
{
public:
    TestItem( SOCKETALT sock,
              const std::string& filename );

    virtual ~TestItem( ) { }

protected:
    virtual void childSetSize( int len ) { }
    virtual void childAppend( unsigned char* buffer, int bufsize ) { }
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::FileSet
 *------------------------------------------------------------*/

/** We can't really support blocking until a set of files has been
 *  downloaded inside this class. The problem is that the variables
 *  that we must wait for exist in netserver, netclient and
 *  accountserver, and they are called in several places and potentially
 *  several threads.
 *  Therefore the waiting itself should be handled in those classes.
 *  This helper class allows a simple check.
 *
 *  Create a FileSet with the appropriate Manager, the socket to the
 *  remote side that you want to use, the list of filenames as string,
 *  and the existing(!) directory where you want to store the files.
 *  Then call isDone() a couple of times. If it returns true, the downloads
 *  are all completed (but they may have failed - if you need to check that,
 *  add a function for reading the int values from the member variable
 *  _files, 1 success, 0 failure).
 */
class FileSet
{
public:
    FileSet( boost::shared_ptr<Manager> mgr,
             SOCKETALT                  sock,
             std::list<std::string>     filesnames,
             std::string                localbasepath );

    bool isDone( ) const;
    void update( std::string s, bool v );

private:
    std::map<std::string,int> _files;
    int                       _to_go;

private:
    class NotifyConclusion;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify_f
 *------------------------------------------------------------*/

class Notify_f : public Notify
{
public:
    typedef void (*NotifyFunction)( std::string str, State s, VSError e, int total, int offset );

public:
    Notify_f( std::string filename, NotifyFunction fun );
    virtual ~Notify_f();

    virtual void notify( State s, VSError e );
    virtual void setTotalBytes( int sz );
    virtual void addBytes( int sz );

private:
    std::string    _filename;
    NotifyFunction _fun;
    int            _total;
    int            _offset;
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::Notify_fp
 *------------------------------------------------------------*/

struct Notify_fp : public NotifyPtr
{
    Notify_fp( std::string filename, Notify_f::NotifyFunction fun )
        : NotifyPtr( new Notify_f(filename,fun) )
    { }
};

/*------------------------------------------------------------*
 * declaration VsnetDownload::Client::VSNotify
 *------------------------------------------------------------*/

class VSNotify : public Notify
{
	public:
		void notify( State s, VSError e);
    	void setTotalBytes( int sz );
    	void addBytes( int sz );
};

}; // namespace Client

}; // namespace VsnetDownload

#endif /* VSNET_DLOADITEM_H */

