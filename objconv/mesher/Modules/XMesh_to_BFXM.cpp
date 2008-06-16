#include "../PrecompiledHeaders/Converter.h"
#include "../Converter.h"
#include "../to_BFXM.h"

using namespace std;

namespace Converter {

	class XMeshToBFXMImpl : public ConversionImpl
	{
	public:
		/*
		 *     ConversionImpl interface
		 */

		virtual RetCodeEnum convert(const std::string &inputFormat, const std::string &outputFormat, const std::string &opCode)
		{
			if (  inputFormat == "XMesh" && outputFormat== "BFXM"  ) {
				if ( opCode == "add" ) {
					bool forcenormals=atoi(getNamedOption("forcenormals").c_str())!=0;
					string input = getNamedOption("inputPath");
					string output= getNamedOption("outputPath");
					FILE *Outputfile=fopen(output.c_str(),"rb+"); //append to end, but not append, which doesn't do what you want it to.
					fseek(Outputfile, 0, SEEK_END);
					XML memfile=(LoadXML(input.c_str(),1));
					xmeshToBFXM(memfile,Outputfile,'a',forcenormals);
					return RC_OK;
				} else if ( opCode == "create" ) {
					bool forcenormals=atoi(getNamedOption("forcenormals").c_str())!=0;
					string input = getNamedOption("inputPath");
					string output= getNamedOption("outputPath");
					FILE *Outputfile=fopen(output.c_str(),"wb+"); //create file for BFXM output
					XML memfile=(LoadXML(input.c_str(),1));
					xmeshToBFXM(memfile,Outputfile,'c',forcenormals);
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
			if (  (inputFormat.empty() || inputFormat == "XMesh")
				&&(outputFormat.empty()|| outputFormat== "BFXM")
				&&(opCode.empty() || (opCode == "add" || opCode == "create"))  )
			{
				cout << "XMesh -> BFXM\n"
					 << "\tSupported operations: add, create\n"
					 << endl;
			}
		}

	};

	static ConversionImplDeclaration<XMeshToBFXMImpl> __xbh_declaration;

}
