#ifndef ACORE_CHARGEMOVEMENTGENERATOR_H
#define ACORE_CHARGEMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "Optional.h"
#include "Position.h"

class Unit;
class Creature;

template <class T>
class ChargeMovementGenerator : public MovementGeneratorMedium<T, ChargeMovementGenerator<T>>
{
public:
    ChargeMovementGenerator(uint32 id, float x, float y, float z, bool generatePath, float speed, const Movement::PointsArray* precomputedPath,
        ObjectGuid chargeTargetGUID, Optional<float> finalOrient = {});

    MovementGeneratorType GetMovementGeneratorType() override { return CHARGE_MOTION_TYPE; }

    void DoInitialize(T*);
    void DoReset(T*);
    bool DoUpdate(T*, uint32);
    void DoFinalize(T*);

private:
    void MovementInform(T*);

    uint32 _movementId;
    float _x, _y, _z;
    bool _generatePath;
    float _speed;
    const Movement::PointsArray* _precomputedPath;
    ObjectGuid _chargeTargetGUID;
    Optional<float> _finalOrient;
};

#endif // ACORE_CHARGEMOVEMENTGENERATOR_H
