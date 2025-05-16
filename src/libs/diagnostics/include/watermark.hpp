#pragma once
#ifdef STORM_WATERMARK_FILE
#include STORM_WATERMARK_FILE
#else
#error "Watermark file is not present. Check build configuration"
#endif
