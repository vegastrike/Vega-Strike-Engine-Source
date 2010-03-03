#ifndef __PROCESS_H__
#define __PROCESS_H__
/*
 * The purpose of this is to control the general processing: reading the input
 * textures into floating point memory textures, allocating memory textures for
 * the output and all its mipmaps, calling filter for each mipmap, writing png's,
 * calling nvcompress, and reassembling all the many dds into a single dds.
 * But it's also called "process" because we might want to implement multithreading
 * or multiple processes here, to run on separate cores, in multi-core machines.
*/

class process
{
public
};



#endif


