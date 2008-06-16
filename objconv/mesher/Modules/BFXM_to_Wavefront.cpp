#include "../PrecompiledHeaders/Converter.h"
#include "../Converter.h"
#include "../from_BFXM.h"

namespace Converter {

	class BFXMToWavefrontImpl : public ConversionImpl
	{
	public:
		/*
		 *     ConversionImpl interface
		 */

		virtual RetCodeEnum convert(const std::string &inputFormat, const std::string &outputFormat, const std::string &opCode)
		{
			if (  inputFormat == "BFXM" && outputFormat== "Wavefront"  ) {
				if ( opCode == "create" ) {
					string input = getNamedOption("inputPath");
					string output= getNamedOption("outputPath");
					string base  = input.substr(0,input.rfind('.'));
					string mtl   = base + string(".mtl");
					FILE * Inputfile=fopen(input.c_str(),"rb");
					FILE * OutputObj=fopen(output.c_str(),"w+"); //create file for text output
					FILE * OutputMtl=fopen(mtl.c_str(),"w");
					FILE * Outputfile=NULL;
					BFXMToXmeshOrOBJ(Inputfile,Outputfile,OutputObj,OutputMtl,base,'o');
					return RC_OK;
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

	static ConversionImplDeclaration<BFXMToWavefrontImpl> __bwh_declaration;

}
