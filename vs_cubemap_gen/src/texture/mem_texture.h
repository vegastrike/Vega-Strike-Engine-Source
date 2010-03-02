#ifndef __MEM_TEXTURE_H__
#define __MEM_TEXTURE_H__


template < size_t SIZE >
class mem_texture
{
    float front [SIZE*SIZE*3];
    float back  [SIZE*SIZE*3];
    float left  [SIZE*SIZE*3];
    float right [SIZE*SIZE*3];
    float up    [SIZE*SIZE*3];
    float down  [SIZE*SIZE*3];
public:
    size_t size() const { return SIZE; }
    





#endif


