//
// - This is the fog header to include for vertex shaders if the shader does support pixel-shader-blended vertex fog.
//

// -- DOWATERFOG is 0 for RANGE FOG, 1 for WATER/HEIGHT FOG --
// DYNAMIC: "DOWATERFOG"	"0..1"

// -- DOPIXELFOG is 1 if the material has $pixelfog 1, or if the material forces pixel fog by default (lightmappedgeneric for instance.) --
// FIXME!  Will need to revisit this once we are doing water/height fog again.  Needs to be pixel for in the water case.
// STATIC: "DOPIXELFOG" "0..1"

// STATIC: "HARDWAREFOGBLEND"		"0..0"	[XBOX]
// STATIC: "HARDWAREFOGBLEND"		"0..1"	[PC] [vs20]
// STATIC: "HARDWAREFOGBLEND"		"0..0"	[PC] [vs30]

// Can't do fixed-function fog and pixelfog at the same time, but only if we aren't doing water since HARDWAREFOGBLEND and DOPIXELFOG get redefined below.
// SKIP: ( $DOWATERFOG == 0 ) && ( $DOPIXELFOG && $HARDWAREFOGBLEND )

#if DOWATERFOG == 1
	#undef HARDWAREFOGBLEND
	#define HARDWAREFOGBLEND 0

	#undef DOPIXELFOG
	#define DOPIXELFOG 1
#endif
