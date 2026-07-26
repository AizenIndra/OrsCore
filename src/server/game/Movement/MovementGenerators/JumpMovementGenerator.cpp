#include "JumpMovementGenerator.h"

#include "Creature.h"
#include "CreatureAI.h"
#include "MoveSplineInit.h"
#include "Player.h"
#include "World.h"

template <class T>
JumpMovementGenerator<T>::JumpMovementGenerator(uint32 id, float x, float y, float z, Unit const* target, float speedXY, float parabolicAmplitude,
    bool hasOrientation, bool orientationFixed) : _movementId(id), _x(x), _y(y), _z(z), _target(target), _speedXY(speedXY), _parabolicAmplitude(parabolicAmplitude),
    _hasOrientation(hasOrientation), _orientationFixed(orientationFixed)
{
}

template <class T>
void JumpMovementGenerator<T>::DoInitialize(T* owner)
{
    if (!owner)
        return;

    owner->SetIsJumping(true);

    Movement::MoveSplineInit init(owner);
    init.MoveTo(_x, _y, _z, false);
    init.SetParabolic(_parabolicAmplitude, 0.0f);
    init.SetVelocity(_speedXY);

    if (_hasOrientation && _target)
    {
        init.SetFacing(_target);
        if (_orientationFixed)
            init.SetOrientationFixed(true);
    }

    init.Launch();
    owner->UpdateSplinePosition();
}

template <class T>
void JumpMovementGenerator<T>::DoReset(T* owner)
{
    DoInitialize(owner);
}

template <class T>
bool JumpMovementGenerator<T>::DoUpdate(T* owner, uint32 /*diff*/)
{
    if (!owner)
        return false;

    return !owner->movespline->Finalized();
}

template <class T>
void JumpMovementGenerator<T>::DoFinalize(T* owner)
{
    if (!owner)
        return;

    MovementInform(owner);
    owner->SetIsJumping(false);
}

template <class T>
void JumpMovementGenerator<T>::MovementInform(T*) { }

template <>
void JumpMovementGenerator<Creature>::MovementInform(Creature* owner)
{
    if (owner && owner->AI())
        owner->AI()->MovementInform(JUMP_MOTION_TYPE, _movementId);
}

template class JumpMovementGenerator<Player>;
template class JumpMovementGenerator<Creature>;
