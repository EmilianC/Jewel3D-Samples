#include "Flock.h"
#include "Boid.h"

#include <Jewel3D/Application/Logging.h>
#include <Jewel3D/Entity/Entity.h>
#include <Jewel3D/Utilities/Random.h>

#define MIN_X_BOUND XBounds.x
#define MAX_X_BOUND XBounds.y
#define MIN_Y_BOUND YBounds.x
#define MAX_Y_BOUND YBounds.y
#define MIN_Z_BOUND ZBounds.x
#define MAX_Z_BOUND ZBounds.y

void Flock::RandomlyPlaceBoids()
{
	ASSERT(MIN_X_BOUND < MAX_X_BOUND, "Invalid bounds for Boid flocking volume.");
	ASSERT(MIN_Y_BOUND < MAX_Y_BOUND, "Invalid bounds for Boid flocking volume.");
	ASSERT(MIN_Z_BOUND < MAX_Z_BOUND, "Invalid bounds for Boid flocking volume.");

	for (auto& boid : All<Boid>())
	{
		// Randomize position and velocity.
		boid.owner.position.x = RandomRangef(MIN_X_BOUND, MAX_X_BOUND);
		boid.owner.position.y = RandomRangef(MIN_Y_BOUND, MAX_Y_BOUND);
		boid.owner.position.z = RandomRangef(MIN_Z_BOUND, MAX_Z_BOUND);
		boid.Velocity = RandomDirection() * RandomRangef(0.0f, MaxVelocity);
	}
}

void Flock::Update(float deltaTime)
{
	vec3 averagePosition;
	vec3 averageVelocity;

	/* Find average position and velocity */
	unsigned numBoids = 0;
	for (auto& boid : All<Boid>())
	{
		averagePosition += boid.owner.position;
		averageVelocity += boid.Velocity;
		numBoids++;
	}

	if (numBoids == 0)
		return;

	// Normalize.
	averagePosition /= static_cast<float>(numBoids);
	averageVelocity /= static_cast<float>(numBoids);

	// This keeps boids moving in the same general direction.
	vec3 alignmentForce = averageVelocity * InertiaFactor;

	/* Update flock */
	for (auto& boid : All<Boid>())
	{
		vec3 proximityForce;
		vec3 localCenter;

		/* Calculate local center */
		unsigned localCount = 0;
		for (auto& other : All<Boid>())
		{
			// Skip comparison of boid with itself.
			if (&boid == &other)
				continue;

			// If the other node if within local range, it contributes to the local center.
			if ((boid.owner.position - other.owner.position).LengthSquared() < ProximityRange)
			{
				localCenter += other.owner.position;
				localCount++;
			}
		}

		/* Compute forces */
		// Normalize and avoid divide by zero error.
		if (localCount != 0)
		{
			localCenter /= static_cast<float>(localCount);
			proximityForce = (boid.owner.position - localCenter) * ProximityFactor;
		}

		// Force away from and boundaries.
		vec3 acceleration = alignmentForce + proximityForce + (averagePosition - boid.owner.position) * PullFactor;
		if (boid.owner.position.x < MIN_X_BOUND)
		{
			acceleration.x += EdgeForce;
		}
		else if (boid.owner.position.x > MAX_X_BOUND)
		{
			acceleration.x -= EdgeForce;
		}

		if (boid.owner.position.y < MIN_Y_BOUND)
		{
			acceleration.y += EdgeForce;
		}
		else if (boid.owner.position.y > MAX_Y_BOUND)
		{
			acceleration.y -= EdgeForce;
		}

		if (boid.owner.position.z < MIN_Z_BOUND)
		{
			acceleration.z += EdgeForce;
		}
		else if (boid.owner.position.z > MAX_Z_BOUND)
		{
			acceleration.z -= EdgeForce;
		}

		/* Step */
		boid.Velocity += acceleration * deltaTime;
		boid.Velocity.ClampLength(MaxVelocity);
		boid.owner.position += boid.Velocity * deltaTime;

		/* Orient boid with velocity */
		vec3 up = vec3(0.0f, 0.0f, 1.0f);
		vec3 forward = boid.Velocity.GetNormalized();
		vec3 right = Cross(forward, up);

		boid.owner.rotation = mat3(right, up, -forward);
	}
}
