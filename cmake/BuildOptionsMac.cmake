#set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvmgcc42")
set(CMAKE_MACOSX_RPATH ON)
if(SUBSYSTEM_NAME STREQUAL "iOS")	
	SET (SDKVER "5.0")
	SET (DEVROOT "/Developer/Platforms/iPhoneOS.platform/Developer")
	SET (SDKROOT "${DEVROOT}/SDKs/iPhoneOS${SDKVER}.sdk")
	SET (CMAKE_OSX_SYSROOT "${SDKROOT}")
	SET (CMAKE_OSX_ARCHITECTURES "armv7") #"$(ARCHS_STANDARD_32_BIT)") #"$(ARCHS_UNIVERSAL_IPHONE_OS)")
	SET (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")
	#Other 'CMAKE_OSX_ARCHITECTURES' iPhone/IOS option examples
	#SET (CMAKE_OSX_ARCHITECTURES "armv7")
	#SET (CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD_32_BIT)		
else()
	find_package(Threads REQUIRED)
	set(CMAKE_OSX_ARCHITECTURES "x86_64")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -arch x86_64")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch x86_64 -std=c++11 -stdlib=libc++")
	set(COMPILE_FLAGS "${COMPILE_FLAGS} -fPIC")
endif()