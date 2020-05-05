#ifndef CONFIG_SIMPLICITY_H
#define CONFIG_SIMPLICITY_H

#include "bsqlengine_internal_config.h"

#define STRINGIZE(x) #x
//STRINGIZE_VALUE_OF will catch a compiler definition -DKEY=VALUE and will convert the VALUE to string
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

#define SIMPLICITY_NAME std::make_pair("SIMPLICITY_NAME", "@CMAKE_PROJECT_NAME@")
#define SIMPLICITY_VERSION std::make_pair("SIMPLICITY_VERSION", "@PROJECT_VERSION@")
//The SIMPLICITY_RELEASE_VALUE will be set using compiler definition
#define SIMPLICITY_RELEASE std::make_pair("SIMPLICITY_RELEASE", STRINGIZE_VALUE_OF(SIMPLICITY_RELEASE_VALUE))

#define SYSTEM_NAME std::make_pair("SYSTEM_NAME", "@CMAKE_SYSTEM_NAME@")
#define SYSTEM std::make_pair("SYSTEM", "@CMAKE_SYSTEM@")
#define SYSTEM_PROCESSOR std::make_pair("SYSTEM_PROCESSOR", "@CMAKE_SYSTEM_PROCESSOR@")

#define OS_RELEASE std::make_pair("OS_RELEASE", "@OS_RELEASE@")
#define LSB_RELEASE std::make_pair("LSB_RELEASE", "@LSB_RELEASE@")

#define CXX_COMPILER_ID std::make_pair("CXX_COMPILER_ID", "@CMAKE_CXX_COMPILER_ID@")
#define CXX_COMPILER std::make_pair("CXX_COMPILER", "@CMAKE_CXX_COMPILER@")
#define CXX_COMPILER_VERSION std::make_pair("CXX_COMPILER_VERSION", "@CMAKE_CXX_COMPILER_VERSION@")

#define CMAKE_VERSION std::make_pair("CMAKE_VERSION", "@CMAKE_VERSION@")
#define CMAKE_GENERATOR std::make_pair("CMAKE_GENERATOR", "@CMAKE_GENERATOR@")

#define GPU_COMPUTE_CAPABILITY_NUMBER std::make_pair("GPU_COMPUTE_CAPABILITY", "@GPU_COMPUTE_CAPABILITY@")
#define GPU_COMPUTE_CAPABILITY @GPU_COMPUTE_CAPABILITY@

#define CUDA_VERSION_NUMBER std::make_pair("CUDA_VERSION", "@CUDA_VERSION@")
#define CUDA_TOOLKIT_ROOT_DIR std::make_pair("CUDA_TOOLKIT_ROOT_DIR", "@CUDA_TOOLKIT_ROOT_DIR@")
#define CUDA_SDK_ROOT_DIR std::make_pair("CUDA_SDK_ROOT_DIR", "@CUDA_SDK_ROOT_DIR@")
#define CUDA_INCLUDE_DIRS std::make_pair("CUDA_INCLUDE_DIRS", "@CUDA_INCLUDE_DIRS@")
#define CUDA_LIBRARIES std::make_pair("CUDA_LIBRARIES", "@CUDA_LIBRARIES@")

#define XERCESC_VERSION std::make_pair("XERCESC_VERSION", "@XercesC_VERSION@")
#define XERCESC_INCLUDE_DIRS std::make_pair("XERCESC_INCLUDE_DIRS", "@XercesC_INCLUDE_DIRS@")
#define XERCESC_LIBRARIES std::make_pair("XERCESC_LIBRARIES", "@XercesC_LIBRARIES@")

#define RAPIDJSON_VERSION std::make_pair("RAPIDJSON_VERSION", "@RAPIDJSON_VERSION@")

#define AWS_SDK_CPP_GIT_BRANCH std::make_pair("AWS_SDK_CPP_GIT_BRANCH", "@AWS_SDK_CPP_GIT_BRANCH@")
#define AWS_SDK_CPP_GIT_COMMIT_HASH std::make_pair("AWS_SDK_CPP_GIT_COMMIT_HASH", "@AWS_SDK_CPP_GIT_COMMIT_HASH@")

#define SIMPLICITY_DESCRIPTIVE_METADATA {\
SIMPLICITY_GIT_BRANCH, \
SIMPLICITY_GIT_COMMIT_HASH, \
SIMPLICITY_NAME, \
SIMPLICITY_VERSION, \
SIMPLICITY_RELEASE, \
SYSTEM_NAME, \
SYSTEM, \
SYSTEM_PROCESSOR, \
OS_RELEASE, \
LSB_RELEASE, \
CXX_COMPILER_ID, \
CXX_COMPILER, \
CXX_COMPILER_VERSION, \
CMAKE_VERSION, \
CMAKE_GENERATOR, \
GPU_COMPUTE_CAPABILITY_NUMBER, \
CUDA_VERSION_NUMBER, \
CUDA_TOOLKIT_ROOT_DIR, \
CUDA_SDK_ROOT_DIR, \
CUDA_INCLUDE_DIRS, \
CUDA_LIBRARIES, \
XERCESC_VERSION, \
XERCESC_INCLUDE_DIRS, \
XERCESC_LIBRARIES, \
RAPIDJSON_VERSION, \
AWS_SDK_CPP_GIT_BRANCH, \
AWS_SDK_CPP_GIT_COMMIT_HASH \
}

#endif // CONFIG_SIMPLICITY_H
