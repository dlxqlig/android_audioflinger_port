#include <bellagio/st_static_component_loader.h>
#include <config.h>
#include "omx_audiodec_component.h"

// The library entry point. It must have the same name for each
// library of the components loaded by the ST static component loader.
int omx_component_library_Setup(stLoaderComponentType **stComponents) {

	OMX_U32 i;
	DEBUG(DEB_LEV_FUNCTION_NAME, "In %s \n",__func__);

	if (stComponents == NULL) {
		DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s \n",__func__);
		return 1;
	}

	// component audio decoder
	stComponents[0]->componentVersion.s.nVersionMajor = 1;
	stComponents[0]->componentVersion.s.nVersionMinor = 1;
	stComponents[0]->componentVersion.s.nRevision = 1;
	stComponents[0]->componentVersion.s.nStep = 1;

	stComponents[0]->name = calloc(1, OMX_MAX_STRINGNAME_SIZE);
	if (stComponents[0]->name == NULL) {
		return OMX_ErrorInsufficientResources;
	}

	strcpy(stComponents[0]->name, "OMX.st.audio_decoder");
	stComponents[0]->name_specific_length = 2;
	stComponents[0]->constructor = omx_audiodec_component_Constructor;

	stComponents[0]->name_specific = calloc(stComponents[0]->name_specific_length,sizeof(char *));
	stComponents[0]->role_specific = calloc(stComponents[0]->name_specific_length,sizeof(char *));

	for(i=0;i<stComponents[0]->name_specific_length;i++) {
		stComponents[0]->name_specific[i] = calloc(1, OMX_MAX_STRINGNAME_SIZE);
		if (stComponents[0]->name_specific[i] == NULL) {
			return OMX_ErrorInsufficientResources;
		}
	}
	for(i=0;i<stComponents[0]->name_specific_length;i++) {
		stComponents[0]->role_specific[i] = calloc(1, OMX_MAX_STRINGNAME_SIZE);
		if (stComponents[0]->role_specific[i] == NULL) {
			return OMX_ErrorInsufficientResources;
		}
	}

	strcpy(stComponents[0]->name_specific[1], "OMX.st.audio_decoder.ac3");
	strcpy(stComponents[0]->role_specific[1], "audio_decoder.ac3");

	DEBUG(DEB_LEV_FUNCTION_NAME, "Out of %s \n",__func__);

	return 1;

}
