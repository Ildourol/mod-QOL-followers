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
#include <cmath>

UtilityFollowerAI::UtilityFollowerAI(Creature* creature, ObjectGuid ownerGuid, FollowerType type, uint32 displayId, float scale)
    : PassiveAI(creature),
      _ownerGuid(ownerGuid),
      _type(type),
      _updateTimer(sUtilityFollowerConfig->FollowUpdateInterval),
      _displayId(displayId),
      _scale(scale),
      _isMovingToOwner(false),
      _lastDestX(0.0f),
      _lastDestY(0.0f)
{
    me->SetSpeed(MOVE_RUN, 1.0f);
    me->SetSpeed(MOVE_WALK, 1.0f);
}

void UtilityFollowerAI::Reset()
{
    me->SetSpeed(MOVE_RUN, 1.0f);
    me->SetSpeed(MOVE_WALK, 1.0f);
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
            if (_isMovingToOwner || me->HasUnitState(UNIT_STATE_FOLLOW_MOVE))
            {
                me->GetMotionMaster()->Clear();
                me->StopMoving();
                _isMovingToOwner = false;
            }
            return;
        }

        float dist = me->GetExactDist(owner);

        // If follower is interacting with player (gossip/bank/trainer), do not interrupt interaction unless far away
        if (me->HasUnitState(UNIT_STATE_DISTRACTED) && dist <= sUtilityFollowerConfig->CatchUpDistance)
            return;

        // 1. Stuck / catch-up recovery
        if (dist > sUtilityFollowerConfig->CatchUpDistance)
        {
            if (sUtilityFollowerConfig->TeleportWhenStuck)
            {
                me->NearTeleportTo(owner->GetPositionX(), owner->GetPositionY(), owner->GetPositionZ(), owner->GetOrientation());
                me->GetMotionMaster()->Clear();
                me->StopMoving();
                _isMovingToOwner = false;
                me->SetFacingToObject(owner);
                me->SetSpeed(MOVE_RUN, 1.0f);
                return;
            }
        }

        // 2. Arrival / Deadzone: If follower has reached close proximity (<= FollowDistance)
        if (dist <= sUtilityFollowerConfig->FollowDistance)
        {
            if (_isMovingToOwner)
            {
                me->GetMotionMaster()->Clear();
                me->StopMoving();
                me->SetFacingToObject(owner);
                _isMovingToOwner = false;
            }
            // If already stopped within the deadzone, do not react to player rotating or micro-movements.
            return;
        }

        // 3. Follow Activation: If player has moved beyond StartFollowingDistance, or if follower is already pursuing
        if (dist > sUtilityFollowerConfig->StartFollowingDistance || (_isMovingToOwner && dist > sUtilityFollowerConfig->FollowDistance))
        {
            // Calculate natural approach destination along current line of bearing from owner to follower.
            // This ensures the companion approaches along its current direction rather than orbiting to the player's left side.
            float bearing = owner->GetAngle(me);
            float destX = owner->GetPositionX() + std::cos(bearing) * sUtilityFollowerConfig->FollowDistance;
            float destY = owner->GetPositionY() + std::sin(bearing) * sUtilityFollowerConfig->FollowDistance;
            float destZ = owner->GetMapHeight(destX, destY, owner->GetPositionZ());

            // Check if recalculation is needed (if player moved significantly since last path was ordered)
            float distFromLastDest = std::hypot(destX - _lastDestX, destY - _lastDestY);
            if (!_isMovingToOwner || distFromLastDest > 1.5f)
            {
                // Always follow at default constant speed (no speed-up when left behind)
                me->SetSpeed(MOVE_RUN, 1.0f);

                me->GetMotionMaster()->MovePoint(POINT_FOLLOWER_APPROACH, destX, destY, destZ);
                _lastDestX = destX;
                _lastDestY = destY;
                _isMovingToOwner = true;
            }
        }
    }
    else
        _updateTimer -= diff;
}

void UtilityFollowerAI::MovementInform(uint32 /*motionType*/, uint32 pointId)
{
    if (pointId == POINT_FOLLOWER_APPROACH)
    {
        _isMovingToOwner = false;
        if (Player* owner = ObjectAccessor::FindPlayer(_ownerGuid))
            me->SetFacingToObject(owner);
    }
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

    if (Player* owner = ObjectAccessor::FindPlayer(_ownerGuid))
        me->SetFaction(owner->GetFaction());

    me->SetUnitFlag(UnitFlags(UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_PACIFIED));

    if (_type == FollowerType::Trainer)
        me->SetNpcFlag(UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_GOSSIP);

    me->SetSpeed(MOVE_RUN, 1.0f);
    me->SetSpeed(MOVE_WALK, 1.0f);
}
