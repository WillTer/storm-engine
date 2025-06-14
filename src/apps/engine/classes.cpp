#include <libs/animals/animals.h>
#include <libs/ball_splash/ball_splash.h>
#include <libs/battle_interface/active_perk_shower.h>
#include <libs/battle_interface/interface_manager/interface_manager.h>
#include <libs/battle_interface/item_entity/item_entity.h>
#include <libs/battle_interface/land/battle_land.h>
#include <libs/battle_interface/land/i_boarding_status.h>
#include <libs/battle_interface/log_and_action.h>
#include <libs/battle_interface/sea/i_battle.h>
#include <libs/battle_interface/sea/ship_pointer.h>
#include <libs/battle_interface/spyglass/spyglass.h>
#include <libs/battle_interface/timer/timer.h>
#include <libs/battle_interface/world_map_interface/interface.h>
#include <libs/blade/blade.h>
#include <libs/blot/blots.h>
#include <libs/core/vma.hpp>
#include <libs/dialog/dialog.hpp>
#include <libs/dialog/legacy_dialog.hpp>
#include <libs/island/foam.h>
#include <libs/island/island.h>
#include <libs/lighter/lighter.h>
#include <libs/location/blood.h>
#include <libs/location/character_animation_kipper.h>
#include <libs/location/characters_groups.h>
#include <libs/location/fader.h>
#include <libs/location/grass.h>
#include <libs/location/lights.h>
#include <libs/location/lizards.h>
#include <libs/location/loc_crabs.h>
#include <libs/location/loc_eagle.h>
#include <libs/location/loc_rats.h>
#include <libs/location/location.h>
#include <libs/location/location_camera.h>
#include <libs/location/location_effects.h>
#include <libs/location/model_realizer.h>
#include <libs/location/np_character.h>
#include <libs/location/player.h>
#include <libs/location/wide_screen.h>
#include <libs/locator/locator.h>
#include <libs/mast/mast.h>
#include <libs/model/modelr.h>
#include <libs/particles/k2_wrapper/particles.h>
#include <libs/rigging/flag.h>
#include <libs/rigging/rope.h>
#include <libs/rigging/sail.h>
#include <libs/rigging/vant.h>
#include <libs/sailors/sailors_editor.h>
#include <libs/sea/sea.h>
#include <libs/sea_ai/ai_sea_goods.h>
#include <libs/sea_ai/locator_show.h>
#include <libs/sea_ai/sea_ai.h>
#include <libs/sea_cameras/deck_camera.h>
#include <libs/sea_cameras/free_camera.h>
#include <libs/sea_cameras/sea_cameras.h>
#include <libs/sea_cameras/ship_camera.h>
#include <libs/sea_creatures/sharks.h>
#include <libs/sea_foam/seafoam.h>
#include <libs/sea_operator/sea_operator.h>
#include <libs/shadow/shadow.h>
#include <libs/ship/ship.h>
#include <libs/ship/ship_lights.h>
#include <libs/ship/track.h>
#include <libs/sink_effect/sink_effect.h>
#include <libs/sound/sound.h>
#include <libs/teleport/teleport.h>
#include <libs/tornado/tornado.h>
#include <libs/touch/touch.h>
#include <libs/water_rings/water_rings.h>
#include <libs/weather/astronomy.h>
#include <libs/weather/lightning.h>
#include <libs/weather/sun_glow.h>
#include <libs/weather/water_flare.h>
#include <libs/weather/weather.h>
#include <libs/worldmap/world_map.h>
#include <libs/xinterface/aviplayer/aviplayer.h>
#include <libs/xinterface/back_scene/back_scene.h>
#include <libs/xinterface/help_chooser/help_chooser.h>
#include <libs/xinterface/info_handler.h>
#include <libs/xinterface/scr_shoter/scr_shoter.h>
#include <libs/xinterface/string_service/obj_str_service.h>
#include <libs/xinterface/texture_sequence/texture_sequence.h>
#include <libs/xinterface/xinterface.h>

CREATE_CLASS(ActivePerkShower)
CREATE_CLASS(AIBalls)
CREATE_CLASS(AIFort)
CREATE_CLASS(AISeaGoods)
CREATE_CLASS(ANIMALS)
CREATE_CLASS(Astronomy)
CREATE_CLASS(BALLSPLASH)
CREATE_CLASS(BATTLE_INTERFACE)
CREATE_CLASS(BATTLE_LAND_INTERFACE)
CREATE_CLASS(BITimer)
CREATE_CLASS(BI_InterfaceManager)
CREATE_CLASS(BLADE)
CREATE_CLASS(BLAST)
CREATE_CLASS(Blood)
CREATE_CLASS(Blots)
CREATE_CLASS(CAviPlayer)
CREATE_CLASS(CharacterAnimationKipper)
CREATE_CLASS(CharactersGroups)
CREATE_CLASS(CoastFoam)
CREATE_CLASS(CONTROLS_CONTAINER)
CREATE_CLASS(DECK_CAMERA)
CREATE_CLASS(DIALOG)
CREATE_CLASS(Fader)
CREATE_CLASS(FINDDIALOGNODES)
CREATE_CLASS(FINDFILESINTODIRECTORY)
CREATE_CLASS(FLAG)
CREATE_CLASS(FREE_CAMERA)
CREATE_CLASS(Grass)
CREATE_CLASS(HELPCHOOSER)
CREATE_CLASS(HULL)
CREATE_CLASS(IBoardingStatus)
CREATE_CLASS(ILogAndActions)
CREATE_CLASS(InfoHandler)
CREATE_CLASS(InterfaceBackScene)
CREATE_CLASS(ISLAND)
CREATE_CLASS(ISPYGLASS)
CREATE_CLASS(ItemEntity)
CREATE_CLASS(LegacyDialog)
CREATE_CLASS(Lighter)
CREATE_CLASS(LIGHTNING)
CREATE_CLASS(Lights)
CREATE_CLASS(Lizards)
CREATE_CLASS(Location)
CREATE_CLASS(LocationCamera)
CREATE_CLASS(LocationEffects)
CREATE_CLASS(LOCATOR)
CREATE_CLASS(LocCrabs)
CREATE_CLASS(LocEagle)
CREATE_CLASS(LocModelRealizer)
CREATE_CLASS(LocRats)
CREATE_CLASS(MAST)
CREATE_CLASS(MODELR)
CREATE_CLASS(NPCharacter)
CREATE_CLASS(OBJ_STRSERVICE)
CREATE_CLASS(PARTICLES)
CREATE_CLASS(Player)
CREATE_CLASS(RAIN)
CREATE_CLASS(ROPE)
CREATE_CLASS(SAIL)
CREATE_CLASS(Sailors)
CREATE_CLASS(SailorsEditor)
CREATE_CLASS(SCRSHOTER)
CREATE_CLASS(SEA)
CREATE_CLASS(SEAFOAM)
CREATE_CLASS(SeaLocatorShow)
CREATE_CLASS(SEA_AI)
CREATE_CLASS(SEA_CAMERAS)
CREATE_CLASS(SEA_OPERATOR)
CREATE_CLASS(Shadow)
CREATE_CLASS(Sharks)
CREATE_CLASS(SHIP)
CREATE_CLASS(ShipLights)
CREATE_CLASS(SHIPPOINTER)
CREATE_CLASS(ShipTracks)
CREATE_CLASS(SHIP_CAMERA)
CREATE_CLASS(SINKEFFECT)
CREATE_CLASS(SKY)
CREATE_CLASS(SOUND)
CREATE_CLASS(SUNGLOW)
CREATE_CLASS(TextureSequence)
CREATE_CLASS(TMPTELEPORT)
CREATE_CLASS(Tornado)
CREATE_CLASS(TOUCH)
CREATE_CLASS(VANT)
CREATE_CLASS(VANTL)
CREATE_CLASS(VANTZ)
CREATE_CLASS(WATERFLARE)
CREATE_CLASS(WaterRings)
CREATE_CLASS(WEATHER)
CREATE_CLASS(WideScreen)
CREATE_CLASS(WM_INTERFACE)
CREATE_CLASS(WorldMap)
CREATE_CLASS(XINTERFACE)