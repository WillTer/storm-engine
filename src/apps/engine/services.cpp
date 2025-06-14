#include <libs/animation/animation_service_imp.h>
#include <libs/collide/vcollide.h>
#include <libs/core/vma.hpp>
#include <libs/geometry/geometry_r.h>
#include <libs/particles/service/particle_service.h>
#include <libs/pcs_controls/pcs_controls.h>
#include <libs/renderer/s_device.h>
#include <libs/sound_service/sound_service.h>
#include <libs/xinterface/string_service/str_service.h>

CREATE_SERVICE(AnimationServiceImp)
CREATE_SERVICE(COLL)
CREATE_SERVICE(DX9RENDER)
CREATE_SERVICE(GEOMETRY)
CREATE_SERVICE(LostDeviceSentinel)
CREATE_SERVICE(ParticleService)
CREATE_SERVICE(PCS_CONTROLS)
CREATE_SERVICE(SoundService)
CREATE_SERVICE(STRSERVICE)