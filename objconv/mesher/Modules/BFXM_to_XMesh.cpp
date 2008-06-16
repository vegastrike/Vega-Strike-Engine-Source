#include "../PrecompiledHeaders/Converter.h"
#include "../Converter.h"
#include "../from_BFXM.h"

using namespace std;

namespace Converter {

	class BFXMToXMeshImpl : public ConversionImpl
	{
	public:
		/*
		 *     ConversionImpl interface
		 */

		virtual RetCodeEnum convert(const std::string &inputFormat, const std::string &outputFormat, const std::string &opCode)
		{
			if (  inputFormat == "BFXM" && outputFormat== "XMesh"  ) {
				if ( opCode == "create" ) {
					string input = getNamedOption("inputPath");
					string output= getNamedOption("outputPath");
					string basename = input.substr(0,input.find_last_of("."));
					FILE * Inputfile=fopen(input.c_str(),"rb");
					FILE * Outputfile=fopen(output.c_str(),"w+"); //create file for text output
					FILE * OutputObj=NULL;
					FILE * OutputMtl=NULL;
					BFXMToXmeshOrOBJ(Inputfile,Outputfile,OutputObj,OutputMtl,basename,'x');
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
				&&(outputFormat.empty()|| outputFormat== "XMesh")
				&&(opCode.empty() || (opCode == "create"))  )
			{
				cout << "BFXM -> XMesh\n"
					 << "\tSupported operations: create\n"
					 << "\nNotes:\n"
					 << "\tMeshes will be created using the naming rules <submesh>_<lod>.xmesh,\n"
					 << "\twith the exception of 0_0.xmesh, which will be the output file.\n"
					 << "\tIt is recomended, though, that the output file be 0_0.xmesh so that\n"
					 << "\tthe resulting names make sense ;-)\n"
					 << endl;
			}
		}

	};

	static ConversionImplDeclaration<BFXMToXMeshImpl> __bxh_declaration;

}
