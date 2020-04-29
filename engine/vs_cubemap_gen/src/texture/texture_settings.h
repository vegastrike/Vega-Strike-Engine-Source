#ifndef __TEXTURE_SETTINGS_H__
#define __TEXTURE_SETTINGS_H__


class texture_settings
{
    //input params:
    mem_texture & source_tex_;
    float source_gamma_;
    float ideal_ambient_light_;
    float correction_power_;
    //computed params:
    float red_scaling;
    float grn_scaling;
    float blu_scaling;
public:
    texture_settings(
                   , mem_texture & source_tex
                   , float  source_gamma        //-e.g. 0.50
                   , float  ideal_ambient_light //-e.g. 0.25
                   , float  correction_power    //0.0 to 1.0
                    )
        : source_tex_(source_tex)
        , source_gamma_(source_gamma)
        , ideal_ambient_light_(ideal_ambient_light)
        , correction_power_(correction_power)
    {
    }
    void init();
};




#endif


