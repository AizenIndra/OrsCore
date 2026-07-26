#ifndef ACORE_JUMPMOVEMENTGENERATOR_H
#define ACORE_JUMPMOVEMENTGENERATOR_H

#include "MovementGenerator.h"

class Unit;

template <class T>
class JumpMovementGenerator : public MovementGeneratorMedium<T, JumpMovementGenerator<T>>
{
public:
    JumpMovementGenerator(uint32 id, float x, float y, float z, Unit const* target, float speedXY, float parabolicAmplitude,
        bool hasOrientation = false, bool orientationFixed = false);

    MovementGeneratorType GetMovementGeneratorType() override { return JUMP_MOTION_TYPE; }

    void DoInitialize(T*);
    void DoReset(T*);
    bool DoUpdate(T*, uint32);
    void DoFinalize(T*);

private:
    void MovementInform(T*);

    uint32 _movementId;
    float _x, _y, _z;
    Unit const* _target;
    float _speedXY;
    float _parabolicAmplitude;
    bool _hasOrientation;
    bool _orientationFixed;
};

#endif // ACORE_JUMPMOVEMENTGENERATOR_H
