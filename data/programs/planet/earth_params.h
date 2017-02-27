
uniform vec4 envColor;

uniform vec4 fvCityLightColor;
uniform vec4 fvShadowColor;
uniform vec4 fCityLightTriggerBias;
uniform vec4 fShininess;
uniform vec4 fFresnelEffect;
uniform vec4 fGroundContrast_SelfShadowFactor_MinMaxScatterFactor;

#define fGroundContrast fGroundContrast_SelfShadowFactor_MinMaxScatterFactor.x
#define fSelfShadowFactor fGroundContrast_SelfShadowFactor_MinMaxScatterFactor.y
#define fMinScatterFactor fGroundContrast_SelfShadowFactor_MinMaxScatterFactor.z
#define fMaxScatterFactor fGroundContrast_SelfShadowFactor_MinMaxScatterFactor.w

uniform vec4 fAtmosphereType_Thickness_Contrast_LAOffs; 
    //type indexes the t coordinate on cosAngleToDepth, cosAngleToAbsorption, cosAngleToScatter
    //thickness scales cosAngleToDepth
    //contrast is a lighting parameter

#define fAtmosphereType fAtmosphereType_Thickness_Contrast_LAOffs.x
#define fAtmosphereThickness fAtmosphereType_Thickness_Contrast_LAOffs.y
#define fAtmosphereContrast fAtmosphereType_Thickness_Contrast_LAOffs.z
#define fAtmosphereAbsorptionOffset fAtmosphereType_Thickness_Contrast_LAOffs.w

uniform vec4 fAtmosphereExtrusionType_Thickness_Contrast_LAOffs; 
    //type indexes the t coordinate on cosAngleToDepth, cosAngleToAbsorption, cosAngleToScatter
    //thickness scales cosAngleToDepth
    //contrast is a lighting parameter

#define fAtmosphereHaloType fAtmosphereExtrusionType_Thickness_Contrast_LAOffs.x
#define fAtmosphereHaloThickness fAtmosphereExtrusionType_Thickness_Contrast_LAOffs.y
#define fAtmosphereHaloContrast fAtmosphereExtrusionType_Thickness_Contrast_LAOffs.z
#define fAtmosphereHaloAbsorptionOffset fAtmosphereExtrusionType_Thickness_Contrast_LAOffs.w

uniform vec4  fAtmosphereAbsorptionColor;
uniform vec4  fAtmosphereScatterColor;
uniform vec4  fAtmosphereShadowInfluence;
uniform vec4  fReyleighRate_Amount;

#define fReyleighRate fReyleighRate_Amount.x
#define fReyleighAmount fReyleighRate_Amount.y

uniform vec4 fAtmosphereExtrusion;
uniform vec4 fvCloudLayerDrift_ShadowRelHeight;
uniform vec4 fCloud_Dens_Thick_CLF_SSF;

#define fCloudLayerDensity      fCloud_Dens_Thick_CLF_SSF.x
#define fCloudLayerThickness    fCloud_Dens_Thick_CLF_SSF.y
#define fCityLightFactor        fCloud_Dens_Thick_CLF_SSF.z
#define fCloudSelfShadowFactor  fCloud_Dens_Thick_CLF_SSF.w

uniform vec4 fBumpScale;

uniform vec4 fShadowRelHeight;

uniform vec4 fvCloudSelfShadowColor;
uniform vec4 fvCloudColor;
uniform vec4 fvCloudLayerDrift;
uniform vec4 fvCloudLayerMix;
uniform vec4 fvCityLightCloudDiffusion;

uniform vec4 fAtmosphereExtrusionNDLScaleOffsSteepThick;

#define fAtmosphereExtrusionNDLScaleOffs fAtmosphereExtrusionNDLScaleOffsSteepThick.xy
#define fAtmosphereExtrusionSteepness fAtmosphereExtrusionNDLScaleOffsSteepThick.z
#define fAtmosphereExtrusionThickness fAtmosphereExtrusionNDLScaleOffsSteepThick.w

