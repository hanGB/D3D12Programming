// 각종 정의를 통한 설정 저장

#define MAX_LIGHTS 16
#define ALPHA_TEST
#define FOG
#define GRAVITY 9.8f
#define PI_VALUE 3.141592f
#define DEBUG_COLOR float4(0.0f, 1.0f, 0.0f, 1.0f)

#ifndef NUM_DIR_LIGHTS
#define NUM_DIR_LIGHTS 1
#endif

#ifndef NUM_POINT_LIGHTS
#define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
#define NUM_SPOT_LIGHTS 0
#endif
