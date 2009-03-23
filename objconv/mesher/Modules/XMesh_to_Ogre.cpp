#include "../PrecompiledHeaders/Converter.h"
#include "../Converter.h"
#include "../from_BFXM.h"
#include "../to_OgreMesh.h"

namespace Converter {

	class XMeshToOgreImpl : public ConversionImpl
	{
	public:
		/*
		 *     ConversionImpl interface
		 */

		virtual RetCodeEnum convert(const std::string &inputFormat, const std::string &outputFormat, const std::string &opCode)
		{
			if (  inputFormat == "XMesh" && outputFormat== "Ogre"  ) {
				if ( opCode == "create" ) {
					string input = getNamedOption("inputPath");
					string output= getNamedOption("outputPath");
					string base  = input.substr(0,input.rfind('.'));
					string mtl   = base + string(".material");
					XML memfile=(LoadXML(input.c_str(),1));
					OgreMeshConverter::ConverterInit();
					void *data = OgreMeshConverter::Init();
					OgreMeshConverter::Add(data, memfile);
					OgreMeshConverter::DoneMeshes(data);
					OgreMeshConverter::Dump(data, output.c_str(), mtl.c_str());
					OgreMeshConverter::ConverterClose();
					return RC_OK;
				} else if (opCode == "append") {
					string input = getNamedOption("inputPath");
					string output= getNamedOption("outputPath");
					string base  = input.substr(0,input.rfind('.'));
					string mtl   = base + string(".material");
					XML memfile=(LoadXML(input.c_str(),1));
					OgreMeshConverter::ConverterInit();
					void *data = OgreMeshConverter::Init(output.c_str(), mtl.c_str());
					OgreMeshConverter::Add(data, memfile);
					OgreMeshConverter::DoneMeshes(data);
					OgreMeshConverter::Dump(data, output.c_str(), mtl.c_str());
					OgreMeshConverter::ConverterClose();
					return RC_OK;
				} else if (opCode == "optimize") {
					string input = getNamedOption("inputPath");
					string output= getNamedOption("outputPath");
					string base  = input.substr(0,input.rfind('.'));
					string mtl   = base + string(".material");
					OgreMeshConverter::ConverterInit();
					void *data = OgreMeshConverter::Init(output.c_str(), mtl.c_str());
					OgreMeshConverter::Optimize(data);
					OgreMeshConverter::DoneMeshes(data);
					OgreMeshConverter::Dump(data, output.c_str(), mtl.c_str());
					OgreMeshConverter::ConverterClose();
				} else {
					return RC_NOT_IMPLEMENTED;
				}
			} else {
				return RC_NOT_IMPLEMENTED;
			}
		}

		virtual void conversionHelp(const std::string &inputFormat, const std::string &outputFormat, const std::string &opCode) const
		{
			if (  (inputFormat.empty() || inputFormat == "BFXM")
				&&(outputFormat.empty()|| outputFormat== "Wavefront")
				&&(opCode.empty() || (opCode == "create"))  )
			{
				std::cout << "BFXM -> Wavefront\n"
					 << "\tSupported operations: create\n"
					 << "\nNotes: Wavefront files usually come in pairs, with an .obj and a .mtl\n"
					 << "\tfile. So, two files will be created: the output file, and another with\n"
					 << "\tthe same name but .mtl extension.\n"
					 << std::endl;
			}
		}

	};

	ConversionImplDeclaration<XMeshToOgreImpl> my_converter_declaration;

}
