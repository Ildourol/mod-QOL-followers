/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#include "UtilityFollowerAI.h"
#include "Creature.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "UtilityFollowerConfig.h"
#include "UtilityFollowerMgr.h"

UtilityFollowerAI::UtilityFollowerAI(Creature* creature, ObjectGuid ownerGuid, FollowerType type, uint32 displayId, float scale)
    : PassiveAI(creature),
      _ownerGuid(ownerGuid),
      _type(type),
      _updateTimer(sUtilityFollowerConfig->FollowUpdateInterval),
      _displayId(displayId),
      _scale(scale)
{
}

void UtilityFollowerAI::UpdateAI(uint32 diff)
{
    if (_updateTimer <= diff)
    {
        _updateTimer = sUtilityFollowerConfig->FollowUpdateInterval;

        Player* owner = ObjectAccessor::FindPlayer(_ownerGuid);
        if (!owner || !owner->IsInWorld())
        {
            sUtilityFollowerMgr->UnregisterFollower(me->GetGUID());
            me->DespawnOrUnsummon();
            return;
        }

        // Map transitions are handled by PlayerScript::OnPlayerMapChanged
        if (owner->GetMap() != me->GetMap())
            return;

        // If player is dead, pause following until resurrection
        if (!owner->IsAlive())
        {
            if (me->HasUnitState(UNIT_STATE_FOLLOW_MOVE))
                me->StopMoving();
            return;
        }

        float dist = me->GetExactDist(owner);

        // Stuck / catch-up recovery
        if (dist > sUtilityFollowerConfig->CatchUpDistance)
        {
            if (sUtilityFollowerConfig->TeleportWhenStuck)
            {
                me->NearTeleportTo(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation());
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveFollow(owner, sUtilityFollowerConfig->FollowDistance, GetFollowerAngle(_type));
            }
        }
        // Start following hysteresis threshold
        else if (dist > sUtilityFollowerConfig->StartFollowingDistance)
        {
            if (!me->HasUnitState(UNIT_STATE_FOLLOW_MOVE))
                me->GetMotionMaster()->MoveFollow(owner, sUtilityFollowerConfig->FollowDistance, GetFollowerAngle(_type));
        }
    }
    else
        _updateTimer -= diff;
}

void UtilityFollowerAI::JustEnteredCombat(Unit* /*who*/)
{
    me->CombatStop(true);
    me->ClearInCombat();
}

void UtilityFollowerAI::DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damagetype*/, SpellSchoolMask /*damageSchoolMask*/)
{
    damage = 0;
}

void UtilityFollowerAI::JustDied(Unit* /*killer*/)
{
    sUtilityFollowerMgr->UnregisterFollower(me->GetGUID());
}

void UtilityFollowerAI::RestoreAppearance()
{
    me->SetDisplayId(_displayId);
    me->SetNativeDisplayId(_displayId);
    me->SetFloatValue(OBJECT_FIELD_SCALE_X, _scale);
}
