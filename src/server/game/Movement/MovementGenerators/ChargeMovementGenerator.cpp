#include "ChargeMovementGenerator.h"

#include "Creature.h"
#include "CreatureAI.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Unit.h"

template <class T>
ChargeMovementGenerator<T>::ChargeMovementGenerator(uint32 id, float x, float y, float z, bool generatePath, float speed, const Movement::PointsArray* precomputedPath,
    ObjectGuid chargeTargetGUID, Optional<float> finalOrient) :
    _movementId(id),
    _x(x),
    _y(y),
    _z(z),
    _generatePath(generatePath),
    _speed(speed),
    _precomputedPath(precomputedPath),
    _chargeTargetGUID(chargeTargetGUID),
    _finalOrient(finalOrient)
{
}

template <class T>
void ChargeMovementGenerator<T>::DoInitialize(T* owner)
{
    if (!owner)
        return;

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE))
    {
        owner->StopMoving();
        return;
    }

    owner->AddUnitState(UNIT_STATE_CHARGING);
    owner->SetIsCharging(true);

    Movement::MoveSplineInit init(owner);
    if (_generatePath && _precomputedPath)
        init.MovebyPath(*_precomputedPath);
    else
        init.MoveTo(_x, _y, _z, _generatePath);

    if (_speed > 0.0f)
        init.SetVelocity(_speed);

    if (_finalOrient)
        init.SetFacing(*_finalOrient);

    init.Launch();
    owner->UpdateSplinePosition();
}

template <class T>
void ChargeMovementGenerator<T>::DoReset(T* owner)
{
    DoInitialize(owner);
}

template <class T>
bool ChargeMovementGenerator<T>::DoUpdate(T* owner, uint32 /*diff*/)
{
    if (!owner)
        return false;

    return !owner->movespline->Finalized();
}

template <class T>
void ChargeMovementGenerator<T>::DoFinalize(T* owner)
{
    if (!owner)
        return;

    if (_chargeTargetGUID && owner->GetTarget() == _chargeTargetGUID)
    {
        if (Unit* target = ObjectAccessor::GetUnit(*owner, _chargeTargetGUID))
            owner->Attack(target, true);
    }

    MovementInform(owner);
    owner->ClearUnitState(UNIT_STATE_CHARGING);
    owner->SetIsCharging(false);
}

template <class T>
void ChargeMovementGenerator<T>::MovementInform(T*) { }

template <>
void ChargeMovementGenerator<Creature>::MovementInform(Creature* owner)
{
    if (owner && owner->AI())
        owner->AI()->MovementInform(CHARGE_MOTION_TYPE, _movementId);
}

template class ChargeMovementGenerator<Player>;
template class ChargeMovementGenerator<Creature>;
