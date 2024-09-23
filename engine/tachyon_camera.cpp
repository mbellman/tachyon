#include <math.h>

#include "engine/tachyon_camera.h"
#include "engine/tachyon_constants.h"

void tOrientation::operator+=(const tOrientation& orientation) {
  roll += orientation.roll;
  pitch += orientation.pitch;
  yaw += orientation.yaw;
}

tVec3f tOrientation::getDirection() const {
  const static tVec3f forward = tVec3f(0, 0, -1.f);

  float cosr = cosf(-roll * 0.5f);
  float sinr = sinf(-roll * 0.5f);
  float cosy = cosf(yaw * -0.5f);
  float siny = sinf(yaw * -0.5f);
  float cosp = cosf(pitch * -0.5f);
  float sinp = sinf(pitch * -0.5f);

  Quaternion q;

  q.w = cosp * cosy * cosr + sinp * siny * sinr;
  q.x = sinp * cosy * cosr - cosp * siny * sinr;
  q.y = cosp * siny * cosr + sinp * cosy * sinr;
  q.z = cosp * cosy * sinr - sinp * siny * cosr;

  return q.toMatrix4f().transformVec3f(forward);
}

tVec3f tOrientation::getLeftDirection() const {
  return tVec3f::cross(getUpDirection(), getDirection()).unit();
}

tVec3f tOrientation::getRightDirection() const {
  return tVec3f::cross(getDirection(), getUpDirection()).unit();
}

tVec3f tOrientation::getUpDirection() const {
  return tOrientation(roll, (pitch - t_PI / 2.0f), yaw).getDirection();
}

void tOrientation::face(const tVec3f& forward, const tVec3f& up) {
  const static tVec3f worldUp = tVec3f(0, 1.f, 0);

  float uDotY = tVec3f::dot(up, worldUp);

  // Calculate yaw as a function of forward z/x
  yaw = atan2f(forward.z, forward.x) + t_HALF_PI;

  if (uDotY < 0.0f) {
    // If upside-down, flip the yaw by 180 degrees
    yaw -= t_PI;
  }

  tVec3f rUp = up;

  // Rotate the up vector back onto the y/z plane,
  // and calculate pitch as a function of y/z
  rUp.z = up.x * sinf(yaw) + up.z * cosf(yaw);

  auto unit_forward = forward.unit();

  pitch = atan2f(unit_forward.xz().magnitude(), unit_forward.y) - t_HALF_PI;
}

tOrientation tOrientation::invert() const {
  return tOrientation(-roll, -pitch, -yaw);
}

// @todo determine why the approach used in getDirection()
// doesn't work when generating a quaternion used for the
// camera view matrix
Quaternion tOrientation::toQuaternion() const {
  Quaternion qp = Quaternion::fromAxisAngle(pitch, 1.0f, 0.0f, 0.0f);
  Quaternion qy = Quaternion::fromAxisAngle(yaw, 0.0f, 1.0f, 0.0f);
  Quaternion qr = Quaternion::fromAxisAngle(roll, 0.0f, 0.0f, 1.0f);

  return (qp * qy * qr);
}

tVec3f tOrientation::toVec3f() const {
  return tVec3f(pitch, yaw, roll);
}

tVec3f tCamera3p::calculatePosition() const {
  float cosAltitude = cosf(altitude);

  return tVec3f(
    cosAltitude * cosf(azimuth) * radius,
    sinf(altitude) * radius,
    cosAltitude * sinf(azimuth) * radius
  );
}

bool tCamera3p::isUpsideDown() const {
  return cosf(altitude) < 0.0f;
}

void tCamera3p::limitAltitude(float factor) {
  constexpr float BASE_ALTITUDE_LIMIT = t_HALF_PI * 0.999f;
  float altitudeLimit = BASE_ALTITUDE_LIMIT * factor;

  if (altitude > altitudeLimit) altitude = altitudeLimit;
  else if (altitude < -altitudeLimit) altitude = -altitudeLimit;
}