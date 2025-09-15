/*
 * YOFlyController.h
 *
 *  Created on: Aug 31, 2025
 *      Author: kurtz
 */
#pragma once
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Input/Input.h>

namespace Urho3D {
class Input;
}

/// Free-fly camera/controller with configurable user basis (Forward/Right/Up).
/// Engine basis is (+X Right, +Y Up, +Z Forward). You can remap which engine axis
/// represents user Forward/Right/Up (e.g., user Up = engine +Z).
class YOFlyController : public Urho3D::LogicComponent
{
    URHO3D_OBJECT(YOFlyController, Urho3D::LogicComponent);

public:
    explicit YOFlyController(Urho3D::Context* context);

    static void RegisterObject(Urho3D::Context* context);

    // --- Tuning ---
    void SetMouseSensitivity(float degPerPixel) { mouseSens_ = degPerPixel; }
    void SetMoveSpeed(float unitsPerSec)       { moveSpeed_ = unitsPerSec; }
    void SetSpeedMul(float slowMul, float fastMul) { slowMul_ = slowMul; fastMul_ = fastMul; }
    void SetInvertY(bool inv)                  { invertY_ = inv; }
    void EnableRoll(bool enable)               { enableRoll_ = enable; }

    // Pitch clamp in degrees (symmetric ±limit). Use >= 0.0f; set to 0 to disable clamp.
    void SetPitchClamp(float degrees)          { pitchClampDeg_ = degrees; }

    // Set user basis using ENGINE-LOCAL unit vectors that represent user axes.
    // Vectors don't need to be perfectly orthonormal; they will be normalized and re-orthogonalized.
    // Example for Z-up, X-forward (automotive):
    //   SetUserBasis(Vector3::RIGHT /*F*/, Vector3::UP /*R*/, Vector3::FORWARD /*U*/);
    void SetUserBasis(const Urho3D::Vector3& forward, const Urho3D::Vector3& right, const Urho3D::Vector3& up);

    // Optional: preset helpers
    void SetBasis_ZUp_XForward();     // Forward=+X, Right=+Y, Up=+Z
    void SetBasis_YUp_ZForward();     // Urho default (Forward=+Z, Right=+X, Up=+Y)

protected:
    void Start() override;
    void Update(float dt) override;
    void OnSetAttribute(const Urho3D::AttributeInfo& attr, const Urho3D::Variant& value) override;

private:
    // Build an orthonormal basis from arbitrary 3 vectors
    void Orthonormalize();

    // Get current user-space axes in WORLD coordinates
    void GetUserAxesWorld(Urho3D::Vector3& fwdW, Urho3D::Vector3& rightW, Urho3D::Vector3& upW) const;

    // Apply yaw/pitch/roll respecting user basis and optional clamping
    void ApplyLook(float dyawDeg, float dpitchDeg, float drollDeg);

private:
    Urho3D::SharedPtr<Urho3D::Input> input_;

    // User basis in ENGINE-LOCAL space
    Urho3D::Vector3 userFwdL_{ Urho3D::Vector3::FORWARD }; // default = engine +Z
    Urho3D::Vector3 userRightL_{ Urho3D::Vector3::RIGHT }; // default = engine +X
    Urho3D::Vector3 userUpL_{ Urho3D::Vector3::UP };       // default = engine +Y

    float mouseSens_{ 0.1f };   // degrees per pixel
    float moveSpeed_{ 8.0f };   // units per second
    float slowMul_{ 0.3f }, fastMul_{ 4.0f };
    bool  invertY_{ false };
    bool  enableRoll_{ false }; // hold Alt to roll (or bind differently below)
    float pitchClampDeg_{ 85.0f }; // 0 = disabled

    // Accumulated pitch around user Right axis (for clamping)
    float pitchAccumDeg_{ 0.0f };
};
