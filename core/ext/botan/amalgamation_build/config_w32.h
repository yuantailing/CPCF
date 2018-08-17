#undef BOTAN_DISTRIBUTION_INFO
#define BOTAN_DISTRIBUTION_INFO ""

#undef BOTAN_VERSION_VC_REVISION
#define BOTAN_VERSION_VC_REVISION ""

#pragma warning(disable:4244)
#pragma warning(disable:4297)
#pragma warning(disable:4996)
#pragma warning(disable:4250)
#pragma warning(disable:4146)
#pragma warning(disable:4334)

#ifdef min
#undef min
#endif


#ifdef max
#undef max
#endif

/*

#ifdef _WIN64
#undef BOTAN_TARGET_ARCH_IS_X86_32
#endif

#define BOTAN_USE_STD_TR1

#ifdef X942_DH_PARAMETERS
#undef X942_DH_PARAMETERS
#endif
*/