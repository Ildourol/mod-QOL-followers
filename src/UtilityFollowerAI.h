/*
 * Copyright (C) 2026+ AzerothCore Project
 * Released under GNU AGPL v3 license.
 */

#ifndef UTILITY_FOLLOWER_AI_H
#define UTILITY_FOLLOWER_AI_H

#include "PassiveAI.h"
#include "UtilityFollowerCommon.h"
#include "ObjectGuid.h"

class Creature;

class UtilityFollowerAI : public PassiveAI
{
public:
    explicit UtilityFollowerAI(Creature* creature, ObjectGuid ownerGuid, FollowerType type, uint32 displayId, float scale);

    void UpdateAI(uint32 diff) override;

    void AttackStart(Unit*) override { }
    void MoveInLineOfSight(Unit*) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }
    void JustEnteredCombat(Unit* /*who*/) override;
    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damagetype*/, SpellSchoolMask /*damageSchoolMask*/) override;
    void JustDied(Unit* /*killer*/) override;

    [[nodiscard]] FollowerType GetType() const { return _type; }
    [[nodiscard]] ObjectGuid GetOwnerGuid() const { return _ownerGuid; }

    void RestoreAppearance();

private:
    ObjectGuid _ownerGuid;
    FollowerType _type;
    uint32 _updateTimer;
    uint32 _displayId;
    float _scale;
};

#endif // UTILITY_FOLLOWER_AI_H
