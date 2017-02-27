#ifndef __FILE_IO_H__
#define __FILE_IO_H__


mem_texture& load_texture_png( std::string filename );
mem_texture& load_texture_set_png
(
      std::string fn_front
    , std::string fn_back
    , std::string fn_left
    , std::string fn_right
    , std::string fn_up
    , std::string fn_down
);

void save_texture_png( ... );
void save_texture_set_png( ... );
void nvcompress_png( ... );
void nvcompress_set_png( ... );

void pack_dds_mip( ... );
void pack_dds_all( ... );


#endif


