/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeaders/Converter.h"
#include "mesh_io.h"
#include "from_BFXM.h"

string inverseblend[16] = {
        "ZERO", "ZERO", "ONE", "SRCCOLOR", "INVSRCCOLOR", "SRCALPHA", "INVSRCALPHA",
        "DESTALPHA", "INVDESTALPHA", "DESTCOLOR", "INVDESTCOLOR", "SRCALPHASAT", "CONSTALPHA", "INVCONSTALPHA",
        "CONSTCOLOR", "INVCONSTCOLOR"
};

void BFXMToXmeshOrOBJ(FILE *Inputfile,
        FILE *Outputfile,
        FILE *OutputObj,
        FILE *mtl,
        std::string meshname,
        char hackfix) {
    float transx = (float) atof(Converter::getNamedOption("addx").c_str());
    float transy = (float) atof(Converter::getNamedOption("addy").c_str());
    float transz = (float) atof(Converter::getNamedOption("addz").c_str());
    bool flips = atoi(Converter::getNamedOption("flips").c_str()) != 0;
    bool flipt = atoi(Converter::getNamedOption("flipt").c_str()) != 0;
    bool flipn = atoi(Converter::getNamedOption("flipn").c_str()) != 0;

    uint32bit intbuf;
    uchar8bit bytebuf;
    uint32bit word32index = 0;
    union chunk32 {
        uint32bit i32val;
        float32bit f32val;
        uchar8bit c8val[4];
    }
            *inmemfile;
    bool isxmesh = true;
    if (hackfix == 'o') {
        isxmesh = false;
    } else if (hackfix == 'x') {
        isxmesh = true;
    } else {
        fprintf(
                stderr,
                "Something is horribly wrong. Procedure is being called, but output type is neither OBJ nor XMESH. Please contact the developers\n");
        exit(-1);
    }
    //CHECK MAGIC WORD
    fseek(Inputfile, 0, SEEK_SET);
    if (fread(&bytebuf, sizeof(uchar8bit), 1, Inputfile) != 1) {
        exit(-1);
    }
    if (bytebuf != 'B') {
        fprintf(stderr, "INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected B got %c", bytebuf);
        exit(-1);
    }
    fseek(Inputfile, 1, SEEK_SET);
    if (fread(&bytebuf, sizeof(uchar8bit), 1, Inputfile) != 1) {
        exit(-1);
    }
    if (bytebuf != 'F') {
        fprintf(stderr, "INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected F got %c", bytebuf);
        exit(-1);
    }
    fseek(Inputfile, 2, SEEK_SET);
    if (fread(&bytebuf, sizeof(uchar8bit), 1, Inputfile) != 1) {
        exit(-1);
    }
    if (bytebuf != 'X') {
        fprintf(stderr, "INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected X got %c", bytebuf);
        exit(-1);
    }
    fseek(Inputfile, 3, SEEK_SET);
    if (fread(&bytebuf, sizeof(uchar8bit), 1, Inputfile) != 1) {
        exit(-1);
    }
    if (bytebuf != 'M') {
        fprintf(stderr, "INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected M got %c", bytebuf);
        exit(-1);
    }
    fseek(Inputfile, 4 + sizeof(uint32bit), SEEK_SET);
    if (fread(&intbuf, sizeof(uint32bit), 1, Inputfile) != 1) {
        exit(-1);
    }      //Length of Inputfile
    uint32bit Inputlength = VSSwapHostIntToLittle(intbuf);
    inmemfile = (chunk32 *) malloc(Inputlength);
    if (!inmemfile) {
        fprintf(stderr, "Buffer allocation failed, Aborting");
        exit(-1);
    }
    rewind(Inputfile);
    if (fread(inmemfile, 1, Inputlength, Inputfile) != Inputlength) {
        exit(-1);
    }
    fclose(Inputfile);
    //Extract superheader fields
    word32index += 1;
    word32index += 2;
    uint32bit Superheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);
    uint32bit NUMFIELDSPERVERTEX =
            VSSwapHostIntToLittle(inmemfile[word32index + 1].i32val);     //Number of fields per vertex:integer (8)
    uint32bit NUMFIELDSPERPOLYGONSTRUCTURE = VSSwapHostIntToLittle(inmemfile[word32index
            + 2].i32val);     //Number of fields per polygon structure: integer (1)
    uint32bit NUMFIELDSPERREFERENCEDVERTEX = VSSwapHostIntToLittle(inmemfile[word32index
            + 3].i32val);     //Number of fields per referenced vertex: integer (3)
    uint32bit NUMFIELDSPERREFERENCEDANIMATION = VSSwapHostIntToLittle(inmemfile[word32index
            + 4].i32val);     //Number of fields per referenced animation: integer (1)
    uint32bit numrecords = VSSwapHostIntToLittle(inmemfile[word32index + 5].i32val);     //Number of records: integer
    uint32bit NUMFIELDSPERANIMATIONDEF = VSSwapHostIntToLittle(inmemfile[word32index
            + 6].i32val);     //Number of fields per animationdef: integer (1)
    word32index = (Superheaderlength / 4);       //Go to first record
    //For each record
    if (!isxmesh) {
        fprintf(OutputObj, "mtllib %s.mtl\n", meshname.c_str());
    }
    int vtxcount = 1;
    int texcount = 1;
    int normcount = 1;
    int indoffset = 1;
    int texoffset = 1;
    int normoffset = 1;
    for (uint32bit recordindex = 0; recordindex < numrecords; recordindex++) {
        uint32bit recordbeginword = word32index;
        //Extract Record Header
        uint32bit recordheaderlength =
                VSSwapHostIntToLittle(inmemfile[word32index].i32val);         //length of record header in bytes
        word32index += 1;
        uint32bit
                recordlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);         //length of record in bytes
        word32index += 1;
        uint32bit nummeshes =
                VSSwapHostIntToLittle(inmemfile[word32index].i32val);         //Number of meshes in the current record
        word32index = recordbeginword + (recordheaderlength / 4);
        //For each mesh
        for (uint32bit meshindex = 0; meshindex < nummeshes; meshindex++) {
            indoffset = vtxcount;
            texoffset = texcount;
            normoffset = normcount;
            if (recordindex > 0 || meshindex > 0) {
                char filenamebuf[56];             //Is more than enough characters - int can't be this big in decimal
                if (sprintf(filenamebuf, "%d_%d.xmesh", recordindex, meshindex) < 0) {
                    exit(-1);
                }
                string filename = string(filenamebuf);
                if (isxmesh) {
                    Outputfile = fopen(filename.c_str(), "w+");
                }
            }
            //Extract Mesh Header
            uint32bit meshbeginword = word32index;
            uint32bit meshheaderlength =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);           //length of record header in bytes
            word32index += 1;
            uint32bit meshlength =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);           //length of record in bytes
            float32bit scale = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 2].f32val);             //scale
            uint32bit reverse = VSSwapHostIntToLittle(inmemfile[meshbeginword + 3].i32val);           //reverse flag
            uint32bit forcetexture =
                    VSSwapHostIntToLittle(inmemfile[meshbeginword + 4].i32val);           //force texture flag
            uint32bit sharevert =
                    VSSwapHostIntToLittle(inmemfile[meshbeginword + 5].i32val);           //share vertex flag
            float32bit polygonoffset =
                    VSSwapHostFloatToLittle(inmemfile[meshbeginword + 6].f32val);             //polygonoffset
            uint32bit bsrc = VSSwapHostIntToLittle(inmemfile[meshbeginword + 7].i32val);           //Blendmode source
            uint32bit
                    bdst = VSSwapHostIntToLittle(inmemfile[meshbeginword + 8].i32val);           //Blendmode destination
            float32bit
                    power = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 9].f32val);             //Specular: power
            float32bit ar = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 10].f32val);             //Ambient: red
            float32bit ag = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 11].f32val);             //Ambient: green
            float32bit ab = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 12].f32val);             //Ambient: blue
            float32bit aa = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 13].f32val);             //Ambient: Alpha
            float32bit dr = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 14].f32val);             //Diffuse: red
            float32bit dg = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 15].f32val);             //Diffuse: green
            float32bit db = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 16].f32val);             //Diffuse: blue
            float32bit da = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 17].f32val);             //Diffuse: Alpha
            float32bit er = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 18].f32val);             //Emmissive: red
            float32bit
                    eg = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 19].f32val);             //Emmissive: green
            float32bit eb = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 20].f32val);             //Emmissive: blue
            float32bit
                    ea = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 21].f32val);             //Emmissive: Alpha
            float32bit sr = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 22].f32val);             //Specular: red
            float32bit sg = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 23].f32val);             //Specular: green
            float32bit sb = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 24].f32val);             //Specular: blue
            float32bit sa = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 25].f32val);             //Specular: Alpha
            uint32bit cullface =
                    (VSSwapHostIntToLittle(inmemfile[meshbeginword + 26].i32val) != 0) ? 1 : 0;             //CullFace
            uint32bit lighting =
                    (VSSwapHostIntToLittle(inmemfile[meshbeginword + 27].i32val) != 0) ? 1 : 0;             //lighting
            uint32bit reflect =
                    (VSSwapHostIntToLittle(inmemfile[meshbeginword + 28].i32val) != 0) ? 1 : 0;             //reflect
            uint32bit usenormals =
                    (VSSwapHostIntToLittle(inmemfile[meshbeginword + 29].i32val) != 0) ? 1 : 0;             //usenormals
            float32bit alphatest = 0;
            if (meshheaderlength > 30 * 4) {
                alphatest = VSSwapHostFloatToLittle(inmemfile[meshbeginword
                        + 30].f32val);
            }                  //Alpha Testing Values

            //End Header
            //Go to Arbitrary Length Attributes section
            word32index = meshbeginword + (meshheaderlength / 4);
            uint32bit VSAbeginword = word32index;
            uint32bit LengthOfArbitraryLengthAttributes =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //Length of Arbitrary length attributes section in bytes
            word32index += 1;
            if (isxmesh) {
                fprintf(
                        Outputfile,
                        "<Mesh scale=\"%f\" reverse=\"%d\" forcetexture=\"%d\" sharevert=\"%d\" polygonoffset=\"%f\" blend=\"%s %s\" alphatest=\"%f\" ",
                        scale,
                        reverse,
                        forcetexture,
                        sharevert,
                        polygonoffset,
                        inverseblend[bsrc % 16].c_str(),
                        inverseblend[bdst % 16].c_str(),
                        alphatest);
            } else {
                fprintf(mtl, "newmtl tex%d_%d\n", recordindex, meshindex);
                fprintf(OutputObj, "usemtl tex%d_%d\n", recordindex, meshindex);
            }
            string detailtexturename = "";
            uint32bit detailtexturenamelen =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //detailtexture name length
            word32index += 1;
            uint32bit stringindex = 0;
            uint32bit namebound = (detailtexturenamelen + 3) / 4;
            for (stringindex = 0; stringindex < namebound; stringindex++) {
                for (uint32bit bytenum = 0; bytenum < 4; bytenum++) {                  //Extract chars
                    if (inmemfile[word32index].c8val[bytenum]) {                      //If not padding
                        detailtexturename += inmemfile[word32index].c8val[bytenum];
                    }
                }
                //Append char to end of string
                word32index += 1;
            }
            if (detailtexturename.size() != 0) {
                if (isxmesh) {
                    fprintf(Outputfile, " detailtexture=\"%s\" ", detailtexturename.c_str());
                } else {
                    fprintf(mtl, "map_detail %s\n", detailtexturename.c_str());
                }
            }
            vector<Mesh_vec3f> Detailplanes;             //store detail planes until finish printing mesh attributes
            uint32bit numdetailplanes =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of detailplanes
            word32index += 1;
            for (uint32bit detailplane = 0; detailplane < numdetailplanes; detailplane++) {
                float32bit x = VSSwapHostFloatToLittle(inmemfile[word32index].f32val);                 //x-coord
                float32bit y = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //y-coord
                float32bit z = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //z-coord
                word32index += 3;
                Mesh_vec3f temp;
                temp.x = x;
                temp.y = y;
                temp.z = z;
                Detailplanes.push_back(temp);
                if (!isxmesh) {
                    fprintf(mtl, "detail_plane %f %f %f\n", x, y, z);
                }
            }             //End detail planes
            //Textures
            uint32bit
                    numtextures = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of textures
            word32index += 1;
            bool emit = false;
            for (uint32bit tex = 0; tex < numtextures; tex++) {
                uint32bit textype = VSSwapHostIntToLittle(inmemfile[word32index].i32val);                 //texture type
                uint32bit texindex =
                        VSSwapHostIntToLittle(inmemfile[word32index + 1].i32val);                 //texture index
                uint32bit texnamelen =
                        VSSwapHostIntToLittle(inmemfile[word32index + 2].i32val);                 //texture name length
                word32index += 3;
                string texname = "";
                uint32bit namebound = (texnamelen + 3) / 4;
                for (stringindex = 0; stringindex < namebound; stringindex++) {
                    for (uint32bit bytenum = 0; bytenum < 4; bytenum++) {                      //Extract chars
                        if (inmemfile[word32index].c8val[bytenum]) {                          //If not padding
                            texname += inmemfile[word32index].c8val[bytenum];
                        }
                    }
                    //Append char to end of string
                    word32index += 1;
                }
                switch (textype) {
                    case ALPHAMAP:
                        if (isxmesh) {
                            fprintf(Outputfile, " alphamap");
                        }
                        break;
                    case ANIMATION:
                        if (isxmesh) {
                            fprintf(Outputfile, " animation");
                        }
                        break;
                    case TEXTURE:
                        if (isxmesh) {
                            fprintf(Outputfile, " texture");
                        }
                        break;
                    case TECHNIQUE:
                        if (isxmesh) {
                            fprintf(Outputfile, " technique");
                        }
                        break;
                    default:
                        fprintf(stderr,
                                "Bad case in switch(textype) in file from_BFXM.cpp: type=%d index=%d\n",
                                textype,
                                texindex);
                        if (isxmesh) {
                            fprintf(Outputfile, " unknown_textype");
                        }
                        break;
                }
                if (isxmesh && texindex) {
                    fprintf(Outputfile, "%d", texindex);
                } //adds 2 or 3 or 4 or 5 to "texture"
                if (isxmesh)
                    fprintf(Outputfile,
                            "=\"%s\" ",
                            texname.c_str()); //does a technique name get here via "texname"??? --chuck
                if (!isxmesh) {
                    string textyp;
                    switch (texindex) {
                        case 4:
                            textyp = "map_Normal";
                            break;
                        case 3:
                            textyp = "map_Ke";
                            break;
                        case 2:
                            textyp = "map_Ka";
                            emit = true;
                            break;
                        case 1:
                            textyp = "map_Ks";
                            break;
                        case 0:
                            if (textype == ALPHAMAP)
                                textyp = "map_opacity";
                            else if (textype == TECHNIQUE)
                                textyp = "technique";
                            else
                                textyp = "map_Kd";
                            break;
                        default:
                            textyp = "map_unknown";
                    }

                    if (textyp.length())
                        fprintf(mtl, "%s %s\n", textyp.c_str(), texname.c_str());
                    else
                        fprintf(mtl, "%d %s\n", texindex, texname.c_str());
                }
            }
            if (isxmesh) {
                fprintf(Outputfile, ">\n");
            }
            //End Textures
            if (!isxmesh) {
                fprintf(mtl, "Ns %f\n", power);
                fprintf(mtl, "Ka %f %f %f\n", emit ? 1.0f : ar, emit ? 1.0f : ag, emit ? 1.0f : ab);
                fprintf(mtl, "Kd %f %f %f\n", dr, dg, db);
                fprintf(mtl, "Ke %f %f %f\n", er, eg, eb);
                fprintf(mtl, "Ks %f %f %f\n", sr, sg, sb);
                if (bsrc == ONE && bdst == ONE) {
                    fprintf(mtl, "Blend 1.0\n");
                } else if (bsrc == SRCALPHA && bdst == INVSRCALPHA) {
                    fprintf(mtl, "Blend 0.5\n");
                }
                if (!usenormals) {
                    fprintf(mtl, "Normals 0\n");
                }
                if (reflect) {
                    fprintf(mtl, "Map_Reflection 1\n");
                }
            } else {
                fprintf(Outputfile,
                        "<Material power=\"%f\" cullface=\"%d\" reflect=\"%d\" lighting=\"%d\" usenormals=\"%d\">\n",
                        power,
                        cullface,
                        reflect,
                        lighting,
                        usenormals);
                fprintf(Outputfile, "\t<Ambient Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n", ar, ag, ab, aa);
                fprintf(Outputfile, "\t<Diffuse Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n", dr, dg, db, da);
                fprintf(Outputfile, "\t<Emissive Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n", er, eg, eb, ea);
                fprintf(Outputfile, "\t<Specular Red=\"%f\" Green=\"%f\" Blue=\"%f\" Alpha=\"%f\"/>\n", sr, sg, sb, sa);
                fprintf(Outputfile, "</Material>\n");
            }
            for (std::vector<Mesh_vec3f>::size_type detplane = 0; detplane < Detailplanes.size(); detplane++) {
                if (isxmesh) {
                    fprintf(Outputfile,
                            "<DetailPlane x=\"%f\" y=\"%f\" z=\"%f\" />\n",
                            Detailplanes[detplane].x,
                            Detailplanes[detplane].y,
                            Detailplanes[detplane].z);
                }
            }
            //Logos
            uint32bit numlogos = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of logos
            word32index += 1;
            for (uint32bit logo = 0; logo < numlogos; logo++) {
                float32bit size = VSSwapHostFloatToLittle(inmemfile[word32index].f32val);                 //size
                float32bit offset = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //offset
                float32bit rotation =
                        VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //rotation
                uint32bit type = VSSwapHostIntToLittle(inmemfile[word32index + 3].i32val);               //type
                uint32bit numrefs = VSSwapHostIntToLittle(inmemfile[word32index
                        + 4].i32val);               //number of reference points
                if (isxmesh) {
                    fprintf(Outputfile,
                            "<Logo type=\"%d\" rotate=\"%f\" size=\"%f\" offset=\"%f\">\n",
                            type,
                            rotation,
                            size,
                            offset);
                }
                word32index += 5;
                for (uint32bit ref = 0; ref < numrefs; ref++) {
                    uint32bit
                            refnum = VSSwapHostIntToLittle(inmemfile[word32index].i32val);                   //Logo ref
                    float32bit weight = VSSwapHostFloatToLittle(inmemfile[word32index
                            + 1].f32val);                     //reference weight
                    if (isxmesh) {
                        fprintf(Outputfile, "\t<Ref point=\"%d\" weight=\"%f\"/>\n", refnum, weight);
                    }
                    word32index += 2;
                }
                if (isxmesh) {
                    fprintf(Outputfile, "</Logo>\n");
                }
            }
            //End logos
            //LODs
            uint32bit numLODs = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of LODs
            word32index += 1;
            for (uint32bit LOD = 0; LOD < numLODs; LOD++) {
                float32bit size = VSSwapHostFloatToLittle(inmemfile[word32index].f32val);                 //Size
                uint32bit index = VSSwapHostIntToLittle(inmemfile[word32index + 1].i32val);               //Mesh index
                if (isxmesh) {
                    fprintf(Outputfile, "<LOD size=\"%f\" meshfile=\"%d_%d.xmesh\"/>\n", size, recordindex, index);
                }
                word32index += 2;
            }
            //End LODs
            //AnimationDefinitions
            uint32bit numanimdefs =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of animation definitions
            word32index += 1;
            if (meshindex == 0) {
                for (unsigned int framecount = numLODs + 1; framecount < nummeshes; framecount++) {
                    if (isxmesh) {
                        fprintf(Outputfile, "<Frame FrameMeshName=\"%d_%d.xmesh\"/>\n", recordindex, framecount);
                    }
                }
            }
            for (uint32bit anim = 0; anim < numanimdefs; anim++) {
                uint32bit animnamelen =
                        VSSwapHostIntToLittle(inmemfile[word32index].i32val);                 //length of name
                word32index += 1;
                string animname = "";
                uint32bit namebound = (animnamelen + 3) / 4;
                for (stringindex = 0; stringindex < namebound; stringindex++) {
                    for (uint32bit bytenum = 0; bytenum < 4; bytenum++) {                      //Extract chars
                        if (inmemfile[word32index].c8val[bytenum]) {                          //If not padding
                            animname += inmemfile[word32index].c8val[bytenum];
                        }
                    }
                    //Append char to end of string
                    word32index += 1;
                }
                float32bit FPS = VSSwapHostFloatToLittle(inmemfile[word32index].f32val);                 //FPS
                if (isxmesh) {
                    fprintf(Outputfile,
                            "<AnimationDefinition AnimationName=\"%s\" FPS=\"%f\">\n",
                            animname.c_str(),
                            FPS);
                }
                word32index += NUMFIELDSPERANIMATIONDEF;
                uint32bit numframerefs =
                        VSSwapHostIntToLittle(inmemfile[word32index].i32val);                 //number of animation frame references
                word32index += 1;
                for (uint32bit fref = 0; fref < numframerefs; fref++) {
                    uint32bit ref =
                            VSSwapHostIntToLittle(inmemfile[word32index].i32val);                     //number of animation frame references
                    word32index += NUMFIELDSPERREFERENCEDANIMATION;
                    if (isxmesh) {
                        fprintf(Outputfile, "<AnimationFrameIndex AnimationMeshIndex=\"%d\"/>\n", ref - 1 - numLODs);
                    }
                }
                if (isxmesh) {
                    fprintf(Outputfile, "</AnimationDefinition>\n");
                }
            }
            //End AnimationDefinitions
            //End VSA
            //go to geometry
            word32index = VSAbeginword + (LengthOfArbitraryLengthAttributes / 4);
            //Vertices
            if (isxmesh) {
                fprintf(Outputfile, "<Points>\n");
            }
            uint32bit
                    numvertices = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit vert = 0; vert < numvertices; vert++) {
                float32bit x = VSSwapHostFloatToLittle(inmemfile[word32index].f32val);                 //x
                float32bit y = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //y
                float32bit z = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //z
                float32bit i = VSSwapHostFloatToLittle(inmemfile[word32index + 3].f32val);                 //i
                float32bit j = VSSwapHostFloatToLittle(inmemfile[word32index + 4].f32val);                 //j
                float32bit k = VSSwapHostFloatToLittle(inmemfile[word32index + 5].f32val);                 //k
                float32bit s = VSSwapHostFloatToLittle(inmemfile[word32index + 6].f32val);                 //s
                float32bit t = VSSwapHostFloatToLittle(inmemfile[word32index + 7].f32val);                 //t
                word32index += NUMFIELDSPERVERTEX;
                if (isxmesh) {
                    fprintf(
                            Outputfile,
                            "<Point>\n\t<Location x=\"%f\" y=\"%f\" z=\"%f\" s=\"%f\" t=\"%f\"/>\n\t<Normal i=\"%f\" j=\"%f\" k=\"%f\"/>\n</Point>\n",
                            x + transx,
                            y + transy,
                            z + transz,
                            s,
                            t,
                            i,
                            j,
                            k);
                } else {
                    fprintf(OutputObj,
                            "v %f %f %f\nvn %f %f %f\n",
                            scale
                                    * (x
                                            + transx),
                            scale
                                    * (y + transy),
                            scale * (z + transz),
                            i * (flipn ? -1 : 1),
                            j * (flipn ? -1 : 1),
                            k * (flipn ? -1 : 1));
                }
                if (sharevert) {
                    if (!isxmesh) {
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s : s), (flipt ? t : 1.0f - t));
                    }
                    texcount += 1;
                }
                vtxcount += 1;
                normcount += 1;
            }
            if (isxmesh) {
                fprintf(Outputfile, "</Points>\n");
            }
            //End Vertices
            //Lines
            if (isxmesh) {
                fprintf(Outputfile, "<Polygons>\n");
            }
            uint32bit numlines = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit rvert = 0; rvert < numlines; rvert++) {
                uint32bit flatshade = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //flatshade
                word32index += NUMFIELDSPERPOLYGONSTRUCTURE;
                uint32bit ind1 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 1
                float32bit s1 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t1 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                uint32bit ind2 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 2
                float32bit s2 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t2 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                if (isxmesh) {
                    fprintf(
                            Outputfile,
                            "\t<Line flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Line>\n",
                            flatshade,
                            ind1,
                            s1,
                            t1,
                            ind2,
                            s2,
                            t2);
                }
                int texind1 = ind1 + texoffset;
                int texind2 = ind2 + texoffset;
                if (!sharevert) {
                    if (!isxmesh) {
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s1 : s1), (flipt ? t1 : 1.0f - t1));
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s2 : s2), (flipt ? t2 : 1.0f - t2));
                    }
                    texcount += 2;
                    texind1 = texcount - 2;
                    texind2 = texcount - 1;
                }
                if (!isxmesh) {
                    fprintf(OutputObj, "f %d/%d/%d %d/%d/%d\n",

                            ind1 + indoffset, texind1, ind1 + normoffset,
                            ind2 + indoffset, texind2, ind2 + normoffset);
                }
            }
            //End Lines
            //Triangles
            uint32bit numtris = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit rtvert = 0; rtvert < numtris; rtvert++) {
                uint32bit flatshade = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //flatshade
                word32index += NUMFIELDSPERPOLYGONSTRUCTURE;
                uint32bit ind1 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 1
                float32bit s1 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t1 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                uint32bit ind2 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 2
                float32bit s2 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t2 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                uint32bit ind3 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 3
                float32bit s3 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t3 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                if (isxmesh) {
                    fprintf(
                            Outputfile,
                            "\t<Tri flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Tri>\n",
                            flatshade,
                            ind1,
                            s1,
                            t1,
                            ind2,
                            s2,
                            t2,
                            ind3,
                            s3,
                            t3);
                }
                int texind1 = ind1 + texoffset;
                int texind2 = ind2 + texoffset;
                int texind3 = ind3 + texoffset;
                if (!sharevert) {
                    if (!isxmesh) {
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s1 : s1), (flipt ? t1 : 1.0f - t1));
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s2 : s2), (flipt ? t2 : 1.0f - t2));
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s3 : s3), (flipt ? t3 : 1.0f - t3));
                    }
                    texcount += 3;
                    texind1 = texcount - 3;
                    texind2 = texcount - 2;
                    texind3 = texcount - 1;
                }
                if (!isxmesh) {
                    fprintf(OutputObj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                            ind1 + indoffset, texind1, ind1 + normoffset,
                            ind2 + indoffset, texind2, ind2 + normoffset,
                            ind3 + indoffset, texind3, ind3 + normoffset);
                }
            }
            //End Triangles
            //Quads
            uint32bit numquads = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit rqvert = 0; rqvert < numquads; rqvert++) {
                uint32bit flatshade = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //flatshade
                word32index += NUMFIELDSPERPOLYGONSTRUCTURE;
                uint32bit ind1 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 1
                float32bit s1 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t1 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                uint32bit ind2 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 2
                float32bit s2 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t2 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                uint32bit ind3 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 3
                float32bit s3 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t3 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                uint32bit ind4 = VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //index 3
                float32bit s4 = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //s
                float32bit t4 = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //t
                word32index += NUMFIELDSPERREFERENCEDVERTEX;
                if (isxmesh) {
                    fprintf(
                            Outputfile,
                            "\t<Quad flatshade=\"%d\">\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n\t</Quad>\n",
                            flatshade,
                            ind1,
                            s1,
                            t1,
                            ind2,
                            s2,
                            t2,
                            ind3,
                            s3,
                            t3,
                            ind4,
                            s4,
                            t4);
                }
                int texind1 = ind1 + texoffset;
                int texind2 = ind2 + texoffset;
                int texind3 = ind3 + texoffset;
                int texind4 = ind4 + texoffset;
                if (!sharevert) {
                    if (!isxmesh) {
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s1 : s1), (flipt ? t1 : 1.0f - t1));
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s2 : s2), (flipt ? t2 : 1.0f - t2));
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s3 : s3), (flipt ? t3 : 1.0f - t3));
                        fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s4 : s4), (flipt ? t4 : 1.0f - t4));
                    }
                    texcount += 4;
                    texind1 = texcount - 4;
                    texind2 = texcount - 3;
                    texind3 = texcount - 2;
                    texind4 = texcount - 1;
                }
                if (!isxmesh) {
                    fprintf(OutputObj, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
                            ind1 + indoffset, texind1, ind1 + normoffset,
                            ind2 + indoffset, texind2, ind2 + normoffset,
                            ind3 + indoffset, texind3, ind3 + normoffset,
                            ind4 + indoffset, texind4, ind4 + normoffset);
                }
            }
            //End Quads
            //Linestrips
            uint32bit numlinestrips =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit lstrip = 0; lstrip < numlinestrips; lstrip++) {
                uint32bit numstripelements =
                        VSSwapHostIntToLittle(inmemfile[word32index].i32val);                 //number of vertices
                uint32bit flatshade =
                        VSSwapHostIntToLittle(inmemfile[word32index + 1].i32val);                 //flatshade
                if (isxmesh) {
                    fprintf(Outputfile, "\t<Linestrip flatshade=\"%d\">\n", flatshade);
                }
                word32index += 1 + NUMFIELDSPERPOLYGONSTRUCTURE;
                for (uint32bit elem = 0; elem < numstripelements; elem++) {
                    uint32bit ind = VSSwapHostIntToLittle(inmemfile[word32index].i32val);                   //index 1
                    float32bit s = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                     //s
                    float32bit t = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                     //t
                    word32index += NUMFIELDSPERREFERENCEDVERTEX;
                    if (isxmesh) {
                        fprintf(Outputfile, "\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n", ind, s, t);
                    }
                }
                if (isxmesh) {
                    fprintf(Outputfile, "\t</Linestrip>");
                }
            }
            //End Linestrips
            //Tristrips
            uint32bit numtristrips =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit tstrip = 0; tstrip < numtristrips; tstrip++) {
                uint32bit numstripelements =
                        VSSwapHostIntToLittle(inmemfile[word32index].i32val);                 //number of vertices
                uint32bit flatshade =
                        VSSwapHostIntToLittle(inmemfile[word32index + 1].i32val);                 //flatshade
                if (isxmesh) {
                    fprintf(Outputfile, "\t<Tristrip flatshade=\"%d\">\n", flatshade);
                }
                word32index += 1 + NUMFIELDSPERPOLYGONSTRUCTURE;
                int to1 = 0, to2 = 0;
                int indo1 = 0, indo2 = 0;
                for (uint32bit elem = 0; elem < numstripelements; elem++) {
                    uint32bit ind = VSSwapHostIntToLittle(inmemfile[word32index].i32val);                   //index 1
                    float32bit s = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                     //s
                    float32bit t = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                     //t
                    word32index += NUMFIELDSPERREFERENCEDVERTEX;
                    if (isxmesh) {
                        fprintf(Outputfile, "\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n", ind, s, t);
                    }
                    int tco = ind + texoffset;
                    int vind = ind + indoffset;
                    int nind = ind + normoffset;
                    if (!sharevert) {
                        if (!isxmesh) {
                            fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s : s), (flipt ? t : 1.0f - t));
                        }
                        tco = texcount;
                        texcount += 1;
                    }
                    if (elem > 1) {
                        if (elem % 2) {
                            if (!isxmesh) {
                                fprintf(OutputObj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                        indo2, to2, indo2, indo1, to1, indo1, vind, tco, nind);
                            }
                        } else if (!isxmesh) {
                            fprintf(OutputObj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                    indo1, to1, indo1, indo2, to2, indo2, vind, tco, nind);
                        }
                    }
                    indo2 = indo1;
                    indo1 = vind;
                    to2 = to1;
                    to1 = tco;
                }
                if (isxmesh) {
                    fprintf(Outputfile, "\t</Tristrip>");
                }
            }
            //End Tristrips
            //Trifans
            uint32bit
                    numtrifans = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit tfan = 0; tfan < numtrifans; tfan++) {
                uint32bit numstripelements =
                        VSSwapHostIntToLittle(inmemfile[word32index].i32val);                 //number of vertices
                uint32bit flatshade =
                        VSSwapHostIntToLittle(inmemfile[word32index + 1].i32val);                 //flatshade
                if (isxmesh) {
                    fprintf(Outputfile, "\t<Trifan flatshade=\"%d\">\n", flatshade);
                }
                word32index += 1 + NUMFIELDSPERPOLYGONSTRUCTURE;
                int indo1 = 0, indo2 = 0, to1 = 0, to2 = 0;
                for (uint32bit elem = 0; elem < numstripelements; elem++) {
                    uint32bit ind = VSSwapHostIntToLittle(inmemfile[word32index].i32val);                   //index 1
                    float32bit s = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                     //s
                    float32bit t = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                     //t
                    word32index += NUMFIELDSPERREFERENCEDVERTEX;
                    if (isxmesh) {
                        fprintf(Outputfile, "\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n", ind, s, t);
                    }
                    int tco = ind + texoffset;
                    int nind = ind + normoffset;
                    int vind = ind + indoffset;
                    if (!sharevert) {
                        if (!isxmesh) {
                            fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s : s), (flipt ? t : 1.0f - t));
                        }
                        tco = texcount;
                        texcount += 1;
                    }
                    if (elem > 1) {
                        if (!isxmesh) {
                            fprintf(OutputObj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                    indo2, to2, indo2, indo1, to1, indo1, vind, tco, nind);
                        }
                    }
                    if (elem == 0) {
                        indo2 = vind;
                        to2 = tco;
                    }
                    indo1 = vind;
                    to1 = tco;
                }
                if (isxmesh) {
                    fprintf(Outputfile, "\t</Trifan>");
                }
            }
            //End Trifans
            //Quadstrips
            uint32bit numquadstrips =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit qstrip = 0; qstrip < numquadstrips; qstrip++) {
                uint32bit numstripelements =
                        VSSwapHostIntToLittle(inmemfile[word32index].i32val);                 //number of vertices
                uint32bit flatshade =
                        VSSwapHostIntToLittle(inmemfile[word32index + 1].i32val);                 //flatshade
                if (isxmesh) {
                    fprintf(Outputfile, "\t<Quadstrip flatshade=\"%d\">\n", flatshade);
                }
                word32index += 1 + NUMFIELDSPERPOLYGONSTRUCTURE;
                int indo1 = 0, indo2 = 0, to1 = 0, to2 = 0;
                for (uint32bit elem = 0; elem < numstripelements; elem++) {
                    uint32bit ind = VSSwapHostIntToLittle(inmemfile[word32index].i32val);                   //index 1
                    float32bit s = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                     //s
                    float32bit t = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                     //t
                    word32index += NUMFIELDSPERREFERENCEDVERTEX;
                    if (isxmesh) {
                        fprintf(Outputfile, "\t\t<Vertex point=\"%d\" s=\"%f\" t=\"%f\"/>\n", ind, s, t);
                    }
                    int tco = ind + texoffset;
                    int nind = ind + normoffset;
                    int vind = ind + indoffset;
                    if (!sharevert) {
                        if (!isxmesh) {
                            fprintf(OutputObj, "vt %f %f\n", (flips ? 1.0f - s : s), (flipt ? t : 1.0f - t));
                        }
                        tco = texcount;
                        texcount += 1;
                    }
                    if (elem > 1) {
                        if (elem % 2) {
                            if (!isxmesh) {
                                fprintf(OutputObj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                        indo2, to2, indo2, indo1, to1, indo1, vind, tco, nind);
                            }
                        } else if (!isxmesh) {
                            fprintf(OutputObj, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
                                    indo1, to1, indo1, indo2, to2, indo2, vind, tco, nind);
                        }
                    }
                    indo2 = indo1;
                    indo1 = vind;
                    to2 = to1;
                    to1 = tco;
                }
                if (isxmesh) {
                    fprintf(Outputfile, "\t</Quadstrip>");
                }
            }
            //End Quadstrips
            if (isxmesh) {
                fprintf(Outputfile, "</Polygons>\n");
            }
            //End Geometry
            //go to next mesh
            if (isxmesh) {
                fprintf(Outputfile, "</Mesh>\n");
            }
            word32index = meshbeginword + (meshlength / 4);
        }
        //go to next record
        word32index = recordbeginword + (recordlength / 4);
    }
}

void BFXMtoBoxDims(FILE *Inputfile, const char *name) {
    uint32bit intbuf;
    uchar8bit bytebuf;
    uint32bit word32index = 0;
    union chunk32 {
        uint32bit i32val;
        float32bit f32val;
        uchar8bit c8val[4];
    }
            *inmemfile;

    float minx, miny, minz, maxx, maxy, maxz;
    minx = miny = minz = 77777777.77;
    maxx = maxy = maxz = -77777777.77;
    bool firstpoint = true;
    //CHECK MAGIC WORD
    fseek(Inputfile, 0, SEEK_SET);
    if (fread(&bytebuf, sizeof(uchar8bit), 1, Inputfile) != 1) {
        exit(-1);
    }
    if (bytebuf != 'B') {
        fprintf(stderr, "INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected B got %c", bytebuf);
        exit(-1);
    }
    fseek(Inputfile, 1, SEEK_SET);
    if (fread(&bytebuf, sizeof(uchar8bit), 1, Inputfile) != 1) {
        exit(-1);
    }
    if (bytebuf != 'F') {
        fprintf(stderr, "INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected F got %c", bytebuf);
        exit(-1);
    }
    fseek(Inputfile, 2, SEEK_SET);
    if (fread(&bytebuf, sizeof(uchar8bit), 1, Inputfile) != 1) {
        exit(-1);
    }
    if (bytebuf != 'X') {
        fprintf(stderr, "INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected X got %c", bytebuf);
        exit(-1);
    }
    fseek(Inputfile, 3, SEEK_SET);
    if (fread(&bytebuf, sizeof(uchar8bit), 1, Inputfile) != 1) {
        exit(-1);
    }
    if (bytebuf != 'M') {
        fprintf(stderr, "INVALID FILE FORMAT ENCOUNTERED - ABORTING\nExpected M got %c", bytebuf);
        exit(-1);
    }
    fseek(Inputfile, 4 + sizeof(uint32bit), SEEK_SET);
    if (fread(&intbuf, sizeof(uint32bit), 1, Inputfile) != 1) {
        exit(-1);
    }      //Length of Inputfile
    uint32bit Inputlength = VSSwapHostIntToLittle(intbuf);
    inmemfile = (chunk32 *) malloc(Inputlength);
    if (!inmemfile) {
        fprintf(stderr, "Buffer allocation failed, Aborting");
        exit(-1);
    }
    rewind(Inputfile);
    if (fread(inmemfile, 1, Inputlength, Inputfile) != Inputlength) {
        exit(-1);
    }
    fclose(Inputfile);
    //Extract superheader fields
    word32index += 1;
    word32index += 2;
    uint32bit Superheaderlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);
    uint32bit NUMFIELDSPERVERTEX =
            VSSwapHostIntToLittle(inmemfile[word32index + 1].i32val);     //Number of fields per vertex:integer (8)
    uint32bit NUMFIELDSPERREFERENCEDANIMATION = VSSwapHostIntToLittle(inmemfile[word32index
            + 4].i32val);     //Number of fields per referenced animation: integer (1)
    uint32bit numrecords = VSSwapHostIntToLittle(inmemfile[word32index + 5].i32val);     //Number of records: integer
    uint32bit NUMFIELDSPERANIMATIONDEF = VSSwapHostIntToLittle(inmemfile[word32index
            + 6].i32val);     //Number of fields per animationdef: integer (1)
    word32index = (Superheaderlength / 4);       //Go to first record
    //For each record

    for (uint32bit recordindex = 0; recordindex < numrecords; recordindex++) {
        uint32bit recordbeginword = word32index;
        //Extract Record Header
        uint32bit recordheaderlength =
                VSSwapHostIntToLittle(inmemfile[word32index].i32val);         //length of record header in bytes
        word32index += 1;
        uint32bit
                recordlength = VSSwapHostIntToLittle(inmemfile[word32index].i32val);         //length of record in bytes
        word32index += 1;
        uint32bit nummeshes =
                VSSwapHostIntToLittle(inmemfile[word32index].i32val);         //Number of meshes in the current record
        word32index = recordbeginword + (recordheaderlength / 4);
        //For each mesh
        for (uint32bit meshindex = 0; meshindex < nummeshes; meshindex++) {
            if (recordindex > 0 || meshindex > 0) {
                char filenamebuf[56];             //Is more than enough characters - int can't be this big in decimal
                if (sprintf(filenamebuf, "%d_%d.xmesh", recordindex, meshindex) < 0) {
                    exit(-1);
                }
                string filename = string(filenamebuf);
            }
            //Extract Mesh Header
            uint32bit meshbeginword = word32index;
            uint32bit meshheaderlength =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);           //length of record header in bytes
            word32index += 1;
            uint32bit meshlength =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);           //length of record in bytes
            float32bit scale = VSSwapHostFloatToLittle(inmemfile[meshbeginword + 2].f32val);             //scale
            //End Header
            //Go to Arbitrary Length Attributes section
            word32index = meshbeginword + (meshheaderlength / 4);
            uint32bit VSAbeginword = word32index;
            uint32bit LengthOfArbitraryLengthAttributes =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //Length of Arbitrary length attributes section in bytes
            word32index += 1;

            string detailtexturename = "";
            uint32bit detailtexturenamelen =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //detailtexture name length
            word32index += 1;
            uint32bit stringindex = 0;
            uint32bit namebound = (detailtexturenamelen + 3) / 4;
            for (stringindex = 0; stringindex < namebound; stringindex++) {
                for (uint32bit bytenum = 0; bytenum < 4; bytenum++) {                  //Extract chars
                    if (inmemfile[word32index].c8val[bytenum]) {                      //If not padding
                        detailtexturename += inmemfile[word32index].c8val[bytenum];
                    }
                }
                //Append char to end of string
                word32index += 1;
            }
            vector<Mesh_vec3f> Detailplanes;             //store detail planes until finish printing mesh attributes
            uint32bit numdetailplanes =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of detailplanes
            word32index += 1;
            for (uint32bit detailplane = 0; detailplane < numdetailplanes; detailplane++) {
                float32bit x = VSSwapHostFloatToLittle(inmemfile[word32index].f32val);                 //x-coord
                float32bit y = VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //y-coord
                float32bit z = VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //z-coord
                word32index += 3;
                Mesh_vec3f temp;
                temp.x = x;
                temp.y = y;
                temp.z = z;
                Detailplanes.push_back(temp);
            }             //End detail planes
            //Textures
            uint32bit
                    numtextures = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of textures
            word32index += 1;
            for (uint32bit tex = 0; tex < numtextures; tex++) {
                uint32bit texnamelen =
                        VSSwapHostIntToLittle(inmemfile[word32index + 2].i32val);                 //texture name length
                word32index += 3;
                string texname = "";
                uint32bit namebound = (texnamelen + 3) / 4;
                for (stringindex = 0; stringindex < namebound; stringindex++) {
                    for (uint32bit bytenum = 0; bytenum < 4; bytenum++) {                      //Extract chars
                        if (inmemfile[word32index].c8val[bytenum]) {                          //If not padding
                            texname += inmemfile[word32index].c8val[bytenum];
                        }
                    }
                    //Append char to end of string
                    word32index += 1;
                }
            }
            //End Textures
            //Logos
            uint32bit numlogos = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of logos
            word32index += 1;
            for (uint32bit logo = 0; logo < numlogos; logo++) {
                uint32bit numrefs = VSSwapHostIntToLittle(inmemfile[word32index
                        + 4].i32val);               //number of reference points
                word32index += 5;
                for (uint32bit ref = 0; ref < numrefs; ref++) {
                    word32index += 2;
                }
            }
            //End logos
            //LODs
            uint32bit numLODs = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of LODs
            word32index += 1;
            for (uint32bit LOD = 0; LOD < numLODs; LOD++) {
                word32index += 2;
            }
            //End LODs
            //AnimationDefinitions
            uint32bit numanimdefs =
                    VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of animation definitions
            word32index += 1;
            if (meshindex == 0) {
                for (uint32bit framecount = numLODs + 1; framecount < nummeshes; framecount++) {
                }
            }
            for (uint32bit anim = 0; anim < numanimdefs; anim++) {
                uint32bit animnamelen =
                        VSSwapHostIntToLittle(inmemfile[word32index].i32val);                 //length of name
                word32index += 1;
                string animname = "";
                uint32bit namebound = (animnamelen + 3) / 4;
                for (stringindex = 0; stringindex < namebound; stringindex++) {
                    for (uint32bit bytenum = 0; bytenum < 4; bytenum++) {                      //Extract chars
                        if (inmemfile[word32index].c8val[bytenum]) {                          //If not padding
                            animname += inmemfile[word32index].c8val[bytenum];
                        }
                    }
                    //Append char to end of string
                    word32index += 1;
                }
                word32index += NUMFIELDSPERANIMATIONDEF;
                uint32bit numframerefs =
                        VSSwapHostIntToLittle(inmemfile[word32index].i32val);               //number of animation frame references
                word32index += 1;
                for (uint32bit fref = 0; fref < numframerefs; fref++) {
                    word32index += NUMFIELDSPERREFERENCEDANIMATION;
                }
            }
            //End AnimationDefinitions
            //End VSA
            //go to geometry
            word32index = VSAbeginword + (LengthOfArbitraryLengthAttributes / 4);
            //Vertices
            uint32bit
                    numvertices = VSSwapHostIntToLittle(inmemfile[word32index].i32val);             //number of vertices
            word32index += 1;
            for (uint32bit vert = 0; vert < numvertices; vert++) {
                float32bit x = scale * VSSwapHostFloatToLittle(inmemfile[word32index].f32val);                 //x
                float32bit y = scale * VSSwapHostFloatToLittle(inmemfile[word32index + 1].f32val);                 //y
                float32bit z = scale * VSSwapHostFloatToLittle(inmemfile[word32index + 2].f32val);                 //z
                word32index += NUMFIELDSPERVERTEX;
                if (firstpoint) {
                    minx = x;
                    maxx = x;
                    miny = y;
                    maxy = y;
                    minz = z;
                    maxz = z;
                    firstpoint = false;
                }
                minx = (x < minx) ? x : minx;
                miny = (y < miny) ? y : miny;
                minz = (z < minz) ? z : minz;
                maxx = (x > maxx) ? x : maxx;
                maxy = (y > maxy) ? y : maxy;
                maxz = (z > maxz) ? z : maxz;
            }
            //End Vertices
            //go to next mesh
            word32index = meshbeginword + (meshlength / 4);
        }
        //go to next record
        word32index = recordbeginword + (recordlength / 4);
    }
    printf("%s\t%g\t%g\t%g\t%g\t%g\t%g\n", name, minx, miny, minz, maxx, maxy, maxz);
}

